#include "bsp_include.h"

// 二通球阀定义
#define V1 32
#define V2 33
#define V4 34
#define V5 35
#define V12 36
#define V15 37
#define V16 38

// 电磁阀
#define V6 48
#define V7 49
#define V8 50
#define V9 51
#define V10 52
#define V11 53

// 三通球阀定义
#define V3_G 40
#define V3_R 41
#define V13_G 42
#define V13_R 43
#define V14_G 44
#define V14_R 45

#define M_CHUZAO 80
#define M1 81
#define M2 82
#define M3 83 // 增压泵电源
#define M4 84 // 配水泵电源

#define SHUILIU 116     // 水流
#define YIJI_YEWEI 117  // 一级液位
#define ERJI_YEWEI 118  // 二级液位
#define SANJI_YEWEI 119 // 三级液位

static const int allCtrl[] = {M_CHUZAO, M1, M2, M4, M3, V1, V2, V3_G, V3_R, V4, V5, V6, V7, V8, V9, V10, V11, V12, V13_G, V13_R, V14_G, V14_R, V15, V16};

static int ValveCtrlForM1M2(void)
{
    if (gM1M2Now == M1M2Type_M1)
    {
        Port_Write(V1, 1);
        Port_Write(V2, 0);
    }

    if (gM1M2Now == M1M2Type_M2)
    {
        Port_Write(V1, 0);
        Port_Write(V2, 1);
    }

    return 0;
}

static int M1M2CtrlOn(void)
{
    if (gM1M2Now == M1M2Type_M1)
    {
        Port_Write(M1, 1);
    }

    if (gM1M2Now == M1M2Type_M2)
    {
        Port_Write(M2, 1);
    }

    return 0;
}

int AllCtrl(int opt)
{
    for (int i = 0; i < sizeof(allCtrl) / sizeof(allCtrl[0]); i++)
    {
        Port_Write(allCtrl[i], opt);
    }

    return 0;
}

// 氨氮补水流程
static const _Flow Flow_AndanBuShui[] = {
    {
        .log = "氨氮补水",
        .log2 = "开启V12|V13-G|V14-R",
        .on = {V12, V13_G, V14_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "氨氮补水",
        .log2 = "开启M4",
        .on = {M4},
    },
    {
        .log = "氨氮补水",
        .log2 = "等待氨氮水样池液位开关",
        .funWait = Wait_AndanForBushui,
    },
    {
        .log = "氨氮补水",
        .log2 = "[感应到]液位开关",
    },
    // 补水完成
    {
        .log = "氨氮补水完成",
        .log2 = "关闭M4|V12|V13-G|V14-R",
        .off = {M4, V12, V13_G, V14_R},
    },
};

int Flows_AndanBuShui_Run_JianYi(void)
{
    for (int i = 0; i < sizeof(Flow_AndanBuShui) / sizeof(_Flow); i++)
    {
        Flow_Run(&Flow_AndanBuShui[i], false); // 补水会走完整个流程,停止信号只是会取消等待
    }
    return 0;
}

// 程序初始化
static const _Flow Flow_ChuShiHua[] = {
    {
        .index = FlowIndex_Init,
        .log = "程序初始化",
        .log2 = "",
        .funNoWait = Flow_Init,
    },
};

int Flows_ChuShiHua_Run_JianYi(void)
{
    int res = 0;
    for (int i = 0; i < sizeof(Flow_ChuShiHua) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_ChuShiHua[i], true)) != IS_OVER)
            break;
    }

    AllCtrl(0);

    return res;
}

// 外部管路及水箱润洗
static const _Flow Flow_ShuiXiangRunXi[] = {
    // 外部管路及流通池润洗
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "外部管路及流通池润洗",
        .log2 = "开启V1或者V2|V3-G|V4",
        .on = {V3_G, V4},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "外部管路及流通池润洗",
        .log2 = "开启潜水泵",
        .funNoWait = M1M2CtrlOn,
        .delayCfg = &cfgFlow.runxi1Time,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "外部管路及流通池润洗",
        .log2 = "关闭潜水泵|V3-G",
        .off = {M1, M2, V3_G},
    },
    // 外部管路及沉淀池润洗
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "外部管路及沉淀池润洗",
        .log2 = "开启V5",
        .on = {V5},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "外部管路及沉淀池润洗",
        .log2 = "开启V1或者V2|V3-R,关闭V4",
        .on = {V3_R},
        .off = {V4},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "外部管路及沉淀池润洗",
        .log2 = "开启潜水泵",
        .funNoWait = M1M2CtrlOn,
        .delayCfg = &cfgFlow.runxi2Time,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "外部管路及沉淀池润洗",
        .log2 = "关闭潜水泵|V3-R|V5",
        .off = {M1, M2, V3_R, V5},
    },
};

