#include "real_world.h"
#include <drogon/drogon.h>

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
    
    drogon::app().registerHandler("/stats", [](const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
        auto response = drogon::HttpResponse::newHttpResponse();
        response->setContentTypeCode(drogon::CT_TEXT_HTML);
        response->setBody(common::get_stats());
        callback(response);
    });

    try {
        drogon::app().addListener("0.0.0.0", 18091).setThreadNum(2).run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}