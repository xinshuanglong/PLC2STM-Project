#ifndef _APP_MEASURE_H
#define _APP_MEASURE_H

enum MeasureMsgType
{
    NONE_TYPE,
    TEST_TYPE_DEBUG,            // ��������  (����ģʽ)
    TEST_TYPE_DEBUG_TEST,       // ��������  (�û�ģʽ)
    TEST_TYPE_PROJECT,          // ����
    TEST_TYPE_PROJECT_NO_RANGE, // ����    �����������Զ��л�
};

typedef struct
{
    uint8_t id;           // ��ĿID,���׻����ܵ�
    uint8_t fid;          // ��ĿFID,С����(0:������������  1:ֻ����ˮ����ϴ 2:ֻ����һ��ˮ���ؽ���)
    uint8_t isOnlyDuoCan; // ֻ��������� 0:�������� 1:ֻ���������
    uint8_t isJiaBiao;    // �Ƿ��Ǽӱ�ģʽ 0:���� 1:�ӱ�
} _Measure;

typedef struct
{
    uint8_t id;            // 0��Ŀid
    uint8_t fid;           // 1��Ŀid
    uint8_t range;         // 2
    uint32_t startTime;    // ��ʼʱ��,FreeRTOS(xTaskGetTickCount)(��λTick)
    uint16_t runTotalTime; // 38    ��������ʱ��
    uint16_t step;         // 40    ����
    char describe[8];      // ��ǰ����ʣ��ʱ��,�Լ������¶ȵ���Ϣ
    char flowDescribe[64]; // ��ǰ����������������
} _tPara;                  // ������һ��ʼ�ýṹ��ȫ������

extern volatile uint8_t gWorkStatus;
extern _tPara gtPara;

void Measure_Stop(int opt);
void Measure_Start(_Measure measure);
void MeasureTaskCreat(void);

#endif
