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

// auto hb_n_a = norm_act.output_folder(base_path + "\\红白\\激活")
//                   .required_types({GARGANTUAR, GIGA_GARGANTUAR})
//                   .banned_types({});
auto hb_n_s = norm_sep.output_folder(base_path + "\\红白\\分离")
                  .required_types({GARGANTUAR, GIGA_GARGANTUAR, ZOMBONI})
                  .banned_types({});

// auto h_n_a = norm_act.output_folder(base_path + "\\红\\激活")
//                  .required_types({GIGA_GARGANTUAR})
//                  .banned_types({GARGANTUAR});
// auto h_h_a = huge_act.output_folder(base_path + "\\红\\大波激活")
//                  .required_types({GIGA_GARGANTUAR})
//                  .banned_types({});
auto h_n_s = norm_sep.output_folder(base_path + "\\红\\分离")
                 .required_types({GIGA_GARGANTUAR, ZOMBONI})
                 .banned_types({GARGANTUAR});

auto b_n_s = norm_sep.output_folder(base_path + "\\白\\分离")
                 .required_types({GARGANTUAR, ZOMBONI})
                 .banned_types({GIGA_GARGANTUAR});

// auto f_h_a = huge_act.output_folder(base_path + "\\快速\\大波激活")
//                  .required_types({})
//                  .banned_types({GARGANTUAR, GIGA_GARGANTUAR});
auto f_n_s = norm_sep.output_folder(base_path + "\\快速\\分离")
                 .required_types({})
                 .banned_types({GARGANTUAR, GIGA_GARGANTUAR});

