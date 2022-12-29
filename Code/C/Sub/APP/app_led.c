#include "bsp_include.h"

#define LED_PORT GPIOE
#define LED_PIN GPIO_Pin_4

void Led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //ʹ��GPIOʱ��

    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      //��ͨ���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       //����
    GPIO_Init(LED_PORT, &GPIO_InitStructure);          //��ʼ��GPIO

    GPIO_ResetBits(LED_PORT, LED_PIN);
}

void Led_Ctl(uint8_t value)
{
    if (value)
    {
        GPIO_SetBits(LED_PORT, LED_PIN);
    }
    else
    {
        GPIO_ResetBits(LED_PORT, LED_PIN);
    }
}

void Led_Reversal(void)
{
    GPIO_ToggleBits(LED_PORT, LED_PIN);
}

/***********************************************/
//�����ջ��С
#define LED_TASK_STK_SIZE 256
//������
TaskHandle_t LedTaskHandler;
//������
void LedTask(void *pvParameters);
/***********************************************/

void LedTaskCreat(void)
{
    xTaskCreate(LedTask,
                "LedTask",
                LED_TASK_STK_SIZE,
                NULL,
                LED_TASK_PRIO,
                LedTaskHandler);
}

void LedTask(void *pvParameters)
{
    while (1)
    {
        Led_Ctl(1);
        vTaskDelay(100);
        Led_Ctl(0);
        vTaskDelay(2000);
    }
}
