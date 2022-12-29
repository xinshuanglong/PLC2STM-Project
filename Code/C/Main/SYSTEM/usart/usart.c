#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "bsp_fifo.h"
#include "bsp_cfg.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "config.h"

#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h" //FreeRTOSʹ��
#endif

#define U6_DE         \
    do                \
    {                 \
        PDout(3) = 1; \
    } while (0)
#define U6_RE         \
    do                \
    {                 \
        PDout(3) = 0; \
    } while (0)

u8 USART1_RX_BUF[USART1_RX_LEN];
u8 USART1_TX_BUF[USART1_TX_LEN];

u8 USART2_RX_BUF[USART2_RX_LEN];
u8 USART2_TX_BUF[USART2_TX_LEN];

u8 USART3_RX_BUF[USART3_RX_LEN];
u8 USART3_TX_BUF[USART3_TX_LEN];

u8 UART4_RX_BUF[UART4_RX_LEN];
u8 UART4_TX_BUF[UART4_TX_LEN];

u8 USART6_RX_BUF[USART6_RX_LEN];
u8 USART6_TX_BUF[USART6_TX_LEN];

FIFO_Type Uart1_Rx_Fifo;                        // ����1����FIFO�ṹ����
static SemaphoreHandle_t Uart1TxSem = NULL;     // ����1�����ź�������ֹ���߳�ͬʱ���ʴ���1����
static SemaphoreHandle_t Uart1TxWaitSem = NULL; // ����1���͵ȴ��ź������ȴ���������ж�

FIFO_Type Uart2_Rx_Fifo;                        // ����2����FIFO�ṹ����
static SemaphoreHandle_t Uart2TxSem = NULL;     // ����2�����ź�������ֹ���߳�ͬʱ���ʴ���1����
static SemaphoreHandle_t Uart2TxWaitSem = NULL; // ����2���͵ȴ��ź������ȴ���������ж�

FIFO_Type Uart3_Rx_Fifo;                        // ����3����FIFO�ṹ����
static SemaphoreHandle_t Uart3TxSem = NULL;     // ����3�����ź�������ֹ���߳�ͬʱ���ʴ���1����
static SemaphoreHandle_t Uart3TxWaitSem = NULL; // ����3���͵ȴ��ź������ȴ���������ж�

FIFO_Type Uart4_Rx_Fifo;                        // ����4����FIFO�ṹ����
static SemaphoreHandle_t Uart4TxSem = NULL;     // ����4�����ź�������ֹ���߳�ͬʱ���ʴ���1����
static SemaphoreHandle_t Uart4TxWaitSem = NULL; // ����4���͵ȴ��ź������ȴ���������ж�

FIFO_Type Uart6_Rx_Fifo;                        // ����6����FIFO�ṹ����
static SemaphoreHandle_t Uart6TxSem = NULL;     // ����6�����ź�������ֹ���߳�ͬʱ���ʴ���1����
static SemaphoreHandle_t Uart6TxWaitSem = NULL; // ����6���͵ȴ��ź������ȴ���������ж�

extern SemaphoreHandle_t CmdRxSemCount; // �������ź���
extern SemaphoreHandle_t DebugRxSem;

extern SemaphoreHandle_t ModBusMasterRxSem; // �ź���

//////////////////////////////////////////////////////////////////
// �������´���,֧��printf����,������Ҫѡ��use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
// ��׼����Ҫ��֧�ֺ���
struct __FILE
{
    int handle;
};

FILE __stdout;
// ����_sys_exit()�Ա���ʹ�ð�����ģʽ
void _sys_exit(int x)
{
    x = x;
}

void _ttywrch(int ch)
{
    ch = ch;
}

int fputc(int Data, FILE *f)
{
    if (debugLevel == 1)
    {
        while (!USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE))
            ;                              // USART_GetFlagStatus���õ�����״̬λ
                                           // USART_FLAG_TXE:���ͼĴ���Ϊ�� 1��Ϊ�գ�0��æ״̬
        USART_SendData(DEBUG_USART, Data); // ����һ���ַ�
    }
    return Data;
}
#endif

