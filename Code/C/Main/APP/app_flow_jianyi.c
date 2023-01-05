#include "bsp_include.h"

// ��ͨ�򷧶���
#define V1 32
#define V2 33
#define V4 34
#define V5 35
#define V12 36
#define V15 37
#define V16 38

// ��ŷ�
#define V6 48
#define V7 49
#define V8 50
#define V9 51
#define V10 52
#define V11 53

// ��ͨ�򷧶���
#define V3_G 40
#define V3_R 41
#define V13_G 42
#define V13_R 43
#define V14_G 44
#define V14_R 45

#define M_CHUZAO 80
#define M1 81
#define M2 82
#define M3 83 // ��ѹ�õ�Դ
#define M4 84 // ��ˮ�õ�Դ

#define SHUILIU 116     // ˮ��
#define YIJI_YEWEI 117  // һ��Һλ
#define ERJI_YEWEI 118  // ����Һλ
#define SANJI_YEWEI 119 // ����Һλ

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

// ������ˮ����
static const _Flow Flow_AndanBuShui[] = {
    {
        .log = "������ˮ",
        .log2 = "����V12|V13-G|V14-R",
        .on = {V12, V13_G, V14_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .log = "������ˮ",
        .log2 = "����M4",
        .on = {M4},
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
        .log2 = "�ر�M4|V12|V13-G|V14-R",
        .off = {M4, V12, V13_G, V14_R},
    },
};

int Flows_AndanBuShui_Run_JianYi(void)
{
    for (int i = 0; i < sizeof(Flow_AndanBuShui) / sizeof(_Flow); i++)
    {
        Flow_Run(&Flow_AndanBuShui[i], false); // ��ˮ��������������,ֹͣ�ź�ֻ�ǻ�ȡ���ȴ�
    }
    return 0;
}

// �����ʼ��
static const _Flow Flow_ChuShiHua[] = {
    {
        .index = FlowIndex_Init,
        .log = "�����ʼ��",
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

// �ⲿ��·��ˮ����ϴ
static const _Flow Flow_ShuiXiangRunXi[] = {
    // �ⲿ��·����ͨ����ϴ
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "�ⲿ��·����ͨ����ϴ",
        .log2 = "����V1����V2|V3-G|V4",
        .on = {V3_G, V4},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "�ⲿ��·����ͨ����ϴ",
        .log2 = "����Ǳˮ��",
        .funNoWait = M1M2CtrlOn,
        .delayCfg = &cfgFlow.runxi1Time,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "�ⲿ��·����ͨ����ϴ",
        .log2 = "�ر�Ǳˮ��|V3-G",
        .off = {M1, M2, V3_G},
    },
    // �ⲿ��·���������ϴ
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "�ⲿ��·���������ϴ",
        .log2 = "����V5",
        .on = {V5},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "�ⲿ��·���������ϴ",
        .log2 = "����V1����V2|V3-R,�ر�V4",
        .on = {V3_R},
        .off = {V4},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "�ⲿ��·���������ϴ",
        .log2 = "����Ǳˮ��",
        .funNoWait = M1M2CtrlOn,
        .delayCfg = &cfgFlow.runxi2Time,
    },
    {
        .index = FlowIndex_ShuiXiangRunXi,
        .log = "�ⲿ��·���������ϴ",
        .log2 = "�ر�Ǳˮ��|V3-R|V5",
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

// ��ͨ�ؽ���
static const _Flow Flow_LiuTongChiJinYang[] = {
    // ��ͨ�ؽ���
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "��ͨ�ؽ���",
        .log2 = "����V1����V2|V3-G",
        .on = {V3_G},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "��ͨ�ؽ���",
        .log2 = "����Ǳˮ��",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "��ͨ�ؽ���",
        .log2 = "�ȴ�ˮ������",
        .funWait = Wait_Shuiliu,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "��ͨ�ؽ���",
        .log2 = "[��Ӧ��]ˮ������",
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "��ͨ�ؽ���",
        .log2 = "�ȴ�һ��Һλ����",
        .funWait = Wait_YiJiYeWei,
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "��ͨ�ؽ���",
        .log2 = "[��Ӧ��]Һλ����",
    },
    {
        .index = FlowIndex_YiJiJinYang,
        .log = "��ͨ�ؽ�������",
        .log2 = "�ر�Ǳˮ��|V1|V2|V3-G",
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

    if (res == IS_OVERTIME && (strstr(Flow_LiuTongChiJinYang[i].log2, "ˮ��") != NULL))
    {
        if (M1M2Change() == 0)
            goto RETRY_LIUTONGCHI;
    }

    AllCtrl(0);

    return res;
}

// ����ؽ���
static const _Flow Flow_ChenDianChiJinYang[] = {
    // ����ؽ���
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ؽ���",
        .log2 = "����V1����V2|V3-R",
        .on = {V3_R},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ؽ���",
        .log2 = "����Ǳˮ��",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ؽ���",
        .log2 = "�ȴ�ˮ������",
        .funWait = Wait_Shuiliu,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ؽ���",
        .log2 = "[��Ӧ��]ˮ������",
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ؽ���",
        .log2 = "�ȴ�����Һλ����",
        .funWait = Wait_ErJiYeWei,
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ؽ���",
        .log2 = "[��Ӧ��]Һλ����",
    },
    {
        .index = FlowIndex_ErJiJinYang,
        .log = "����ؽ�������",
        .log2 = "�ر�Ǳˮ��|V1|V2|V3-R",
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

    if (res == IS_OVERTIME && (strstr(Flow_ChenDianChiJinYang[i].log2, "ˮ��") != NULL))
    {
        if (M1M2Change() == 0)
            goto RETRY_CHENDIANCHI;
    }

    AllCtrl(0);

    return res;
}

// ˮ������
static const _Flow Flow_ChenDian[] = {
    {
        .index = FlowIndex_ChenDia,
        .log = "ˮ������",
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

// ����ˮ������
static const _Flow Flow_ShuiYangGuoLv[] = {
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����װ����Һ",
        .log2 = "����V15",
        .on = {V15},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����װ����ϴ",
        .log2 = "����V12|V13-R|V14-R",
        .on = {V12, V13_R, V14_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����װ����ϴ",
        .log2 = "����M4,��������",
        .on = {M4},
        .funNoWait = Baohu_Start,
        .delayCfg = &cfgFlow.runxiGuoLvTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����װ����ϴ����",
        .log2 = "�ر�M4|V13-R",
        .off = {M4, V13_R},
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����ˮ������ϴ",
        .log2 = "����V13-G",
        .on = {V13_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����ˮ������ϴ",
        .log2 = "����M4",
        .on = {M4},
        .delayCfg = &cfgFlow.runxiAnDanTime,
    },
    {
        .index = FlowIndex_ShuiYangGuoLv,
        .log = "ˮ������.����ˮ������ϴ����",
        .log2 = "�ر�V15,�رձ���",
        .off = {V15},
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
        .log2 = "�ر�M4|V12|V13-G",
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

    // �ڼ��п�������,��Ҫ�ر�
    Baohu_Stop();

    AllCtrl(0);

    return res;
}

// ������ˮ
static const _Flow Flow_PeiShui_ChangGui[] = {
    {
        .index = FlowIndex_PeiShui,
        .log = "������ˮ��ʼ",
        .log2 = "����������ˮ",
        .funNoWait = BuShuiAndan_Start_JianYi,
        .delayCfg = &cfgFlow.peiShuiChangGuiTime,
    },
    {
        .index = FlowIndex_PeiShui,
        .log = "������ˮ����",
        .log2 = "�رհ�����ˮ",
        .funNoWait = BuShuiAndan_Stop_JianYi,
    },
};

// �ӱ���ˮ
static const _Flow Flow_PeiShui_JiaBiao[] = {
    {
        .index = FlowIndex_PeiShui,
        .log = "�ӱ���ˮ��ʼ",
        .log2 = "����������ˮ",
        .funNoWait = BuShuiAndan_Start_JianYi,
        .delayCfg = &cfgFlow.peiShuiJiaBiaoTime,
    },
    {
        .index = FlowIndex_PeiShui,
        .log = "�ӱ���ˮ����",
        .log2 = "�رհ�����ˮ",
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

    // �ڼ��п�����ˮ,��Ҫ�ر�
    BuShuiAndan_Stop_JianYi();

    AllCtrl(0);

    return res;
}

// ����ϴ
static const _Flow Flow_FanChongXi[] = {
    // ��ͨ����Һ��
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.��ͨ����Һ",
        .log2 = "����V4|V5",
        .on = {V4, V5},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.��ͨ����Һ",
        .log2 = "����V15",
        .on = {V15},
        .delayCfg = &cfgFlow.paiyeTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.��ͨ����Һ����",
        .log2 = "�ر�V4|V5|V15",
        .off = {V4, V5, V15},
    },

    // �ⲿ��·����ϴ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����",
        .log2 = "����V1|�����",
        .on = {V1, M_CHUZAO},
        .delayCfg = &cfgFlow.chuZaoTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�������",
        .log2 = "�رճ����",
        .off = {M_CHUZAO},
    },

    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ⲿ��·����ϴ",
        .log2 = "����V7|V10|V14-R",
        .on = {V7, V10, V14_R},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ⲿ��·����ϴ",
        .log2 = "����M3",
        .on = {M3},
        .delayCfg = &cfgFlow.guanLuFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ⲿ��·����ϴ����",
        .log2 = "�ر�M3|V1|V7|V10|V14-R",
        .off = {M3, V1, V7, V10, V14_R},
    },

    // �ڲ���·����ϴ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ڲ���·����ϴ",
        .log2 = "����V7|V16",
        .on = {V7, V16},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ڲ���·����ϴ",
        .log2 = "����M3",
        .on = {M3},
        .delayCfg = &cfgFlow.guanLuFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ڲ���·����ϴ����",
        .log2 = "�ر�M3|V7",
        .off = {M3, V7},
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ڲ���·����",
        .log2 = "����V6",
        .on = {V6},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.�ڲ���·��������",
        .log2 = "�ر�V6|V16",
        .off = {V6, V16},
    },

    // ����װ�÷���ϴ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�÷���ϴ",
        .log2 = "����V7|V13-R|V14-G",
        .on = {V7, V13_R, V14_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�÷���ϴ",
        .log2 = "����M3",
        .on = {M3},
        .delayCfg = &cfgFlow.guolvFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�÷���ϴ����",
        .log2 = "�ر�M3|V7",
        .off = {M3, V7},
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�ý���",
        .log2 = "����V6",
        .on = {V6},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�ý�������",
        .log2 = "�ر�V6|V13-R|V14-G,����V14-R",
        .on = {V14_R},
        .off = {V6, V13_R, V14_G},
    },

    // ��ͨ�سط���ϴ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.��ͨ�سط���ϴ",
        .log2 = "����V4|V7|V8",
        .on = {V4, V7, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.��ͨ�سط���ϴ",
        .log2 = "����M3",
        .on = {M3},
        .delayCfg = &cfgFlow.yiJiFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.��ͨ�سط���ϴ����",
        .log2 = "�ر�M3|V4|V7|V8",
        .off = {M3, V4, V7, V8},
    },

    // ����ط���ϴ
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����ط���ϴ",
        .log2 = "����V5|V7|V9",
        .on = {V5, V7, V9},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����ط���ϴ",
        .log2 = "����M3",
        .on = {M3},
        .delayCfg = &cfgFlow.erJiFanChongXiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����ط���ϴ����",
        .log2 = "�ر�M3|V5|V7|V9",
        .off = {M3, V5, V7, V9},
    },

    // ����װ�ý���
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�ý���",
        .log2 = "����V13-R|V14-G",
        .on = {V13_R, V14_G},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�ý���",
        .log2 = "����V6",
        .on = {V6},
        .delayCfg = &cfgFlow.jinQiTime,
    },
    {
        .index = FlowIndex_FanChongXi,
        .log = "����ϴ.����װ�ý�������",
        .log2 = "�ر�V6|V13-R|V14-G",
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

// ��ͨ��ˮ����ˮ
static const _Flow Flow_YiJiChuShui_ShuiYang[] = {
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "��ͨ��ˮ����ˮ",
        .log2 = "����V1��V2|V3-G",
        .on = {V3_G},
        .funNoWait = ValveCtrlForM1M2,
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "��ͨ��ˮ����ˮ",
        .log2 = "����Ǳˮ��",
        .funNoWait = M1M2CtrlOn,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "��ͨ��ˮ����ˮ",
        .log2 = "�ȴ�һ��Һλ���ػ�ˮʱ��",
        .funWait = Wait_YiJiYeWei_Or_ChuShuiTime,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "��ͨ��ˮ����ˮ",
        .log2 = "[��Ӧ��]Һλ���ػ�[����]ʱ��",
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "��ͨ��ˮ����ˮ����",
        .log2 = "�ر�Ǳˮ��|V1|V2|V3-G",
        .off = {M1, M2, V1, V2, V3_G},
    },
};

// ��ͨ������ˮ��ˮ
static const _Flow Flow_YiJiChuShui_ZiLaiShui[] = {
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "��ͨ������ˮ��ˮ",
        .log2 = "����V14-R|V7|V8",
        .on = {V14_R, V7, V8},
        .delay = VALVE_SWITCH_TIME,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "��ͨ������ˮ��ˮ",
        .log2 = "����M3",
        .on = {M3},
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "��ͨ������ˮ��ˮ",
        .log2 = "�ȴ�һ��Һλ���ػ�ˮʱ��",
        .funWait = Wait_YiJiYeWei_Or_ChuShuiTime,
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "��ͨ������ˮ��ˮ",
        .log2 = "[��Ӧ��]Һλ���ػ�[����]ʱ��",
    },
    {
        .index = FlowIndex_YiJiChuShui,
        .log = "��ͨ������ˮ��ˮ����",
        .log2 = "�ر�M3|V7|V8|V14-R",
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
