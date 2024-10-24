#include "real_world.h"
#include <crow.h>


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
    
    crow::SimpleApp app;
    CROW_ROUTE(app, "/stats")
    ([](){
        return common::get_stats();
    });

    try {
        app.port(18081).concurrency(2).run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}