// ��ʼ��IO ����1
//  bound:������
void uart1_init(u32 bound)
{
    // GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // ʹ��GPIOʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // ʹ��USART1ʱ��

    // ����1��Ӧ���Ÿ���ӳ��
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);  // GPIOB6����ΪUSART1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); // GPIOB7����ΪUSART1

    // USART1�˿�����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      // ���ù���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // �ٶ�50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    // ���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      // ����
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART1 ��ʼ������
    USART_InitStructure.USART_BaudRate = bound;                                     // ����������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // �ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // ����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // �շ�ģʽ
    USART_Init(USART1, &USART_InitStructure);                                       // ��ʼ������1

    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);                   // ʹ�ܿ����ж�
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);                     // ʹ�ܷ�������ж�
    USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE); // ֧��DMA ���ͺͽ���
    USART_Cmd(USART1, ENABLE);                                       // ʹ�ܴ���1

    // Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;         // ����1�ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; // ��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // �����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);                           // ����ָ���Ĳ�����ʼ��VIC�Ĵ�����
}

void uart2_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // ʹ��GPIOʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); // ʹ��USART2ʱ��

    // ����2���Ÿ���ӳ��
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); // GPIOA2����ΪUSART2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); // GPIOA3����ΪUSART2

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; // GPIOA2��GPIOA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;           // ���ù���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;     // �ٶ�100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;         // ���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;           // ����
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 // ��ʼ��PA2��PA3

    // USART2 ��ʼ������
    USART_InitStructure.USART_BaudRate = bound;                                     // ����������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // �ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // ����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // �շ�ģʽ
    USART_Init(USART2, &USART_InitStructure);                                       // ��ʼ������2

    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);                   // ʹ�ܿ����ж�
    USART_ITConfig(USART2, USART_IT_TC, ENABLE);                     // ʹ�ܷ�������ж�
    USART_DMACmd(USART2, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE); // ֧��DMA ���ͺͽ���
    USART_Cmd(USART2, ENABLE);                                       // ʹ�ܴ��� 2

    // Usart2 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; // ��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // �����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);                           // ����ָ���Ĳ�����ʼ��VIC�Ĵ�����
}

void uart3_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  // ʹ��GPIOʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); // ʹ��USART3ʱ��

    // ����3���Ÿ���ӳ��
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3); // GPIOB10����ΪUSART3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3); // GPIOB11����ΪUSART3

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; // GPIOB10��GPIOB11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;             // ���ù���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;       // �ٶ�100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;           // ���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;             // ����
    GPIO_Init(GPIOB, &GPIO_InitStructure);                   // ��ʼ��PB10��PB11

    // USART3 ��ʼ������
    USART_InitStructure.USART_BaudRate = bound;                                     // ����������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // �ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // ����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // �շ�ģʽ
    USART_Init(USART3, &USART_InitStructure);                                       // ��ʼ������3

    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);                   // ʹ�ܿ����ж�
    USART_ITConfig(USART3, USART_IT_TC, ENABLE);                     // ʹ�ܷ�������ж�
    USART_DMACmd(USART3, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE); // ֧��DMA ���ͺͽ���
    USART_Cmd(USART3, ENABLE);                                       // ʹ�ܴ��� 3

    // Usart NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; // ��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // �����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);                           // ����ָ���Ĳ�����ʼ��VIC�Ĵ�����
}

// ��ʼ��IO ����4
//  bound:������
void uart4_init(u32 bound)
{
    // GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // ʹ��GPIOʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE); // ʹ��UART4ʱ��

    // ����4��Ӧ���Ÿ���ӳ��
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4); // GPIOA0����ΪUART4
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4); // GPIOA1����ΪUART4

    // UART4�˿�����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; // GPIOA0��GPIOA1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;           // ���ù���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      // �ٶ�50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;         // ���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;           // ����
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 // ��ʼ��PA0��PA1

    // UART4 ��ʼ������
    USART_InitStructure.USART_BaudRate = bound;                                     // ����������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // �ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // ����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // �շ�ģʽ
    USART_Init(UART4, &USART_InitStructure);                                        // ��ʼ������1

    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);                   // ʹ�ܿ����ж�
    USART_ITConfig(UART4, USART_IT_TC, ENABLE);                     // ʹ�ܷ�������ж�
    USART_DMACmd(UART4, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE); // ֧��DMA ���ͺͽ���
    USART_Cmd(UART4, ENABLE);                                       // ʹ�ܴ���4

    // Usart NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;          // ����4�ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; // ��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // �����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);                           // ����ָ���Ĳ�����ʼ��VIC�Ĵ�����
}

