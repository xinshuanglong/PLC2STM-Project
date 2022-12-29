#include "bsp_include.h"

volatile _Board gBoardSubRead;
volatile _Board gBoardSubWrite;

// xx:A0,B1,F10,G7等
#define GPIO_DEF(xx)                                  \
    void GPIO_##xx##_Init(void)                       \
    {                                                 \
        Gpio_Bit_Init(&GPIO_##xx);                    \
        if (GPIO_##xx.reverse == true)                \
            Gpio_Set_Bit(&GPIO_##xx);                 \
    }                                                 \
    void GPIO_##xx##_Write(int index, u8 opt)         \
    {                                                 \
        if (GPIO_##xx.reverse == false)               \
        {                                             \
            if (opt == 0)                             \
                Gpio_Reset_Bit(&GPIO_##xx);           \
            else                                      \
                Gpio_Set_Bit(&GPIO_##xx);             \
        }                                             \
        else                                          \
        {                                             \
            if (opt != 0)                             \
                Gpio_Reset_Bit(&GPIO_##xx);           \
            else                                      \
                Gpio_Set_Bit(&GPIO_##xx);             \
        }                                             \
    }                                                 \
    u8 GPIO_##xx##_Read_Input(int index)              \
    {                                                 \
        if (GPIO_##xx.reverse == true)                \
            return !Gpio_Read_Input_Bit(&GPIO_##xx);  \
        else                                          \
            return Gpio_Read_Input_Bit(&GPIO_##xx);   \
    }                                                 \
    u8 GPIO_##xx##_Read_Output(int index)             \
    {                                                 \
        if (GPIO_##xx.reverse == true)                \
            return !Gpio_Read_Output_Bit(&GPIO_##xx); \
        else                                          \
            return Gpio_Read_Output_Bit(&GPIO_##xx);  \
    }

// IO输出(高电平使能)
const Gpio_Type GPIO_F0 = {GPIOF, GPIO_Pin_0, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F1 = {GPIOF, GPIO_Pin_1, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F2 = {GPIOF, GPIO_Pin_2, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F3 = {GPIOF, GPIO_Pin_3, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F4 = {GPIOF, GPIO_Pin_4, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F5 = {GPIOF, GPIO_Pin_5, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F6 = {GPIOF, GPIO_Pin_6, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F7 = {GPIOF, GPIO_Pin_7, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F8 = {GPIOF, GPIO_Pin_8, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F9 = {GPIOF, GPIO_Pin_9, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F10 = {GPIOF, GPIO_Pin_10, GPIO_Mode_OUT, true};

const Gpio_Type GPIO_G7 = {GPIOG, GPIO_Pin_7, GPIO_Mode_OUT, true};

// IO读取(高电平异常)
const Gpio_Type GPIO_E0 = {GPIOE, GPIO_Pin_0, GPIO_Mode_IN, true};
const Gpio_Type GPIO_E1 = {GPIOE, GPIO_Pin_1, GPIO_Mode_IN, true};
const Gpio_Type GPIO_E2 = {GPIOE, GPIO_Pin_2, GPIO_Mode_IN, true};
const Gpio_Type GPIO_E3 = {GPIOE, GPIO_Pin_3, GPIO_Mode_IN, true};
const Gpio_Type GPIO_E4 = {GPIOE, GPIO_Pin_4, GPIO_Mode_IN, true};
const Gpio_Type GPIO_E5 = {GPIOE, GPIO_Pin_5, GPIO_Mode_IN, true};
const Gpio_Type GPIO_E6 = {GPIOE, GPIO_Pin_6, GPIO_Mode_IN, true};
const Gpio_Type GPIO_E7 = {GPIOE, GPIO_Pin_7, GPIO_Mode_IN, true};

GPIO_DEF(F0);
GPIO_DEF(F1);
GPIO_DEF(F2);
GPIO_DEF(F3);
GPIO_DEF(F4);
GPIO_DEF(F5);
GPIO_DEF(F6);
GPIO_DEF(F7);
GPIO_DEF(F8);
GPIO_DEF(F9);
GPIO_DEF(F10);

GPIO_DEF(G7);

GPIO_DEF(E0);
GPIO_DEF(E1);
GPIO_DEF(E2);
GPIO_DEF(E3);
GPIO_DEF(E4);
GPIO_DEF(E5);
GPIO_DEF(E6);
GPIO_DEF(E7);

void GPIO_Write_Sub(int index, u8 opt)
{
    uint8_t *ctl = (uint8_t *)&gBoardSubWrite.ctl;
    int a = index / 8;
    int b = index % 8;
    if (opt == 0)
        ctl[a] &= ~(1 << b);
    else
        ctl[a] |= (1 << b);
}

u8 GPIO_Read_Sub(int index)
{
    uint8_t *read = (uint8_t *)&gBoardSubRead.ctl;
    int a = index / 8;
    int b = index % 8;
    uint8_t io = read[a] & (1 << b);

    return io != 0;
}

// index:0-15   模块电源
// index:16-31  常规电源
// index:32-63  阀电源
// index:64-79  动力电源(潜水泵，采水泵等需要延时动作)
const Gpio_Strategy gIOCtl[] = {
    {
        .index = 1,
        .log = "多参数电源",
        .init = GPIO_G7_Init,
        .write = GPIO_G7_Write,
        .read = GPIO_G7_Read_Output,
    },
    {
        .index = 8,
        .log = "氨氮电源",
        .init = GPIO_F0_Init,
        .write = GPIO_F0_Write,
        .read = GPIO_F0_Read_Output,
    },
    {
        .index = 9,
        .log = "总磷电源",
        .init = GPIO_F1_Init,
        .write = GPIO_F1_Write,
        .read = GPIO_F1_Read_Output,
    },
    {
        .index = 10,
        .log = "总氮电源",
        .init = GPIO_F2_Init,
        .write = GPIO_F2_Write,
        .read = GPIO_F2_Read_Output,
    },
    {
        .index = 11,
        .log = "CODmn电源",
        .init = GPIO_F3_Init,
        .write = GPIO_F3_Write,
        .read = GPIO_F3_Read_Output,
    },
    {
        .index = 12,
        .log = "CODcr电源",
        .init = GPIO_F4_Init,
        .write = GPIO_F4_Write,
        .read = GPIO_F4_Read_Output,
    },
    {
        .index = 16,
        .log = "声光报警电源",
        .init = GPIO_F5_Init,
        .write = GPIO_F5_Write,
        .read = GPIO_F5_Read_Output,
    },
    {
        .index = 17,
        .log = "排气扇电源",
        .init = GPIO_F10_Init,
        .write = GPIO_F10_Write,
        .read = GPIO_F10_Read_Output,
    },
    {
        .index = 32,
        .log = "2通电动球阀1",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 33,
        .log = "2通电动球阀2",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 34,
        .log = "2通电动球阀3",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 35,
        .log = "2通电动球阀4",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 36,
        .log = "2通电动球阀5",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 37,
        .log = "2通电动球阀6",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 38,
        .log = "2通电动球阀7",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 39,
        .log = "2通电动球阀8",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 40,
        .log = "3通电动球阀1-G",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 41,
        .log = "3通电动球阀1-R",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 42,
        .log = "3通电动球阀2-G",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 43,
        .log = "3通电动球阀2-R",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 44,
        .log = "3通电动球阀3-G",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 45,
        .log = "3通电动球阀3-R",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 46,
        .log = "3通电动球阀4-G",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 47,
        .log = "3通电动球阀4-R",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 48,
        .log = "电磁阀1",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 49,
        .log = "电磁阀2",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 50,
        .log = "电磁阀3",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 51,
        .log = "电磁阀4",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 52,
        .log = "电磁阀5",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 53,
        .log = "电磁阀6",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 80,
        .log = "除藻泵电源",
        .init = GPIO_F6_Init,
        .write = GPIO_F6_Write,
        .read = GPIO_F6_Read_Output,
    },
    {
        .index = 81,
        .log = "1路采水泵电源",
        .init = GPIO_F7_Init,
        .write = GPIO_F7_Write,
        .read = GPIO_F7_Read_Output,
    },
    {
        .index = 82,
        .log = "2路采水泵电源",
        .init = GPIO_F8_Init,
        .write = GPIO_F8_Write,
        .read = GPIO_F8_Read_Output,
    },
    {
        .index = 83,
        .log = "增压泵电源",
        .init = GPIO_F9_Init,
        .write = GPIO_F9_Write,
        .read = GPIO_F9_Read_Output,
    },
    {
        .index = 84,
        .log = "配水泵电源",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 85,
        .log = "河水净化增压泵电源",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 86,
        .log = "搅拌电机电源",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 87,
        .log = "絮凝剂进样泵电源",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 88,
        .log = "隔膜泵电源",
        .init = NULL,
        .write = GPIO_Write_Sub,
        .read = GPIO_Read_Sub,
    },

    // IO状态
    {
        .index = 96,
        .log = "烟雾报警",
        .init = GPIO_E0_Init,
        .write = NULL,
        .read = GPIO_E0_Read_Input,
    },
    {
        .index = 97,
        .log = "红外报警",
        .init = GPIO_E1_Init,
        .write = NULL,
        .read = GPIO_E1_Read_Input,
    },
    {
        .index = 98,
        .log = "振动传感器",
        .init = GPIO_E2_Init,
        .write = NULL,
        .read = GPIO_E2_Read_Input,
    },
    {
        .index = 99,
        .log = "门禁系统",
        .init = GPIO_E3_Init,
        .write = NULL,
        .read = GPIO_E3_Read_Input,
    },
    {
        .index = 100,
        .log = "外部电源断电报警",
        .init = GPIO_E4_Init,
        .write = NULL,
        .read = GPIO_E4_Read_Input,
    },
    {
        .index = 101,
        .log = "漏水报警",
        .init = GPIO_E5_Init,
        .write = NULL,
        .read = GPIO_E5_Read_Input,
    },
    {
        .index = 102,
        .log = "泵1过载报警",
        .init = GPIO_E6_Init,
        .write = NULL,
        .read = GPIO_E6_Read_Input,
    },
    {
        .index = 103,
        .log = "泵2过载报警",
        .init = GPIO_E7_Init,
        .write = NULL,
        .read = GPIO_E7_Read_Input,
    },
    {
        .index = 112,
        .log = "1路河水净化液位传感器",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 113,
        .log = "2路河水净化液位传感器",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 114,
        .log = "废水桶传感器报警",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 115,
        .log = "废液桶传感器报警",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 116,
        .log = "水流开关报警",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 117,
        .log = "液位1开关报警识别",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 118,
        .log = "液位2开关报警识别",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 119,
        .log = "液位3开关报警识别",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 128,
        .log = "2通电动球阀1关闭感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 129,
        .log = "2通电动球阀1开启感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 130,
        .log = "2通电动球阀2关闭感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 131,
        .log = "2通电动球阀2开启感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 132,
        .log = "2通电动球阀3关闭感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 133,
        .log = "2通电动球阀3开启感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 134,
        .log = "2通电动球阀4关闭感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 135,
        .log = "2通电动球阀4开启感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 136,
        .log = "2通电动球阀5关闭感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 137,
        .log = "2通电动球阀5开启感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 138,
        .log = "2通电动球阀6关闭感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 139,
        .log = "2通电动球阀6开启感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 140,
        .log = "2通电动球阀7关闭感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 141,
        .log = "2通电动球阀7开启感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 142,
        .log = "2通电动球阀8关闭感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 143,
        .log = "2通电动球阀8开启感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 144,
        .log = "3通电动球阀1-G感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 145,
        .log = "3通电动球阀1-R感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 146,
        .log = "3通电动球阀2-G感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 147,
        .log = "3通电动球阀2-R感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 148,
        .log = "3通电动球阀3-G感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 149,
        .log = "3通电动球阀3-R感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 150,
        .log = "3通电动球阀4-G感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
    {
        .index = 151,
        .log = "3通电动球阀4-R感应",
        .init = NULL,
        .write = NULL,
        .read = GPIO_Read_Sub,
    },
};

void Port_Init(void)
{
    memset((void *)&gBoardSubRead, 0, sizeof(_Board));
    memset((void *)&gBoardSubWrite, 0, sizeof(_Board));

    for (int i = 0; i < sizeof(gIOCtl) / sizeof(Gpio_Strategy); i++)
    {
        Gpio_Strategy gpioStrategy = gIOCtl[i];
        if (gpioStrategy.init != NULL)
            gpioStrategy.init();
    }
}

// opt 0:OFF   非0:ON
bool Port_Write(int index, u8 opt)
{
    if (index == 0)
        return true;

    for (int i = 0; i < sizeof(gIOCtl) / sizeof(Gpio_Strategy); i++)
    {
        Gpio_Strategy gpioStrategy = gIOCtl[i];
        if (gpioStrategy.index == index)
        {
            if (gpioStrategy.write != NULL)
            {
                gpioStrategy.write(index, opt);
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool Port_WriteAll(u8 opt)
{
    for (int i = 0; i < sizeof(gIOCtl) / sizeof(Gpio_Strategy); i++)
    {
        if (gIOCtl[i].write != NULL)
        {
            gIOCtl[i].write(i, opt);
        }
    }

    return true;
}

bool Port_Read(int index, u8 *opt)
{
    if (index == 0)
        return true;

    for (int i = 0; i < sizeof(gIOCtl) / sizeof(Gpio_Strategy); i++)
    {
        Gpio_Strategy gpioStrategy = gIOCtl[i];
        if (gpioStrategy.index == index)
        {
            if (gpioStrategy.read != NULL)
            {
                *opt = gpioStrategy.read(index);
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

float Digita_Read(int index)
{
    if (index < 4)
    {
        return gAdcKfp[index] / 4096.0f * 20;
    }
    else if (index < 8)
    {
        return gBoardSubRead.read.digitalValue[index]; // 电流值已在副板转化
    }
    else
    {
        return 0;
    }
}
