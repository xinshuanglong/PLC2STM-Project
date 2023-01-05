#include "bsp_include.h"

// ��ͨ�򷧶���
#define V2 32
#define V3 33
#define V4 34
#define V5 35
#define V8 36
#define V9 37
#define V10 38
#define V13 39

// ��ŷ�
#define V7 48
#define V14 49

// ��ͨ�򷧶���
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
#define M3 84 // ��ˮ�õ�Դ
#define M4 83 // ��ѹ�õ�Դ

static const int allCtrl[] = {M1, M2, M3, M4, V1_G, V1_R, V2, V3, V4, V5, V6_G, V6_R, V7, V8, V9, V10, V11_G, V11_R, V12_G, V12_R, V13, V14};

#define SHUILIU 116     // ˮ��
#define YIJI_YEWEI 117  // һ��Һλ
#define ERJI_YEWEI 118  // ����Һλ
#define SANJI_YEWEI 119 // ����Һλ

const ConfigFlow cfgFlow _AT_CON_SEC = {
    .runxi1Time = 3,     // Ĭ��30��
    .runxi2Time = 3,     // Ĭ��30��
    .runxiGuoLvTime = 3, // Ĭ��10��
    .runxiAnDanTime = 3, // Ĭ��10��
    .paiyeTime = 3,      // Ĭ��30��
    .chushuiTime = 3,
    .guanLuFanChongXiTime = 3, // Ĭ��120��
    .guolvFanChongXiTime = 3,  // Ĭ��180��
    .yiJiFanChongXiTime = 3,   // Ĭ��60��
    .erJiFanChongXiTime = 3,   // Ĭ��60��
    .jinQiTime = 3,            // Ĭ��30��
    .chenDianTime = 3,         // Ĭ��1800��
    .chuZaoTime = 3,
    .peiShuiChangGuiTime = 3, // Ĭ��1800��
    .peiShuiJiaBiaoTime = 3,  // Ĭ��5400��

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

// M1ʧЧ�л�M2,M2ʧЧ�л�M1
//  �ɹ��л�����0,���򷵻�1
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

// �ȴ�����ʱ��
void Wait_HuanChong(EventGroupHandle_t *xEventGroup, uint32_t breakBit)
{
    xEventGroupWaitBits(*xEventGroup, breakBit, pdFALSE, pdFALSE, HUANCHONG_TIME * 1000);
}

// һ��Һλ���,�������߳�
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

// ����Һλ���,�������߳�
int Wait_ErJiYeWei(void)
{
    int res = 0;
    res = WaitIO(ERJI_YEWEI, true, cfgFlow.yeWeiOverTime, &xMeasureEventGroup, STOP_EV_BIT);
    if (res == IS_OVERTIME)
        Alarm_Add(0, erji_yewei_alarm, 0);

    return res;
}

// ����Һλ���,�������߳�
int Wait_SanJiYeWei(void)
{
    int res = 0;
    res = WaitIO(SANJI_YEWEI, true, cfgFlow.jinYangOverTime, &xMeasureEventGroup, STOP_EV_BIT);
    if (res == IS_OVERTIME)
        Alarm_Add(0, sanji_yewei_alarm, 0);

    return res;
}

// ��ˮһ��Һλ���,�������߳�
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

// ��ˮ����Һλ���,�������߳�
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

// ˮ�����ؼ��,�������߳�
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

// ˮ���ز�ˮ����
static const _Flow Flow_ShuiXiangBuShui[] = {
    {
        .log = "һ��ˮ���ز�ˮ",
        .log2 = "����V1-G��V1-R",
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "һ��ˮ���ز�ˮ",
        .log2 = "����Ǳˮ��",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .log = "һ��ˮ���ز�ˮ",
        .log2 = "�ȴ�һ��Һλ����",
        .funWait = Wait_YiJiYeWeiForBushui,
    },
    {
        .log = "һ��ˮ���ز�ˮ",
        .log2 = "[��Ӧ��]Һλ����",
    },
    // ��ˮ���
    {
        .log = "һ��ˮ���ز�ˮ���",
        .log2 = "�ر�Ǳˮ��|V1-G|V1-R",
        .off = {M1, M2, V1_G, V1_R},
    },
};

// ������ˮ����
static const _Flow Flow_AndanBuShui[] = {
    {
        .log = "������ˮ",
        .log2 = "����V6-G|V11-G|V12-R",
        .on = {V6_G, V11_G, V12_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "������ˮ",
        .log2 = "����M3",
        .on = {M3},
    },
    {
        .log = "������ˮ",
        .log2 = "�ȴ�����ˮ����Һλ����",
        .funWait = Wait_AndanForBushui,
    },
    {
        .log = "������ˮ",
        .log2 = "[��Ӧ��]Һλ����",
    },
    // ��ˮ���
    {
        .log = "������ˮ���",
        .log2 = "�ر�M3|V6-G|V11-G|V12-R",
        .off = {M3, V6_G, V11_G, V12_R},
    },
};

// �����ʼ��
static const _Flow Flow_ChuShiHua[] = {
    {
        .index = FlowIndex_Init,
        .log = "�����ʼ��",
        .log2 = "����V4|V5",
        .on = {V4, V5},
        .funNoWait = Flow_Init,
        .delayCfg = &cfgFlow.paiyeTime,
    },
};

// ˮ������ϴ����
static const _Flow Flow_ShuiXiangRunXi[] = {
    // һ��ˮ������ϴ
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "һ��ˮ������ϴ",
        .log2 = "����V1-G��V1-R|V5",
        .on = {V5},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "һ��ˮ������ϴ",
        .log2 = "����Ǳˮ��",
        .funNoWait = M1M2CtrlOn,
        .delayCfg = &cfgFlow.runxi1Time,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "һ��ˮ������ϴ",
        .log2 = "�ر�V5",
        .off = {V5},
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "һ��ˮ������ϴ",
        .log2 = "�ȴ�һ��Һλ����",
        .funWait = Wait_YiJiYeWei,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "һ��ˮ������ϴ",
        .log2 = "[��Ӧ��]Һλ����",
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "һ��ˮ������ϴ",
        .log2 = "�ر�Ǳˮ��|V1-G|V1-R",
        .off = {M1, M2, V1_G, V1_R},
    },
    // ����ˮ������ϴ
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "����ˮ������ϴ",
        .log2 = "����V4|V6-R",
        .on = {V4, V6_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "����ˮ������ϴ",
        .log2 = "����M3,������ˮ",
        .on = {M3},
        .funNoWait = ShuiXiangBuShui_Start,
        .delayCfg = &cfgFlow.runxi2Time,
    },
    // ��ϴʱ�䵽֮��
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "����ˮ������ϴ",
        .log2 = "�ر�M3|V6-R,����V4|V5,�رղ�ˮ",
        .on = {V4, V5},
        .off = {M3, V6_R},
        .funNoWait = ShuiXiangBuShui_Stop,
        .delayCfg = &cfgFlow.paiyeTime,
    },
    // ��Һʱ�䵽֮��
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "����ˮ������ϴ",
        .log2 = "�ر�V4|V5",
        .off = {V4, V5},
    },
};

