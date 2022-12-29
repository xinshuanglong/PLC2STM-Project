#include "sys.h"
#include "delay.h"
#include "bsp_adc.h"

__IO uint16_t ADC_ConvertedValue[ADC_Converted_Count] = {0}; // ADCת���洢����

// PB0 PB1 PC0 PC1
void Adc_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // ʹ��GPIOʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // ʹ��GPIOʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);  // ADC��λ
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE); // ��λ����

    // GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     // ģ������
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);           // ��ʼ��GPIO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     // ģ������
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);           // ��ʼ��GPIO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     // ģ������
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);           // ��ʼ��GPIO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     // ģ������
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);           // ��ʼ��GPIO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;     // ģ������
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);           // ��ʼ��GPIO

    // ADC
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                     // ����ģʽ
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // ���������׶�֮����ӳ�5��ʱ��
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;      // DMAʧ��
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                  // Ԥ��Ƶ4��Ƶ��ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz
    ADC_CommonInit(&ADC_CommonInitStructure);                                    // ��ʼ��

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      // 12λģʽ
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;                                // ɨ��ģʽ
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                          // ����ת��
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // ��ֹ������⣬ʹ���������
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      // �Ҷ���
    ADC_InitStructure.ADC_NbrOfConversion = ADC_Channel_Count;
    ADC_Init(ADC1, &ADC_InitStructure); // ADC��ʼ��

    // DMA
    ADC_DMA_Config(DMA2_Stream0, DMA_Channel_0, (u32) & (ADC1->DR), (u32)ADC_ConvertedValue, ADC_Converted_Count);

    // �ڲ���׼��ѹ
    ADC_TempSensorVrefintCmd(ENABLE);

    // ����ת��ͨ����˳��
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
// DMAx�ĸ�ͨ������
// ����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
// �Ӵ洢��->����ģʽ/8λ���ݿ��/�洢������ģʽ
// DMA_Streamx:DMA������,DMA1_Stream0~7/DMA2_Stream0~7
// chx:DMAͨ��ѡ��,@ref DMA_channel DMA_Channel_0~DMA_Channel_7
// par:�����ַ
// mar:�洢����ַ
// ndtr:���ݴ�����
void ADC_DMA_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr)
{
    DMA_InitTypeDef DMA_InitStructure;

    if ((u32)DMA_Streamx > (u32)DMA2) // �õ���ǰstream������DMA2����DMA1
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); // DMA2ʱ��ʹ��
    }
    else
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); // DMA1ʱ��ʹ��
    }
    DMA_DeInit(DMA_Streamx);

    while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
    {
    } // �ȴ�DMA������

    /* ���� DMA Stream */
    DMA_InitStructure.DMA_Channel = chx;                                        // ͨ��ѡ��
    DMA_InitStructure.DMA_PeripheralBaseAddr = par;                             // DMA�����ַ
    DMA_InitStructure.DMA_Memory0BaseAddr = mar;                                // DMA �洢��0��ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                     // ����ģʽ���洢��
    DMA_InitStructure.DMA_BufferSize = ndtr;                                    // ���ݴ�����
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // ���������ģʽ
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // �洢������ģʽ
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // �������ݳ���:16λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         // �洢�����ݳ���:16λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             // ʹ��ѭ��ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                       // �е����ȼ�
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;         // �洢��ͻ�����δ���
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // ����ͻ�����δ���
    DMA_Init(DMA_Streamx, &DMA_InitStructure);                          // ��ʼ��DMA Stream
    DMA_Cmd(DMA_Streamx, ENABLE);
}
