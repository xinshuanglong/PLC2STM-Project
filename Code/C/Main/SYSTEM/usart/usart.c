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
#include "FreeRTOS.h" //FreeRTOS使用
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

FIFO_Type Uart1_Rx_Fifo;                        // 串口1环形FIFO结构变量
static SemaphoreHandle_t Uart1TxSem = NULL;     // 串口1发送信号量，阻止多线程同时访问串口1发送
static SemaphoreHandle_t Uart1TxWaitSem = NULL; // 串口1发送等待信号量，等待发送完成中断

FIFO_Type Uart2_Rx_Fifo;                        // 串口2环形FIFO结构变量
static SemaphoreHandle_t Uart2TxSem = NULL;     // 串口2发送信号量，阻止多线程同时访问串口1发送
static SemaphoreHandle_t Uart2TxWaitSem = NULL; // 串口2发送等待信号量，等待发送完成中断

FIFO_Type Uart3_Rx_Fifo;                        // 串口3环形FIFO结构变量
static SemaphoreHandle_t Uart3TxSem = NULL;     // 串口3发送信号量，阻止多线程同时访问串口1发送
static SemaphoreHandle_t Uart3TxWaitSem = NULL; // 串口3发送等待信号量，等待发送完成中断

FIFO_Type Uart4_Rx_Fifo;                        // 串口4环形FIFO结构变量
static SemaphoreHandle_t Uart4TxSem = NULL;     // 串口4发送信号量，阻止多线程同时访问串口1发送
static SemaphoreHandle_t Uart4TxWaitSem = NULL; // 串口4发送等待信号量，等待发送完成中断

FIFO_Type Uart6_Rx_Fifo;                        // 串口6环形FIFO结构变量
static SemaphoreHandle_t Uart6TxSem = NULL;     // 串口6发送信号量，阻止多线程同时访问串口1发送
static SemaphoreHandle_t Uart6TxWaitSem = NULL; // 串口6发送等待信号量，等待发送完成中断

extern SemaphoreHandle_t CmdRxSemCount; // 计数型信号量
extern SemaphoreHandle_t DebugRxSem;

extern SemaphoreHandle_t ModBusMasterRxSem; // 信号量

//////////////////////////////////////////////////////////////////
// 加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
// 标准库需要的支持函数
struct __FILE
{
    int handle;
};

FILE __stdout;
// 定义_sys_exit()以避免使用半主机模式
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
            ;                              // USART_GetFlagStatus：得到发送状态位
                                           // USART_FLAG_TXE:发送寄存器为空 1：为空；0：忙状态
        USART_SendData(DEBUG_USART, Data); // 发送一个字符
    }
    return Data;
}
#endif

// 初始化IO 串口1
//  bound:波特率
void uart1_init(u32 bound)
{
    // GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // 使能GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // 使能USART1时钟

    // 串口1对应引脚复用映射
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);  // GPIOB6复用为USART1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); // GPIOB7复用为USART1

    // USART1端口配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速度50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      // 上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART1 初始化设置
    USART_InitStructure.USART_BaudRate = bound;                                     // 波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式
    USART_Init(USART1, &USART_InitStructure);                                       // 初始化串口1

    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);                   // 使能空闲中断
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);                     // 使能发送完成中断
    USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE); // 支持DMA 发送和接收
    USART_Cmd(USART1, ENABLE);                                       // 使能串口1

    // Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;         // 串口1中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                           // 根据指定的参数初始化VIC寄存器、
}

void uart2_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // 使能GPIO时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); // 使能USART2时钟

    // 串口2引脚复用映射
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); // GPIOA2复用为USART2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); // GPIOA3复用为USART2

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; // GPIOA2与GPIOA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;           // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;     // 速度100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;         // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;           // 上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 // 初始化PA2，PA3

    // USART2 初始化设置
    USART_InitStructure.USART_BaudRate = bound;                                     // 波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式
    USART_Init(USART2, &USART_InitStructure);                                       // 初始化串口2

    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);                   // 使能空闲中断
    USART_ITConfig(USART2, USART_IT_TC, ENABLE);                     // 使能发送完成中断
    USART_DMACmd(USART2, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE); // 支持DMA 发送和接收
    USART_Cmd(USART2, ENABLE);                                       // 使能串口 2

    // Usart2 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                           // 根据指定的参数初始化VIC寄存器、
}

