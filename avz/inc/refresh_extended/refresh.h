#include <numeric>

#include "DanceCheat/DanceCheat.h"
#include "SelectCardsPlus/SelectCardsPlus.h"
#include "avz.h"
#include "global_vars.h"
#include "io.h"
#include "merge_csv.h"
#include "mod/mod.h"
#include "summon_simulate.h"
#include "task.h"

using namespace std;
using namespace AvZ;

vector<Task> get_tasks();

bool first_run = true;
clock_t start_time;
vector<Task> all_tasks;
int initial_total_hp[21];

int wave_total_hp(int wave)
{
    int ret;
    asm volatile("pushl %[wave];"
                 "movl 0x6a9ec0, %%ebx;"
                 "movl 0x768(%%ebx), %%ebx;"
                 "movl $0x412e30, %%ecx;"
                 "calll *%%ecx;"
                 "mov %%eax, %[ret];"
                 : [ret] "=rm"(ret)
                 : [wave] "rm"(wave - 1)
                 : "ebx", "ecx", "edx");
    return ret;
}

void unify_zombie_spawn_x(WaveType wave_type)
{
    if (wave_type == DEFAULT)
        return;
    auto x_offset = (wave_type == ALL_HUGE) ? 40 : -40;
    std::set<ZombieType> exclude_zombie_list = {FLAG_ZOMBIE, POLE_VAULTING_ZOMBIE, ZOMBONI,
        CATAPULT_ZOMBIE, GARGANTUAR, GIGA_GARGANTUAR, BUNGEE_ZOMBIE};
    for (auto wave = 1; wave <= 20; wave++) {
        if ((wave_type == ALL_HUGE) && (wave == 10 || wave == 20))
            continue;
        if ((wave_type == ALL_NORMAL) && !(wave == 10 || wave == 20))
            continue;
        AvZ::InsertTimeOperation(
            1, wave,
            [=]() {
                for (auto& z : alive_zombie_filter) {
                    if (z.existTime() < 5
                        && exclude_zombie_list.find((ZombieType)z.type())
                            != exclude_zombie_list.end()) {
                        z.abscissa() += x_offset;
                    }
                }
            },
            "unify_zombie_spawn_x");
    }
}

void kill_all_zombies(
    const std::set<int>& wave, const std::set<ZombieType>& type_list = {}, int time = 1)
{
    if (time < 0) {
        time = 1;
    }
    for (auto w : wave) {
        if (w >= 1 && w <= 20) {
            AvZ::InsertTimeOperation(
                time, w,
                [=]() {
                    for (auto& z : alive_zombie_filter) {
                        if (type_list.empty()
                            || type_list.find((ZombieType)z.type()) != type_list.end()) {
                            z.state() = 3;
                        }
                    }
                },
                "kill_all_zombies");
        }
    }
}

std::vector<Task> get_tasks_in_current_batch()
{
    auto tasks = get_tasks(); // 拆分测试任务
    int batch_count = 1;
    int batch_index = 1; // 从1开始
#ifdef BATCH_COUNT
    batch_count = BATCH_COUNT;
#endif
#ifdef BATCH_INDEX
    batch_index = BATCH_INDEX;
#endif
    vector<Task> tasks_in_current_batch;
    for (auto i = 0; i < tasks.size(); i++) {
        if (i % batch_count == batch_index - 1)
            tasks_in_current_batch.push_back(tasks[i]);
    }
    return tasks_in_current_batch;
}

void initialize_task()
{
    progress = 0;
    refresh_data.clear();
    sort(cur_task->check_time.begin(), cur_task->check_time.end());
    for (int i = 0; i < cur_task->check_time.size(); i++) {
        RefreshData rd;
        rd.result = rd.sum = 0;
        memset(rd.hist, 0, sizeof(rd.hist));
        refresh_data.push_back(rd);
    }
}

