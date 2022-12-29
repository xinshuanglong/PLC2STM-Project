#include "bsp_include.h"

#define CMD_RX_SIZE 2048
#define CMD_TX_SIZE 2048
static uint8_t CMD_RxBuffer[CMD_RX_SIZE];
static uint8_t CMD_TxBuffer[CMD_TX_SIZE];

uint16_t cmdRxLen = 0;
uint16_t cmdTxLen = 0;

SemaphoreHandle_t CmdRxSemCount = NULL; // 计数型信号量

u8 gSysReboot = 0;

/***********************************************/
// 任务堆栈大小
#define CMD_TASK_STK_SIZE 1024
// 任务句柄
TaskHandle_t CmdTaskHandler;
// 任务函数
void CmdTask(void *pvParameters);
/***********************************************/

void CmdTaskCreat(void)
{
    xTaskCreate(CmdTask,
                "CmdTask",
                CMD_TASK_STK_SIZE,
                NULL,
                CMD_TASK_PRIO,
                CmdTaskHandler);

    CmdRxSemCount = xSemaphoreCreateCounting(10, 0); // 信号量最大值10，初始值0
}

void CmdTask(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(CmdRxSemCount, portMAX_DELAY) == pdPASS)
        {
            memset(CMD_RxBuffer, 0, CMD_RX_SIZE); // 清空数据
            memset(CMD_TxBuffer, 0, CMD_TX_SIZE); // 清空数据

            cmdRxLen = Uart_Get(CMD_USART, CMD_RxBuffer, CMD_RX_SIZE); // 首先串口数据
            if (cmdRxLen >= 5)
            { // 接收到数据
                // 数据处理
                cmdTxLen = ModBus_Protocol(CMD_RxBuffer, cmdRxLen, CMD_TxBuffer);
                delay_ms(25);
                Uart_Send(CMD_USART, CMD_TxBuffer, cmdTxLen);
            }
        }
    }
}
