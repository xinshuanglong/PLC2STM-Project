#include "sys.h"
#include "delay.h"
#include "bsp_adc.h"

__IO uint16_t ADC_ConvertedValue[ADC_Converted_Count] = {0}; // ADC转化存储数组

// PB0 PB1 PC0 PC1
void Adc_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // 使能GPIO时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // 使能GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);  // ADC复位
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE); // 复位结束

    // GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     // 模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 不带上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);           // 初始化GPIO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     // 模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 不带上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);           // 初始化GPIO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     // 模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 不带上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);           // 初始化GPIO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     // 模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 不带上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);           // 初始化GPIO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     // 模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 不带上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);           // 初始化GPIO

    // ADC
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                     // 独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // 两个采样阶段之间的延迟5个时钟
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;      // DMA失能
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                  // 预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
    ADC_CommonInit(&ADC_CommonInitStructure);                                    // 初始化

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      // 12位模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;                                // 扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                          // 连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // 禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      // 右对齐
    ADC_InitStructure.ADC_NbrOfConversion = ADC_Channel_Count;
    ADC_Init(ADC1, &ADC_InitStructure); // ADC初始化

    // DMA
    ADC_DMA_Config(DMA2_Stream0, DMA_Channel_0, (u32) & (ADC1->DR), (u32)ADC_ConvertedValue, ADC_Converted_Count);

    // 内部基准电压
    ADC_TempSensorVrefintCmd(ENABLE);

    // 设置转换通道的顺序
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_480Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 2, ADC_SampleTime_480Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 3, ADC_SampleTime_480Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 4, ADC_SampleTime_480Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 5, ADC_SampleTime_480Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 6, ADC_SampleTime_480Cycles);

    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
    ADC_DMACmd(ADC1, ENABLE);

    ADC_Cmd(ADC1, ENABLE);
    ADC_SoftwareStartConv(ADC1);
}

/********************************DMA********************************************************/
// DMAx的各通道配置
// 这里的传输形式是固定的,这点要根据不同的情况来修改
// 从存储器->外设模式/8位数据宽度/存储器增量模式
// DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
// chx:DMA通道选择,@ref DMA_channel DMA_Channel_0~DMA_Channel_7
// par:外设地址
// mar:存储器地址
// ndtr:数据传输量
void ADC_DMA_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr)
{
    DMA_InitTypeDef DMA_InitStructure;

    if ((u32)DMA_Streamx > (u32)DMA2) // 得到当前stream是属于DMA2还是DMA1
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); // DMA2时钟使能
    }
    else
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); // DMA1时钟使能
    }
    DMA_DeInit(DMA_Streamx);

    while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
    {
    } // 等待DMA可配置

    /* 配置 DMA Stream */
    DMA_InitStructure.DMA_Channel = chx;                                        // 通道选择
    DMA_InitStructure.DMA_PeripheralBaseAddr = par;                             // DMA外设地址
    DMA_InitStructure.DMA_Memory0BaseAddr = mar;                                // DMA 存储器0地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                     // 外设模式到存储器
    DMA_InitStructure.DMA_BufferSize = ndtr;                                    // 数据传输量
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // 外设非增量模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // 存储器增量模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 外设数据长度:16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         // 存储器数据长度:16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             // 使用循环模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                       // 中等优先级
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;         // 存储器突发单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // 外设突发单次传输
    DMA_Init(DMA_Streamx, &DMA_InitStructure);                          // 初始化DMA Stream
    DMA_Cmd(DMA_Streamx, ENABLE);
}