int Flows_ShuiXiangRunXi_Run_JianYi(void)
{
    int res = 0;
    for (int i = 0; i < sizeof(Flow_ShuiXiangRunXi) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_ShuiXiangRunXi[i], true)) != IS_OVER)
            break;
    }

    AllCtrl(0);

    return res;
}

// 流通池进样
static const _Flow Flow_LiuTongChiJinYang[] = {
    // 流通池进样
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "流通池进样",
        .log2 = "开启V1或者V2|V3-G",
        .on = {V3_G},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "流通池进样",
        .log2 = "开启潜水泵",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "流通池进样",
        .log2 = "等待水流开关",
        .funWait = Wait_Shuiliu,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "流通池进样",
        .log2 = "[感应到]水流开关",
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "流通池进样",
        .log2 = "等待一级液位开关",
        .funWait = Wait_YiJiYeWei,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "流通池进样",
        .log2 = "[感应到]液位开关",
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "流通池进样结束",
        .log2 = "关闭潜水泵|V1|V2|V3-G",
        .off = {M1, M2, V1, V2, V3_G},
    },
};

int Flows_LiuTongChiJinYang_Run_JianYi(void)
{
    int res = 0;
    int i = 0;
RETRY_LIUTONGCHI:
    for (i = 0; i < sizeof(Flow_LiuTongChiJinYang) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_LiuTongChiJinYang[i], true)) != IS_OVER)
            break;
    }

    if (res == IS_OVERTIME && (strstr(Flow_LiuTongChiJinYang[i].log2, "水流") != NULL))
    {
        if (M1M2Change() == 0)
            goto RETRY_LIUTONGCHI;
    }

    AllCtrl(0);

    return res;
}

// 沉淀池进样
static const _Flow Flow_ChenDianChiJinYang[] = {
    // 沉淀池进样
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "沉淀池进样",
        .log2 = "开启V1或者V2|V3-R",
        .on = {V3_R},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "沉淀池进样",
        .log2 = "开启潜水泵",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "沉淀池进样",
        .log2 = "等待水流开关",
        .funWait = Wait_Shuiliu,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "沉淀池进样",
        .log2 = "[感应到]水流开关",
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "沉淀池进样",
        .log2 = "等待二级液位开关",
        .funWait = Wait_ErJiYeWei,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "沉淀池进样",
        .log2 = "[感应到]液位开关",
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "沉淀池进样结束",
        .log2 = "关闭潜水泵|V1|V2|V3-R",
        .off = {M1, M2, V1, V2, V3_R},
    },
};

int Flows_ChenDianChiJinYang_Run_JianYi(void)
{
    int res = 0;
    int i = 0;
RETRY_CHENDIANCHI:
    for (i = 0; i < sizeof(Flow_ChenDianChiJinYang) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_ChenDianChiJinYang[i], true)) != IS_OVER)
            break;
    }

    if (res == IS_OVERTIME && (strstr(Flow_ChenDianChiJinYang[i].log2, "水流") != NULL))
    {
        if (M1M2Change() == 0)
            goto RETRY_CHENDIANCHI;
    }

    AllCtrl(0);

    return res;
}

// 水样沉淀
static const _Flow Flow_ChenDian[] = {
    {
        .index = FlowIndex_ChenDia,
        .log = "水样沉淀",
        .log2 = "",
        .delayCfg = &cfgFlow.chenDianTime,
    },
};

int Flows_ChenDian_Run_JianYi(void)
{
    int res = 0;
    for (int i = 0; i < sizeof(Flow_ChenDian) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_ChenDian[i], true)) != IS_OVER)
            break;
    }

    AllCtrl(0);

    return res;
}

