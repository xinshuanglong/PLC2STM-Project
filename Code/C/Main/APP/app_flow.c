#include "bsp_include.h"

// 二通球阀定义
#define V2 32
#define V3 33
#define V4 34
#define V5 35
#define V8 36
#define V9 37
#define V10 38
#define V13 39

// 电磁阀
#define V7 48
#define V14 49

// 三通球阀定义
#define V1_G 40
#define V1_R 41
#define V6_G 42
#define V6_R 43
#define V11_G 44
#define V11_R 45
#define V12_G 46
#define V12_R 47

#define M1 81
#define M2 82
#define M3 84 // 配水泵电源
#define M4 83 // 增压泵电源

static const int allCtrl[] = {M1, M2, M3, M4, V1_G, V1_R, V2, V3, V4, V5, V6_G, V6_R, V7, V8, V9, V10, V11_G, V11_R, V12_G, V12_R, V13, V14};

#define SHUILIU 116     // 水流
#define YIJI_YEWEI 117  // 一级液位
#define ERJI_YEWEI 118  // 二级液位
#define SANJI_YEWEI 119 // 三级液位

const ConfigFlow cfgFlow _AT_CON_SEC = {
    .runxi1Time = 3,     // 默认30秒
    .runxi2Time = 3,     // 默认30秒
    .runxiGuoLvTime = 3, // 默认10秒
    .runxiAnDanTime = 3, // 默认10秒
    .paiyeTime = 3,      // 默认30秒
    .chushuiTime = 3,
    .guanLuFanChongXiTime = 3, // 默认120秒
    .guolvFanChongXiTime = 3,  // 默认180秒
    .yiJiFanChongXiTime = 3,   // 默认60秒
    .erJiFanChongXiTime = 3,   // 默认60秒
    .jinQiTime = 3,            // 默认30秒
    .chenDianTime = 3,         // 默认1800秒
    .chuZaoTime = 3,
    .peiShuiChangGuiTime = 3, // 默认1800秒
    .peiShuiJiaBiaoTime = 3,  // 默认5400秒

    .m1m2Type = M1M2Type_M1M2,
    .chuishuiType = CHUISHUITYPE_SHUIYANG,
    .haveGuoLv = 1,

    .yeWeiOverTime = 30,
    .shuiLiuOverTime = 30,
    .jinYangOverTime = 30,
};

extern EventGroupHandle_t xMeasureEventGroup;
extern EventGroupHandle_t xBushuiEventGroup;
extern EventGroupHandle_t xBushuiAndanEventGroup;

int gM1M2Now = M1M2Type_M1;
int M1M2Error = 0;
int Flow_Init(void)
{
    M1M2Error = 0;

    if (cfgFlow.m1m2Type == M1M2Type_M1)
    {
        gM1M2Now = M1M2Type_M1;
    }
    else if (cfgFlow.m1m2Type == M1M2Type_M2)
    {
        gM1M2Now = M1M2Type_M2;
    }
    else
    {
        if (gtTimeBdc.date & 1)
            gM1M2Now = M1M2Type_M1;
        else
            gM1M2Now = M1M2Type_M2;
    }

    return 0;
}

// M1失效切换M2,M2失效切换M1
//  成功切换返回0,否则返回1
int M1M2Change(void)
{
    if (cfgFlow.m1m2Type == M1M2Type_M1M2 && M1M2Error == 0)
    {
        M1M2Error++;

        if (gM1M2Now == M1M2Type_M2)
        {
            gM1M2Now = M1M2Type_M1;
        }
        else if (gM1M2Now == M1M2Type_M1)
        {
            gM1M2Now = M1M2Type_M2;
        }

        Alarm_Add(0, moto_switch_alarm, 0);

        return 0;
    }

    return 1;
}