void uart6_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);  // ʹ��GPIOʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); // ʹ��USART6ʱ��

    // ����6���Ÿ���ӳ��
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6); // ����ΪUSART6
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6); // ����ΪUSART6

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;       // ���ù���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // �ٶ�100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // ���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // ����
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 485����
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); // ʹ��GPIOʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // ���ù���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // �ٶ�100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // ���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // ����
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    U6_RE;

    // USART6 ��ʼ������
    USART_InitStructure.USART_BaudRate = bound;                                     // ����������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // �ֳ�Ϊ9λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // ��У��
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // �շ�ģʽ
    USART_Init(USART6, &USART_InitStructure);                                       // ��ʼ������2

    USART_ITConfig(USART6, USART_IT_IDLE, ENABLE);                   // ʹ�ܿ����ж�
    USART_ITConfig(USART6, USART_IT_TC, ENABLE);                     // ʹ�ܷ�������ж�
    USART_DMACmd(USART6, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE); // ֧��DMA ���ͺͽ���
    USART_Cmd(USART6, ENABLE);                                       // ʹ�ܴ���

    // Usart NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; // ��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // �����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);                           // ����ָ���Ĳ�����ʼ��VIC�Ĵ���
}

void UART1_INIT(void)
{
    uart1_init(115200);
    /*****����***************/
    MYDMA_Send_Config(DMA2_Stream7, DMA_Channel_4, (u32)&USART1->DR, (u32)USART1_TX_BUF, 0); // DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
    /*****����***************/
    MYDMA_Receive_Config(DMA2_Stream5, DMA_Channel_4, (u32)&USART1->DR, (u32)USART1_RX_BUF, USART1_RX_LEN); // DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
                                                                                                            // ����FIFO��ʼ��
    Fifo_Init(&Uart1_Rx_Fifo, USART1_RX_BUF, USART1_RX_LEN);                                                // FIFO��ʼ�������ջ����Ի���FIFO��ʽ����

    // �����ź�����ʼ��
    Uart1TxSem = xSemaphoreCreateBinary();
    xSemaphoreGive(Uart1TxSem); // �ͷŷ����ź���

    Uart1TxWaitSem = xSemaphoreCreateBinary();
}

void UART2_INIT(void)
{
    uart2_init(115200);
    /*****����***************/
    MYDMA_Send_Config(DMA1_Stream6, DMA_Channel_4, (u32)&USART2->DR, (u32)USART2_TX_BUF, 0); // DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
    /*****����***************/
    MYDMA_Receive_Config(DMA1_Stream5, DMA_Channel_4, (u32)&USART2->DR, (u32)USART2_RX_BUF, USART2_RX_LEN); // DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
    // ����FIFO��ʼ��
    Fifo_Init(&Uart2_Rx_Fifo, USART2_RX_BUF, USART2_RX_LEN);

    // �����ź�����ʼ��
    Uart2TxSem = xSemaphoreCreateBinary();
    xSemaphoreGive(Uart2TxSem); // �ͷŷ����ź���

    Uart2TxWaitSem = xSemaphoreCreateBinary();
}

void UART3_INIT(void)
{
    uart3_init(115200);
    /*****����***************/
    MYDMA_Send_Config(DMA1_Stream3, DMA_Channel_4, (u32)&USART3->DR, (u32)USART3_TX_BUF, 0); // DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
    /*****����***************/
    MYDMA_Receive_Config(DMA1_Stream1, DMA_Channel_4, (u32)&USART3->DR, (u32)USART3_RX_BUF, USART3_RX_LEN); // DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
    // ����FIFO��ʼ��
    Fifo_Init(&Uart3_Rx_Fifo, USART3_RX_BUF, USART3_RX_LEN);

    // �����ź�����ʼ��
    Uart3TxSem = xSemaphoreCreateBinary();
    xSemaphoreGive(Uart3TxSem); // �ͷŷ����ź���

    Uart3TxWaitSem = xSemaphoreCreateBinary();
}

