#include "bsp_include.h"

_Time gtTimeBdc, gtTimeBin;

/***********************************************/
// �����ջ��С
#define ENV_TASK_STK_SIZE 128
// ������
TaskHandle_t EnvTaskHandler;
// ������
void EnvTask(void *pvParameters);
/***********************************************/

void EnvTaskCreat(void)
{
    xTaskCreate(EnvTask,
                "EnvTask",
                ENV_TASK_STK_SIZE,
                NULL,
                ENV_TASK_PRIO,
                EnvTaskHandler);
}

void EnvTask(void *pvParameters)
{
    uint32_t count = 0;

    memset(&gtTimeBdc, 0, sizeof(gtTimeBdc));

    vTaskDelay(1000); // �ȴ�ADC��ʼ���ȶ�

    while (1)
    {
        // ʱ��
        if (RTC_Get_Time(&gtTimeBdc, &gtTimeBin))
        {
        }

        delay_ms(500);

        count++;
    }
}