// 氨氮水样过滤
static const _Flow Flow_ShuiYangGuoLv[] = {
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.过滤装置排液",
        .log2 = "开启V15",
        .on = {V15},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.过滤装置润洗",
        .log2 = "开启V12|V13-R|V14-R",
        .on = {V12, V13_R, V14_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.过滤装置润洗",
        .log2 = "开启M4,开启保护",
        .on = {M4},
        .funNoWait = Baohu_Start,
        .delayCfg = &cfgFlow.runxiGuoLvTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.过滤装置润洗结束",
        .log2 = "关闭M4|V13-R",
        .off = {M4, V13_R},
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.氨氮水样池润洗",
        .log2 = "开启V13-G",
        .on = {V13_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.氨氮水样池润洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.runxiAnDanTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.氨氮水样池润洗结束",
        .log2 = "关闭V15,关闭保护",
        .off = {V15},
        .funNoWait = Baohu_Stop,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.氨氮水样池过滤进样",
        .log2 = "等待氨氮水样池液位开关",
        .funWait = Wait_SanJiYeWei,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.氨氮水样池过滤进样",
        .log2 = "[感应到]液位开关",
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.氨氮水样池过滤进样结束",
        .log2 = "关闭M4|V12|V13-G",
        .off = {M4, V12, V13_G},
    },
};

int Flows_ShuiYangGuoLv_Run_JianYi(void)
{
    int res = 0;
    for (int i = 0; i < sizeof(Flow_ShuiYangGuoLv) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_ShuiYangGuoLv[i], true)) != IS_OVER)
            break;
    }

    // 期间有开启保护,需要关闭
    Baohu_Stop();

    AllCtrl(0);

    return res;
}

// 常规配水
static const _Flow Flow_PeiShui_ChangGui[] = {
    {
        .index = FlowIndex_PeiShui,
        .log = "常规配水开始",
        .log2 = "开启氨氮补水",
        .funNoWait = BuShuiAndan_Start_JianYi,
        .delayCfg = &cfgFlow.peiShuiChangGuiTime,
    },
    {
        .index = FlowIndex_PeiShui,
        .log = "常规配水结束",
        .log2 = "关闭氨氮补水",
        .funNoWait = BuShuiAndan_Stop_JianYi,
    },
};

// 加标配水
static const _Flow Flow_PeiShui_JiaBiao[] = {
    {
        .index = FlowIndex_PeiShui,
        .log = "加标配水开始",
        .log2 = "开启氨氮补水",
        .funNoWait = BuShuiAndan_Start_JianYi,
        .delayCfg = &cfgFlow.peiShuiJiaBiaoTime,
    },
    {
        .index = FlowIndex_PeiShui,
        .log = "加标配水结束",
        .log2 = "关闭氨氮补水",
        .funNoWait = BuShuiAndan_Stop_JianYi,
    },
};

int Flows_PeiShui_Run_JianYi(uint8_t isJiaBiao)
{
    int res = 0;
    if (isJiaBiao == 0)
    {
        for (int i = 0; i < sizeof(Flow_PeiShui_ChangGui) / sizeof(_Flow); i++)
        {
            if ((res = Flow_Run(&Flow_PeiShui_ChangGui[i], true)) != IS_OVER)
                break;
        }
    }
    else
    {
        for (int i = 0; i < sizeof(Flow_PeiShui_JiaBiao) / sizeof(_Flow); i++)
        {
            if ((res = Flow_Run(&Flow_PeiShui_JiaBiao[i], true)) != IS_OVER)
                break;
        }
    }

    // 期间有开启补水,需要关闭
    BuShuiAndan_Stop_JianYi();

    AllCtrl(0);

    return res;
}

