#ifndef __BSP_PORT_H
#define __BSP_PORT_H

#define POWER_INDEX_START 0
#define VALVE_INDEX_START 32
#define MOTO_INDEX_START 80

#define CTL_INDEX_END 96 // ���Ʋ��ֽ����±�

#define IO_INDEX_START 96
#define IO_INDEX_COUNT 64 // ����Ϊ32������

#pragma pack(1)
typedef struct
{
    u16 powerModule;    // ģ��
    u16 powerRegular;   // ����
    u16 powerBallValve; // ��
    u32 powerValve;     // ��ŷ�
    u16 powerPower;     // ����(���,��ˮ�õ�)
} _BoardCtl;

typedef struct
{
    u32 digitalIO1;        // IO״̬(�������⣬�𶯵�)
    u32 digitalIO2;        // ��״̬
    float digitalValue[8]; // ������ѹֵ(������)
} _BoardRead;

typedef struct
{
    _BoardCtl ctl;
    _BoardRead read;
} _Board;

#pragma pack()

typedef struct
{
    int index;    // ���
    int isCom;    // 0:ֱ�Ӱ��ؿ��ƣ�1��ͨ�ſ���
    char log[32]; // ����
    void (*init)(void);
    void (*write)(int, u8);
    u8 (*read)(int);
} Gpio_Strategy;

void Port_Init(void);
bool Port_Write(int index, u8 opt);
bool Port_Read(int index, u8 *opt);

#endif