void UART4_INIT(void)
{
    uart4_init(115200);
    /*****����***************/
    MYDMA_Send_Config(DMA1_Stream4, DMA_Channel_4, (u32)&UART4->DR, (u32)UART4_TX_BUF, 0); // DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
    /*****����***************/
    MYDMA_Receive_Config(DMA1_Stream2, DMA_Channel_4, (u32)&UART4->DR, (u32)UART4_RX_BUF, UART4_RX_LEN); // DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
    // ����FIFO��ʼ��
    Fifo_Init(&Uart4_Rx_Fifo, UART4_RX_BUF, UART4_RX_LEN);

    // �����ź�����ʼ��
    Uart4TxSem = xSemaphoreCreateBinary();
    xSemaphoreGive(Uart4TxSem); // �ͷŷ����ź���

    Uart4TxWaitSem = xSemaphoreCreateBinary();
}

void UART6_INIT(void)
{
    uart6_init(9600);
    /*****����***************/
    MYDMA_Send_Config(DMA2_Stream6, DMA_Channel_5, (u32)&USART6->DR, (u32)USART6_TX_BUF, 0); // DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
    /*****����***************/
    MYDMA_Receive_Config(DMA2_Stream2, DMA_Channel_5, (u32)&USART6->DR, (u32)USART6_RX_BUF, USART6_RX_LEN); // DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
    // ����FIFO��ʼ��
    Fifo_Init(&Uart6_Rx_Fifo, USART6_RX_BUF, USART6_RX_LEN);

    // �����ź�����ʼ��
    Uart6TxSem = xSemaphoreCreateBinary();
    xSemaphoreGive(Uart6TxSem); // �ͷŷ����ź���

    Uart6TxWaitSem = xSemaphoreCreateBinary();
}

void Uart_1_Send(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }

    xSemaphoreTake(Uart1TxSem, 3000); // �ȴ������ź�������ֹ���̷߳���
    xSemaphoreTake(Uart1TxWaitSem, 0);

    DMA_Cmd(DMA2_Stream7, DISABLE); // �ر�DMA����
    while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE)
    {
    }                                          // ȷ��DMA���Ա�����
    DMA_SetCurrDataCounter(DMA2_Stream7, len); // ���ݴ�����
    DMA2_Stream7->M0AR = (uint32_t)buf;        // ��������
    DMA_Cmd(DMA2_Stream7, ENABLE);             // ����DMA����

    xSemaphoreTake(Uart1TxWaitSem, 3000);
    DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7); // ���DMA2_Steam7������ɱ�־
    DMA_Cmd(DMA2_Stream7, DISABLE);              // �رմ��ڷ���ͨ��

    xSemaphoreGive(Uart1TxSem); // �ͷŷ����ź���
}

void Uart_2_Send(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }

    xSemaphoreTake(Uart2TxSem, 3000); // �ȴ������ź�������ֹ���̷߳���
    xSemaphoreTake(Uart2TxWaitSem, 0);

    DMA_Cmd(DMA1_Stream6, DISABLE); // �ر�DMA����
    while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE)
    {
    }                                          // ȷ��DMA���Ա�����
    DMA_SetCurrDataCounter(DMA1_Stream6, len); // ���ݴ�����
    DMA1_Stream6->M0AR = (uint32_t)buf;        // ��������
    DMA_Cmd(DMA1_Stream6, ENABLE);             // ����DMA����

    xSemaphoreTake(Uart2TxWaitSem, 3000);
    DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6); // ���DMA2_Steam7������ɱ�
    DMA_Cmd(DMA1_Stream6, DISABLE);              // �رմ��ڷ���ͨ��

    xSemaphoreGive(Uart2TxSem); // �ͷŷ����ź���
}

