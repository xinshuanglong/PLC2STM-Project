#include "bsp_include.h"

uint8_t ucHeap[configTOTAL_HEAP_SIZE] _AT_CCMRAM_SEC;

// �������ȼ�
#define START_TASK_PRIO 1
// �����ջ��С
#define START_STK_SIZE 256
// ������
TaskHandle_t StartTask_Handler;
// ������
void start_task(void *pvParameters);

void Bsp_Init(void);
void PeriphReset(void);

int main(void)
{
  SCB->VTOR = APP_FLASH_ADDR;

  debugLevel = 1;
  PeriphReset();
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); // ����ϵͳ�ж����ȼ�����4
  delay_init(168);                                // ��ʼ����ʱ����
  delay_ms(100);                                  // �ȴ��ⲿ��Դ�ȶ����ڽ���������ʼ������
  UART_INIT();                                    // ��ʼ������

  printf("\r\n\r\n");
  printf("-------- System Reboot --------\r\n");
  printf("APP Version: %s\r\n", VERSION);
  printf("-------------------------------\r\n");

  Bsp_Init();

  // IWDG_Init();


  // ������ʼ����
  xTaskCreate((TaskFunction_t)start_task,          // ������
              (const char *)"start_task",          // ��������
              (uint16_t)START_STK_SIZE,            // �����ջ��С
              (void *)NULL,                        // ���ݸ��������Ĳ���
              (UBaseType_t)START_TASK_PRIO,        // �������ȼ�
              (TaskHandle_t *)&StartTask_Handler); // ������
  vTaskStartScheduler();                           // �����������
}

// ��ʼ����������
void start_task(void *pvParameters)
{
  taskENTER_CRITICAL(); // �����ٽ���

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

  taskEXIT_CRITICAL(); // �˳��ٽ���

  // vTaskDelete(StartTask_Handler); //ɾ����ʼ����
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
  TIM3_Int_Init(100, 84); // ����FreeRTOS����ͳ��
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
