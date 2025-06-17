#include "real_world.h"
#include <httplib.h>
#include <thread>
#include <iostream>
#include <pthread.h>

int main() {
    common::clear_timings();

    // Start the threads
    std::thread load_thread(common::load);
    // Assign load_thread to CPU core 0
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    pthread_setaffinity_np(load_thread.native_handle(), sizeof(cpu_set_t), &cpuset);

    std::thread push_results_to_db_thread(common::push_results_to_db);
    // Assign push_results_to_db_thread to CPU core 1
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    pthread_setaffinity_np(push_results_to_db_thread.native_handle(), sizeof(cpu_set_t), &cpuset);

    load_thread.detach();
    push_results_to_db_thread.detach();

    httplib::Server svr;

    svr.Get("/stats", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(common::get_stats(), "text/html");
    });

    try {
        std::cout << "Server listening on 0.0.0.0:18111" << std::endl;
        svr.listen("0.0.0.0", 18111);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}