// 反冲洗
static const _Flow Flow_FanChongXi[] = {
    // 流通池排液阀
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.流通池排液",
        .log2 = "开启V4|V5",
        .on = {V4, V5},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.流通池排液",
        .log2 = "开启V15",
        .on = {V15},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.流通池排液结束",
        .log2 = "关闭V4|V5|V15",
        .off = {V4, V5, V15},
    },

    // 外部管路反冲洗
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.除藻",
        .log2 = "开启V1|除藻泵",
        .on = {V1, M_CHUZAO},
        .delayCfg = &cfgFlow.chuZaoTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.除藻结束",
        .log2 = "关闭除藻泵",
        .off = {M_CHUZAO},
    },

    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.外部管路反冲洗",
        .log2 = "开启V7|V10|V14-R",
        .on = {V7, V10, V14_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.外部管路反冲洗",
        .log2 = "开启M3",
        .on = {M3},
        .delayCfg = &cfgFlow.guanLuFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.外部管路反冲洗结束",
        .log2 = "关闭M3|V1|V7|V10|V14-R",
        .off = {M3, V1, V7, V10, V14_R},
    },

    // 内部管路反冲洗
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.内部管路反冲洗",
        .log2 = "开启V7|V16",
        .on = {V7, V16},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.内部管路反冲洗",
        .log2 = "开启M3",
        .on = {M3},
        .delayCfg = &cfgFlow.guanLuFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.内部管路反冲洗结束",
        .log2 = "关闭M3|V7",
        .off = {M3, V7},
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.内部管路进气",
        .log2 = "开启V6",
        .on = {V6},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.内部管路进气结束",
        .log2 = "关闭V6|V16",
        .off = {V6, V16},
    },

    // 过滤装置反冲洗
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置反冲洗",
        .log2 = "开启V7|V13-R|V14-G",
        .on = {V7, V13_R, V14_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置反冲洗",
        .log2 = "开启M3",
        .on = {M3},
        .delayCfg = &cfgFlow.guolvFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置反冲洗结束",
        .log2 = "关闭M3|V7",
        .off = {M3, V7},
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置进气",
        .log2 = "开启V6",
        .on = {V6},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置进气结束",
        .log2 = "关闭V6|V13-R|V14-G,开启V14-R",
        .on = {V14_R},
        .off = {V6, V13_R, V14_G},
    },

    // 流通池池反冲洗
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.流通池池反冲洗",
        .log2 = "开启V4|V7|V8",
        .on = {V4, V7, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.流通池池反冲洗",
        .log2 = "开启M3",
        .on = {M3},
        .delayCfg = &cfgFlow.yiJiFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.流通池池反冲洗结束",
        .log2 = "关闭M3|V4|V7|V8",
        .off = {M3, V4, V7, V8},
    },

    // 沉淀池反冲洗
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.沉淀池反冲洗",
        .log2 = "开启V5|V7|V9",
        .on = {V5, V7, V9},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.沉淀池反冲洗",
        .log2 = "开启M3",
        .on = {M3},
        .delayCfg = &cfgFlow.erJiFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.沉淀池反冲洗结束",
        .log2 = "关闭M3|V5|V7|V9",
        .off = {M3, V5, V7, V9},
    },

    // 过滤装置进气
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置进气",
        .log2 = "开启V13-R|V14-G",
        .on = {V13_R, V14_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置进气",
        .log2 = "开启V6",
        .on = {V6},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置进气结束",
        .log2 = "关闭V6|V13-R|V14-G",
        .off = {V6, V13_R, V14_G},
    },
};

int Flows_FanChongXi_Run_JianYi(void)
{
    int res = 0;
    for (int i = 0; i < sizeof(Flow_FanChongXi) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_FanChongXi[i], true)) != IS_OVER)
            break;
    }

    AllCtrl(0);

    return res;
}

// 流通池水样储水
static const _Flow Flow_YiJiChuShui_ShuiYang[] = {
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "流通池水样储水",
        .log2 = "开启V1或V2|V3-G",
        .on = {V3_G},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "流通池水样储水",
        .log2 = "开启潜水泵",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "流通池水样储水",
        .log2 = "等待一级液位开关或储水时间",
        .funWait = Wait_YiJiYeWei_Or_ChuShuiTime,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "流通池水样储水",
        .log2 = "[感应到]液位开关或[到达]时间",
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "流通池水样储水结束",
        .log2 = "关闭潜水泵|V1|V2|V3-G",
        .off = {M1, M2, V1, V2, V3_G},
    },
};

// 流通池自来水储水
static const _Flow Flow_YiJiChuShui_ZiLaiShui[] = {
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "流通池自来水储水",
        .log2 = "开启V14-R|V7|V8",
        .on = {V14_R, V7, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "流通池自来水储水",
        .log2 = "开启M3",
        .on = {M3},
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "流通池自来水储水",
        .log2 = "等待一级液位开关或储水时间",
        .funWait = Wait_YiJiYeWei_Or_ChuShuiTime,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "流通池自来水储水",
        .log2 = "[感应到]液位开关或[到达]时间",
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "流通池自来水储水结束",
        .log2 = "关闭M3|V7|V8|V14-R",
        .off = {M3, V7, V8, V14_R},
    },
};

int Flows_YiJiChuShui_Run_JianYi(void)
{
    int res = 0;

    if (cfgFlow.chuishuiType == CHUISHUITYPE_SHUIYANG)
    {

        for (int i = 0; i < sizeof(Flow_YiJiChuShui_ShuiYang) / sizeof(_Flow); i++)
        {
            if ((res = Flow_Run(&Flow_YiJiChuShui_ShuiYang[i], true)) != IS_OVER)
                break;
        }
    }
    else if (cfgFlow.chuishuiType == CHUISHUITYPE_ZILAISHUI)
    {

        for (int i = 0; i < sizeof(Flow_YiJiChuShui_ZiLaiShui) / sizeof(_Flow); i++)
        {
            if ((res = Flow_Run(&Flow_YiJiChuShui_ZiLaiShui[i], true)) != IS_OVER)
                break;
        }
    }
    else
    {
        res = IS_ERROR;
    }

    AllCtrl(0);

    return res;
}
