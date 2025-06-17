#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/json.hpp>
#include <sstream>
#include <string>
#include <memory>
#include <iostream>
#include <map>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace json = boost::json;
using tcp = net::ip::tcp;

// Utility functions
std::string echo_parameter(const std::string &message) {
    return message;
}

json::value request_as_json(const http::request<http::string_body>& req) {
    json::object result;
    result["method"] = req.method_string();
    result["body"] = req.body();
    
    json::object params_dict;
    // Parse query parameters from target
    std::string target = req.target().to_string();
    size_t pos = target.find('?');
    if (pos != std::string::npos) {
        std::string query = target.substr(pos + 1);
        size_t param_pos = 0;
        while (param_pos < query.size()) {
            size_t amp_pos = query.find('&', param_pos);
            if (amp_pos == std::string::npos) amp_pos = query.size();
            
            std::string param = query.substr(param_pos, amp_pos - param_pos);
            size_t eq_pos = param.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = param.substr(0, eq_pos);
                std::string value = param.substr(eq_pos + 1);
                params_dict[key] = value;
            }
            param_pos = amp_pos + 1;
        }
    }
    result["params"] = params_dict;
    
    return result;
}

std::string http_version_to_string(unsigned version) {
    switch (version) {
        case 10: return "HTTP/1.0";
        case 11: return "HTTP/1.1";
        default: return "Unknown";
    }
}

std::string request_as_html(const http::request<http::string_body>& req) {
    std::ostringstream os;
    os << "<style>";
    os << "body { padding-left: 20px; padding-right: 20px; }";
    os << "table { border-collapse: collapse; width: 100%; border: 1px solid #ddd; }";
    os << "th, td { padding: 8px; text-align: left; }";
    os << "th { border-bottom: 1px solid #ddd; }";
    os << "tr:nth-child(even) { background-color: #f2f2f2; }";
    os << "tr:nth-child(odd) { background-color: #fff; }";
    os << "</style>";

    os << "<h1>Request info</h1>";
    os << "<p><strong>Method:</strong> " << req.method_string() << "</p>";
    os << "<p><strong>Body:</strong> " << req.body() << "</p>";
    os << "<p><strong>HTTP version:</strong> " << http_version_to_string(req.version()) << "</p>";
    
    os << "<p><strong>Headers:</strong></p>";
    os << "<table>";
    os << "<thead><tr><th>Key</th><th>Value</th></tr></thead>";
    os << "<tbody>";
    for (const auto& header : req) {
        os << "<tr><td>" << header.name_string() << "</td><td>" << header.value() << "</td></tr>";
    }
    os << "</tbody></table>";
    
    os << "<p><strong>URL:</strong> " << req.target() << "</p>";
    
    // Parse and display URL parameters
    std::string target = req.target().to_string();
    os << "<p><strong>URL params:</strong></p>";
    os << "<table>";
    os << "<thead><tr><th>Key</th><th>Value</th></tr></thead>";
    os << "<tbody>";
    
    size_t pos = target.find('?');
    if (pos != std::string::npos) {
        std::string query = target.substr(pos + 1);
        size_t param_pos = 0;
        while (param_pos < query.size()) {
            size_t amp_pos = query.find('&', param_pos);
            if (amp_pos == std::string::npos) amp_pos = query.size();
            
            std::string param = query.substr(param_pos, amp_pos - param_pos);
            size_t eq_pos = param.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = param.substr(0, eq_pos);
                std::string value = param.substr(eq_pos + 1);
                os << "<tr><td>" << key << "</td><td>" << value << "</td></tr>";
            }
            param_pos = amp_pos + 1;
        }
    }
    os << "</tbody></table>";
    return os.str();
}

// Session class to handle individual connections
class http_session : public std::enable_shared_from_this<http_session> {
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;

public:
    http_session(tcp::socket&& socket)
        : stream_(std::move(socket)) {
    }

    void run() {
        do_read();
    }

private:
    void do_read() {
        auto self = shared_from_this();

        http::async_read(stream_, buffer_, req_,
            [self](beast::error_code ec, std::size_t bytes_transferred) {
                boost::ignore_unused(bytes_transferred);
                if (!ec)
                    self->handle_request();
            });
    }

    void handle_request() {
        auto self = shared_from_this();
        http::response<http::string_body> res{http::status::ok, req_.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req_.keep_alive());

        std::string path = req_.target().to_string();
        size_t query_pos = path.find('?');
        if (query_pos != std::string::npos) {
            path = path.substr(0, query_pos);
        }

        if (path == "/" || path == "/index.html") {
            res.body() = "Hello world";
        }
        else if (path.substr(0, 6) == "/echo/") {
            std::string message = path.substr(6);
            res.body() = echo_parameter(message);
        }
        else if (path == "/request") {
            res.set(http::field::content_type, "application/json");
            res.body() = json::serialize(request_as_json(req_));
        }
        else if (path == "/info") {
            res.body() = request_as_html(req_);
        }
        else {
            res.result(http::status::not_found);
            res.body() = "404 Not Found";
        }

        res.prepare_payload();
        http::async_write(stream_, res,
            [self](beast::error_code ec, std::size_t bytes_transferred) {
                boost::ignore_unused(bytes_transferred);
                self->stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
            });
    }
};

// Listener class to accept connections
class listener : public std::enable_shared_from_this<listener> {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;

public:
    listener(net::io_context& ioc, tcp::endpoint endpoint)
        : ioc_(ioc), acceptor_(ioc) {
        beast::error_code ec;

        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            std::cerr << "open: " << ec.message() << "\n";
            return;
        }

        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) {
            std::cerr << "set_option: " << ec.message() << "\n";
            return;
        }

        acceptor_.bind(endpoint, ec);
        if (ec) {
            std::cerr << "bind: " << ec.message() << "\n";
            return;
        }

        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec) {
            std::cerr << "listen: " << ec.message() << "\n";
            return;
        }
    }

    void run() {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(&listener::on_accept, shared_from_this()));
    }

    void on_accept(beast::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::make_shared<http_session>(std::move(socket))->run();
        }
        do_accept();
    }
};

int main() {
    try {
        auto const address = net::ip::make_address("0.0.0.0");
        unsigned short port = 18090;
        
        net::io_context ioc{1};
        std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();
        
        std::cout << "Server running at http://0.0.0.0:" << port << std::endl;
        ioc.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}