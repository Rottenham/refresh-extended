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

int WaveTotalHp(int wave)
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

void set_wisdom_tree_height()
{
    if (ReadMemory<int>(0x6a9ec0, 0x82c, 0xf4) < 1000)
        WriteMemory<int>(1000, 0x6a9ec0, 0x82c, 0xf4);
}

void unify_zombie_spawn_x(WaveType wave_type)
{
    if (wave_type == DEFAULT)
        return;
    const set<ZombieType> EXCLUDED_ZOMBIE_TYPES = {FLAG_ZOMBIE, POLE_VAULTING_ZOMBIE, ZOMBONI,
        CATAPULT_ZOMBIE, GARGANTUAR, GIGA_GARGANTUAR, BUNGEE_ZOMBIE};
    auto x_offset = (wave_type == ALL_HUGE) ? 40 : -40;
    for (auto wave = 1; wave <= 20; wave++) {
        if ((wave_type == ALL_HUGE) && (wave == 10 || wave == 20))
            continue;
        if ((wave_type == ALL_NORMAL) && !(wave == 10 || wave == 20))
            continue;
        AvZ::InsertTimeOperation(
            1, wave,
            [=]() {
                for (auto& z : alive_zombie_filter) {
                    if (z.existTime() >= 5 || EXCLUDED_ZOMBIE_TYPES.count((ZombieType)z.type()))
                        continue;
                    z.abscissa() += x_offset;
                }
            },
            "unify_zombie_spawn_x");
    }
}

void kill_all_zombies(const set<int>& waves, const set<ZombieType>& type_list = {}, int time = 1)
{
    time = time < 1 ? 1 : time;
    for (const auto& wave : waves) {
        if (wave >= 1 && wave <= 20)
            AvZ::InsertTimeOperation(
                time, wave,
                [=]() {
                    for (auto& z : alive_zombie_filter) {
                        if (type_list.empty() || type_list.count((ZombieType)z.type())) {
                            z.state() = 3;
                        }
                    }
                },
                "kill_all_zombies");
    }
}

vector<Task> get_tasks_in_current_batch()
{
    int batch_count = 1;
    int batch_index = 1; // 从1开始
#ifdef BATCH_COUNT
    batch_count = BATCH_COUNT;
#endif
#ifdef BATCH_INDEX
    batch_index = BATCH_INDEX;
#endif
    auto tasks = get_tasks();
    vector<Task> tasks_in_current_batch;
    int index = 0;
    for (const auto& task : tasks) {
        if (index % batch_count == batch_index - 1)
            tasks_in_current_batch.push_back(task);
        index++;
    }
    return tasks_in_current_batch;
}

bool initialize_task()
{
    if (!ensure_folder_exists(cur_task->output_folder)) {
        return false;
    }
    progress = 0;
    refresh_data.clear();
    sort(cur_task->check_time.begin(), cur_task->check_time.end());
    for (auto _ : cur_task->check_time) {
        RefreshData rd;
        rd.result = rd.sum = 0;
        memset(rd.hist, 0, sizeof(rd.hist));
        refresh_data.push_back(rd);
    }
    return true;
}

void Script()
{
    if (first_run) {
        first_run = false;
        set_wisdom_tree_height();
        start_time = clock();
        all_tasks = get_tasks_in_current_batch();
        if (all_tasks.empty()) {
            ShowErrorNotInQueue("没有测试任务. 中止.");
            return;
        }
        cur_task = all_tasks.begin();
        if (!initialize_task()) {
            return;
        }
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
            int total_time_in_secs = int(elapsed + 0.5);
            int hours = total_time_in_secs / 3600;
            int minutes = (total_time_in_secs % 3600) / 60;
            int seconds = (total_time_in_secs % 3600) % 60;
            ShowErrorNotInQueue("完成\n用时：#小时 #分 #秒", hours, minutes, seconds);
            return;
        }
        if (!initialize_task()) {
            return;
        }
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
                    int hp = 0;
                    rd.left_count[wave].clear();
                    for (auto& z : alive_zombie_filter) {
                        if (z.standState() == wave - 1 && !z.mRef<bool>(0xb8)) {
                            if (!contains({BACKUP_DANCER, BUNGEE_ZOMBIE}, z.type()))
                                hp += z.hp() + z.oneHp() + z.twoHp() / 5 + z.mRef<int>(0xe4);
                            int type = z.type();
                            if (contains({GARGANTUAR, GIGA_GARGANTUAR}, z.type()))
                                type = z.type() * 10 + ceil(z.hp() / 1800.0);
                            rd.left_count[wave][type]++;
                        }
                    }
                    int hp2 = WaveTotalHp(wave);
                    if (hp != hp2) {
                        ShowErrorNotInQueue("计算总血量 = #\n实际总血量 = #", hp, hp2);
                        throw Exception("");
                    }
                    rd.hp_ratio[wave] = 1.0 * hp / initial_total_hp[wave];
                    double refresh_prob = (0.65 - min(max(rd.hp_ratio[wave], 0.5), 0.65)) / 0.15;
                    rd.wave_prob[wave] = cur_task->assume_refresh ? 1 - refresh_prob : refresh_prob;
                },
                "check_refresh");
        if (cur_task->clear_zombies || RangeIn(wave, {9, 19, 20})) {
            kill_all_zombies({wave}, {}, last_time);
            if (!RangeIn(wave, {9, 19, 20}))
                SetWavelength({{wave, last_time + 200}});
        }
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