void Uart_3_Send(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }

    xSemaphoreTake(Uart3TxSem, 3000); // �ȴ������ź�������ֹ���̷߳���
    xSemaphoreTake(Uart3TxWaitSem, 0);

    DMA_Cmd(DMA1_Stream3, DISABLE); // �ر�DMA����
    while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE)
    {
    }                                          // ȷ��DMA���Ա�����
    DMA_SetCurrDataCounter(DMA1_Stream3, len); // ���ݴ�����
    DMA1_Stream3->M0AR = (uint32_t)buf;        // ��������
    DMA_Cmd(DMA1_Stream3, ENABLE);             // ����DMA����

    xSemaphoreTake(Uart3TxWaitSem, 3000);
    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3); // ���DMA1_Steam3������ɱ�
    DMA_Cmd(DMA1_Stream3, DISABLE);              // �رմ��ڷ���ͨ��

    xSemaphoreGive(Uart3TxSem); // �ͷŷ����ź���
}

void Uart_4_Send(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }

    xSemaphoreTake(Uart4TxSem, 3000); // �ȴ������ź�������ֹ���̷߳���
    xSemaphoreTake(Uart4TxWaitSem, 0);

    DMA_Cmd(DMA1_Stream4, DISABLE); // �ر�DMA����
    while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE)
    {
    }                                          // ȷ��DMA���Ա�����
    DMA_SetCurrDataCounter(DMA1_Stream4, len); // ���ݴ�����
    DMA1_Stream4->M0AR = (uint32_t)buf;        // ��������
    DMA_Cmd(DMA1_Stream4, ENABLE);             // ����DMA����

    xSemaphoreTake(Uart4TxWaitSem, 3000);
    DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4); // ���DMA2_Steam7������ɱ�
    DMA_Cmd(DMA1_Stream4, DISABLE);              // �رմ��ڷ���ͨ��

    xSemaphoreGive(Uart4TxSem); // �ͷŷ����ź���
}

void Uart_6_Send(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }

    xSemaphoreTake(Uart6TxSem, 3000); // �ȴ������ź�������ֹ���̷߳���
    xSemaphoreTake(Uart6TxWaitSem, 0);

    U6_DE;
    delay_ms(1);
    DMA_Cmd(DMA2_Stream6, DISABLE); // �ر�DMA����
    while (DMA_GetCmdStatus(DMA2_Stream6) != DISABLE)
    {
    }                                          // ȷ��DMA���Ա�����
    DMA_SetCurrDataCounter(DMA2_Stream6, len); // ���ݴ�����
    DMA2_Stream6->M0AR = (uint32_t)buf;        // ��������
    DMA_Cmd(DMA2_Stream6, ENABLE);             // ����DMA����

    xSemaphoreTake(Uart6TxWaitSem, 3000);
    DMA_ClearFlag(DMA2_Stream6, DMA_FLAG_TCIF6);
    DMA_Cmd(DMA2_Stream6, DISABLE); // �رմ��ڷ���ͨ��
    U6_RE;

    xSemaphoreGive(Uart6TxSem); // �ͷŷ����ź���
}

/*
 * ��������Uart_1_Select
 * ����  ������1���յȴ�
 * ����  ��delay---�ȴ�ʱ�䣬��λms
 * ���  �����������ݳ���
 */
uint16_t Uart_1_Select(uint16_t delay)
{
    uint16_t count = 0;
    uint16_t len = 0;

    while (count < delay)
    {
        delay_ms(10);
        count += 10;
        Uart1_Rx_Fifo.in = USART1_RX_LEN - DMA_GetCurrDataCounter(DMA2_Stream5);
        len = Fifo_Status(&Uart1_Rx_Fifo);
        if (len != 0)
            return len;
    }
    return len;
}

/*
 * ��������Uart_2_Select
 * ����  ������1���յȴ�
 * ����  ��delay---�ȴ�ʱ�䣬��λms
 * ���  �����������ݳ���
 */
