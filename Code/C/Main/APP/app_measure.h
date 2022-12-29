#ifndef _APP_MEASURE_H
#define _APP_MEASURE_H

enum MeasureMsgType
{
    NONE_TYPE,
    TEST_TYPE_DEBUG,            // 单步测试  (流程模式)
    TEST_TYPE_DEBUG_TEST,       // 单步测试  (用户模式)
    TEST_TYPE_PROJECT,          // 测量
    TEST_TYPE_PROJECT_NO_RANGE, // 测量    不进行量程自动切换
};

typedef struct
{
    uint8_t id;           // 项目ID,总磷还是总氮
    uint8_t fid;          // 项目FID,小流程(0:运行所有流程  1:只运行水箱润洗 2:只运行一级水样池进样)
    uint8_t isOnlyDuoCan; // 只测量多参数 0:测量所有 1:只测量多参数
    uint8_t isJiaBiao;    // 是否是加标模式 0:常规 1:加标
} _Measure;

typedef struct
{
    uint8_t id;            // 0项目id
    uint8_t fid;           // 1项目id
    uint8_t range;         // 2
    uint32_t startTime;    // 开始时间,FreeRTOS(xTaskGetTickCount)(单位Tick)
    uint16_t runTotalTime; // 38    流程运行时间
    uint16_t step;         // 40    步骤
    char describe[8];      // 当前步骤剩余时间,试剂量，温度等信息
    char flowDescribe[64]; // 当前步骤流程中文名字
} _tPara;                  // 跑流程一开始该结构体全部清零

extern volatile uint8_t gWorkStatus;
extern _tPara gtPara;

void Measure_Stop(int opt);
void Measure_Start(_Measure measure);
void MeasureTaskCreat(void);

#endif
