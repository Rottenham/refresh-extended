#pragma once

#include <fstream>
#include <Windows.h>

#include "avz.h"
#include "global_vars.h"
#include "common.h"

using namespace AvZ;

int get_hp(int type)
{
    switch (type) {
    case GARGANTUAR:
        return 2;
    case GIGA_GARGANTUAR:
        return 4;
    default:
        return 1;
    }
}

std::vector<int> get_ids(int type)
{
    switch (type) {
    case GARGANTUAR:
        return {231, 232};
    case GIGA_GARGANTUAR:
        return {321, 322, 323, 324};
    default:
        return {type};
    }
}

bool ensure_folder_exists(const std::string& folder_path)
{
    std::string current_path = "";
    std::vector<std::string> paths = split(folder_path, "\\");
    for (const auto& part : paths) {
        if (!current_path.empty()) {
            current_path += "\\";
        }
        current_path += part;
        if (!CreateDirectoryA(current_path.c_str(), NULL)) {
            DWORD last_error = GetLastError();
            if (last_error != ERROR_ALREADY_EXISTS) {
                ShowErrorNotInQueue("尝试创建目录(" + current_path + ")时失败: #", last_error);
                return false;
            }
        }
    }
    return true;
}

void generate_stats(RefreshData& rd, const std::string& suffix)
{
    std::ofstream fout(
        cur_task->output_folder + "\\" + cur_task->output_filename + suffix + "-stats.csv");
    fout << std::fixed << std::setprecision(3);
    fout << "average," << rd.sum / progress / 20 * 100 << "%\n";
    fout << "count,waves\n";
    int recorded = 0;
    for (int i = 0; i <= 40; i++) {
        if (i == 0)
            fout << "0";
        else
            fout << std::defaultfloat << (i - 1) * 0.5 << '-' << i * 0.5;
        fout << ',' << rd.hist[i] << std::endl;
        recorded += rd.hist[i];
        if (recorded == progress)
            break;
    }
    fout.close();
}

std::string get_data_string(RefreshData& rd)
{
    std::set<int> type_list_(type_list.begin(), type_list.end());
    if (type_list_.count(GARGANTUAR)) {
        type_list_.erase(GARGANTUAR);
        for (int i = 231; i <= 232; i++)
            type_list_.insert(i);
    }
    if (type_list_.count(GIGA_GARGANTUAR)) {
        type_list_.erase(GIGA_GARGANTUAR);
        for (int i = 321; i <= 324; i++)
            type_list_.insert(i);
    }
    std::map<int, double> total, left;
    for (int wave = 1; wave <= 20; wave++) {
        auto b = GetMainObject()->zombieList() + (wave - 1) * 50, e = b + 50;
        for (auto x : type_list)
            total[x] += (rd.wave_prob[wave] + 1e-12) / (rd.result + 2e-11) * std::count(b, e, x);
        for (auto x : type_list_)
            left[x] += (rd.wave_prob[wave] + 1e-12) / (rd.result + 2e-11) * rd.left_count[wave][x];
    }
    std::ostringstream sout;
    sout << std::setprecision(3);
    auto output_num = [&](double x) { sout << ',' << round(x * 1e3) * 1e-3; };
    for (auto x : ZOMBIE_NAME)
        if (find(type_list.begin(), type_list.end(), x.first) != type_list.end()) {
            output_num(total[x.first]);
            for (int i : get_ids(x.first))
                output_num(left[i]);
        } else
            for (int i = 0; i < get_hp(x.first) + 1; i++)
                sout << ',';
    return sout.str();
}

void generate_data(RefreshData& rd, const std::string& suffix)
{
    std::ofstream fout(
        cur_task->output_folder + "\\" + cur_task->output_filename + suffix + "-data.csv");
    fout << std::fixed << std::setprecision(3);
    fout << "count,index";
    for (auto x : ZOMBIE_NAME) {
        fout << ',' << x.second;
        for (int i = 1; i <= get_hp(x.first); i++)
            fout << ',' << i;
    }
    fout << std::endl;
    rd.ranking.push_back(
        std::tuple<double, int, std::string>(-rd.result, progress, get_data_string(rd)));
    std::sort(rd.ranking.begin(), rd.ranking.end());
    for (auto x : rd.ranking)
        fout << -std::get<0>(x) << ',' << std::get<1>(x) << std::get<2>(x) << std::endl;
    fout.close();
}

void generate_raw(RefreshData& rd, const std::string& suffix)
{
    std::string fn
        = cur_task->output_folder + "\\" + cur_task->output_filename + suffix + "-raw.csv";
    if (progress == 1) {
        std::ofstream fout(fn);
        fout << "index,wave,hp";
        for (auto x : ZOMBIE_NAME) {
            fout << ',' << x.second;
            for (int i = 1; i <= get_hp(x.first); i++)
                fout << ',' << i;
        }
        fout << std::endl;
    }
    std::ofstream fout(fn, std::ios::app);
    fout << std::fixed << std::setprecision(3);
    for (int wave = 1; wave <= 20; wave++) {
        fout << progress << ',' << wave << ',' << rd.hp_ratio[wave];
        for (auto x : ZOMBIE_NAME)
            if (find(type_list.begin(), type_list.end(), x.first) != type_list.end()) {
                auto b = GetMainObject()->zombieList() + (wave - 1) * 50, e = b + 50;
                fout << ',' << std::count(b, e, x.first);
                for (int i : get_ids(x.first))
                    fout << ',' << rd.left_count[wave][i];
            } else
                for (int i = 0; i < get_hp(x.first) + 1; i++)
                    fout << ',';
        fout << std::endl;
    }
    fout.close();
}
