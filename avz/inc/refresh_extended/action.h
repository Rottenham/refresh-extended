#pragma once

#include "avz.h"
#include "common.h"

using namespace AvZ;

std::string plant_type_to_symbol(PlantType plant_type)
{
    auto non_imitator_plant_type = plant_type > IMITATOR ? plant_type - IMITATOR - 1 : plant_type;
    switch (non_imitator_plant_type) {
    case ICE_SHROOM:
        return "I";
    case CHERRY_BOMB:
        return "A";
    case JALAPENO:
        return "J"; // 使用J而非A'，节省文件名长度且更清晰
    case SQUASH:
        return "W"; // 使用W而非a，因为NTFS不区分大小写
    default:
        return "C" + std::to_string(non_imitator_plant_type);
    }
}

std::string concat(const std::vector<std::string>& strings, const std::string& sep)
{
    std::ostringstream os;
    bool first = true;
    for (const auto& str : strings) {
        if (first) {
            first = false;
        } else {
            os << sep;
        }
        os << str;
    }
    return os.str();
}

std::string ints_to_string(const std::vector<int>& ints, const std::string& sep)
{
    std::vector<std::string> int_strings(ints.size());
    std::transform(
        ints.begin(), ints.end(), int_strings.begin(), [](int i) { return std::to_string(i); });
    return concat(int_strings, sep);
}

struct Action {
    int time;
    std::string str;
    std::function<void(int)> operation;
    int plant_type;

    bool operator<(const Action& other) const { return time < other.time; }
};

Action FixedCard(int time, PlantType plant_type, int row, int col, int shovel_delay = -1)
{
    std::ostringstream os;
    auto symbol = plant_type_to_symbol(plant_type);
    if (symbol == "I") {
        os << symbol << "(" << time << ")";
    } else {
        os << symbol << "(" << time << "," << row << "-" << col << ")";
    }
    return {time, os.str(),
        [time, plant_type, row, col, shovel_delay](int wave) {
            if (plant_type == SQUASH) {
                SetTime(time - 182, wave);
            } else if (plant_type == BLOVER) {
                SetTime(time - 150, wave); // 此处留有一定允差，等气球被吹出屏幕
            } else if (RangeIn(plant_type, {ICE_SHROOM, DOOM_SHROOM, CHERRY_BOMB, JALAPENO})) {
                SetTime(time - 100, wave);
            } else if (plant_type == SPIKEWEED) {
                SetTime(time - 26, wave);
            }
            Card(plant_type, row, col);
            if (shovel_delay > 0) {
                SetTime(time + shovel_delay, wave);
                Shovel(row, col);
            }
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

std::string strategies_to_string(const std::vector<Strategy>& strategies)
{
    std::vector<std::string> strategy_strings(strategies.size());
    std::transform(strategies.begin(), strategies.end(), strategy_strings.begin(),
        [](Strategy s) { return s.to_string(); });
    return concat(strategy_strings, ",");
}

Action SmartCard(int time, PlantType plant_type, std::vector<ZombieType> assessed_types,
    const std::vector<Strategy>& strategies, int col)
{
    std::sort(assessed_types.begin(), assessed_types.end(),
        [](ZombieType a, ZombieType b) { return a > b; });
    std::ostringstream os;
    auto symbol = plant_type_to_symbol(plant_type);
    if (symbol == "I") {
        os << symbol << "(" << time << ")";
    } else {
        os << symbol << "(" << time << "," << zombie_types_to_string(assessed_types) << "["
           << strategies_to_string(strategies) << "]-" << col << ")";
    }
    return {time, os.str(),
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
                        if (contains(assessed_types, (ZombieType)z.type()))
                            zombie_count[z.row() + 1]++;
                    }
                    auto best_strategy = std::max_element(strategies.begin(), strategies.end(),
                        [zombie_count](const Strategy& s1, const Strategy& s2) {
                            auto get_strategy_count = [zombie_count](const Strategy& s) {
                                int count = 0;
                                for (const auto& r : s.assessed_rows)
                                    count += zombie_count[r];
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
    std::ostringstream os;
    os << symbol << "(" << time << ",";
    if (rows.size() > 1) {
        os << "{" << ints_to_string(rows, ",") << "}";
    } else {
        os << rows[0];
    }
    os << "-" << col << ")";
    return {time, os.str(),
        [time, rows, col](int wave) {
            auto roof = GetMainObject()->scene() >= 4;
            auto backyard = RangeIn(GetMainObject()->scene(), {2, 3});
            for (const auto& row : rows)
                if (roof) {
                    SetTime(time - 387, wave);
                    pao_operator.roofPao(row, col);
                } else if (backyard && (RangeIn(row, {3, 4}))) {
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
