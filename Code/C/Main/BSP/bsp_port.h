#ifndef __BSP_PORT_H
#define __BSP_PORT_H

#define POWER_INDEX_START 0
#define VALVE_INDEX_START 32
#define MOTO_INDEX_START 80

#define CTL_INDEX_COUNT 96

#define IO_INDEX_START 96
#define IO_INDEX_COUNT 64 // 必须为32整数倍

#pragma pack(1)
typedef struct
{
    u16 powerModule;    // 模块
    u16 powerRegular;   // 常规
    u16 powerBallValve; // 球阀
    u32 powerValve;     // 电磁阀
    u16 powerPower;     // 动力(电机,抽水泵等)
} _BoardCtl;

typedef struct
{
    u32 digitalIO1;        // IO状态(烟雾，红外，震动等)
    u32 digitalIO2;        // 球阀状态
    float digitalValue[8]; // 电流电压值(数字量)
} _BoardRead;

typedef struct
{
    _BoardCtl ctl;
    _BoardRead read;
} _Board;

#pragma pack()

typedef struct
{
    int index;    // 序号
    char log[32]; // 描述
    void (*init)(void);
    void (*write)(int, u8);
    u8 (*read)(int);
} Gpio_Strategy;

extern volatile _Board gBoardSubRead;
extern volatile _Board gBoardSubWrite;

void Port_Init(void);
bool Port_Write(int index, u8 opt);
bool Port_WriteAll(u8 opt);
bool Port_Read(int index, u8 *opt);
float Digita_Read(int index);

#endif
