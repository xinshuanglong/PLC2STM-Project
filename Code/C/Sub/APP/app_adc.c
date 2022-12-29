#include "bsp_include.h"

float gAdcRef = 0, gVcc = 0;
float gAdcKfp[ADC_Channel_Count_Without_Refint] = {0};

/***********************************************/
//�����ջ��С
#define ADC_TASK_STK_SIZE 256
//������
TaskHandle_t AdcTaskHandler;
//������
void AdcTask(void *pvParameters);
/***********************************************/

void AdcTaskCreat(void)
{
    xTaskCreate(AdcTask,
                "AdcTask",
                ADC_TASK_STK_SIZE,
                NULL,
                ADC_TASK_PRIO,
                AdcTaskHandler);
}

#define Vcc_Fliter_Para 0.025f

void AdcTask(void *pvParameters)
{
    KFP kfp[ADC_Channel_Count_Without_Refint];
    vTaskDelay(50); //�ȴ�ADC��ʼ���ȶ�

    for (int k = 0; k < ADC_Channel_Count_Without_Refint; k++)
    {
        kalmanParamInit(&kfp[k], 10, 0.001, 0.003);
    }

    while (1)
    {
        { //�����ڲ��ο���ѹ
            uint32_t tempAdcRef = 0;
            float adcRef = 0;
            for (int i = 0; i < ADC_Mean_Count; i++)
            {
                uint16_t adc = ADC_ConvertedValue[i * ADC_Channel_Count + REFINT];
                tempAdcRef += adc;
            }

            adcRef = (float)tempAdcRef / ADC_Mean_Count;

            if (gAdcRef == 0)
                gAdcRef = adcRef;
            gAdcRef = gAdcRef * (1 - Vcc_Fliter_Para) + adcRef * Vcc_Fliter_Para; //�˲�
            gVcc = 4095 / gAdcRef * 1.2f;
        }

        { //��������ADֵ
            uint32_t tempAdcRef[ADC_Channel_Count_Without_Refint] = {0};
            float adcRef[ADC_Channel_Count_Without_Refint] = {0};

            for (int k = 0; k < ADC_Channel_Count_Without_Refint; k++)
            {
                for (int i = 0; i < ADC_Mean_Count; i++)
                {
                    uint16_t adc = ADC_ConvertedValue[i * ADC_Channel_Count + k];
                    tempAdcRef[k] += adc;
                }

                adcRef[k] = (float)tempAdcRef[k] / ADC_Mean_Count;
                gAdcKfp[k] = kalmanFilter(&kfp[k], adcRef[k]); //�˲�
            }
        }

        vTaskDelay(100);
    }
}
