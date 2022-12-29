#include "bsp_include.h"

#define CMD_RX_SIZE 2048
#define CMD_TX_SIZE 2048
static uint8_t CMD_RxBuffer[CMD_RX_SIZE];
static uint8_t CMD_TxBuffer[CMD_TX_SIZE];

uint16_t cmdRxLen = 0;
uint16_t cmdTxLen = 0;

SemaphoreHandle_t CmdRxSemCount = NULL; // �������ź���

u8 gSysReboot = 0;

/***********************************************/
// �����ջ��С
#define CMD_TASK_STK_SIZE 1024
// ������
TaskHandle_t CmdTaskHandler;
// ������
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

    CmdRxSemCount = xSemaphoreCreateCounting(10, 0); // �ź������ֵ10����ʼֵ0
}

void CmdTask(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(CmdRxSemCount, portMAX_DELAY) == pdPASS)
        {
            memset(CMD_RxBuffer, 0, CMD_RX_SIZE); // �������
            memset(CMD_TxBuffer, 0, CMD_TX_SIZE); // �������

            cmdRxLen = Uart_Get(CMD_USART, CMD_RxBuffer, CMD_RX_SIZE); // ���ȴ�������
            if (cmdRxLen >= 5)
            { // ���յ�����
                // ���ݴ���
                cmdTxLen = ModBus_Protocol(CMD_RxBuffer, cmdRxLen, CMD_TxBuffer);
                delay_ms(25);
                Uart_Send(CMD_USART, CMD_TxBuffer, cmdTxLen);
            }
        }
    }
}
