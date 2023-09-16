#pragma once

#include "task.h"

struct RefreshData {
    double wave_prob[21], hp_ratio[21];
    std::map<int, int> left_count[21];
    double result, sum;
    int hist[41];
    std::vector<std::tuple<double, int, std::string>> ranking;
};

std::vector<Task>::iterator cur_task;
int progress;
std::vector<int> type_list;
std::vector<RefreshData> refresh_data;