static int ValveCtrlForM1M2(void)
{
    if (gM1M2Now == M1M2Type_M1)
    {
        Port_Write(V1_G, 1);
        Port_Write(V1_R, 0);
    }

    if (gM1M2Now == M1M2Type_M2)
    {
        Port_Write(V1_G, 0);
        Port_Write(V1_R, 1);
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

static int AllCtrl(int opt)
{
    for (int i = 0; i < sizeof(allCtrl) / sizeof(allCtrl[0]); i++)
    {
        Port_Write(allCtrl[i], opt);
    }

    return 0;
}

int WaitIO(int index, bool waitStates, uint32_t secondToWait, EventGroupHandle_t *xEventGroup, uint32_t breakBit)
{
    for (int i = 0; i < secondToWait; i++)
    {
        if (Port_Read_Filter(index) == waitStates)
        {
            return IS_OK;
        }

        if (xEventGroup != NULL)
        {
            EventBits_t ev = xEventGroupWaitBits(*xEventGroup, breakBit, pdFALSE, pdFALSE, 1000);
            if ((ev & breakBit) != 0)
            {
                return IS_STOP;
            }
        }
        else
        {
            vTaskDelay(1000);
        }

        printf(".");
    }

    return IS_OVERTIME;
}

// 等待缓冲时间
void Wait_HuanChong(EventGroupHandle_t *xEventGroup, uint32_t breakBit)
{
    xEventGroupWaitBits(*xEventGroup, breakBit, pdFALSE, pdFALSE, HUANCHONG_TIME * 1000);
}

// 一级液位检查,会阻塞线程
int Wait_YiJiYeWei(void)
{
    int res = 0;
    res = WaitIO(YIJI_YEWEI, true, cfgFlow.yeWeiOverTime, &xMeasureEventGroup, STOP_EV_BIT);
    if (res == IS_OK)
        Wait_HuanChong(&xMeasureEventGroup, STOP_EV_BIT);
    if (res == IS_OVERTIME)
        Alarm_Add(0, yiji_yewei_alarm, 0);

    return res;
}

// 二级液位检查,会阻塞线程
int Wait_ErJiYeWei(void)
{
    int res = 0;
    res = WaitIO(ERJI_YEWEI, true, cfgFlow.yeWeiOverTime, &xMeasureEventGroup, STOP_EV_BIT);
    if (res == IS_OVERTIME)
        Alarm_Add(0, erji_yewei_alarm, 0);

    return res;
}

// 三级液位检查,会阻塞线程
int Wait_SanJiYeWei(void)
{
    int res = 0;
    res = WaitIO(SANJI_YEWEI, true, cfgFlow.jinYangOverTime, &xMeasureEventGroup, STOP_EV_BIT);
    if (res == IS_OVERTIME)
        Alarm_Add(0, sanji_yewei_alarm, 0);

    return res;
}

// 补水一级液位检查,会阻塞线程
int Wait_YiJiYeWeiForBushui(void)
{
    int res = 0;
    res = WaitIO(YIJI_YEWEI, true, cfgFlow.yeWeiOverTime, &xBushuiEventGroup, STOP_BUSHUI_EV_BIT);
    if (res == IS_OK)
        Wait_HuanChong(&xBushuiEventGroup, STOP_BUSHUI_EV_BIT);
    if (res == IS_OVERTIME)
        Alarm_Add(0, yiji_yewei_bushui_alarm, 0);

    return res;
}

// 补水三级液位检查,会阻塞线程
int Wait_AndanForBushui(void)
{
    int res = 0;
    res = WaitIO(SANJI_YEWEI, true, cfgFlow.jinYangOverTime, &xBushuiAndanEventGroup, STOP_BUSHUI_ANDAN_EV_BIT);
    if (res == IS_OK)
        Wait_HuanChong(&xBushuiAndanEventGroup, STOP_BUSHUI_ANDAN_EV_BIT);
    if (res == IS_OVERTIME)
        Alarm_Add(0, sanji_yewei_bushui_alarm, 0);
    return res;
}

// 水流开关检查,会阻塞线程
int Wait_Shuiliu(void)
{
    int res = 0;
    res = WaitIO(SHUILIU, true, cfgFlow.shuiLiuOverTime, &xMeasureEventGroup, STOP_EV_BIT);
    if (res == IS_OVERTIME)
        Alarm_Add(0, shuiliu_alarm, 0);

    return res;
}

int Wait_YiJiYeWei_Or_ChuShuiTime(void)
{
    WaitIO(YIJI_YEWEI, true, cfgFlow.chushuiTime, &xMeasureEventGroup, STOP_BUSHUI_EV_BIT);
    return 0;
}

// 水样池补水流程
static const _Flow Flow_ShuiXiangBuShui[] = {
    {
        .log = "一级水样池补水",
        .log2 = "开启V1-G或V1-R",
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "一级水样池补水",
        .log2 = "开启潜水泵",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .log = "一级水样池补水",
        .log2 = "等待一级液位开关",
        .funWait = Wait_YiJiYeWeiForBushui,
    },
    {
        .log = "一级水样池补水",
        .log2 = "[感应到]液位开关",
    },
    // 补水完成
    {
        .log = "一级水样池补水完成",
        .log2 = "关闭潜水泵|V1-G|V1-R",
        .off = {M1, M2, V1_G, V1_R},
    },
};

// 氨氮补水流程
static const _Flow Flow_AndanBuShui[] = {
    {
        .log = "氨氮补水",
        .log2 = "开启V6-G|V11-G|V12-R",
        .on = {V6_G, V11_G, V12_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "氨氮补水",
        .log2 = "开启M3",
        .on = {M3},
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
        .log2 = "关闭M3|V6-G|V11-G|V12-R",
        .off = {M3, V6_G, V11_G, V12_R},
    },
};

// 程序初始化
static const _Flow Flow_ChuShiHua[] = {
    {
        .index = FlowIndex_Init,
        .log = "程序初始化",
        .log2 = "开启V4|V5",
        .on = {V4, V5},
        .funNoWait = Flow_Init,
        .delayCfg = &cfgFlow.paiyeTime,
    },
};

// 水样池润洗流程
static const _Flow Flow_ShuiXiangRunXi[] = {
    // 一级水样池润洗
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "一级水样池润洗",
        .log2 = "开启V1-G或V1-R|V5",
        .on = {V5},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "一级水样池润洗",
        .log2 = "开启潜水泵",
        .funNoWait = M1M2CtrlOn,
        .delayCfg = &cfgFlow.runxi1Time,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "一级水样池润洗",
        .log2 = "关闭V5",
        .off = {V5},
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "一级水样池润洗",
        .log2 = "等待一级液位开关",
        .funWait = Wait_YiJiYeWei,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "一级水样池润洗",
        .log2 = "[感应到]液位开关",
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "一级水样池润洗",
        .log2 = "关闭潜水泵|V1-G|V1-R",
        .off = {M1, M2, V1_G, V1_R},
    },
    // 二级水样池润洗
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "二级水样池润洗",
        .log2 = "开启V4|V6-R",
        .on = {V4, V6_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "二级水样池润洗",
        .log2 = "开启M3,开启补水",
        .on = {M3},
        .funNoWait = ShuiXiangBuShui_Start,
        .delayCfg = &cfgFlow.runxi2Time,
    },
    // 润洗时间到之后
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "二级水样池润洗",
        .log2 = "关闭M3|V6-R,开启V4|V5,关闭补水",
        .on = {V4, V5},
        .off = {M3, V6_R},
        .funNoWait = ShuiXiangBuShui_Stop,
        .delayCfg = &cfgFlow.paiyeTime,
    },
    // 排液时间到之后
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "二级水样池润洗",
        .log2 = "关闭V4|V5",
        .off = {V4, V5},
    },
};

