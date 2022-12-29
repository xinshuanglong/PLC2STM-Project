#include "bsp_include.h"

volatile u32 gIoStateInBit[IO_INDEX_COUNT / 32];
volatile bool gIoState[IO_INDEX_COUNT];
volatile _Board gBoardState;

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
    memset((void *)gIoStateInBit, 0, sizeof(gIoStateInBit));
    memset((void *)gIoState, 0, sizeof(gIoState));

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

        // 更新主副板状态
        for (int i = 0; i < CTL_INDEX_COUNT; i++)
        {
            u8 res = 0;
            if (Port_Read(POWER_INDEX_START + i, &res) == true)
            {
                uint8_t *ctl = (uint8_t *)&gBoardState.ctl;
                int a = i / 8;
                int b = i % 8;
                if (res == 0)
                {
                    ctl[a] &= ~(1 << b);
                }
                else
                {
                    ctl[a] |= (1 << b);
                }
            }
        }

        gBoardState.read.digitalIO1 = gIoStateInBit[0];
        gBoardState.read.digitalIO2 = gIoStateInBit[1];
        for (int i = 0; i < 8; i++)
        {
            gBoardState.read.digitalValue[i] = Digita_Read(i);
        }

        vTaskDelay(100);
    }
}

bool Port_Read_Filter(int index)
{
    uint8_t *read = (uint8_t *)&gBoardState;
    int a = index / 8;
    int b = index % 8;
    uint8_t io = read[a] & (1 << b);

    return (bool)(io != 0);
}