vector<Task> get_tasks()
{
    vector<Task> tasks;

    // 测试红白车、红车无白、白车无红
    for (auto& base : {hb_n_s, h_n_s, b_n_s}) {
        for (const auto& separate_cob : {Cob(173, "P", 1, 8.225), Cob(301, "P", 1, 9)}) {
            auto IP = base.actions(FixedCard(1, ICE_SHROOM, 1, 1), separate_cob);

            // IP
            tasks.push_back(IP);

            // IPC
            tasks.push_back(IP
                                .actions(FixedCard(350, SPIKEWEED, 5, 9, 1)));

            for (int t : {1077, 1191}) {
                // IP-B
                tasks.push_back(IP
                                    .actions(Cob(t, "B", 3, 8.1125)));

                // IPC-B
                tasks.push_back(IP
                                    .actions(FixedCard(350, SPIKEWEED, 5, 9, 1),
                                        Cob(t, "B", 3, 8.1125)));
            }
        }

        for (const auto& separate_cobs : {Cob(173, "PP", {1, 5}, 8.225), Cob(301, "PP", {1, 5}, 9)}) {
            auto IPP = base.actions(FixedCard(1, ICE_SHROOM, 1, 1), separate_cobs);

            // IPP
            tasks.push_back(IPP);

            // IPP + BLOVER
            tasks.push_back(IPP
                                .actions(FixedCard(300, BLOVER, 1, 1)));

            // IPP-B
            for (int t : {1077, 1191}) {
                tasks.push_back(IPP
                                    .actions(Cob(t, "B", 3, 8.1125)));
            }
            tasks.push_back(IPP
                                .actions(Cob(1191, "B", 3, 7.2375)));
            tasks.push_back(IPP
                                .actions(Cob(1191, "B", 3, 7.2375),
                                    FixedCard(1191, BLOVER, 1, 1)));
        }

        for (const auto& separate_cobs : {Cob(33, "PP", {1, 5}, 9), Cob(433, "PP", {1, 5}, 7.5), Cob(460, "PP", {1, 5}, 8.325)}) {
            // IPP
            tasks.push_back(base
                                .actions(FixedCard(1, ICE_SHROOM, 1, 1),
                                    separate_cobs));
        }
    }

    // 测试红白车、红车无白
    for (auto& base : {hb_n_s, h_n_s}) {
        {
            auto BBI = base.actions(FixedCard(96, ICE_SHROOM, 1, 1),
                Cob(104, "BB", {1, 5}, 9));

            // BBI (减速PDD)
            tasks.push_back(BBI);

            // BBI-B
            for (int t : {1077, 1191}) {
                tasks.push_back(BBI
                                    .actions(Cob(t, "B", 3, 8.1125)));
            }

            // BBI-B(虚)
            tasks.push_back(BBI
                                .actions(Cob(1191, "B", 3, 7.2375)));
        }

        {
            auto I3Pd = base.actions(FixedCard(11, ICE_SHROOM, 1, 1),
                Cob(579, "P", 2, 9));

            // I3Pd
            tasks.push_back(I3Pd);

            // I3PdC
            tasks.push_back(I3Pd
                                .actions(FixedCard(350, SPIKEWEED, 5, 9, 1)));

            for (int t : {1077, 1191}) {
                // I3Pd-B
                tasks.push_back(I3Pd
                                    .actions(Cob(t, "B", 3, 8.1125)));

                // I3PdC-B
                tasks.push_back(I3Pd
                                    .actions(FixedCard(350, SPIKEWEED, 5, 9, 1),
                                        Cob(t, "B", 3, 8.1125)));
            }
        }

        {
            auto I3PPdd = base.actions(FixedCard(11, ICE_SHROOM, 1, 1),
                Cob(579, "PP", {2, 5}, 9));

            // I3PPdd
            tasks.push_back(I3PPdd);

            // I3PPdd-B
            for (int t : {1077, 1191}) {
                tasks.push_back(I3PPdd
                                    .actions(Cob(t, "B", 3, 8.1125)));
            }

            // I3PPdd-B(虚)
            tasks.push_back(I3PPdd
                                .actions(Cob(1191, "B", 3, 7.2375)));

            // I3PPdd(炸1、6路)
            tasks.push_back(base
                                .actions(FixedCard(11, ICE_SHROOM, 1, 1),
                                    Cob(579, "PP", {1, 6}, 9)));
        }

        {
            // I3PPDD
            tasks.push_back(base
                                .actions(FixedCard(11, ICE_SHROOM, 1, 1),
                                    Cob(411, "PP", {2, 5}, 9), Cob(622, "DD", {1, 5}, 8.5875)));
            tasks.push_back(base
                                .actions(FixedCard(11, ICE_SHROOM, 1, 1),
                                    Cob(458, "PP", {2, 5}, 9), Cob(678, "DD", {1, 5}, 8.525)));
        }
    }

    // 测试红白车、红车无白限制红眼数量的情况
    for (auto& base : {hb_n_s, h_n_s}) {
        for (int giga_count = 0; giga_count <= 2; giga_count++) {
            auto base_with_giga_count = base.giga_count(giga_count);

            for (const auto& separate_cob : {
                     Cob(173, "P", 1, 8.225),
                     Cob(301, "P", 1, 9),
                 }) {
                auto IP = base_with_giga_count.actions(FixedCard(1, ICE_SHROOM, 1, 1), separate_cob);

                // IP
                tasks.push_back(IP);

                // IPC
                tasks.push_back(IP
                                    .actions(FixedCard(350, SPIKEWEED, 5, 9, 1)));
            }

            for (const auto& separate_cobs : {Cob(173, "PP", {1, 5}, 8.225), Cob(301, "PP", {1, 5}, 9)}) {
                // IPP
                tasks.push_back(base_with_giga_count
                                    .actions(FixedCard(1, ICE_SHROOM, 1, 1),
                                        separate_cobs));
            }

            // I3Pd
            tasks.push_back(base_with_giga_count
                                .actions(FixedCard(11, ICE_SHROOM, 1, 1),
                                    Cob(579, "P", 2, 9)));

            // I3PdC
            tasks.push_back(base_with_giga_count
                                .actions(FixedCard(11, ICE_SHROOM, 1, 1),
                                    Cob(579, "P", 2, 9),
                                    FixedCard(350, SPIKEWEED, 5, 9, 1)));

            // I3PPdd
            tasks.push_back(base_with_giga_count
                                .actions(FixedCard(11, ICE_SHROOM, 1, 1),
                                    Cob(579, "PP", {2, 5}, 9)));
        }
    }

    // IPP-173-B-实
    tasks.push_back(hb_n_s
                        .actions(FixedCard(1, ICE_SHROOM, 1, 1),
                            Cob(173, "PP", {1, 5}, 8.225),
                            Cob(1184, "B", 3, 9)));

    // 测试快速
    for (const auto& separate_cobs : {Cob(173, "PP", {1, 5}, 8.225), Cob(301, "PP", {1, 5}, 9)}) {
        tasks.push_back(f_n_s
                            .actions(FixedCard(1, ICE_SHROOM, 1, 1),
                                separate_cobs));
    }

    return tasks;
}
