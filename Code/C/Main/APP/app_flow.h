#ifndef _APP_FLOW_H
#define _APP_FLOW_H

#define SWITCH_COUNT 16

enum RunType
{
    IS_OK = 0,              // 正常
    IS_OVER = 0,            // 运行完成
    IS_STOP = (1 << 0),     // 手动停止
    IS_OVERTIME = (1 << 1), // 超时
    IS_ERROR = (1 << 2),    // 出现错误停止
};

enum ChuiShuiType
{
    CHUISHUITYPE_SHUIYANG = 0,
    CHUISHUITYPE_ZILAISHUI
};

typedef struct
{
    uint32_t runxi1Time;           // 一级水样池润洗时间
    uint32_t runxi2Time;           // 二级级水样池润洗时间
    uint32_t runxiGuoLvTime;       // 过滤池润洗时间
    uint32_t runxiAnDanTime;       // 氨氮水样池润洗时间
    uint32_t paiyeTime;            // 排液时间
    uint32_t chushuiTime;          // 储水时间
    uint32_t guanLuFanChongXiTime; // 管路反冲洗时间
    uint32_t guolvFanChongXiTime;  // 过滤装置反冲洗时间
    uint32_t yiJiFanChongXiTime;   // 一级水样池反冲洗时间
    uint32_t erJiFanChongXiTime;   // 二级水样池反冲洗时间
    uint32_t jinQiTime;            // 进气时间
    uint32_t chenDianTime;         // 沉淀时间
    uint32_t peiShuiChangGuiTime;  // 常规配水时间
    uint32_t peiShuiJiaBiaoTime;   // 加标配水时间

    uint16_t chuishuiType; // 取值 ChuiShuiType
    uint16_t haveGuoLv;    // 是否有氨氮过滤装置 0:没有 1:有

    uint32_t yeWeiOverTime;   // 一二级液位超时时间
    uint32_t shuiLiuOverTime; // 水流超时时间
    uint32_t jinYangOverTime; // 过滤进样超时时间
} ConfigFlow;

typedef struct
{
    int index;             // 序号
    char log[48];          // 描述
    char log2[48];         // 描述2
    int on[SWITCH_COUNT];  // 开信号
    int off[SWITCH_COUNT]; // 关信号
    int (*funNoWait)(void);
    int (*funWait)(void); // 会阻塞线程

    int delay; // delay最后执行
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
