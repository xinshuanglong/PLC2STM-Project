#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "stmflash.h"
#include "iap.h"

#include "bsp_cpu.h"
#include "bsp_iwdg.h"
#include "bsp_led.h"
#include "bsp_tim.h"
#include "bsp_wifi.h"

#include "app_cmd.h"

#include "config.h"


void Bsp_Init(void);


int main(void)
{
  Boot2App();

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组
  delay_init(168);    //延时初始化
  UART_INIT();        //串口初始化波特率为115200
  IWDG_Init();

  Log(VERBOSE,"Enter Bootload Mode\r\n");
  Log(VERBOSE,"Bootload Version: %s\r\n",VERSION);

  Bsp_Init();

  while(1){
    CmdTask();
  }
  
}


void Bsp_Init(void)
{
  CPU_ID();
  Led_Init();
  TIM3_Int_Init(10000,840);
}