// 一级水样池进样
static const _Flow Flow_YiJiJinYang[] = {
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "一级水样池进样",
        .log2 = "开启V1-G或V1-R",
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "一级水样池进样",
        .log2 = "开启潜水泵",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "一级水样池进样",
        .log2 = "等待水流开关",
        .funWait = Wait_Shuiliu,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "一级水样池进样",
        .log2 = "[感应到]水流开关",
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "一级水样池进样",
        .log2 = "等待一级液位开关",
        .funWait = Wait_YiJiYeWei,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "一级水样池进样",
        .log2 = "[感应到]液位开关",
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "一级水样池进样结束",
        .log2 = "关闭潜水泵|V1-G|V1-R",
        .off = {M1, M2, V1_G, V1_R},
    },
};

// 二级水样池进样
static const _Flow Flow_ErJiJinYang[] = {
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "二级水样池进样",
        .log2 = "开启V6_R",
        .on = {V6_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "二级水样池进样",
        .log2 = "开启M3,开启补水",
        .on = {M3},
        .funNoWait = ShuiXiangBuShui_Start,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "二级水样池进样",
        .log2 = "等待二级液位开关",
        .funWait = Wait_ErJiYeWei,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "二级水样池进样",
        .log2 = "[感应到]液位开关",
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "二级水样池进样结束",
        .log2 = "关闭M3|V6-R,关闭补水",
        .off = {M3, V6_R},
        .funNoWait = ShuiXiangBuShui_Stop,
    },
};

