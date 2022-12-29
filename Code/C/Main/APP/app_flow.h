#ifndef _APP_FLOW_H
#define _APP_FLOW_H

#define SWITCH_COUNT 16

enum RunType
{
    IS_OK = 0,              // ����
    IS_OVER = 0,            // �������
    IS_STOP = (1 << 0),     // �ֶ�ֹͣ
    IS_OVERTIME = (1 << 1), // ��ʱ
    IS_ERROR = (1 << 2),    // ���ִ���ֹͣ
};

enum ChuiShuiType
{
    CHUISHUITYPE_SHUIYANG = 0,
    CHUISHUITYPE_ZILAISHUI
};

typedef struct
{
    uint32_t runxi1Time;           // һ��ˮ������ϴʱ��
    uint32_t runxi2Time;           // ������ˮ������ϴʱ��
    uint32_t runxiGuoLvTime;       // ���˳���ϴʱ��
    uint32_t runxiAnDanTime;       // ����ˮ������ϴʱ��
    uint32_t paiyeTime;            // ��Һʱ��
    uint32_t chushuiTime;          // ��ˮʱ��
    uint32_t guanLuFanChongXiTime; // ��·����ϴʱ��
    uint32_t guolvFanChongXiTime;  // ����װ�÷���ϴʱ��
    uint32_t yiJiFanChongXiTime;   // һ��ˮ���ط���ϴʱ��
    uint32_t erJiFanChongXiTime;   // ����ˮ���ط���ϴʱ��
    uint32_t jinQiTime;            // ����ʱ��
    uint32_t chenDianTime;         // ����ʱ��
    uint32_t peiShuiChangGuiTime;  // ������ˮʱ��
    uint32_t peiShuiJiaBiaoTime;   // �ӱ���ˮʱ��

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

extern const ConfigFlow cfgFlow;

bool ShuiXiangNeedBuShui(void);
int Flows_ShuiXiangBuShui_Run(void);
void Baohu(void);
bool AndanNeedBuShui(void);
int Flows_AndanBuShui_Run(void);

int Flows_ShuiXiangRunXi_Run(void);
int Flows_YiJiJinYang_Run(void);
int Flows_ErJiJinYang_Run(void);
int Flows_ChenDian_Run(void);
int Flows_ShuiYangGuoLv_Run(void);
int Flows_PeiShui_Run(uint8_t isJiaBiao);
int Flows_FanChongXi_Run(void);
int Flows_YiJiChuShui_Run(void);

#endif
