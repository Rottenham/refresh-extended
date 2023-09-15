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

const std::map<ZombieType, std::string> zombie_name = {
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
        auto name = zombie_name.find(type);
        if (name != zombie_name.end()) {
            output += name->second;
        }
    }
    return output;
}