#include "bsp_include.h"

uint8_t ucHeap[configTOTAL_HEAP_SIZE] _AT_CCMRAM_SEC;

// 任务优先级
#define START_TASK_PRIO 1
// 任务堆栈大小
#define START_STK_SIZE 256
// 任务句柄
TaskHandle_t StartTask_Handler;
// 任务函数
void start_task(void *pvParameters);

void Bsp_Init(void);
void PeriphReset(void);

int main(void)
{
  SCB->VTOR = APP_FLASH_ADDR;

  debugLevel = 1;
  PeriphReset();
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // 设置系统中断优先级分组4
  delay_init(168);                                // 初始化延时函数
  delay_ms(100);                                  // 等待外部电源稳定，在进行其他初始化工作
  UART_INIT();                                    // 初始化串口

  printf("\r\n\r\n");
  printf("-------- System Reboot --------\r\n");
  printf("APP Version: %s\r\n", VERSION);
  printf("-------------------------------\r\n");

  Bsp_Init();

  // IWDG_Init();


  // 创建开始任务
  xTaskCreate((TaskFunction_t)start_task,          // 任务函数
              (const char *)"start_task",          // 任务名称
              (uint16_t)START_STK_SIZE,            // 任务堆栈大小
              (void *)NULL,                        // 传递给任务函数的参数
              (UBaseType_t)START_TASK_PRIO,        // 任务优先级
              (TaskHandle_t *)&StartTask_Handler); // 任务句柄
  vTaskStartScheduler();                           // 开启任务调度
}

// 开始任务任务函数
void start_task(void *pvParameters)
{
  taskENTER_CRITICAL(); // 进入临界区

  DebugPrintMsgTaskCreat();
  DebugCmdParseTaskCreat();
  IwdgTaskCreat();
  EnvTaskCreat();
  AdcTaskCreat();

  PortTaskCreat();
  ComSubTaskCreat();
  SDTaskCreat();
  CmdTaskCreat();
  MeasureTaskCreat();
  BushuiTaskCreat();
  BaohuTaskCreat();
  BushuiAndanTaskCreat();

  taskEXIT_CRITICAL(); // 退出临界区

  // vTaskDelete(StartTask_Handler); //删除开始任务
  // vTaskDelay(portMAX_DELAY);

  while (1)
  {
    if (gSysReboot)
    {
      vTaskDelay(1000);
      SysReboot();
    }
    vTaskDelay(1000);
  }
}

void Bsp_Init(void)
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CCMDATARAMEN, ENABLE);

  Adc_Init();
  TIM3_Int_Init(100, 84); // 用于FreeRTOS运行统计
  ModBus_Master_Init();
  SHT2X_Init();
  SHT2x_SoftReset();
  MY_RTC_Init();
  Alarm_Init();
  Port_Init();

  SDCardMount();
}

void PeriphReset(void)
{
  RCC_AHB1PeriphResetCmd(0xFFFFFFFF, ENABLE);
  RCC_AHB2PeriphResetCmd(0xFFFFFFFF, ENABLE);
  RCC_APB1PeriphResetCmd(0xFFFFFFFF, ENABLE);
  RCC_APB2PeriphResetCmd(0xFFFFFFFF, ENABLE);

  RCC_AHB1PeriphResetCmd(0xFFFFFFFF, DISABLE);
  RCC_AHB2PeriphResetCmd(0xFFFFFFFF, DISABLE);
  RCC_APB1PeriphResetCmd(0xFFFFFFFF, DISABLE);
  RCC_APB2PeriphResetCmd(0xFFFFFFFF, DISABLE);
}

void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  debugLevel = 1;
  printf("HardFault_Handler\r\n");
  printf("HardFault_Handler\r\n");
  printf("HardFault_Handler\r\n");
  printf("HardFault_Handler\r\n");
  printf("HardFault_Handler\r\n");
  while (1)
  {
  }
}