void uart3_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  // 使能GPIO时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); // 使能USART3时钟

    // 串口3引脚复用映射
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3); // GPIOB10复用为USART3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3); // GPIOB11复用为USART3

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; // GPIOB10与GPIOB11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;             // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;       // 速度100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;           // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;             // 上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);                   // 初始化PB10，PB11

    // USART3 初始化设置
    USART_InitStructure.USART_BaudRate = bound;                                     // 波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式
    USART_Init(USART3, &USART_InitStructure);                                       // 初始化串口3

    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);                   // 使能空闲中断
    USART_ITConfig(USART3, USART_IT_TC, ENABLE);                     // 使能发送完成中断
    USART_DMACmd(USART3, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE); // 支持DMA 发送和接收
    USART_Cmd(USART3, ENABLE);                                       // 使能串口 3

    // Usart NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                           // 根据指定的参数初始化VIC寄存器、
}

// 初始化IO 串口4
//  bound:波特率
void uart4_init(u32 bound)
{
    // GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // 使能GPIO时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE); // 使能UART4时钟

    // 串口4对应引脚复用映射
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4); // GPIOA0复用为UART4
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4); // GPIOA1复用为UART4

    // UART4端口配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; // GPIOA0与GPIOA1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;           // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      // 速度50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;         // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;           // 上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);                 // 初始化PA0，PA1

    // UART4 初始化设置
    USART_InitStructure.USART_BaudRate = bound;                                     // 波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式
    USART_Init(UART4, &USART_InitStructure);                                        // 初始化串口1

    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);                   // 使能空闲中断
    USART_ITConfig(UART4, USART_IT_TC, ENABLE);                     // 使能发送完成中断
    USART_DMACmd(UART4, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE); // 支持DMA 发送和接收
    USART_Cmd(UART4, ENABLE);                                       // 使能串口4

    // Usart NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;          // 串口4中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                           // 根据指定的参数初始化VIC寄存器、
}

void uart6_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);  // 使能GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); // 使能USART6时钟

    // 串口6引脚复用映射
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6); // 复用为USART6
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6); // 复用为USART6

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;       // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 速度100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // 上拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 485设置
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); // 使能GPIO时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 速度100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // 上拉
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    U6_RE;

    // USART6 初始化设置
    USART_InitStructure.USART_BaudRate = bound;                                     // 波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为9位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 不校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式
    USART_Init(USART6, &USART_InitStructure);                                       // 初始化串口2

    USART_ITConfig(USART6, USART_IT_IDLE, ENABLE);                   // 使能空闲中断
    USART_ITConfig(USART6, USART_IT_TC, ENABLE);                     // 使能发送完成中断
    USART_DMACmd(USART6, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE); // 支持DMA 发送和接收
    USART_Cmd(USART6, ENABLE);                                       // 使能串口

    // Usart NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                           // 根据指定的参数初始化VIC寄存器
}

void UART1_INIT(void)
{
    uart1_init(115200);
    /*****发送***************/
    MYDMA_Send_Config(DMA2_Stream7, DMA_Channel_4, (u32)&USART1->DR, (u32)USART1_TX_BUF, 0); // DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
    /*****接收***************/
    MYDMA_Receive_Config(DMA2_Stream5, DMA_Channel_4, (u32)&USART1->DR, (u32)USART1_RX_BUF, USART1_RX_LEN); // DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
                                                                                                            // 接收FIFO初始化
    Fifo_Init(&Uart1_Rx_Fifo, USART1_RX_BUF, USART1_RX_LEN);                                                // FIFO初始化，接收缓存以环形FIFO形式管理

    // 发送信号量初始化
    Uart1TxSem = xSemaphoreCreateBinary();
    xSemaphoreGive(Uart1TxSem); // 释放发送信号量

    Uart1TxWaitSem = xSemaphoreCreateBinary();
}

void UART2_INIT(void)
{
    uart2_init(115200);
    /*****发送***************/
    MYDMA_Send_Config(DMA1_Stream6, DMA_Channel_4, (u32)&USART2->DR, (u32)USART2_TX_BUF, 0); // DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
    /*****接收***************/
    MYDMA_Receive_Config(DMA1_Stream5, DMA_Channel_4, (u32)&USART2->DR, (u32)USART2_RX_BUF, USART2_RX_LEN); // DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
    // 接收FIFO初始化
    Fifo_Init(&Uart2_Rx_Fifo, USART2_RX_BUF, USART2_RX_LEN);

    // 发送信号量初始化
    Uart2TxSem = xSemaphoreCreateBinary();
    xSemaphoreGive(Uart2TxSem); // 释放发送信号量

    Uart2TxWaitSem = xSemaphoreCreateBinary();
}

