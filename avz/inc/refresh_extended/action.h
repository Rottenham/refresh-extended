#pragma once

#include "avz.h"
#include "common.h"

using namespace AvZ;

std::string float_to_human_friendly_string(float f)
{
    auto str = std::to_string(f);
    str.erase(str.find_last_not_of('0') + 1, std::string::npos); // 移除尾随零
    str.erase(str.find_last_not_of('.') + 1, std::string::npos);
    return str;
}

std::string floats_to_human_friendly_string(
    const std::vector<float>& floats, const std::string& sep = ",")
{
    std::stringstream ss;
    bool first = true;
    for (const auto& f : floats) {
        if (first) {
            first = false;
        } else {
            ss << sep;
        }
        ss << float_to_human_friendly_string(f);
    }
    return ss.str();
}

std::string ints_to_string(const std::vector<int>& ints, const std::string& sep = ",")
{
    std::stringstream ss;
    bool first = true;
    for (const auto& i : ints) {
        if (first) {
            first = false;
        } else {
            ss << sep;
        }
        ss << std::to_string(i);
    }
    return ss.str();
}

std::string plant_type_to_symbol(PlantType plant_type)
{
    auto non_imitator_plant_type = plant_type > IMITATOR ? plant_type - IMITATOR - 1 : plant_type;
    if (non_imitator_plant_type == ICE_SHROOM) {
        return "I";
    } else if (non_imitator_plant_type == CHERRY_BOMB) {
        return "A";
    } else if (non_imitator_plant_type == JALAPENO) {
        return "J"; // 使用J而非A'，节省文件名长度且更清晰
    } else if (non_imitator_plant_type == SQUASH) {
        return "W"; // 使用W而非a，因为NTFS不区分大小写
    } else {
        return "C" + std::to_string(plant_type);
    }
}

struct Action {
    int time;
    std::string str;
    std::function<void(int)> operation;
    int plant_type;

    bool operator<(const Action& other) const { return time < other.time; }
};

Action FixedCard(int time, PlantType plant_type, int row, int col)
{
    std::stringstream ss;
    auto symbol = plant_type_to_symbol(plant_type);
    if (symbol == "I") {
        ss << symbol << "(" << time << ")";
    } else {
        ss << symbol << "(" << time << "," << row << "-" << col << ")";
    }
    return {time, ss.str(),
        [time, plant_type, row, col](int wave) {
            if (plant_type == SQUASH) {
                SetTime(time - 182, wave);
            } else {
                SetTime(time - 100, wave);
            }
            Card(plant_type, row, col);
        },
        plant_type};
}

struct Strategy {
    std::vector<int> assessed_rows;
    int row; // 从1开始

    Strategy(std::vector<int> assessed_rows, int row)
        : row(row)
    {
        std::sort(assessed_rows.begin(), assessed_rows.end());
        this->assessed_rows = assessed_rows;
    }

    Strategy(int assessed_row, int row)
        : assessed_rows {assessed_row}
        , row(row)
    {
    }

    std::string to_string() const
    {
        return ints_to_string(assessed_rows, "") + "→" + std::to_string(row);
    }
};

Action SmartCard(int time, PlantType plant_type, std::vector<ZombieType> assessed_types,
    const std::vector<Strategy>& strategies, int col)
{
    std::sort(assessed_types.begin(), assessed_types.end(),
        [](ZombieType a, ZombieType b) { return a > b; });
    std::stringstream ss;
    auto symbol = plant_type_to_symbol(plant_type);
    if (symbol == "I") {
        ss << symbol << "(" << time << ")";
    } else {
        ss << symbol << "(" << time << "," << zombie_types_to_string(assessed_types) << "[";
        bool first = true;
        for (const auto& strategy : strategies) {
            if (first) {
                first = false;
            } else {
                ss << ",";
            }
            ss << strategy.to_string();
        }
        ss << "]-" << col << ")";
    }
    return {time, ss.str(),
        [time, plant_type, assessed_types, strategies, col](int wave) {
            InsertTimeOperation(
                time - 100, wave,
                [=]() {
                    SetNowTime();
                    if (GetRefreshedWave().size() != wave) {
                        ShowError("已刷新波数与预期波数不符（已刷新=#，预期=#）",
                            GetRefreshedWave().size(), wave);
                        throw "";
                    }
                    int zombie_count[7] = {0};
                    for (auto& z : alive_zombie_filter) {
                        if (std::find(assessed_types.begin(), assessed_types.end(), z.type())
                            != assessed_types.end())
                            zombie_count[z.row() + 1]++;
                    }
                    auto best_strategy = std::max_element(strategies.begin(), strategies.end(),
                        [zombie_count](const Strategy& s1, const Strategy& s2) {
                            auto get_strategy_count = [zombie_count](const Strategy& s) {
                                int count = 0;
                                for (const auto& row : s.assessed_rows)
                                    count += zombie_count[row];
                                return count;
                            };
                            return get_strategy_count(s1) < get_strategy_count(s2);
                        });
                    if (best_strategy != strategies.end()) {
                        Card(plant_type, best_strategy->row, col);
                    }
                },
                "smart_card");
        },
        plant_type};
}

Action Cob(int time, std::string symbol, const std::vector<int>& rows, float col)
{
    std::stringstream ss;
    ss << symbol << "(" << time << ",";
    if (rows.size() > 1) {
        ss << "{" << ints_to_string(rows) << "}";
    } else {
        ss << rows[0];
    }
    ss << "-" << col << ")";
    return {time, ss.str(),
        [time, rows, col](int wave) {
            for (const auto& row : rows)
                if (GetMainObject()->scene() >= 4) { // 屋顶场合
                    SetTime(time - 387, wave);
                    pao_operator.roofPao(row, col);
                } else if ((GetMainObject()->scene() == 2 || GetMainObject()->scene() == 3)
                    && (RangeIn(row, {3, 4}))) { // 炮击泳池
                    SetTime(time - 378, wave);
                    pao_operator.pao(row, col);
                } else {
                    SetTime(time - 373, wave);
                    pao_operator.pao(row, col);
                }
        },
        -1};
}

Action Cob(int time, std::string symbol, int row, float col)
{
    return Cob(time, symbol, {{row}}, col);
}