uint16_t Uart_2_Select(uint16_t delay)
{
    uint16_t count = 0;
    uint16_t len = 0;

    while (count < delay)
    {
        delay_ms(10);
        count += 10;
        Uart2_Rx_Fifo.in = USART2_RX_LEN - DMA_GetCurrDataCounter(DMA1_Stream5);
        len = Fifo_Status(&Uart2_Rx_Fifo);
        if (len != 0)
            return len;
    }
    return len;
}

/*
 * ��������Uart_3_Select
 * ����  ������1���յȴ�
 * ����  ��delay---�ȴ�ʱ�䣬��λms
 * ���  �����������ݳ���
 */
uint16_t Uart_3_Select(uint16_t delay)
{
    uint16_t count = 0;
    uint16_t len = 0;

    while (count < delay)
    {
        delay_ms(10);
        count += 10;
        Uart3_Rx_Fifo.in = USART3_RX_LEN - DMA_GetCurrDataCounter(DMA1_Stream1);
        len = Fifo_Status(&Uart3_Rx_Fifo);
        if (len != 0)
            return len;
    }
    return len;
}

/*
 * ��������Uart_4_Select
 * ����  ������1���յȴ�
 * ����  ��delay---�ȴ�ʱ�䣬��λms
 * ���  �����������ݳ���
 */
uint16_t Uart_4_Select(uint16_t delay)
{
    uint16_t count = 0;
    uint16_t len = 0;

    while (count < delay)
    {
        delay_ms(10);
        count += 10;
        Uart4_Rx_Fifo.in = UART4_RX_LEN - DMA_GetCurrDataCounter(DMA1_Stream2);
        len = Fifo_Status(&Uart4_Rx_Fifo);
        if (len != 0)
            return len;
    }
    return len;
}

/*
 * ��������Uart_4_Select
 * ����  ������1���յȴ�
 * ����  ��delay---�ȴ�ʱ�䣬��λms
 * ���  �����������ݳ���
 */
uint16_t Uart_6_Select(uint16_t delay)
{
    uint16_t count = 0;
    uint16_t len = 0;

    while (count < delay)
    {
        delay_ms(10);
        count += 10;
        Uart6_Rx_Fifo.in = USART6_RX_LEN - DMA_GetCurrDataCounter(DMA2_Stream2);
        len = Fifo_Status(&Uart6_Rx_Fifo);
        if (len != 0)
            return len;
    }
    return len;
}

/*
 * ��������Uart_1_Get
 * ����  ������1����
 * ����  ��buffer---���ջ��棬len---���ջ��泤��
 * ���  ���������ݳ���
 */
uint16_t Uart_1_Get(uint8_t *buffer, uint16_t len)
{
    return Fifo_Get(&Uart1_Rx_Fifo, buffer, len);
}

/*
 * ��������Uart_2_Get
 * ����  ������1����
 * ����  ��buffer---���ջ��棬len---���ջ��泤��
 * ���  ���������ݳ���
 */
uint16_t Uart_2_Get(uint8_t *buffer, uint16_t len)
{
    return Fifo_Get(&Uart2_Rx_Fifo, buffer, len);
}

/*
 * ��������Uart_3_Get
 * ����  ������1����
 * ����  ��buffer---���ջ��棬len---���ջ��泤��
 * ���  ���������ݳ���
 */
uint16_t Uart_3_Get(uint8_t *buffer, uint16_t len)
{
    return Fifo_Get(&Uart3_Rx_Fifo, buffer, len);
}

/*
 * ��������Uart_4_Get
 * ����  ������4����
 * ����  ��buffer---���ջ��棬len---���ջ��泤��
 * ���  ���������ݳ���
 */
uint16_t Uart_4_Get(uint8_t *buffer, uint16_t len)
{
    return Fifo_Get(&Uart4_Rx_Fifo, buffer, len);
}

/*
 * ��������Uart_4_Get
 * ����  ������4����
 * ����  ��buffer---���ջ��棬len---���ջ��泤��
 * ���  ���������ݳ���
 */
uint16_t Uart_6_Get(uint8_t *buffer, uint16_t len)
{
    return Fifo_Get(&Uart6_Rx_Fifo, buffer, len);
}

