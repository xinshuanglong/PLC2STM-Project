#include "bsp_include.h"

#define IWDG_PORT GPIOC
#define IWDG_PIN GPIO_Pin_5

void IWDG_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // ʹ��GPIOʱ��

    DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE); // ���Զϵ�ʱ��ֹͣ���Ź�����
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_128); // 40Khz
    IWDG_SetReload(3125);                  // 10�뿴�Ź�ʱ��
    IWDG_Enable();

    // Ӳ����������
    GPIO_InitStructure.GPIO_Pin = IWDG_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // ��ͨ���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // �������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // ����
    GPIO_Init(IWDG_PORT, &GPIO_InitStructure);         // ��ʼ��GPIO

    GPIO_SetBits(IWDG_PORT, IWDG_PIN);

    IWDG_Feed();
}

// ����Ӳ�����Ź���Ҫ��1.6���ڸ�λ��������Ҫ1.6��֮�ڵ��ô˺���������ϵͳ��λ
void IWDG_Feed(void)
{
    IWDG_ReloadCounter();
    GPIO_ToggleBits(IWDG_PORT, IWDG_PIN);
}

/***********************************************/
// �����ջ��С
#define IWDG_TASK_STK_SIZE 128
// ������
TaskHandle_t IwdgTaskHandler;
TaskHandle_t IwdgCheckTaskHandler;
// ������
void IwdgTask(void *pvParameters);
void IwdgCheckTask(void *pvParameters);
/***********************************************/

void IwdgTaskCreat(void)
{
    xTaskCreate(IwdgTask,
                "IwdgTask",
                IWDG_TASK_STK_SIZE,
                NULL,
                IWDG_TASK_PRIO,
                IwdgTaskHandler);
}

void IwdgTask(void *pvParameters)
{
    while (1)
    {
        IWDG_Feed();
        vTaskDelay(200);
    }
}
