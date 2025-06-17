#include "real_world.h"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/network/Server.hpp"

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
    
    // Initialize oatpp Environment
    oatpp::base::Environment::init();
    
    try {
        // Create Router for HTTP requests
        auto router = oatpp::web::server::HttpRouter::createShared();
        
        // Add /stats endpoint
        router->route("GET", "/stats", [](const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) {
            return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                oatpp::web::protocol::http::Status::CODE_200, 
                common::get_stats()
            );
        });
        
        // Create HTTP connection handler with router
        auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
        
        // Create TCP connection provider
        auto connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared(
            {"0.0.0.0", 18081}
        );
        
        // Create server
        oatpp::network::Server server(connectionProvider, connectionHandler);
        
        // Print server info
        OATPP_LOGI("Server", "Running on port %s...", connectionProvider->getProperty("port").getData());
        
        // Run server with 2 threads
        server.run(2);
    }
    catch (const std::exception& e) {
        OATPP_LOGE("Server", "Error: %s", e.what());
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Destroy oatpp Environment
    oatpp::base::Environment::destroy();

    return 0;
}