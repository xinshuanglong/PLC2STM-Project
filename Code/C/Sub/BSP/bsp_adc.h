#ifndef _BSP_ADC_H
#define _BSP_ADC_H

typedef enum
{
    AD_1 = 0,
    AD_2,
    AD_3,
    AD_4,
    AD_5,
    REFINT,
} AD_Ch;

#define ADC_Channel_Count_Without_Refint (5)
#define ADC_Channel_Count (5 + 1) // 包含内部基准电压
#define ADC_Mean_Count 50
#define ADC_Converted_Count (ADC_Channel_Count * ADC_Mean_Count)

extern __IO uint16_t ADC_ConvertedValue[ADC_Converted_Count]; // ADC转化存储数组

void Adc_Init(void);
void ADC_DMA_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr);

#endif