// һ��ˮ���ؽ���
static const _Flow Flow_YiJiJinYang[] = {
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "һ��ˮ���ؽ���",
        .log2 = "����V1-G��V1-R",
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "һ��ˮ���ؽ���",
        .log2 = "����Ǳˮ��",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "һ��ˮ���ؽ���",
        .log2 = "�ȴ�ˮ������",
        .funWait = Wait_Shuiliu,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "һ��ˮ���ؽ���",
        .log2 = "[��Ӧ��]ˮ������",
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "һ��ˮ���ؽ���",
        .log2 = "�ȴ�һ��Һλ����",
        .funWait = Wait_YiJiYeWei,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "һ��ˮ���ؽ���",
        .log2 = "[��Ӧ��]Һλ����",
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "һ��ˮ���ؽ�������",
        .log2 = "�ر�Ǳˮ��|V1-G|V1-R",
        .off = {M1, M2, V1_G, V1_R},
    },
};

// ����ˮ���ؽ���
static const _Flow Flow_ErJiJinYang[] = {
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ˮ���ؽ���",
        .log2 = "����V6_R",
        .on = {V6_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ˮ���ؽ���",
        .log2 = "����M3,������ˮ",
        .on = {M3},
        .funNoWait = ShuiXiangBuShui_Start,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ˮ���ؽ���",
        .log2 = "�ȴ�����Һλ����",
        .funWait = Wait_ErJiYeWei,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ˮ���ؽ���",
        .log2 = "[��Ӧ��]Һλ����",
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ˮ���ؽ�������",
        .log2 = "�ر�M3|V6-R,�رղ�ˮ",
        .off = {M3, V6_R},
        .funNoWait = ShuiXiangBuShui_Stop,
    },
};

// ˮ������
static const _Flow Flow_ChenDian[] = {
    {
        .index = FlowIndex_ChenDia,
        .log = "ˮ������",
        .log2 = "",
        .delayCfg = &cfgFlow.chenDianTime,
    },
};

