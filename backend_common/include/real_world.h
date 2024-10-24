#pragma once
#include <string>

namespace common {
    void load();
    void push_results_to_db();
    std::string get_stats();

    void clear_timings();
}