void Script()
{
    if (first_run) {
        first_run = false;
        start_time = clock();
        all_tasks = get_tasks_in_current_batch();
        cur_task = all_tasks.begin();
        initialize_task();
        extract_exe();
    }
    if (progress == cur_task->total) {
        vector<string> idents;
        for (int i = 0; i < cur_task->check_time.size(); i++)
            if (cur_task->check_time.size() != 1)
                idents.push_back("(" + to_string(cur_task->check_time[i]) + ")");
            else
                idents.push_back("");
        merge_csv(idents);
        cur_task++;
        if (cur_task == all_tasks.end()) {
            double elapsed = 1.0 * (clock() - start_time) / CLOCKS_PER_SEC;
            ShowErrorNotInQueue("完成\n用时：#s", int(elapsed + 0.5));
            return;
        }
        initialize_task();
    }

    EnableModsScoped(SaveDataReadOnly, FreePlantingCheat, PlantAnywhere, CobInstantRecharge,
        DisableItemDrop, PlantInvincible, DisableSpecialAttack, CobFixedDelay, MushroomAwake);
    OpenMultipleEffective('Q', MAIN_UI_OR_FIGHT_UI);
    if (cur_task->debug) {
        SetErrorMode(CONSOLE);
    } else {
        SetErrorMode(POP_WINDOW);
        SkipTick([]() { return true; });
    }
    SelectCardsPlus(cur_task->card_selection);

    type_list = random_type_list(cur_task->required_types, cur_task->banned_types);
    if (cur_task->uniform_summon) {
        SetZombies(type_list);
    } else {
        simulate_summon(
            type_list, 1000, cur_task->huge ? ALL_HUGE : ALL_NORMAL, cur_task->giga_count);
    }
    unify_zombie_spawn_x(cur_task->huge ? ALL_HUGE : ALL_NORMAL);
    SetTime(-599, 1);
    pao_operator.autoGetPaoList();
    if (cur_task->dance_cheat) {
        DanceCheat(cur_task->assume_refresh ? DanceCheatMode::FAST : DanceCheatMode::SLOW);
        if (cur_task->assume_refresh)
            MaidCheats::move();
        else
            MaidCheats::dancing();
    }

    int last_time = cur_task->check_time[cur_task->check_time.size() - 1];
    for (int wave = 1; wave <= 20; wave++) {
        InsertTimeOperation(
            1, wave,
            [=]() {
                initial_total_hp[wave] = GetMainObject()->mRef<int>(0x5598);
                if (cur_task->clear_zombies)
                    GetMainObject()->zombieRefreshHp() = 0;
            },
            "get_initial_total_hp");
        SetTime(0, wave);
        for (const auto& operation : cur_task->operations) {
            operation(wave);
        }
        for (int i = 0; i < cur_task->check_time.size(); i++)
            InsertTimeOperation(
                cur_task->check_time[i], wave,
                [=]() {
                    auto& rd = refresh_data[i];
                    rd.left_count[wave].clear();
                    for (auto& z : alive_zombie_filter)
                        if (z.standState() == wave - 1 && !z.mRef<bool>(0xb8)) {
                            int type = z.type();
                            if (RangeIn(z.type(), {GARGANTUAR, GIGA_GARGANTUAR}))
                                type = z.type() * 10 + ceil(z.hp() / 1800.0);
                            rd.left_count[wave][type]++;
                        }
                    int hp = wave_total_hp(wave);
                    rd.hp_ratio[wave] = 1.0 * hp / initial_total_hp[wave];
                    double refresh_prob = (0.65 - min(max(rd.hp_ratio[wave], 0.5), 0.65)) / 0.15;
                    rd.wave_prob[wave] = cur_task->assume_refresh ? 1 - refresh_prob : refresh_prob;
                },
                "check_refresh");
        if (cur_task->clear_zombies || RangeIn(wave, {9, 19, 20}))
            kill_all_zombies({wave}, {}, last_time);
    }

    InsertTimeOperation(
        0, 20, []() { progress = refresh_data[0].ranking.size() + 1; }, "update_progress");
    for (int i = 0; i < cur_task->check_time.size(); i++)
        InsertTimeOperation(
            cur_task->check_time[i], 20,
            [=]() {
                auto& rd = refresh_data[i];
                rd.result = accumulate(rd.wave_prob + 1, rd.wave_prob + 21, 0.0);
                rd.sum += rd.result;
                rd.hist[int(ceil(rd.result * 2))]++;
                string suffix = "(" + to_string(cur_task->check_time[i]) + ")";
                if (cur_task->check_time.size() == 1)
                    suffix = "";
                generate_stats(rd, suffix);
                generate_data(rd, suffix);
                generate_raw(rd, suffix);
            },
            "generate_csv");
}
