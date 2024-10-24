#include "real_world.h"
#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <thread>
#include <iostream>

using namespace Pistache;

void setupRoutes(Rest::Router& router) {
    Rest::Routes::Get(router, "/stats", [](const Rest::Request&, Http::ResponseWriter response){
        auto stats = common::get_stats();
        response.send(Http::Code::Ok, stats);
        return Rest::Route::Result::Ok;
    });
}

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

    Http::Endpoint server(Address(Ipv4::any(), Port(18101)));
    auto opts = Http::Endpoint::options().threads(2);
    server.init(opts);

    Rest::Router router;
    setupRoutes(router);
    server.setHandler(router.handler());

    try {
        server.serve();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    server.shutdown();

    return 0;
}