void UART3_INIT(void)
{
    uart3_init(115200);
    /*****发送***************/
    MYDMA_Send_Config(DMA1_Stream3, DMA_Channel_4, (u32)&USART3->DR, (u32)USART3_TX_BUF, 0); // DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
    /*****接收***************/
    MYDMA_Receive_Config(DMA1_Stream1, DMA_Channel_4, (u32)&USART3->DR, (u32)USART3_RX_BUF, USART3_RX_LEN); // DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
    // 接收FIFO初始化
    Fifo_Init(&Uart3_Rx_Fifo, USART3_RX_BUF, USART3_RX_LEN);

    // 发送信号量初始化
    Uart3TxSem = xSemaphoreCreateBinary();
    xSemaphoreGive(Uart3TxSem); // 释放发送信号量

    Uart3TxWaitSem = xSemaphoreCreateBinary();
}

void UART4_INIT(void)
{
    uart4_init(115200);
    /*****发送***************/
    MYDMA_Send_Config(DMA1_Stream4, DMA_Channel_4, (u32)&UART4->DR, (u32)UART4_TX_BUF, 0); // DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
    /*****接收***************/
    MYDMA_Receive_Config(DMA1_Stream2, DMA_Channel_4, (u32)&UART4->DR, (u32)UART4_RX_BUF, UART4_RX_LEN); // DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
    // 接收FIFO初始化
    Fifo_Init(&Uart4_Rx_Fifo, UART4_RX_BUF, UART4_RX_LEN);

    // 发送信号量初始化
    Uart4TxSem = xSemaphoreCreateBinary();
    xSemaphoreGive(Uart4TxSem); // 释放发送信号量

    Uart4TxWaitSem = xSemaphoreCreateBinary();
}

void UART6_INIT(void)
{
    uart6_init(9600);
    /*****发送***************/
    MYDMA_Send_Config(DMA2_Stream6, DMA_Channel_5, (u32)&USART6->DR, (u32)USART6_TX_BUF, 0); // DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
    /*****接收***************/
    MYDMA_Receive_Config(DMA2_Stream2, DMA_Channel_5, (u32)&USART6->DR, (u32)USART6_RX_BUF, USART6_RX_LEN); // DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
    // 接收FIFO初始化
    Fifo_Init(&Uart6_Rx_Fifo, USART6_RX_BUF, USART6_RX_LEN);

    // 发送信号量初始化
    Uart6TxSem = xSemaphoreCreateBinary();
    xSemaphoreGive(Uart6TxSem); // 释放发送信号量

    Uart6TxWaitSem = xSemaphoreCreateBinary();
}

void Uart_1_Send(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }

    xSemaphoreTake(Uart1TxSem, 3000); // 等待发送信号量，阻止多线程访问
    xSemaphoreTake(Uart1TxWaitSem, 0);

    DMA_Cmd(DMA2_Stream7, DISABLE); // 关闭DMA传输
    while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE)
    {
    }                                          // 确保DMA可以被设置
    DMA_SetCurrDataCounter(DMA2_Stream7, len); // 数据传输量
    DMA2_Stream7->M0AR = (uint32_t)buf;        // 传输数组
    DMA_Cmd(DMA2_Stream7, ENABLE);             // 开启DMA传输

    xSemaphoreTake(Uart1TxWaitSem, 3000);
    DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7); // 清除DMA2_Steam7传输完成标志
    DMA_Cmd(DMA2_Stream7, DISABLE);              // 关闭串口发送通道

    xSemaphoreGive(Uart1TxSem); // 释放发送信号量
}

void Uart_2_Send(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }

    xSemaphoreTake(Uart2TxSem, 3000); // 等待发送信号量，阻止多线程访问
    xSemaphoreTake(Uart2TxWaitSem, 0);

    DMA_Cmd(DMA1_Stream6, DISABLE); // 关闭DMA传输
    while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE)
    {
    }                                          // 确保DMA可以被设置
    DMA_SetCurrDataCounter(DMA1_Stream6, len); // 数据传输量
    DMA1_Stream6->M0AR = (uint32_t)buf;        // 传输数组
    DMA_Cmd(DMA1_Stream6, ENABLE);             // 开启DMA传输

    xSemaphoreTake(Uart2TxWaitSem, 3000);
    DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6); // 清除DMA2_Steam7传输完成标
    DMA_Cmd(DMA1_Stream6, DISABLE);              // 关闭串口发送通道

    xSemaphoreGive(Uart2TxSem); // 释放发送信号量
}

