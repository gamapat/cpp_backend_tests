#include "real_world.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <thread>
#include <memory>
#include <string>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// HTTP session class to handle individual requests
class http_session : public std::enable_shared_from_this<http_session> {
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;

public:
    explicit http_session(tcp::socket&& socket)
        : stream_(std::move(socket)) {}

    void start() {
        read_request();
    }

private:
    void read_request() {
        auto self = shared_from_this();
        http::async_read(stream_, buffer_, req_,
            [self](beast::error_code ec, std::size_t) {
                if (!ec) self->handle_request();
            });
    }

    void handle_request() {
        http::response<http::string_body> res{http::status::ok, req_.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req_.keep_alive());

        if (req_.target() == "/stats") {
            res.body() = common::get_stats();
        } else {
            res.result(http::status::not_found);
            res.body() = "Not Found";
        }

        res.prepare_payload();
        write_response(std::move(res));
    }

    void write_response(http::response<http::string_body>&& res) {
        auto self = shared_from_this();
        http::async_write(stream_, std::move(res),
            [self](beast::error_code ec, std::size_t) {
                self->stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
            });
    }
};

// HTTP listener class to accept incoming connections
class http_listener {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;

public:
    http_listener(net::io_context& ioc, tcp::endpoint endpoint)
        : ioc_(ioc), acceptor_(ioc) {
        beast::error_code ec;
        
        acceptor_.open(endpoint.protocol(), ec);
        if (ec) throw std::runtime_error("open: " + ec.message());
        
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) throw std::runtime_error("set_option: " + ec.message());
        
        acceptor_.bind(endpoint, ec);
        if (ec) throw std::runtime_error("bind: " + ec.message());
        
        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec) throw std::runtime_error("listen: " + ec.message());
    }

    void run() {
        accept();
    }

private:
    void accept() {
        acceptor_.async_accept(
            [this](beast::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<http_session>(std::move(socket))->start();
                }
                accept();
            });
    }
};

int main() {
    common::clear_timings();

    // Start the threads
    std::thread load_thread(common::load);
    // Make sure load_thread is assigned to specific cpu core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    pthread_setaffinity_np(load_thread.native_handle(), sizeof(cpu_set_t), &cpuset);

    std::thread push_results_to_db_thread(common::push_results_to_db);
    // Make sure push_results_to_db_thread is assigned to other specific cpu core
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    pthread_setaffinity_np(push_results_to_db_thread.native_handle(), sizeof(cpu_set_t), &cpuset);

    load_thread.detach();
    push_results_to_db_thread.detach();
    
    try {
        net::io_context ioc{2}; // 2 threads as in original
        tcp::endpoint endpoint{net::ip::make_address("0.0.0.0"), 18091};
        
        // Create and run the HTTP listener
        http_listener listener{ioc, endpoint};
        listener.run();
        
        // Run the I/O service on the requested number of threads
        std::vector<std::thread> threads;
        for(int i = 0; i < 2; ++i) {
            threads.emplace_back([&ioc] { ioc.run(); });
        }
        
        // Wait for all threads to exit
        for(auto& t : threads) {
            t.join();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}