void UART_INIT(void)
{
    UART1_INIT();
    UART2_INIT();
    UART3_INIT();
    UART4_INIT();
    UART6_INIT();
}

void Uart_Send(USART_TypeDef *uart, const uint8_t *buffer, uint16_t len)
{
    if (uart == USART1)
    {
        Uart_1_Send(buffer, len);
    }
    else if (uart == USART2)
    {
        Uart_2_Send(buffer, len);
    }
    else if (uart == USART3)
    {
        Uart_3_Send(buffer, len);
    }
    else if (uart == UART4)
    {
        Uart_4_Send(buffer, len);
    }
    else if (uart == USART6)
    {
        Uart_6_Send(buffer, len);
    }
}

uint16_t Uart_Get(USART_TypeDef *uart, uint8_t *buffer, uint16_t len)
{
    uint16_t lenght = 0;
    if (uart == USART1)
    {
        lenght = Uart_1_Get(buffer, len);
    }
    else if (uart == USART2)
    {
        lenght = Uart_2_Get(buffer, len);
    }
    else if (uart == USART3)
    {
        lenght = Uart_3_Get(buffer, len);
    }
    else if (uart == UART4)
    {
        lenght = Uart_4_Get(buffer, len);
    }
    else if (uart == USART6)
    {
        lenght = Uart_6_Get(buffer, len);
    }
    return lenght;
}

uint16_t Uart_Select(USART_TypeDef *uart, uint16_t delay)
{
    uint16_t lenght = 0;
    if (uart == USART1)
    {
        lenght = Uart_1_Select(delay);
    }
    else if (uart == USART2)
    {
        lenght = Uart_2_Select(delay);
    }
    else if (uart == USART3)
    {
        lenght = Uart_3_Select(delay);
    }
    else if (uart == UART4)
    {
        lenght = Uart_4_Select(delay);
    }
    else if (uart == USART6)
    {
        lenght = Uart_6_Select(delay);
    }
    return lenght;
}

// ��λ�������ж�
void USART1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) // �����ж�
    {
        USART_ReceiveData(USART1);
        Uart1_Rx_Fifo.in = USART1_RX_LEN - DMA_GetCurrDataCounter(DMA2_Stream5);
        USART_ClearITPendingBit(USART1, USART_IT_IDLE);
    }
    else if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
    {
        USART_ClearITPendingBit(USART1, USART_IT_TC);
        if (Uart1TxWaitSem != NULL)
            xSemaphoreGiveFromISR(Uart1TxWaitSem, &xHigherPriorityTaskWoken);
    }
}