void Uart_3_Send(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }

    xSemaphoreTake(Uart3TxSem, 3000); // 等待发送信号量，阻止多线程访问
    xSemaphoreTake(Uart3TxWaitSem, 0);

    DMA_Cmd(DMA1_Stream3, DISABLE); // 关闭DMA传输
    while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE)
    {
    }                                          // 确保DMA可以被设置
    DMA_SetCurrDataCounter(DMA1_Stream3, len); // 数据传输量
    DMA1_Stream3->M0AR = (uint32_t)buf;        // 传输数组
    DMA_Cmd(DMA1_Stream3, ENABLE);             // 开启DMA传输

    xSemaphoreTake(Uart3TxWaitSem, 3000);
    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3); // 清除DMA1_Steam3传输完成标
    DMA_Cmd(DMA1_Stream3, DISABLE);              // 关闭串口发送通道

    xSemaphoreGive(Uart3TxSem); // 释放发送信号量
}

void Uart_4_Send(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }

    xSemaphoreTake(Uart4TxSem, 3000); // 等待发送信号量，阻止多线程访问
    xSemaphoreTake(Uart4TxWaitSem, 0);

    DMA_Cmd(DMA1_Stream4, DISABLE); // 关闭DMA传输
    while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE)
    {
    }                                          // 确保DMA可以被设置
    DMA_SetCurrDataCounter(DMA1_Stream4, len); // 数据传输量
    DMA1_Stream4->M0AR = (uint32_t)buf;        // 传输数组
    DMA_Cmd(DMA1_Stream4, ENABLE);             // 开启DMA传输

    xSemaphoreTake(Uart4TxWaitSem, 3000);
    DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4); // 清除DMA2_Steam7传输完成标
    DMA_Cmd(DMA1_Stream4, DISABLE);              // 关闭串口发送通道

    xSemaphoreGive(Uart4TxSem); // 释放发送信号量
}

void Uart_6_Send(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
    {
        return;
    }

    xSemaphoreTake(Uart6TxSem, 3000); // 等待发送信号量，阻止多线程访问
    xSemaphoreTake(Uart6TxWaitSem, 0);

    U6_DE;
    delay_ms(1);
    DMA_Cmd(DMA2_Stream6, DISABLE); // 关闭DMA传输
    while (DMA_GetCmdStatus(DMA2_Stream6) != DISABLE)
    {
    }                                          // 确保DMA可以被设置
    DMA_SetCurrDataCounter(DMA2_Stream6, len); // 数据传输量
    DMA2_Stream6->M0AR = (uint32_t)buf;        // 传输数组
    DMA_Cmd(DMA2_Stream6, ENABLE);             // 开启DMA传输

    xSemaphoreTake(Uart6TxWaitSem, 3000);
    DMA_ClearFlag(DMA2_Stream6, DMA_FLAG_TCIF6);
    DMA_Cmd(DMA2_Stream6, DISABLE); // 关闭串口发送通道
    U6_RE;

    xSemaphoreGive(Uart6TxSem); // 释放发送信号量
}

/*
 * 函数名：Uart_1_Select
 * 描述  ：串口1接收等待
 * 输入  ：delay---等待时间，单位ms
 * 输出  ：待接收数据长度
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
 * 函数名：Uart_2_Select
 * 描述  ：串口1接收等待
 * 输入  ：delay---等待时间，单位ms
 * 输出  ：待接收数据长度
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
 * 函数名：Uart_3_Select
 * 描述  ：串口1接收等待
 * 输入  ：delay---等待时间，单位ms
 * 输出  ：待接收数据长度
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
 * 函数名：Uart_4_Select
 * 描述  ：串口1接收等待
 * 输入  ：delay---等待时间，单位ms
 * 输出  ：待接收数据长度
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
 * 函数名：Uart_4_Select
 * 描述  ：串口1接收等待
 * 输入  ：delay---等待时间，单位ms
 * 输出  ：待接收数据长度
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
 * 函数名：Uart_1_Get
 * 描述  ：串口1接收
 * 输入  ：buffer---接收缓存，len---接收缓存长度
 * 输出  ：接收数据长度
 */
