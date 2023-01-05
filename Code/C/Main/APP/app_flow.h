#ifndef _APP_FLOW_H
#define _APP_FLOW_H

#define SWITCH_COUNT 16

#define VALVE_SWITCH_TIME 5 // ���л�ʱ��,��λ��
#define HUANCHONG_TIME 5    // ����Һλ����Ƶ������,��ӻ���ʱ��,��λ��

enum RunType
{
    IS_OK = 0,              // ����
    IS_OVER = 0,            // �������
    IS_STOP = (1 << 0),     // �ֶ�ֹͣ
    IS_OVERTIME = (1 << 1), // ��ʱ
    IS_ERROR = (1 << 2),    // ���ִ���ֹͣ
};

enum FlowIndex
{
    FlowIndex_NULL = -1,
    FlowIndex_IDLE = 0,
    FlowIndex_Init = 1,           // ��ʼ��
    FlowIndex_ShuiXiangRunXi = 2, // ˮ����ϴ
    FlowIndex_YiJiJinYang = 3,    // һ��ˮ���ؽ���,��ͨ�ؽ���
    FlowIndex_ErJiJinYang = 4,    // ����ˮ���ؽ���,����ؽ���
    FlowIndex_ChenDia = 5,        // ����
    FlowIndex_ShuiYangGuoLv = 6,  // ����ˮ������
    FlowIndex_PeiShui = 7,        // һ��վ
    FlowIndex_FanChongXi = 8,     // һ��վ
    FlowIndex_YiJiChuShui = 9,    // һ��վ
};

enum ChuiShuiType
{
    CHUISHUITYPE_SHUIYANG = 0,
    CHUISHUITYPE_ZILAISHUI,
};

enum M1M2Type
{
    M1M2Type_M1 = 0,
    M1M2Type_M2,
    M1M2Type_M1M2,
};

typedef struct
{
    uint32_t runxi1Time;           // һ��ˮ������ϴʱ��/��ͨ����ϴʱ��
    uint32_t runxi2Time;           // ������ˮ������ϴʱ��/�������ϴʱ��
    uint32_t runxiGuoLvTime;       // ���˳���ϴʱ��
    uint32_t runxiAnDanTime;       // ����ˮ������ϴʱ��
    uint32_t paiyeTime;            // ��Һʱ��
    uint32_t chushuiTime;          // ��ˮʱ��
    uint32_t guanLuFanChongXiTime; // ��·����ϴʱ��
    uint32_t guolvFanChongXiTime;  // ����װ�÷���ϴʱ��
    uint32_t yiJiFanChongXiTime;   // һ��ˮ���ط���ϴʱ��/��ͨ�ط���ϴʱ��
    uint32_t erJiFanChongXiTime;   // ����ˮ���ط���ϴʱ��/����ط���ϴʱ��
    uint32_t jinQiTime;            // ����ʱ��
    uint32_t chenDianTime;         // ����ʱ��
    uint32_t chuZaoTime;           // ����ʱ��
    uint32_t peiShuiChangGuiTime;  // ������ˮʱ��
    uint32_t peiShuiJiaBiaoTime;   // �ӱ���ˮʱ��

    uint16_t m1m2Type;
    uint16_t chuishuiType; // ȡֵ ChuiShuiType
    uint16_t haveGuoLv;    // �Ƿ��а�������װ�� 0:û�� 1:��

    uint32_t yeWeiOverTime;   // һ����Һλ��ʱʱ��
    uint32_t shuiLiuOverTime; // ˮ����ʱʱ��
    uint32_t jinYangOverTime; // ���˽�����ʱʱ��
} ConfigFlow;

typedef struct
{
    int index;             // ���
    char log[48];          // ����
    char log2[48];         // ����2
    int on[SWITCH_COUNT];  // ���ź�
    int off[SWITCH_COUNT]; // ���ź�
    int (*funNoWait)(void);
    int (*funWait)(void); // �������߳�

    int delay; // delay���ִ��
    const uint32_t *delayCfg;
} _Flow;

extern int gM1M2Now;
extern const ConfigFlow cfgFlow;

// ��������
int Flow_Init(void);
int M1M2Change(void);

int Wait_AndanForBushui(void);
int Wait_Shuiliu(void);
int Wait_YiJiYeWei(void);
int Wait_ErJiYeWei(void);
int Wait_SanJiYeWei(void);

int Wait_YiJiYeWei_Or_ChuShuiTime(void);

int Flow_Run(const _Flow *flow, bool isMaster);

bool ShuiXiangNeedBuShui(void);
int Flows_ShuiXiangBuShui_Run(void);
void Baohu(void);
bool AndanNeedBuShui(void);
int Flows_AndanBuShui_Run(void);

// һ��վ
int Flows_ChuShiHua_Run(void);
int Flows_ShuiXiangRunXi_Run(void);
int Flows_YiJiJinYang_Run(void);
int Flows_ErJiJinYang_Run(void);
int Flows_ChenDian_Run(void);
int Flows_ShuiYangGuoLv_Run(void);
int Flows_PeiShui_Run(uint8_t isJiaBiao);
int Flows_FanChongXi_Run(void);
int Flows_YiJiChuShui_Run(void);

// ����վ
int Flows_AndanBuShui_Run_JianYi(void);
int Flows_ChuShiHua_Run_JianYi(void);
int Flows_ShuiXiangRunXi_Run_JianYi(void);
int Flows_LiuTongChiJinYang_Run_JianYi(void);
int Flows_ChenDianChiJinYang_Run_JianYi(void);
int Flows_ChenDian_Run_JianYi(void);
int Flows_ShuiYangGuoLv_Run_JianYi(void);
int Flows_PeiShui_Run_JianYi(uint8_t isJiaBiao);
int Flows_FanChongXi_Run_JianYi(void);
int Flows_YiJiChuShui_Run_JianYi(void);

#endif