// 水样沉淀
static const _Flow Flow_ChenDian[] = {
    {
        .index = FlowIndex_ChenDia,
        .log = "水样沉淀",
        .log2 = "",
        .delayCfg = &cfgFlow.chenDianTime,
    },
};

// 氨氮水样过滤
static const _Flow Flow_ShuiYangGuoLv[] = {
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.过滤装置排液",
        .log2 = "开启V13",
        .on = {V13},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.过滤装置润洗",
        .log2 = "开启V6-G|V11-R|V12-R",
        .on = {V6_G, V11_R, V12_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.过滤装置润洗",
        .log2 = "开启M3,开启保护",
        .on = {M3},
        .funNoWait = Baohu_Start,
        .delayCfg = &cfgFlow.runxiGuoLvTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.过滤装置润洗结束",
        .log2 = "关闭M3|V11-R",
        .off = {M3, V11_R},
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.氨氮水样池润洗",
        .log2 = "开启V11-G",
        .on = {V11_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.氨氮水样池润洗",
        .log2 = "开启M3",
        .on = {M3},
        .delayCfg = &cfgFlow.runxiAnDanTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "水样过滤.氨氮水样池润洗结束",
        .log2 = "关闭V13,关闭保护",
        .off = {V13},
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
        .log2 = "关闭M3|V6-G|V11-G",
        .off = {M3, V6_G, V11_G},
    },
};

// 常规配水
static const _Flow Flow_PeiShui_ChangGui[] = {
    {
        .index = FlowIndex_PeiShui,
        .log = "常规配水开始",
        .log2 = "开启氨氮补水",
        .funNoWait = BuShuiAndan_Start,
        .delayCfg = &cfgFlow.peiShuiChangGuiTime,
    },
    {
        .index = FlowIndex_PeiShui,
        .log = "常规配水结束",
        .log2 = "关闭氨氮补水",
        .funNoWait = BuShuiAndan_Stop,
    },
};

// 加标配水
static const _Flow Flow_PeiShui_JiaBiao[] = {
    {
        .index = FlowIndex_PeiShui,
        .log = "加标配水开始",
        .log2 = "开启氨氮补水",
        .funNoWait = BuShuiAndan_Start,
        .delayCfg = &cfgFlow.peiShuiJiaBiaoTime,
    },
    {
        .index = FlowIndex_PeiShui,
        .log = "加标配水结束",
        .log2 = "关闭氨氮补水",
        .funNoWait = BuShuiAndan_Stop,
    },
};

// 反冲洗
static const _Flow Flow_FanChongXi[] = {
    // 水样池排液
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.一二级水样池排液",
        .log2 = "开启V4|V5",
        .on = {V4, V5},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤水样池排液",
        .log2 = "开启V13",
        .on = {V13},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.水样池排液结束",
        .log2 = "关闭V4|V5|V13",
        .off = {V4, V5, V13},
    },

    // 外部管路反冲洗
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.外部管路反冲洗",
        .log2 = "开启V1-G|V8|V9|V12-R",
        .on = {V1_G, V8, V9, V12_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.外部管路反冲洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.guanLuFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.外部管路反冲洗结束",
        .log2 = "关闭M4|V1-G|V8|V9|V12-R",
        .off = {M4, V1_G, V8, V9, V12_R},
    },

    // 内部管路反冲洗
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.内部管路反冲洗",
        .log2 = "开启V6-R|V6-G|V8|V11-R|V14",
        .on = {V6_R, V6_G, V8, V11_R, V14},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.内部管路反冲洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.guanLuFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.内部管路反冲洗结束",
        .log2 = "关闭M4|V8",
        .off = {M4, V8},
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.内部管路进气",
        .log2 = "开启V7",
        .on = {V7},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.内部管路进气结束",
        .log2 = "关闭V6-R|V6-G|V7|V11-R|V14",
        .off = {V6_R, V6_G, V7, V11_R, V14},
    },

    // 过滤装置反冲洗
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置反冲洗",
        .log2 = "开启V6-R|V8|V11-R|V12-G",
        .on = {V6_R, V8, V11_R, V12_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置反冲洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.guolvFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置反冲洗结束",
        .log2 = "关闭M4|V8",
        .off = {M4, V8},
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置进气",
        .log2 = "开启V7",
        .on = {V7},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置进气结束",
        .log2 = "关闭V6-R|V7|V11-R|V12-G,开启V12-R",
        .on = {V12_R},
        .off = {V6_R, V7, V11_R, V12_G},
    },

    // 一级水样池反冲洗
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.一级水样池反冲洗",
        .log2 = "开启V2|V5|V8",
        .on = {V2, V5, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.一级水样池反冲洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.yiJiFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.一级水样池反冲洗结束",
        .log2 = "关闭M4|V2|V5|V8",
        .off = {M4, V2, V5, V8},
    },

    // 二级水样池反冲洗
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.二级水样池反冲洗",
        .log2 = "开启V3|V4|V8",
        .on = {V3, V4, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.二级水样池反冲洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.erJiFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.二级水样池反冲洗结束",
        .log2 = "关闭M4|V3|V4|V8",
        .off = {M4, V3, V4, V8},
    },

    // 过滤装置进气
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置进气",
        .log2 = "开启V6-R|V11-R|V12-G",
        .on = {V6_R, V11_R, V12_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置进气",
        .log2 = "开启V7",
        .on = {V7},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "反冲洗.过滤装置进气结束",
        .log2 = "关闭V6-R|V7|V11-R|V12-G",
        .off = {V6_R, V7, V11_R, V12_G},
    },
};

