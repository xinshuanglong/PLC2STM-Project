#include "bsp_include.h"

#define VALVE_SWITCH_TIME 5 // 阀切换时间,单位秒
#define CUSHION_TIME 5      // 避免液位开关频繁启动,添加缓冲时间,单位秒

// 二通球阀定义
#define V2 32
#define V3 33
#define V4 34
#define V5 35
#define V7 48
#define V8 36
#define V9 37
#define V10 38
#define V13 39
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
#define M3 84
#define M4 83

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
    .chushuiTime = 5,
    .guanLuFanChongXiTime = 5, // 默认120秒
    .guolvFanChongXiTime = 5,  // 默认180秒
    .yiJiFanChongXiTime = 5,   // 默认60秒
    .erJiFanChongXiTime = 5,   // 默认60秒
    .jinQiTime = 5,            // 默认30秒
    .chenDianTime = 10,        // 默认1800秒
    .peiShuiChangGuiTime = 20, // 默认1800秒
    .peiShuiJiaBiaoTime = 30,  // 默认5400秒

    .chuishuiType = CHUISHUITYPE_SHUIYANG,
    .haveGuoLv = 1,

    .yeWeiOverTime = 30,
    .shuiLiuOverTime = 30,
    .jinYangOverTime = 30,
};

extern EventGroupHandle_t xMeasureEventGroup;
extern EventGroupHandle_t xBushuiEventGroup;
extern EventGroupHandle_t xBushuiAndanEventGroup;

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
void Wait_Cushion(EventGroupHandle_t *xEventGroup, uint32_t breakBit)
{
    xEventGroupWaitBits(*xEventGroup, breakBit, pdFALSE, pdFALSE, CUSHION_TIME * 1000);
}

// 一级液位检查,会阻塞线程
int Wait_YiJiYeWei(void)
{
    int res = 0;
    res = WaitIO(YIJI_YEWEI, true, cfgFlow.yeWeiOverTime, &xMeasureEventGroup, STOP_EV_BIT);
    Wait_Cushion(&xMeasureEventGroup, STOP_EV_BIT);
    return res;
}

// 二级液位检查,会阻塞线程
int Wait_ErJiYeWei(void)
{
    int res = 0;
    res = WaitIO(ERJI_YEWEI, true, cfgFlow.yeWeiOverTime, &xMeasureEventGroup, STOP_EV_BIT);
    return res;
}

// 三级液位检查,会阻塞线程
int Wait_SanJiYeWei(void)
{
    int res = 0;
    res = WaitIO(SANJI_YEWEI, true, cfgFlow.jinYangOverTime, &xMeasureEventGroup, STOP_EV_BIT);
    return res;
}

// 补水一级液位检查,会阻塞线程
int Wait_YiJiYeWeiForBushui(void)
{
    int res = 0;
    res = WaitIO(YIJI_YEWEI, true, cfgFlow.yeWeiOverTime, &xBushuiEventGroup, STOP_BUSHUI_EV_BIT);
    Wait_Cushion(&xBushuiEventGroup, STOP_BUSHUI_EV_BIT);
    return res;
}

// 补水一级液位检查,会阻塞线程
int Wait_AndanForBushui(void)
{
    int res = 0;
    res = WaitIO(SANJI_YEWEI, true, cfgFlow.jinYangOverTime, &xBushuiAndanEventGroup, STOP_BUSHUI_ANDAN_EV_BIT);
    Wait_Cushion(&xBushuiAndanEventGroup, STOP_BUSHUI_ANDAN_EV_BIT);
    return res;
}

// 水流开关检查,会阻塞线程
int Wait_Shuiliu(void)
{
    int res = 0;
    res = WaitIO(SHUILIU, true, cfgFlow.shuiLiuOverTime, &xMeasureEventGroup, STOP_EV_BIT);
    return res;
}

int Wait_YiJiYeWei_Or_ChuShuiTime(void)
{
    WaitIO(YIJI_YEWEI, true, cfgFlow.chushuiTime, &xMeasureEventGroup, STOP_BUSHUI_EV_BIT);
    return 0;
}

