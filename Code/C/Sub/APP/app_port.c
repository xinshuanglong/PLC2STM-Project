#include "bsp_include.h"

u32 gIoStateInBit[IO_INDEX_COUNT / 32];
bool gIoState[IO_INDEX_COUNT];

volatile _Board gBoardOpt;

/***********************************************/
// 任务堆栈大小
#define PORT_TASK_STK_SIZE 256
// 任务句柄
TaskHandle_t PortTaskHandler;
// 任务函数
void PortTask(void *pvParameters);
/***********************************************/

void PortTaskCreat(void)
{
    xTaskCreate(PortTask,
                "PortTask",
                PORT_TASK_STK_SIZE,
                NULL,
                PORT_TASK_PRIO,
                PortTaskHandler);
}

#define FILTER 5
void PortTask(void *pvParameters)
{
    memset(gIoStateInBit, 0, sizeof(gIoStateInBit));
    memset(gIoState, 0, sizeof(gIoState));
    memset((void *)&gBoardOpt, 0, sizeof(_Board));

    int IoReadCount[IO_INDEX_COUNT] = {0};
    while (1)
    {
        // 更新Master IO read状态
        for (int i = 0; i < IO_INDEX_COUNT; i++)
        {
            u8 res = 0;
            if (Port_Read(IO_INDEX_START + i, &res) == true)
            {
                if (res == 0)
                    IoReadCount[i]--;
                else
                    IoReadCount[i]++;

                IoReadCount[i] = IoReadCount[i] < -FILTER ? -FILTER : IoReadCount[i];
                IoReadCount[i] = IoReadCount[i] > FILTER ? FILTER : IoReadCount[i];
            }

            if (IoReadCount[i] >= FILTER)
            {
                gIoStateInBit[i / 32] |= (1 << (i % 32));
                gIoState[i] = true;
            }

            if (IoReadCount[i] <= -FILTER)
            {
                gIoStateInBit[i / 32] &= ~(1 << (i % 32));
                gIoState[i] = false;
            }
        }
        // IO状态填充至gBoardOpt
        gBoardOpt.read.digitalIO1 = gIoStateInBit[0];
        gBoardOpt.read.digitalIO2 = gIoStateInBit[1];

        // 填充ADC至gBoardOpt
        for (int i = 0; i < 4; i++)
        {
            gBoardOpt.read.digitalValue[4 + i] = gAdcKfp[i] / 4096.0f * 20;
        }

        // gBoardOpt控制部分，写至IO
        for (int i = 0; i < CTL_INDEX_END; i++)
        {
            uint8_t *ctl = (uint8_t *)&gBoardOpt.ctl;
            int a = i / 8;
            int b = i % 8;
            uint8_t io = (ctl[a] & (1 << b)) != 0;
            Port_Write(i, io);
        }

        vTaskDelay(100);
    }
}