// 一级水样池水样储水
static const _Flow Flow_YiJiChuShui_ShuiYang[] = {
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "一级水样池水样储水",
        .log2 = "开启V1-G或V1-R",
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "一级水样池水样储水",
        .log2 = "开启潜水泵",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "一级水样池水样储水",
        .log2 = "等待一级液位开关或储水时间",
        .funWait = Wait_YiJiYeWei_Or_ChuShuiTime,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "一级水样池水样储水",
        .log2 = "[感应到]液位开关或[到达]时间",
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "一级水样池水样储水结束",
        .log2 = "关闭潜水泵|V1-G|V1-R",
        .off = {M1, M2, V1_G, V1_R},
    },
};

// 一级水样池自来水储水
static const _Flow Flow_YiJiChuShui_ZiLaiShui[] = {
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "一级水样池自来水储水",
        .log2 = "开启V12-R|V2|V8",
        .on = {V12_R, V2, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "一级水样池自来水储水",
        .log2 = "开启M4",
        .on = {M4},
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "一级水样池自来水储水",
        .log2 = "等待一级液位开关或储水时间",
        .funWait = Wait_YiJiYeWei_Or_ChuShuiTime,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "一级水样池自来水储水",
        .log2 = "[感应到]液位开关或[到达]时间",
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "一级水样池自来水储水结束",
        .log2 = "关闭M4|V12-R|V2|V8",
        .off = {M4, V12_R, V2, V8},
    },
};

