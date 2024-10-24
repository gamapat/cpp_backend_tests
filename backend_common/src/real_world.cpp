#include <deque>
#include <chrono>
#include <mutex>
#include <thread>
#include <sstream>
#include <condition_variable>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include "real_world.h"

std::deque<std::chrono::nanoseconds> timing_results;
std::mutex timing_results_mutex;
std::condition_variable timing_results_cv;
mongocxx::instance inst;

void common::load() {
    // infinite loop that counts to 10m and sets timing result in the global structure
    while (true) {
        auto start = std::chrono::steady_clock::now();
        // prevent optimisation out of loop
        volatile int dummy = 0;
        for (int i = 0; i < 10000000; i++) {
            dummy = i;
        }
        auto end = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(timing_results_mutex);
        timing_results.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));
        timing_results_cv.notify_one();
    }
}

std::string get_environment_variable(const std::string& name) {
    char* value = std::getenv(name.c_str());
    if (value == nullptr) {
        return "";
    }
    return value;
}

mongocxx::client& get_mongo_client() {
    static std::string mongodb_uri_str = get_environment_variable("MONGODB_URI");
    static const mongocxx::uri mongodb_uri = mongocxx::uri{ mongodb_uri_str };

    static thread_local mongocxx::options::client client_options;
    static auto api = mongocxx::options::server_api{ mongocxx::options::server_api::version::k_version_1 };
    client_options.server_api_opts(api);
    static thread_local mongocxx::client client{ mongodb_uri, client_options};
    return client;
}

mongocxx::database get_crow_database() {
    // connect to local mongodb database
    auto& client = get_mongo_client();

    // get database
    auto db = client["crow"];
    return db;
}

mongocxx::collection get_timings_collection() {
    // connect to local mongodb database
    auto db = get_crow_database();

    // get timings collection
    auto timings = db["timings"];
    return timings;
}

void common::push_results_to_db() {
    // connect to local mongodb database
    auto timings = get_timings_collection();

    // infinite loop that waits for timing results and pushes them to the database
    while (true) {
        std::cout << timing_results.size() << std::endl;
        std::deque<std::chrono::nanoseconds> results_to_process;

        std::unique_lock<std::mutex> lock(timing_results_mutex);
        timing_results_cv.wait(lock, []{ return !timing_results.empty(); });
        std::swap(results_to_process, timing_results);
        lock.unlock();

        std::vector<bsoncxx::document::value> documents;
        for (const auto& result : results_to_process) {
            auto document = bsoncxx::builder::basic::document{};
            document.append(bsoncxx::builder::basic::kvp("result", result.count()));
            document.append(bsoncxx::builder::basic::kvp("time", std::chrono::system_clock::now().time_since_epoch().count()));
            documents.push_back(document.extract());
        }

        if (!documents.empty()) {
            timings.insert_many(documents);
        }
    }
}

std::string common::get_stats() {
    // connect to local mongodb database
    auto timings = get_timings_collection();

    // Get max time, min time, 90th percentile, 95th percentile, 99th percentile for last 1000 results, sort by id
    auto options = mongocxx::options::find{}.sort(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("_id", -1))).limit(1000);
    auto cursor = timings.find({}, options);
    std::vector<std::chrono::nanoseconds> results;
    for (auto&& doc : cursor) {
        auto result = doc["result"].get_int64().value;
        results.push_back(std::chrono::nanoseconds(result));
    }

    std::sort(results.begin(), results.end());
    // Format result as html
    std::stringstream ss;
    ss << "<h1>Timing results</h1>";
    ss << "<p><strong>Max time:</strong> " << results.back().count() << "ns</p>";
    ss << "<p><strong>Min time:</strong> " << results.front().count() << "ns</p>";
    ss << "<p><strong>90th percentile:</strong> " << results[results.size() * 0.9].count() << "ns</p>";
    ss << "<p><strong>95th percentile:</strong> " << results[results.size() * 0.95].count() << "ns</p>";
    ss << "<p><strong>99th percentile:</strong> " << results[results.size() * 0.99].count() << "ns</p>";
    return ss.str();
}

void common::clear_timings() {
    // connect to local mongodb database
    auto timings = get_timings_collection();

    // Clear all timings
    timings.delete_many({});
}