// ����ˮ������
static const _Flow Flow_ShuiYangGuoLv[] = {
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����װ����Һ",
        .log2 = "����V13",
        .on = {V13},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����װ����ϴ",
        .log2 = "����V6-G|V11-R|V12-R",
        .on = {V6_G, V11_R, V12_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����װ����ϴ",
        .log2 = "����M3,��������",
        .on = {M3},
        .funNoWait = Baohu_Start,
        .delayCfg = &cfgFlow.runxiGuoLvTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����װ����ϴ����",
        .log2 = "�ر�M3|V11-R",
        .off = {M3, V11_R},
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����ˮ������ϴ",
        .log2 = "����V11-G",
        .on = {V11_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����ˮ������ϴ",
        .log2 = "����M3",
        .on = {M3},
        .delayCfg = &cfgFlow.runxiAnDanTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����ˮ������ϴ����",
        .log2 = "�ر�V13,�رձ���",
        .off = {V13},
        .funNoWait = Baohu_Stop,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����ˮ���ع��˽���",
        .log2 = "�ȴ�����ˮ����Һλ����",
        .funWait = Wait_SanJiYeWei,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����ˮ���ع��˽���",
        .log2 = "[��Ӧ��]Һλ����",
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����ˮ���ع��˽�������",
        .log2 = "�ر�M3|V6-G|V11-G",
        .off = {M3, V6_G, V11_G},
    },
};

// ������ˮ
static const _Flow Flow_PeiShui_ChangGui[] = {
    {
        .index = FlowIndex_PeiShui,
        .log = "������ˮ��ʼ",
        .log2 = "����������ˮ",
        .funNoWait = BuShuiAndan_Start,
        .delayCfg = &cfgFlow.peiShuiChangGuiTime,
    },
    {
        .index = FlowIndex_PeiShui,
        .log = "������ˮ����",
        .log2 = "�رհ�����ˮ",
        .funNoWait = BuShuiAndan_Stop,
    },
};

// �ӱ���ˮ
static const _Flow Flow_PeiShui_JiaBiao[] = {
    {
        .index = FlowIndex_PeiShui,
        .log = "�ӱ���ˮ��ʼ",
        .log2 = "����������ˮ",
        .funNoWait = BuShuiAndan_Start,
        .delayCfg = &cfgFlow.peiShuiJiaBiaoTime,
    },
    {
        .index = FlowIndex_PeiShui,
        .log = "�ӱ���ˮ����",
        .log2 = "�رհ�����ˮ",
        .funNoWait = BuShuiAndan_Stop,
    },
};

// ����ϴ
static const _Flow Flow_FanChongXi[] = {
    // ˮ������Һ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.һ����ˮ������Һ",
        .log2 = "����V4|V5",
        .on = {V4, V5},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����ˮ������Һ",
        .log2 = "����V13",
        .on = {V13},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.ˮ������Һ����",
        .log2 = "�ر�V4|V5|V13",
        .off = {V4, V5, V13},
    },

    // �ⲿ��·����ϴ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ⲿ��·����ϴ",
        .log2 = "����V1-G|V8|V9|V12-R",
        .on = {V1_G, V8, V9, V12_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ⲿ��·����ϴ",
        .log2 = "����M4",
        .on = {M4},
        .delayCfg = &cfgFlow.guanLuFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ⲿ��·����ϴ����",
        .log2 = "�ر�M4|V1-G|V8|V9|V12-R",
        .off = {M4, V1_G, V8, V9, V12_R},
    },

    // �ڲ���·����ϴ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ڲ���·����ϴ",
        .log2 = "����V6-R|V6-G|V8|V11-R|V14",
        .on = {V6_R, V6_G, V8, V11_R, V14},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ڲ���·����ϴ",
        .log2 = "����M4",
        .on = {M4},
        .delayCfg = &cfgFlow.guanLuFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ڲ���·����ϴ����",
        .log2 = "�ر�M4|V8",
        .off = {M4, V8},
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ڲ���·����",
        .log2 = "����V7",
        .on = {V7},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ڲ���·��������",
        .log2 = "�ر�V6-R|V6-G|V7|V11-R|V14",
        .off = {V6_R, V6_G, V7, V11_R, V14},
    },

    // ����װ�÷���ϴ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�÷���ϴ",
        .log2 = "����V6-R|V8|V11-R|V12-G",
        .on = {V6_R, V8, V11_R, V12_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�÷���ϴ",
        .log2 = "����M4",
        .on = {M4},
        .delayCfg = &cfgFlow.guolvFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�÷���ϴ����",
        .log2 = "�ر�M4|V8",
        .off = {M4, V8},
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�ý���",
        .log2 = "����V7",
        .on = {V7},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�ý�������",
        .log2 = "�ر�V6-R|V7|V11-R|V12-G,����V12-R",
        .on = {V12_R},
        .off = {V6_R, V7, V11_R, V12_G},
    },

    // һ��ˮ���ط���ϴ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.һ��ˮ���ط���ϴ",
        .log2 = "����V2|V5|V8",
        .on = {V2, V5, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.һ��ˮ���ط���ϴ",
        .log2 = "����M4",
        .on = {M4},
        .delayCfg = &cfgFlow.yiJiFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.һ��ˮ���ط���ϴ����",
        .log2 = "�ر�M4|V2|V5|V8",
        .off = {M4, V2, V5, V8},
    },

    // ����ˮ���ط���ϴ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����ˮ���ط���ϴ",
        .log2 = "����V3|V4|V8",
        .on = {V3, V4, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����ˮ���ط���ϴ",
        .log2 = "����M4",
        .on = {M4},
        .delayCfg = &cfgFlow.erJiFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����ˮ���ط���ϴ����",
        .log2 = "�ر�M4|V3|V4|V8",
        .off = {M4, V3, V4, V8},
    },

    // ����װ�ý���
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�ý���",
        .log2 = "����V6-R|V11-R|V12-G",
        .on = {V6_R, V11_R, V12_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�ý���",
        .log2 = "����V7",
        .on = {V7},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�ý�������",
        .log2 = "�ر�V6-R|V7|V11-R|V12-G",
        .off = {V6_R, V7, V11_R, V12_G},
    },
};