int Flow_Run(const _Flow *flow, bool isMaster)
{
    char msg[64] = {0};
    snprintf(msg, 64, "%s.%s", flow->log, flow->log2);
    LogDatabase_Insert(msg);

    if (isMaster)
    {
        gtPara.step = flow->index;
        memcpy(gtPara.flowDescribe, msg, sizeof(gtPara.flowDescribe));
    }

    for (int i = 0; i < SWITCH_COUNT; i++)
    {
        Port_Write(flow->on[i], 1);
        Port_Write(flow->off[i], 0);
    }

    if (flow->funNoWait != NULL)
    {
        flow->funNoWait();
    }

    int res = 0;
    if (flow->funWait != NULL)
    {
        res = flow->funWait();
        if (res == IS_OVERTIME)
            LogDatabase_Insert("%s[超时]", msg);
        if (res == IS_ERROR)
            LogDatabase_Insert("%s[异常]", msg);
    }

    int delayTime = flow->delay;
    if (flow->delayCfg != NULL)
        delayTime += *flow->delayCfg;
    int stepLeftTime = delayTime;

    for (int k = 0; k < delayTime; k++)
    {
        EventBits_t ev;
        ev = xEventGroupWaitBits(xMeasureEventGroup, STOP_EV_BIT, pdFALSE, pdFALSE, 1000);
        if (ev & STOP_EV_BIT)
        {
            return IS_STOP;
        }
        stepLeftTime--;
        if (isMaster)
        {
            memset(gtPara.describe, 0, sizeof(gtPara.describe));
            snprintf(gtPara.describe, sizeof(gtPara.describe), "%ds", stepLeftTime);
            printf("%s ", gtPara.describe);
        }
    }
    if (delayTime > 0)
        printf("\r\n");

    return res;
}

// 水箱补水相关
bool ShuiXiangNeedBuShui(void)
{
    if (Port_Read_Filter(YIJI_YEWEI) == false)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int Flows_ShuiXiangBuShui_Run(void)
{
    for (int i = 0; i < sizeof(Flow_ShuiXiangBuShui) / sizeof(_Flow); i++)
    {
        Flow_Run(&Flow_ShuiXiangBuShui[i], false); // 补水会走完整个流程,停止信号只是会取消等待
    }
    return 0;
}

// 过滤装置保护
void Baohu(void)
{
    if (Port_Read_Filter(SANJI_YEWEI) == true)
    {
        Port_Write(M3, 0);
    }
}

// 氨氮补水相关
bool AndanNeedBuShui(void)
{
    if (Port_Read_Filter(SANJI_YEWEI) == false)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int Flows_AndanBuShui_Run(void)
{
    for (int i = 0; i < sizeof(Flow_AndanBuShui) / sizeof(_Flow); i++)
    {
        Flow_Run(&Flow_AndanBuShui[i], false); // 补水会走完整个流程,停止信号只是会取消等待
    }
    return 0;
}

int Flows_ChuShiHua_Run(void)
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

int Flows_ShuiXiangRunXi_Run(void)
{
    int res = 0;
    for (int i = 0; i < sizeof(Flow_ShuiXiangRunXi) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_ShuiXiangRunXi[i], true)) != IS_OVER)
            break;
    }

    // 期间有开启补水,需要关闭
    ShuiXiangBuShui_Stop();

    AllCtrl(0);

    return res;
}

int Flows_YiJiJinYang_Run(void)
{
    int res = 0;
    int i = 0;
RETRY_YIJIJINYANG:
    for (i = 0; i < sizeof(Flow_YiJiJinYang) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_YiJiJinYang[i], true)) != IS_OVER)
            break;
    }

    if (res == IS_OVERTIME && (strstr(Flow_YiJiJinYang[i].log2, "水流") != NULL))
    {
        if (M1M2Change() == 0)
            goto RETRY_YIJIJINYANG;
    }

    AllCtrl(0);

    return res;
}

int Flows_ErJiJinYang_Run(void)
{
    int res = 0;
    for (int i = 0; i < sizeof(Flow_ErJiJinYang) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_ErJiJinYang[i], true)) != IS_OVER)
            break;
    }

    // 期间有开启补水,需要关闭
    ShuiXiangBuShui_Stop();

    AllCtrl(0);

    return res;
}

int Flows_ChenDian_Run(void)
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

int Flows_ShuiYangGuoLv_Run(void)
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

int Flows_PeiShui_Run(uint8_t isJiaBiao)
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
    BuShuiAndan_Stop();

    AllCtrl(0);

    return res;
}

int Flows_FanChongXi_Run(void)
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

int Flows_YiJiChuShui_Run(void)
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
