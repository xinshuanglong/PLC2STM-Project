#include "bsp_include.h"

#define CMD_RX_SIZE 2048
#define CMD_TX_SIZE 2048
static uint8_t CMD_RxBuffer[CMD_RX_SIZE];

/***********************************************/
// 任务堆栈大小
#define COM_SUB_TASK_STK_SIZE 512
// 任务句柄
TaskHandle_t ComSubTaskHandler;
// 任务函数
void ComSubTask(void *pvParameters);
/***********************************************/

void ComSubTaskCreat(void)
{
    xTaskCreate(ComSubTask,
                "ComSubTask",
                COM_SUB_TASK_STK_SIZE,
                NULL,
                COM_SUB_TASK_PRIO,
                ComSubTaskHandler);
}

void ComSubTask(void *pvParameters)
{
    static int err = 0;

    while (1)
    {
        int count;
        Uart_Get(CMD_SUB, CMD_RxBuffer, CMD_RX_SIZE);
        count = ModBus_Master_03(CMD_SUB, DEVICE_SUB_ADDR, 0x2600, sizeof(_Board), (uint8_t *)&gBoardSubRead);
        err = count < 0 ? err + 1 : 0;
        vTaskDelay(100);

        Uart_Get(CMD_SUB, CMD_RxBuffer, CMD_RX_SIZE);
        count = ModBus_Master_16(CMD_SUB, DEVICE_SUB_ADDR, 0x2600, (uint8_t *)&gBoardSubWrite.ctl, sizeof(_BoardCtl));
        err = count < 0 ? err + 1 : 0;
        vTaskDelay(100);
    }
}
