#pragma once

#include <string>

#include "avz.h"

namespace AvZ {
void Utf8ToGbk(std::string&);
};

std::string to_gbk(std::string s)
{
    AvZ::Utf8ToGbk(s);
    return s;
}

enum WaveType { DEFAULT, ALL_NORMAL, ALL_HUGE };

const std::map<ZombieType, std::string> ZOMBIE_NAME = {
    {ZOMBIE, "普"},
    {CONEHEAD_ZOMBIE, "路"},
    {POLE_VAULTING_ZOMBIE, "杆"},
    {BUCKETHEAD_ZOMBIE, "桶"},
    {NEWSPAPER_ZOMBIE, "报"},
    {SCREEN_DOOR_ZOMBIE, "门"},
    {FOOTBALL_ZOMBIE, "橄"},
    {DANCING_ZOMBIE, "舞"},
    {SNORKEL_ZOMBIE, "潜"},
    {ZOMBONI, "车"},
    {DOLPHIN_RIDER_ZOMBIE, "豚"},
    {JACK_IN_THE_BOX_ZOMBIE, "丑"},
    {BALLOON_ZOMBIE, "气"},
    {DIGGER_ZOMBIE, "矿"},
    {POGO_ZOMBIE, "跳"},
    {ZOMBIE_YETI, "雪"},
    {BUNGEE_ZOMBIE, "偷"},
    {LADDER_ZOMBIE, "梯"},
    {CATAPULT_ZOMBIE, "篮"},
    {GARGANTUAR, "白"},
    {GIGA_GARGANTUAR, "红"},
};

std::string zombie_types_to_string(const std::vector<ZombieType>& types)
{
    std::string output = "";
    for (const auto& type : types) {
        auto name = ZOMBIE_NAME.find(type);
        if (name != ZOMBIE_NAME.end()) {
            output += name->second;
        }
    }
    return output;
}

template <typename T> bool contains(const std::vector<T>& vec, T val)
{
    return std::find(vec.begin(), vec.end(), val) != vec.end();
}

std::vector<std::string> split(std::string string, const std::string& delimiter)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    while ((pos = string.find(delimiter)) != std::string::npos) {
        tokens.push_back(string.substr(0, pos));
        string.erase(0, pos + delimiter.length());
    }
    if (!string.empty())
        tokens.push_back(string);
    return tokens;
}