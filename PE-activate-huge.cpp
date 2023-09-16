#include "refresh_extended/refresh.h"

/***** 配置部分 *****/
string base_path = R"(C:\Games\Plants vs. Zombies\Data\刷新\自然自测\PE)"; // 输出文件夹位置（若不存在则自动创建）
const int TOTAL_ROUND = 1000;                                              // 测试选卡数
/***** 配置部分结束 *****/

/*
测试用阵型的布阵码：LI43bJyUlFSWXNR1tiSEVdUzbnA2RzV0lkhU5K1E11Y=
(现成存档：saves\game1_13.dat)

PvZ 10开的情况下，运行以下脚本需要约 [TOTAL_ROUND * ??]s
*/

#ifdef TEST_RUN
const int ROUND_NUM = 1;
#else
const int ROUND_NUM = TOTAL_ROUND;
#endif

auto default_conf = TaskBuilder().total(ROUND_NUM).uniform_summon(false);
auto norm_act = default_conf.huge(false).assume_refresh(true);
auto norm_sep = default_conf.huge(false).assume_refresh(false);
auto huge_act = default_conf.huge(true).assume_refresh(true);
auto huge_sep = default_conf.huge(true).assume_refresh(false);

auto hb_n_a = norm_act.output_folder(base_path + "\\红白\\激活")
                  .required_types({GARGANTUAR, GIGA_GARGANTUAR})
                  .banned_types({});
auto hb_n_s = norm_sep.output_folder(base_path + "\\红白\\分离")
                  .required_types({GARGANTUAR, GIGA_GARGANTUAR, ZOMBONI})
                  .banned_types({});

auto h_n_a = norm_act.output_folder(base_path + "\\红\\激活")
                 .required_types({GIGA_GARGANTUAR})
                 .banned_types({GARGANTUAR});
auto h_h_a = huge_act.output_folder(base_path + "\\红\\大波激活")
                 .required_types({GIGA_GARGANTUAR})
                 .banned_types({});
auto h_n_s = norm_sep.output_folder(base_path + "\\红\\分离")
                 .required_types({GIGA_GARGANTUAR, ZOMBONI})
                 .banned_types({GARGANTUAR});

auto b_n_s = norm_sep.output_folder(base_path + "\\白\\分离")
                 .required_types({GARGANTUAR, ZOMBONI})
                 .banned_types({GIGA_GARGANTUAR});

auto f_h_a = huge_act.output_folder(base_path + "\\快速\\大波激活")
                 .required_types({})
                 .banned_types({GARGANTUAR, GIGA_GARGANTUAR});
auto f_h_s = huge_act.output_folder(base_path + "\\快速\\分离")
                 .required_types({})
                 .banned_types({GARGANTUAR, GIGA_GARGANTUAR});

/*
测试用阵型的布阵码：LI43bJyUlFSWXNR1tiSEVdUzbnA2RzV0lkhU5K1E11Y=

运行以下脚本约需 N * ?s（10开）
*/

vector<Task> get_tasks()
{
    vector<Task> tasks;

    // 测试含红关（包括单红与红白）
    {
        auto& base = h_h_a;
        // PPD(smart)
        for (int t : {296, 341, 401, 441}) {
            tasks.push_back(base
                                .check_time(max(t, 401))
                                .actions(Cob(t, "PP", {2, 5}, 9),
                                    SmartCard(t, CHERRY_BOMB, {GARGANTUAR, GIGA_GARGANTUAR}, {{{1, 2}, 2}, {{5, 6}, 5}}, 9)));
        }

        // PPD
        for (int t : {296, 341, 401, 441}) {
            tasks.push_back(base
                                .check_time(max(t, 401))
                                .actions(Cob(t, "PP", {2, 5}, 9),
                                    Cob(t, "D", 2, 9)));
        }

        // P/PSD
        for (int t : {296, 341, 359, 401}) {
            tasks.push_back(base
                                .check_time(401)
                                .actions(Cob(t, "P", 2, 9),
                                    Cob(t - 107, "PS", {5, 5}, 9), Cob(t, "D", 5, 9)));
        }

        // PSD/P
        for (int t : {296, 341, 359, 401}) {
            tasks.push_back(base
                                .check_time(401)
                                .actions(Cob(t - 107, "PS", {2, 2}, 9), Cob(t, "D", 1, 9),
                                    Cob(t, "P", 5, 9)));
        }
    }

    // 测试快速关
    {
        auto& base = f_h_a;
        // PP
        for (int t = 341; t <= 441; t += 10) {
            tasks.push_back(base
                                .check_time(max(t, 401))
                                .actions(Cob(t, "PP", {2, 5}, 9)));

            tasks.push_back(base
                                .dance_cheat(false)
                                .check_time(max(t, 401))
                                .actions(Cob(t, "PP", {2, 5}, 9)));
        }
    }

    return tasks;
}