uint16_t Uart_1_Get(uint8_t *buffer, uint16_t len)
{
    return Fifo_Get(&Uart1_Rx_Fifo, buffer, len);
}

/*
 * 函数名：Uart_2_Get
 * 描述  ：串口1接收
 * 输入  ：buffer---接收缓存，len---接收缓存长度
 * 输出  ：接收数据长度
 */
uint16_t Uart_2_Get(uint8_t *buffer, uint16_t len)
{
    return Fifo_Get(&Uart2_Rx_Fifo, buffer, len);
}

/*
 * 函数名：Uart_3_Get
 * 描述  ：串口1接收
 * 输入  ：buffer---接收缓存，len---接收缓存长度
 * 输出  ：接收数据长度
 */
uint16_t Uart_3_Get(uint8_t *buffer, uint16_t len)
{
    return Fifo_Get(&Uart3_Rx_Fifo, buffer, len);
}

/*
 * 函数名：Uart_4_Get
 * 描述  ：串口4接收
 * 输入  ：buffer---接收缓存，len---接收缓存长度
 * 输出  ：接收数据长度
 */
uint16_t Uart_4_Get(uint8_t *buffer, uint16_t len)
{
    return Fifo_Get(&Uart4_Rx_Fifo, buffer, len);
}

/*
 * 函数名：Uart_4_Get
 * 描述  ：串口4接收
 * 输入  ：buffer---接收缓存，len---接收缓存长度
 * 输出  ：接收数据长度
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

// 上位机串口中断
void USART1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) // 空闲中断
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
    if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) // 空闲中断
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
    if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) // 空闲中断
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
    if (USART_GetITStatus(UART4, USART_IT_IDLE) != RESET) // 空闲中断
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
    if (USART_GetITStatus(USART6, USART_IT_IDLE) != RESET) // 空闲中断
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
// DMAx的各通道配置
// 这里的传输形式是固定的,这点要根据不同的情况来修改
// 从存储器->外设模式/8位数据宽度/存储器增量模式
// DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
// chx:DMA通道选择,@ref DMA_channel DMA_Channel_0~DMA_Channel_7
// par:外设地址
// mar:存储器地址
// ndtr:数据传输量
void MYDMA_Send_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr)
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
    DMA_InitStructure.DMA_Channel = chx;                                    // 通道选择
    DMA_InitStructure.DMA_PeripheralBaseAddr = par;                         // DMA外设地址
    DMA_InitStructure.DMA_Memory0BaseAddr = mar;                            // DMA 存储器0地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                 // 存储器到外设模式
    DMA_InitStructure.DMA_BufferSize = ndtr;                                // 数据传输量
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // 外设非增量模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // 存储器增量模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据长度:8位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // 存储器数据长度:8位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // 使用普通模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   // 中等优先级
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;         // 存储器突发单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // 外设突发单次传输
    DMA_Init(DMA_Streamx, &DMA_InitStructure);                          // 初始化DMA Stream
    DMA_Cmd(DMA_Streamx, DISABLE);
}

void MYDMA_Receive_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr)
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
    DMA_InitStructure.DMA_Channel = chx;                                    // 通道选择
    DMA_InitStructure.DMA_PeripheralBaseAddr = par;                         // DMA外设地址
    DMA_InitStructure.DMA_Memory0BaseAddr = mar;                            // DMA 存储器0地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                 // 外设模式到存储器
    DMA_InitStructure.DMA_BufferSize = ndtr;                                // 数据传输量
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // 外设非增量模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // 存储器增量模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据长度:8位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // 存储器数据长度:8位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         // 使用普通模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   // 中等优先级
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;         // 存储器突发单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // 外设突发单次传输
    DMA_Init(DMA_Streamx, &DMA_InitStructure);                          // 初始化DMA Stream

    DMA_Cmd(DMA_Streamx, ENABLE);
}

// 开启一次DMA传输
//  DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
//  ndtr:数据传输量
void MYDMA_Enable(const uint8_t *buf, DMA_Stream_TypeDef *DMA_Streamx, u16 ndtr)
{
    DMA_Cmd(DMA_Streamx, DISABLE); // 关闭DMA传输

    while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
    {
    } // 确保DMA可以被设置

    DMA_SetCurrDataCounter(DMA_Streamx, ndtr); // 数据传输量

    DMA_Streamx->M0AR = (uint32_t)buf; // 传输数组

    DMA_Cmd(DMA_Streamx, ENABLE); // 开启DMA传输
}
