#pragma once

#include <fstream>

#include "avz.h"
#include "global_vars.h"
#include "common.h"

using namespace std;
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

vector<int> get_ids(int type)
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

void generate_stats(RefreshData& rd, const string& suffix)
{
    ofstream fout(
        cur_task->output_folder + "\\" + cur_task->output_filename + suffix + "-stats.csv");
    fout << fixed << setprecision(3);
    fout << "average," << rd.sum / progress / 20 * 100 << "%\n";
    fout << "count,waves\n";
    int recorded = 0;
    for (int i = 0; i <= 40; i++) {
        if (i == 0)
            fout << "0";
        else
            fout << defaultfloat << (i - 1) * 0.5 << '-' << i * 0.5;
        fout << ',' << rd.hist[i] << endl;
        recorded += rd.hist[i];
        if (recorded == progress)
            break;
    }
    fout.close();
}

string get_data_string(RefreshData& rd)
{
    set<int> type_list_(type_list.begin(), type_list.end());
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
    map<int, double> total, left;
    for (int wave = 1; wave <= 20; wave++) {
        auto b = GetMainObject()->zombieList() + (wave - 1) * 50, e = b + 50;
        for (auto x : type_list)
            total[x] += (rd.wave_prob[wave] + 1e-12) / (rd.result + 2e-11) * count(b, e, x);
        for (auto x : type_list_)
            left[x] += (rd.wave_prob[wave] + 1e-12) / (rd.result + 2e-11) * rd.left_count[wave][x];
    }
    ostringstream sout;
    sout << setprecision(3);
    auto output_num = [&](double x) { sout << ',' << round(x * 1e3) * 1e-3; };
    for (auto x : zombie_name)
        if (find(type_list.begin(), type_list.end(), x.first) != type_list.end()) {
            output_num(total[x.first]);
            for (int i : get_ids(x.first))
                output_num(left[i]);
        } else
            for (int i = 0; i < get_hp(x.first) + 1; i++)
                sout << ',';
    return sout.str();
}

void generate_data(RefreshData& rd, const string& suffix)
{
    ofstream fout(
        cur_task->output_folder + "\\" + cur_task->output_filename + suffix + "-data.csv");
    fout << fixed << setprecision(3);
    fout << "count,index";
    for (auto x : zombie_name) {
        fout << ',' << x.second;
        for (int i = 1; i <= get_hp(x.first); i++)
            fout << ',' << i;
    }
    fout << endl;
    rd.ranking.push_back(tuple<double, int, string>(-rd.result, progress, get_data_string(rd)));
    sort(rd.ranking.begin(), rd.ranking.end());
    for (auto x : rd.ranking)
        fout << -get<0>(x) << ',' << get<1>(x) << get<2>(x) << endl;
    fout.close();
}

void generate_raw(RefreshData& rd, const string& suffix)
{
    string fn = cur_task->output_folder + "\\" + cur_task->output_filename + suffix + "-raw.csv";
    if (progress == 1) {
        ofstream fout(fn);
        fout << "index,wave,hp";
        for (auto x : zombie_name) {
            fout << ',' << x.second;
            for (int i = 1; i <= get_hp(x.first); i++)
                fout << ',' << i;
        }
        fout << endl;
    }
    ofstream fout(fn, ios::app);
    fout << fixed << setprecision(3);
    for (int wave = 1; wave <= 20; wave++) {
        fout << progress << ',' << wave << ',' << rd.hp_ratio[wave];
        for (auto x : zombie_name)
            if (find(type_list.begin(), type_list.end(), x.first) != type_list.end()) {
                auto b = GetMainObject()->zombieList() + (wave - 1) * 50, e = b + 50;
                fout << ',' << count(b, e, x.first);
                for (int i : get_ids(x.first))
                    fout << ',' << rd.left_count[wave][i];
            } else
                for (int i = 0; i < get_hp(x.first) + 1; i++)
                    fout << ',';
        fout << endl;
    }
    fout.close();
}
