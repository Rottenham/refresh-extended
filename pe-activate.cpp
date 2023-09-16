#include "refresh_extended/refresh.h"

/***** 配置部分 *****/
string base_path = R"(C:\Games\Plants vs. Zombies\Data\刷新\自然自测\PE)"; // 输出文件夹位置（若不存在则自动创建）
const int TOTAL_ROUND = 1000;                                              // 测试选卡数
/***** 配置部分结束 *****/

/*
测试用阵型的布阵码：LI43bJyUlFSWXNR1tiSEVdUzbnA2RzV0lkhU5K1E11Y=
(现成存档：saves\game1_13.dat)

PvZ 10开的情况下，运行以下脚本需要约 [TOTAL_ROUND * 3]s
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

vector<Task> get_tasks()
{
    vector<Task> tasks;

    // 测试红白出怪和单红出怪
    for (auto& base : {hb_n_a, h_n_a}) {
        // PSDB
        for (int t : {264, 323}) {
            float col = t == 264 ? 8.2625 : 8.2;
            tasks.push_back(base
                                .actions(Cob(290, "PS", {2, 2}, 9), Cob(400, "D", 1, 9),
                                    Cob(t, "B", 5, col)));
        }

        // PPDD
        for (int t : {225, 260, 295}) {
            tasks.push_back(base
                                .check_time(401)
                                .actions(Cob(t, "PP", {2, 5}, 9),
                                    Cob(t + 107, "DD", {1, 5}, 8)));
        }
        for (int t : {225, 260, 295, 318}) {
            tasks.push_back(base
                                .check_time(401)
                                .actions(Cob(359, "P", 2, 9), Cob(468, "D", 1, 8),
                                    Cob(t, "P", 5, 9), Cob(t + 107, "D", 5, 8)));
        }

        // PP
        for (int t : {225, 260, 295, 318, 341, 359, 401, 500}) {
            float col = t >= 401 ? 8.75 : 9;
            tasks.push_back(base
                                .actions(Cob(t, "PP", {2, 5}, col)));
        }

        // AA'a
        for (int t : {200, 225, 260, 300, 340, 480, 400}) {
            tasks.push_back(base
                                .check_time(401)
                                .actions(FixedCard(t, CHERRY_BOMB, 2, 9),
                                    FixedCard(t, JALAPENO, 5, 1),
                                    FixedCard(t, SQUASH, 6, 9)));
        }

        // I-PP / IPP-PP
        for (int t = 1050; t <= 1550; t += 100) {
            tasks.push_back(base
                                .check_time(t)
                                .actions(FixedCard(1, ICE_SHROOM, 1, 1),
                                    Cob(t, "PP", {2, 5}, 8.75)));
            tasks.push_back(base
                                .check_time(t)
                                .actions(FixedCard(1, ICE_SHROOM, 1, 1),
                                    Cob(301, "PP", {1, 5}, 9),
                                    Cob(t, "PP", {2, 5}, 8.75)));
        }

        // 100I-PP / 100IPP-PP
        for (int t = 950; t <= 1550; t += 100) {
            float col = t == 1450 ? 8.625 : (t == 1550 ? 8.4 : 8.75); // 长冰波落点需要适当左移收跳跳
            tasks.push_back(base
                                .check_time(t)
                                .actions(FixedCard(100, ICE_SHROOM, 1, 1),
                                    Cob(t, "PP", {2, 5}, col)));
            tasks.push_back(base
                                .check_time(t)
                                .actions(FixedCard(100, ICE_SHROOM, 1, 1),
                                    Cob(301, "PP", {1, 5}, 8.6375),
                                    Cob(t, "PP", {2, 5}, col)));
        }
    }

    // 仅红白出怪
    for (auto& base : {hb_n_a}) {
        // PPI
        for (int t : {225, 258, 291, 318}) {
            tasks.push_back(base
                                .check_time(401)
                                .actions(Cob(t, "PP", {2, 5}, 9),
                                    FixedCard(t + 1, ICE_SHROOM, 1, 1)));
        }

        // PPA(A')-fixed
        for (int t : {225, 260, 296, 318}) {
            tasks.push_back(base
                                .check_time(401)
                                .actions(Cob(t, "PP", {2, 5}, 9),
                                    FixedCard(t + 1, JALAPENO, 1, 1)));
            tasks.push_back(base
                                .check_time(401)
                                .actions(Cob(t, "PP", {2, 5}, 9),
                                    FixedCard(t + 1, CHERRY_BOMB, 1, 9)));
        }

        // PPA(A')-smart
        for (int t : {225, 260}) {
            tasks.push_back(base
                                .check_time(401)
                                .actions(Cob(t, "PP", {2, 5}, 9),
                                    SmartCard(t + 1, JALAPENO, {GARGANTUAR, GIGA_GARGANTUAR}, {{1, 1}, {2, 2}, {5, 5}, {6, 6}}, 1)));
            tasks.push_back(base
                                .check_time(401)
                                .actions(Cob(t, "PP", {2, 5}, 9),
                                    SmartCard(t + 1, CHERRY_BOMB, {GARGANTUAR, GIGA_GARGANTUAR}, {{{1, 2}, 2}, {{5, 6}, 5}}, 9)));
        }

        // BBDD / BBA'DD
        tasks.push_back(base
                            .check_time(401)
                            .actions(Cob(260, "BB", {1, 5}, 9), Cob(370, "DD", {1, 5}, 8)));
        tasks.push_back(base
                            .check_time(401)
                            .actions(Cob(260, "BB", {2, 5}, 9), FixedCard(261, JALAPENO, 1, 1),
                                Cob(370, "DD", {1, 5}, 8)));

        // BB / BBA'
        tasks.push_back(base
                            .check_time(401)
                            .actions(Cob(260, "BB", {1, 5}, 9)));
        tasks.push_back(base
                            .check_time(401)
                            .actions(Cob(260, "BB", {2, 5}, 9), FixedCard(261, JALAPENO, 1, 1)));
    }

    return tasks;
}