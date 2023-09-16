#pragma once

#include <functional>
#include <string>
#include <vector>

#include "avz.h"
#include "common.h"
#include "action.h"

using namespace AvZ;

struct Task {
    std::string output_folder, output_filename;
    bool huge, assume_refresh, dance_cheat, uniform_summon, debug, clear_zombies;
    int giga_count, total;
    std::vector<int> check_time, card_selection;
    std::vector<ZombieType> required_types, banned_types;
    std::vector<std::function<void(int)>> operations;
};

#define _REISEN_TASK_H_MEMBER_REQUIRED(type, name)                                                 \
private:                                                                                           \
    type name##_;                                                                                  \
                                                                                                   \
public:                                                                                            \
    void _set_##name(const type& val)                                                              \
    {                                                                                              \
        name##_ = val;                                                                             \
        initalized.insert(__COUNTER__);                                                            \
    }                                                                                              \
    auto name(const type& val) const                                                               \
    {                                                                                              \
        TaskBuilder x = *this;                                                                     \
        x._set_##name(val);                                                                        \
        return x;                                                                                  \
    }

#define _REISEN_TASK_H_MEMBER_OPTIONAL(type, name, ...)                                            \
private:                                                                                           \
    type name##_ = __VA_ARGS__;                                                                    \
                                                                                                   \
public:                                                                                            \
    void _set_##name(const type& val) { name##_ = val; }                                           \
    TaskBuilder name(const type& val) const                                                        \
    {                                                                                              \
        TaskBuilder x = *this;                                                                     \
        x._set_##name(val);                                                                        \
        return x;                                                                                  \
    }

std::vector<int> deduce_check_time(
    const std::vector<int>& original_check_time, const std::vector<Action>& sorted_actions)
{
    if (!original_check_time.empty()) {
        return original_check_time;
    } else {
        return {sorted_actions.back().time};
    }
}

std::vector<int> deduce_card_selection(const std::vector<Action>& sorted_actions)
{
    std::set<int> card_selection;
    for (const auto& action : sorted_actions) {
        if (action.plant_type >= 0)
            card_selection.insert(action.plant_type);
    }
    return std::vector<int>(card_selection.begin(), card_selection.end());
}

std::vector<std::function<void(int)>> get_operations(const std::vector<Action>& sorted_actions)
{
    std::vector<std::function<void(int)>> operations;
    for (const auto& action : sorted_actions) {
        operations.push_back(action.operation);
    }
    return operations;
}

class TaskBuilder {
private:
    std::set<int> initalized;
    _REISEN_TASK_H_MEMBER_REQUIRED(std::string, output_folder)
    _REISEN_TASK_H_MEMBER_REQUIRED(bool, huge)
    _REISEN_TASK_H_MEMBER_REQUIRED(bool, assume_refresh)
    _REISEN_TASK_H_MEMBER_REQUIRED(int, total)
    _REISEN_TASK_H_MEMBER_REQUIRED(std::vector<ZombieType>, required_types)
    _REISEN_TASK_H_MEMBER_REQUIRED(std::vector<ZombieType>, banned_types)
    _REISEN_TASK_H_MEMBER_REQUIRED(std::vector<Action>, actions)
    _REISEN_TASK_H_MEMBER_OPTIONAL(std::vector<int>, check_time, {})
    _REISEN_TASK_H_MEMBER_OPTIONAL(bool, debug, false)
    _REISEN_TASK_H_MEMBER_OPTIONAL(bool, uniform_summon, false)
    _REISEN_TASK_H_MEMBER_OPTIONAL(bool, clear_zombies, true)
    _REISEN_TASK_H_MEMBER_OPTIONAL(bool, dance_cheat, true)
    _REISEN_TASK_H_MEMBER_OPTIONAL(int, giga_count, -1)

    TaskBuilder check_time(const int& val) const { return check_time(std::vector<int> {val}); }

    template <typename... Args> TaskBuilder actions(Args... args) const
    {
        return actions(std::vector<Action> {args...});
    }

public:
    Task build() const
    {
        if (initalized.size() != 7) {
            ShowErrorNotInQueue("Task 未完成初始化");
            throw Exception("");
        }
        auto sorted_actions = actions_;
        std::sort(sorted_actions.begin(), sorted_actions.end());

        auto sorted_required_types = required_types_;
        std::sort(sorted_required_types.begin(), sorted_required_types.end(),
            [](ZombieType a, ZombieType b) { return a > b; });

        auto sorted_banned_types = banned_types_;
        std::sort(sorted_banned_types.begin(), sorted_banned_types.end(),
            [](ZombieType a, ZombieType b) { return a > b; });

        auto deduced_check_time = deduce_check_time(check_time_, sorted_actions);
        auto deduced_card_selection = deduce_card_selection(sorted_actions);
        auto operations = get_operations(sorted_actions);

        // 构建 output filename
        std::stringstream ss;
        for (const auto& action : sorted_actions) {
            ss << action.str << " ";
        }
        ss << "y(" << zombie_types_to_string(sorted_required_types) << ") ";
        ss << "n(" << zombie_types_to_string(sorted_banned_types) << ")";
        if (giga_count_ >= 0)
            ss << "g(" << std::to_string(giga_count_) << ")";
        ss << assume_refresh_ ? "R" : "S";
        if (huge_)
            ss << "H";
        if (uniform_summon_)
            ss << "U";
        if (dance_cheat_)
            ss << "D";
        auto output_filename = ss.str();

        return {to_gbk(output_folder_), to_gbk(output_filename), huge_, assume_refresh_,
            dance_cheat_, uniform_summon_, debug_, clear_zombies_, giga_count_, total_, check_time_,
            deduced_card_selection, sorted_required_types, sorted_banned_types, operations};
    }

    operator Task() const { return build(); }
};