void USART2_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) // �����ж�
    {
        USART_ReceiveData(USART2);
        Uart2_Rx_Fifo.in = USART2_RX_LEN - DMA_GetCurrDataCounter(DMA1_Stream5);
        USART_ClearITPendingBit(USART2, USART_IT_IDLE);

        if (debugLevel == 1)
        {
            if (DebugRxSem != NULL)
            {
                xSemaphoreGiveFromISR(DebugRxSem, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
        else
        {
            if (CmdRxSemCount != NULL)
            {
                xSemaphoreGiveFromISR(CmdRxSemCount, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
    }
    else if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)
    {
        USART_ClearITPendingBit(USART2, USART_IT_TC);
        if (Uart2TxWaitSem != NULL)
            xSemaphoreGiveFromISR(Uart2TxWaitSem, &xHigherPriorityTaskWoken);
    }
}

void USART3_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) // �����ж�
    {
        USART_ReceiveData(USART3);
        Uart3_Rx_Fifo.in = USART3_RX_LEN - DMA_GetCurrDataCounter(DMA1_Stream1);
        USART_ClearITPendingBit(USART3, USART_IT_IDLE);
    }
    else if (USART_GetITStatus(USART3, USART_IT_TC) != RESET)
    {
        USART_ClearITPendingBit(USART3, USART_IT_TC);
        if (Uart3TxWaitSem != NULL)
            xSemaphoreGiveFromISR(Uart3TxWaitSem, &xHigherPriorityTaskWoken);
    }
}

void UART4_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (USART_GetITStatus(UART4, USART_IT_IDLE) != RESET) // �����ж�
    {
        USART_ReceiveData(UART4);
        Uart4_Rx_Fifo.in = UART4_RX_LEN - DMA_GetCurrDataCounter(DMA1_Stream2);
        USART_ClearITPendingBit(UART4, USART_IT_IDLE);
    }
    else if (USART_GetITStatus(UART4, USART_IT_TC) != RESET)
    {
        USART_ClearITPendingBit(UART4, USART_IT_TC);
        if (Uart4TxWaitSem != NULL)
            xSemaphoreGiveFromISR(Uart4TxWaitSem, &xHigherPriorityTaskWoken);
    }
}

void USART6_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (USART_GetITStatus(USART6, USART_IT_IDLE) != RESET) // �����ж�
    {
        USART_ReceiveData(USART6);
        Uart6_Rx_Fifo.in = USART6_RX_LEN - DMA_GetCurrDataCounter(DMA2_Stream2);
        USART_ClearITPendingBit(USART6, USART_IT_IDLE);

        if (ModBusMasterRxSem != NULL)
        {
            xSemaphoreGiveFromISR(ModBusMasterRxSem, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else if (USART_GetITStatus(USART6, USART_IT_TC) != RESET)
    {
        USART_ClearITPendingBit(USART6, USART_IT_TC);
        U6_RE;
        if (Uart6TxWaitSem != NULL)
            xSemaphoreGiveFromISR(Uart6TxWaitSem, &xHigherPriorityTaskWoken);
    }
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
void MYDMA_Send_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr)
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
    DMA_InitStructure.DMA_Channel = chx;                                    // ͨ��ѡ��
    DMA_InitStructure.DMA_PeripheralBaseAddr = par;                         // DMA�����ַ
    DMA_InitStructure.DMA_Memory0BaseAddr = mar;                            // DMA �洢��0��ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                 // �洢��������ģʽ
    DMA_InitStructure.DMA_BufferSize = ndtr;                                // ���ݴ�����
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // ���������ģʽ
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // �洢������ģʽ
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݳ���:8λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // �洢�����ݳ���:8λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // ʹ����ͨģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   // �е����ȼ�
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;         // �洢��ͻ�����δ���
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // ����ͻ�����δ���
    DMA_Init(DMA_Streamx, &DMA_InitStructure);                          // ��ʼ��DMA Stream
    DMA_Cmd(DMA_Streamx, DISABLE);
}

void MYDMA_Receive_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr)
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
    DMA_InitStructure.DMA_Channel = chx;                                    // ͨ��ѡ��
    DMA_InitStructure.DMA_PeripheralBaseAddr = par;                         // DMA�����ַ
    DMA_InitStructure.DMA_Memory0BaseAddr = mar;                            // DMA �洢��0��ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                 // ����ģʽ���洢��
    DMA_InitStructure.DMA_BufferSize = ndtr;                                // ���ݴ�����
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // ���������ģʽ
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // �洢������ģʽ
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݳ���:8λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // �洢�����ݳ���:8λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         // ʹ����ͨģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   // �е����ȼ�
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;         // �洢��ͻ�����δ���
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // ����ͻ�����δ���
    DMA_Init(DMA_Streamx, &DMA_InitStructure);                          // ��ʼ��DMA Stream

    DMA_Cmd(DMA_Streamx, ENABLE);
}

// ����һ��DMA����
//  DMA_Streamx:DMA������,DMA1_Stream0~7/DMA2_Stream0~7
//  ndtr:���ݴ�����
void MYDMA_Enable(const uint8_t *buf, DMA_Stream_TypeDef *DMA_Streamx, u16 ndtr)
{
    DMA_Cmd(DMA_Streamx, DISABLE); // �ر�DMA����

    while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
    {
    } // ȷ��DMA���Ա�����

    DMA_SetCurrDataCounter(DMA_Streamx, ndtr); // ���ݴ�����

    DMA_Streamx->M0AR = (uint32_t)buf; // ��������

    DMA_Cmd(DMA_Streamx, ENABLE); // ����DMA����
}
