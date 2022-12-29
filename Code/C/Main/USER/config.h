#ifndef _CONFIG_H
#define _CONFIG_H

#define VERSION_CODE 0x100
#define VERSION "PLC2STM-Main-V1.00-2022-11-29"

// 配置地址
#define APP_CONFIG_FLASH_ADDR_START 0x0800F000
#define APP_FLASH_ADDR 0x08010000

#define FLASH_SOFTWARE_CRC_ADDR 0X0800FFFC     // 设置FLASH 保存地址(必须为4的倍数，且其值要大于本代码所占用FLASH的大小+0X08000000)
#define FLASH_SOFTWARE_LENGTH_ADDR 0X0800FFF8  // 设置FLASH 保存地址(必须为4的倍数，且其值要大于本代码所占用FLASH的大小+0X08000000)
#define FLASH_SOFTWARE_VERSION_ADDR 0X0800FF08 // 设置FLASH 保存地址(必须为4的倍数，且其值要大于本代码所占用FLASH的大小+0X08000000)

#define DEBUG_USART USART2
#define CMD_USART USART2

#define CMD_SUB USART6 // 副板通信

#define DEVICE_ADDR 21       // Device addr
#define DEVICE_SUB_ADDR 0xF1 // Device sub addr

/*********************************************************************/
//****************FLASH前面4K存放参数(不经常修改的参数)
/*********************************************************************/
// FALSH地址与大小配置
#define FLASH_SIZE (8 * 1024 * 1024)
#define FLASH_PASE_SIZE (4096)

/************************************************/
/*****************所有任务优先级配置****************/
/****任务优先级(FreeRTOS数值越大优先级越高)*********/
// 以下值大小不能超过32
typedef enum
{
    DEBUG_PRINT_MSG_TASK_PRIO = 2,
    DEBUG_CMD_PARSE_TASK_PRIO,
    SD_TASK_PRIO,
    ENV_TASK_PRIO,
    ADC_TASK_PRIO,
    LED_TASK_PRIO,
    COM_SUB_TASK_PRIO,
    PORT_TASK_PRIO,
    BUSHUI_TASK_PRIO,
    BAOHU_TASK_PRIO,
    BUSHUI_ANDAN_TASK_PRIO,
    MEASURE_TASK_PRIO,
    CMD_TASK_PRIO,
    IWDG_TASK_PRIO,
} eTaskPriority;

#endif