// һ��ˮ����ˮ����ˮ
static const _Flow Flow_YiJiChuShui_ShuiYang[] = {
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "һ��ˮ����ˮ����ˮ",
        .log2 = "����V1-G��V1-R",
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "һ��ˮ����ˮ����ˮ",
        .log2 = "����Ǳˮ��",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "һ��ˮ����ˮ����ˮ",
        .log2 = "�ȴ�һ��Һλ���ػ�ˮʱ��",
        .funWait = Wait_YiJiYeWei_Or_ChuShuiTime,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "һ��ˮ����ˮ����ˮ",
        .log2 = "[��Ӧ��]Һλ���ػ�[����]ʱ��",
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "һ��ˮ����ˮ����ˮ����",
        .log2 = "�ر�Ǳˮ��|V1-G|V1-R",
        .off = {M1, M2, V1_G, V1_R},
    },
};

// һ��ˮ��������ˮ��ˮ
static const _Flow Flow_YiJiChuShui_ZiLaiShui[] = {
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "һ��ˮ��������ˮ��ˮ",
        .log2 = "����V12-R|V2|V8",
        .on = {V12_R, V2, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "һ��ˮ��������ˮ��ˮ",
        .log2 = "����M4",
        .on = {M4},
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "һ��ˮ��������ˮ��ˮ",
        .log2 = "�ȴ�һ��Һλ���ػ�ˮʱ��",
        .funWait = Wait_YiJiYeWei_Or_ChuShuiTime,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "һ��ˮ��������ˮ��ˮ",
        .log2 = "[��Ӧ��]Һλ���ػ�[����]ʱ��",
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "һ��ˮ��������ˮ��ˮ����",
        .log2 = "�ر�M4|V12-R|V2|V8",
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
            LogDatabase_Insert("%s[��ʱ]", msg);
        if (res == IS_ERROR)
            LogDatabase_Insert("%s[�쳣]", msg);
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

// ˮ�䲹ˮ���
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
        Flow_Run(&Flow_ShuiXiangBuShui[i], false); // ��ˮ��������������,ֹͣ�ź�ֻ�ǻ�ȡ���ȴ�
    }
    return 0;
}

// ����װ�ñ���
void Baohu(void)
{
    if (Port_Read_Filter(SANJI_YEWEI) == true)
    {
        Port_Write(M3, 0);
    }
}

// ������ˮ���
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
        Flow_Run(&Flow_AndanBuShui[i], false); // ��ˮ��������������,ֹͣ�ź�ֻ�ǻ�ȡ���ȴ�
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

    // �ڼ��п�����ˮ,��Ҫ�ر�
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

    if (res == IS_OVERTIME && (strstr(Flow_YiJiJinYang[i].log2, "ˮ��") != NULL))
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

    // �ڼ��п�����ˮ,��Ҫ�ر�
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

    // �ڼ��п�������,��Ҫ�ر�
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

    // �ڼ��п�����ˮ,��Ҫ�ر�
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