// 水样池补水流程
const _Flow Flow_ShuiXiangBuShui[] = {
    {
        .log = "一级水样池补水",
        .log2 = "开启V1-G",
        .on = {V1_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "一级水样池补水",
        .log2 = "开启M1或M2",
        .on = {M1},
    },
    {
        .log = "一级水样池补水",
        .log2 = "等待一级液位开关",
        .funWait = Wait_YiJiYeWeiForBushui,
    },
    // 补水完成
    {
        .log = "一级水样池补水完成",
        .log2 = "关闭M1或M2|V1-G",
        .off = {M1 | V1_G},
    },
};

// 氨氮补水流程
const _Flow Flow_AndanBuShui[] = {
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
    // 补水完成
    {
        .log = "氨氮补水完成",
        .log2 = "关闭M3|V6-G|V11-G|V12-R",
        .off = {M3, V6_G, V11_G, V12_R},
    },
};

// 水样池润洗流程
const _Flow Flow_ShuiXiangRunXi[] = {
    // 一级水样池润洗
    {
        .log = "一级水样池润洗",
        .log2 = "开启V1-G|V5",
        .on = {V1_G, V5},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "一级水样池润洗",
        .log2 = "开启M1或M2",
        .on = {M1},
        .delayCfg = &cfgFlow.runxi1Time,
    },
    {
        .log = "一级水样池润洗",
        .log2 = "关闭V5",
        .off = {V5},
    },
    {
        .log = "一级水样池润洗",
        .log2 = "等待一级液位开关",
        .funWait = Wait_YiJiYeWei,
    },
    {
        .log = "一级水样池润洗",
        .log2 = "关闭M1或M2|V1-G",
        .off = {M1, V1_G},
    },
    // 二级水样池润洗
    {
        .log = "二级水样池润洗",
        .log2 = "开启V4|V6-R",
        .on = {V4, V6_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "二级水样池润洗",
        .log2 = "开启M3,开启补水",
        .on = {M3},
        .funNoWait = ShuiXiangBuShui_Start,
        .delayCfg = &cfgFlow.runxi2Time,
    },
    // 润洗时间到之后
    {
        .log = "二级水样池润洗",
        .log2 = "关闭M3|V6-R,开启V4|V5,关闭补水",
        .on = {V4, V5},
        .off = {M3, V6_R},
        .funNoWait = ShuiXiangBuShui_Stop,
        .delayCfg = &cfgFlow.paiyeTime,
    },
    // 排液时间到之后
    {
        .log = "二级水样池润洗",
        .log2 = "关闭V4|V5",
        .off = {V4, V5},
    },
};

// 一级水样池进样
const _Flow Flow_YiJiJinYang[] = {
    {
        .log = "一级水样池进样",
        .log2 = "开启V1-G",
        .on = {V1_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "一级水样池进样",
        .log2 = "开启M1或M2",
        .on = {M1},
    },
    {
        .log = "一级水样池进样",
        .log2 = "等待水流开关",
        .funWait = Wait_Shuiliu,
    },
    {
        .log = "一级水样池进样",
        .log2 = "等待一级液位开关",
        .funWait = Wait_YiJiYeWei,
    },
    {
        .log = "一级水样池进样结束",
        .log2 = "关闭M1或M2|V1-G",
        .off = {M1 | V1_G},
    },
};

// 二级水样池进样
const _Flow Flow_ErJiJinYang[] = {
    {
        .log = "二级水样池进样",
        .log2 = "开启V6_R",
        .on = {V6_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "二级水样池进样",
        .log2 = "开启M3,开启补水",
        .on = {M3},
        .funNoWait = ShuiXiangBuShui_Start,
    },
    {
        .log = "二级水样池进样",
        .log2 = "等待二级液位开关",
        .funWait = Wait_ErJiYeWei,
    },
    {
        .log = "二级水样池进样结束",
        .log2 = "关闭M3|V6-R,关闭补水",
        .off = {M3 | V6_R},
        .funNoWait = ShuiXiangBuShui_Stop,
    },
};

// 水样沉淀
const _Flow Flow_ChenDian[] = {
    {
        .log = "水样沉淀",
        .log2 = "",
        .delayCfg = &cfgFlow.chenDianTime,
    },
};

// 氨氮水样过滤
const _Flow Flow_ShuiYangGuoLv[] = {
    {
        .log = "水样过滤.过滤装置排液",
        .log2 = "开启V13",
        .on = {V13},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .log = "水样过滤.过滤装置润洗",
        .log2 = "开启V6-G|V11-R|V12-R",
        .on = {V6_G, V11_R, V12_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "水样过滤.过滤装置润洗",
        .log2 = "开启M3,开启保护",
        .on = {M3},
        .funNoWait = Baohu_Start,
        .delayCfg = &cfgFlow.runxiGuoLvTime,
    },
    {
        .log = "水样过滤.过滤装置润洗结束",
        .log2 = "关闭M3|V11-R",
        .off = {M3, V11_R},
    },
    {
        .log = "水样过滤.氨氮水样池润洗",
        .log2 = "开启V11-G",
        .on = {V11_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "水样过滤.氨氮水样池润洗",
        .log2 = "开启M3",
        .on = {M3},
        .delayCfg = &cfgFlow.runxiAnDanTime,
    },
    {
        .log = "水样过滤.氨氮水样池润洗结束",
        .log2 = "关闭V13,关闭保护",
        .off = {V13},
        .funNoWait = Baohu_Stop,
    },
    {
        .log = "水样过滤.氨氮水样池过滤进样",
        .log2 = "等待氨氮水样池液位开关",
        .funWait = Wait_SanJiYeWei,
    },
    {
        .log = "水样过滤.氨氮水样池过滤进样结束",
        .log2 = "关闭M3|V6-G|V11-G",
        .off = {M3, V6_G, V11_G},
    },
};

// 常规配水
const _Flow Flow_PeiShui_ChangGui[] = {
    {
        .log = "常规配水开始",
        .log2 = "开启氨氮补水",
        .funNoWait = BuShuiAndan_Start,
        .delayCfg = &cfgFlow.peiShuiChangGuiTime,
    },
    {
        .log = "常规配水结束",
        .log2 = "关闭氨氮补水",
        .funNoWait = BuShuiAndan_Stop,
    },
};

// 加标配水
const _Flow Flow_PeiShui_JiaBiao[] = {
    {
        .log = "加标配水开始",
        .log2 = "开启氨氮补水",
        .funNoWait = BuShuiAndan_Start,
        .delayCfg = &cfgFlow.peiShuiJiaBiaoTime,
    },
    {
        .log = "加标配水结束",
        .log2 = "关闭氨氮补水",
        .funNoWait = BuShuiAndan_Stop,
    },
};

// 反冲洗
const _Flow Flow_FanChongXi[] = {
    // 水样池排液
    {
        .log = "反冲洗.一二级水样池排液",
        .log2 = "开启V4|V5",
        .on = {V4, V5},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .log = "反冲洗.过滤水样池排液",
        .log2 = "开启V13",
        .on = {V13},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .log = "反冲洗.水样池排液结束",
        .log2 = "关闭V4|V5|V13",
        .off = {V4, V5, V13},
    },

    // 外部管路反冲洗
    {
        .log = "反冲洗.外部管路反冲洗",
        .log2 = "开启V1-G|V8|V9|V12-R",
        .on = {V1_G, V8, V9, V12_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "反冲洗.外部管路反冲洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.guanLuFanChongXiTime,
    },
    {
        .log = "反冲洗.外部管路反冲洗结束",
        .log2 = "关闭M4|V1-G|V8|V9|V12-R",
        .off = {M4, V1_G, V8, V9, V12_R},
    },

    // 内部管路反冲洗
    {
        .log = "反冲洗.内部管路反冲洗",
        .log2 = "开启V6-R|V6-G|V8|V11-R|V14",
        .on = {V6_R, V6_G, V8, V11_R, V14},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "反冲洗.内部管路反冲洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.guanLuFanChongXiTime,
    },
    {
        .log = "反冲洗.内部管路反冲洗结束",
        .log2 = "关闭M4|V8",
        .off = {M4, V8},
    },
    {
        .log = "反冲洗.内部管路进气",
        .log2 = "开启V7",
        .on = {V7},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .log = "反冲洗.内部管路进气结束",
        .log2 = "关闭V6-R|V6-G|V7|V11-R|V14",
        .off = {V6_R, V6_G, V7, V11_R, V14},
    },

    // 过滤装置反冲洗
    {
        .log = "反冲洗.过滤装置反冲洗",
        .log2 = "开启V6-R|V8|V11-R|V12-G",
        .on = {V6_R, V8, V11_R, V12_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "反冲洗.过滤装置反冲洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.guolvFanChongXiTime,
    },
    {
        .log = "反冲洗.过滤装置反冲洗结束",
        .log2 = "关闭M4|V8",
        .off = {M4, V8},
    },
    {
        .log = "反冲洗.过滤装置进气",
        .log2 = "开启V7",
        .on = {V7},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .log = "反冲洗.过滤装置进气结束",
        .log2 = "关闭V6-R|V7|V11-R|V12-G,开启V12-R",
        .on = {V12_R},
        .off = {V6_R, V7, V11_R, V12_G},
    },

    // 一级水样池反冲洗
    {
        .log = "反冲洗.一级水样池反冲洗",
        .log2 = "开启V2|V5|V8",
        .on = {V2, V5, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "反冲洗.一级水样池反冲洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.yiJiFanChongXiTime,
    },
    {
        .log = "反冲洗.一级水样池反冲洗结束",
        .log2 = "关闭M4|V2|V5|V8",
        .off = {M4, V2, V5, V8},
    },

    // 二级水样池反冲洗
    {
        .log = "反冲洗.二级水样池反冲洗",
        .log2 = "开启V3|V4|V8",
        .on = {V3, V4, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "反冲洗.二级水样池反冲洗",
        .log2 = "开启M4",
        .on = {M4},
        .delayCfg = &cfgFlow.erJiFanChongXiTime,
    },
    {
        .log = "反冲洗.二级水样池反冲洗结束",
        .log2 = "关闭M4|V3|V4|V8",
        .off = {M4, V3, V4, V8},
    },

    // 过滤装置进气
    {
        .log = "反冲洗.过滤装置进气",
        .log2 = "开启V6-R|V11-R|V12-G",
        .on = {V6_R, V11_R, V12_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "反冲洗.过滤装置进气",
        .log2 = "开启V7",
        .on = {V7},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .log = "反冲洗.过滤装置进气结束",
        .log2 = "关闭V6-R|V7|V11-R|V12-G",
        .off = {V6_R, V7, V11_R, V12_G},
    },
};

// 一级水样池水样储水
const _Flow Flow_YiJiChuShui_ShuiYang[] = {
    {
        .log = "一级水样池水样储水",
        .log2 = "开启V1-G",
        .on = {V1_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "一级水样池水样储水",
        .log2 = "开启M1",
        .on = {M1},
    },
    {
        .log = "一级水样池水样储水",
        .log2 = "等待一级液位开关或储水时间",
        .funWait = Wait_YiJiYeWei_Or_ChuShuiTime,
    },
    {
        .log = "一级水样池水样储水结束",
        .log2 = "关闭M1|V1-G",
        .off = {M1 | V1_G},
    },
};

// 一级水样池自来水储水
const _Flow Flow_YiJiChuShui_ZiLaiShui[] = {
    {
        .log = "一级水样池自来水储水",
        .log2 = "开启V12-R|V2|V8",
        .on = {V12_R, V2, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "一级水样池自来水储水",
        .log2 = "开启M4",
        .on = {M4},
    },
    {
        .log = "一级水样池自来水储水",
        .log2 = "等待一级液位开关或储水时间",
        .funWait = Wait_YiJiYeWei_Or_ChuShuiTime,
    },
    {
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

    return res;
}

int Flows_YiJiJinYang_Run(void)
{
    int res = 0;
    for (int i = 0; i < sizeof(Flow_YiJiJinYang) / sizeof(_Flow); i++)
    {
        if ((res = Flow_Run(&Flow_YiJiJinYang[i], true)) != IS_OVER)
            break;
    }

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

    return res;
}
