#include "bsp_include.h"

// xx:A0,B1,F10,G4等
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
const Gpio_Type GPIO_E0 = {GPIOE, GPIO_Pin_0, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E1 = {GPIOE, GPIO_Pin_1, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E2 = {GPIOE, GPIO_Pin_2, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E3 = {GPIOE, GPIO_Pin_3, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E4 = {GPIOE, GPIO_Pin_4, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E5 = {GPIOE, GPIO_Pin_5, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E6 = {GPIOE, GPIO_Pin_6, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E7 = {GPIOE, GPIO_Pin_7, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E8 = {GPIOE, GPIO_Pin_8, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E9 = {GPIOE, GPIO_Pin_9, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E10 = {GPIOE, GPIO_Pin_10, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E11 = {GPIOE, GPIO_Pin_11, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E12 = {GPIOE, GPIO_Pin_12, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_E13 = {GPIOE, GPIO_Pin_13, GPIO_Mode_OUT, true};

const Gpio_Type GPIO_F3 = {GPIOF, GPIO_Pin_3, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F4 = {GPIOF, GPIO_Pin_4, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F5 = {GPIOF, GPIO_Pin_5, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F6 = {GPIOF, GPIO_Pin_6, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F7 = {GPIOF, GPIO_Pin_7, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F8 = {GPIOF, GPIO_Pin_8, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F9 = {GPIOF, GPIO_Pin_9, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F10 = {GPIOF, GPIO_Pin_10, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F11 = {GPIOF, GPIO_Pin_11, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F12 = {GPIOF, GPIO_Pin_12, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_F13 = {GPIOF, GPIO_Pin_13, GPIO_Mode_OUT, true};

const Gpio_Type GPIO_G0 = {GPIOG, GPIO_Pin_0, GPIO_Mode_OUT, true};
const Gpio_Type GPIO_G1 = {GPIOG, GPIO_Pin_1, GPIO_Mode_OUT, true};

// IO读取(高电平异常)
const Gpio_Type GPIO_A8 = {GPIOA, GPIO_Pin_8, GPIO_Mode_IN, true};
const Gpio_Type GPIO_A11 = {GPIOA, GPIO_Pin_11, GPIO_Mode_IN, true};
const Gpio_Type GPIO_A12 = {GPIOA, GPIO_Pin_12, GPIO_Mode_IN, true};
const Gpio_Type GPIO_A15 = {GPIOA, GPIO_Pin_15, GPIO_Mode_IN, true};

const Gpio_Type GPIO_B3 = {GPIOB, GPIO_Pin_3, GPIO_Mode_IN, true};
const Gpio_Type GPIO_B4 = {GPIOB, GPIO_Pin_4, GPIO_Mode_IN, true};
const Gpio_Type GPIO_B5 = {GPIOB, GPIO_Pin_5, GPIO_Mode_IN, true};
const Gpio_Type GPIO_B6 = {GPIOB, GPIO_Pin_6, GPIO_Mode_IN, true};

const Gpio_Type GPIO_C6 = {GPIOC, GPIO_Pin_6, GPIO_Mode_IN, true};
const Gpio_Type GPIO_C7 = {GPIOC, GPIO_Pin_7, GPIO_Mode_IN, true};
const Gpio_Type GPIO_C8 = {GPIOC, GPIO_Pin_8, GPIO_Mode_IN, true};
const Gpio_Type GPIO_C9 = {GPIOC, GPIO_Pin_9, GPIO_Mode_IN, true};
const Gpio_Type GPIO_C10 = {GPIOC, GPIO_Pin_10, GPIO_Mode_IN, true};
const Gpio_Type GPIO_C11 = {GPIOC, GPIO_Pin_11, GPIO_Mode_IN, true};
const Gpio_Type GPIO_C12 = {GPIOC, GPIO_Pin_12, GPIO_Mode_IN, true};
const Gpio_Type GPIO_C13 = {GPIOC, GPIO_Pin_13, GPIO_Mode_IN, true};
const Gpio_Type GPIO_C14 = {GPIOC, GPIO_Pin_14, GPIO_Mode_IN, true};
const Gpio_Type GPIO_C15 = {GPIOC, GPIO_Pin_15, GPIO_Mode_IN, true};

const Gpio_Type GPIO_D0 = {GPIOD, GPIO_Pin_0, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D1 = {GPIOD, GPIO_Pin_1, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D2 = {GPIOD, GPIO_Pin_2, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D3 = {GPIOD, GPIO_Pin_3, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D4 = {GPIOD, GPIO_Pin_4, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D5 = {GPIOD, GPIO_Pin_5, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D6 = {GPIOD, GPIO_Pin_6, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D7 = {GPIOD, GPIO_Pin_7, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D8 = {GPIOD, GPIO_Pin_8, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D9 = {GPIOD, GPIO_Pin_9, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D10 = {GPIOD, GPIO_Pin_10, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D11 = {GPIOD, GPIO_Pin_11, GPIO_Mode_IN, true};
const Gpio_Type GPIO_D15 = {GPIOD, GPIO_Pin_15, GPIO_Mode_IN, true};

const Gpio_Type GPIO_F0 = {GPIOF, GPIO_Pin_0, GPIO_Mode_IN, true};

GPIO_DEF(A8);
GPIO_DEF(A11);
GPIO_DEF(A12);
GPIO_DEF(A15);

GPIO_DEF(B3);
GPIO_DEF(B4);
GPIO_DEF(B5);
GPIO_DEF(B6);

GPIO_DEF(C6);
GPIO_DEF(C7);
GPIO_DEF(C8);
GPIO_DEF(C9);
GPIO_DEF(C10);
GPIO_DEF(C11);
GPIO_DEF(C12);
GPIO_DEF(C13);
GPIO_DEF(C14);
GPIO_DEF(C15);

GPIO_DEF(D0);
GPIO_DEF(D1);
GPIO_DEF(D2);
GPIO_DEF(D3);
GPIO_DEF(D4);
GPIO_DEF(D5);
GPIO_DEF(D6);
GPIO_DEF(D7);
GPIO_DEF(D8);
GPIO_DEF(D9);
GPIO_DEF(D10);
GPIO_DEF(D11);
GPIO_DEF(D15);

GPIO_DEF(E0);
GPIO_DEF(E1);
GPIO_DEF(E2);
GPIO_DEF(E3);
GPIO_DEF(E4);
GPIO_DEF(E5);
GPIO_DEF(E6);
GPIO_DEF(E7);
GPIO_DEF(E8);
GPIO_DEF(E9);
GPIO_DEF(E10);
GPIO_DEF(E11);
GPIO_DEF(E12);
GPIO_DEF(E13);

GPIO_DEF(F0);
GPIO_DEF(F3);
GPIO_DEF(F4);
GPIO_DEF(F5);
GPIO_DEF(F6);
GPIO_DEF(F7);
GPIO_DEF(F8);
GPIO_DEF(F9);
GPIO_DEF(F10);
GPIO_DEF(F11);
GPIO_DEF(F12);
GPIO_DEF(F13);

GPIO_DEF(G0);
GPIO_DEF(G1);

// index:0-15   模块电源
// index:16-31  常规电源
// index:32-63  阀电源
// index:64-79  动力电源(潜水泵，采水泵等需要延时动作)
const Gpio_Strategy gIOCtl[] = {
    {
        .index = 32,
        .log = "2通电动球阀1-1",
        .init = GPIO_E2_Init,
        .write = GPIO_E2_Write,
        .read = GPIO_E2_Read_Output,
    },
    {
        .index = 33,
        .log = "2通电动球阀2",
        .init = GPIO_E3_Init,
        .write = GPIO_E3_Write,
        .read = GPIO_E3_Read_Output,
    },
    {
        .index = 34,
        .log = "2通电动球阀3",
        .init = GPIO_E4_Init,
        .write = GPIO_E4_Write,
        .read = GPIO_E4_Read_Output,
    },
    {
        .index = 35,
        .log = "2通电动球阀4",
        .init = GPIO_E5_Init,
        .write = GPIO_E5_Write,
        .read = GPIO_E5_Read_Output,
    },
    {
        .index = 36,
        .log = "2通电动球阀5",
        .init = GPIO_E6_Init,
        .write = GPIO_E6_Write,
        .read = GPIO_E6_Read_Output,
    },
    {
        .index = 37,
        .log = "2通电动球阀6",
        .init = GPIO_F3_Init,
        .write = GPIO_F3_Write,
        .read = GPIO_F3_Read_Output,
    },
    {
        .index = 38,
        .log = "2通电动球阀7",
        .init = GPIO_F4_Init,
        .write = GPIO_F4_Write,
        .read = GPIO_F4_Read_Output,
    },
    {
        .index = 39,
        .log = "2通电动球阀8",
        .init = GPIO_F5_Init,
        .write = GPIO_F5_Write,
        .read = GPIO_F5_Read_Output,
    },
    {
        .index = 40,
        .log = "3通电动球阀V1-G",
        .init = GPIO_F6_Init,
        .write = GPIO_F6_Write,
        .read = GPIO_F6_Read_Output,
    },
    {
        .index = 41,
        .log = "3通电动球阀1-R",
        .init = GPIO_F7_Init,
        .write = GPIO_F7_Write,
        .read = GPIO_F7_Read_Output,
    },
    {
        .index = 42,
        .log = "3通电动球阀V2-G",
        .init = GPIO_F8_Init,
        .write = GPIO_F8_Write,
        .read = GPIO_F8_Read_Output,
    },
    {
        .index = 43,
        .log = "3通电动球阀2-R",
        .init = GPIO_F9_Init,
        .write = GPIO_F9_Write,
        .read = GPIO_F9_Read_Output,
    },
    {
        .index = 44,
        .log = "3通电动球阀V3-G",
        .init = GPIO_F10_Init,
        .write = GPIO_F10_Write,
        .read = GPIO_F10_Read_Output,
    },
    {
        .index = 45,
        .log = "3通电动球阀3-R",
        .init = GPIO_F11_Init,
        .write = GPIO_F11_Write,
        .read = GPIO_F11_Read_Output,
    },
    {
        .index = 46,
        .log = "3通电动球阀V4-G",
        .init = GPIO_F12_Init,
        .write = GPIO_F12_Write,
        .read = GPIO_F12_Read_Output,
    },
    {
        .index = 47,
        .log = "3通电动球阀4-R",
        .init = GPIO_F13_Init,
        .write = GPIO_F13_Write,
        .read = GPIO_F13_Read_Output,
    },
    {
        .index = 48,
        .log = "电磁阀1",
        .init = GPIO_G0_Init,
        .write = GPIO_G0_Write,
        .read = GPIO_G0_Read_Output,
    },
    {
        .index = 49,
        .log = "电磁阀2",
        .init = GPIO_G1_Init,
        .write = GPIO_G1_Write,
        .read = GPIO_G1_Read_Output,
    },
    {
        .index = 50,
        .log = "电磁阀3",
        .init = GPIO_E7_Init,
        .write = GPIO_E7_Write,
        .read = GPIO_E7_Read_Output,
    },
    {
        .index = 51,
        .log = "电磁阀4",
        .init = GPIO_E8_Init,
        .write = GPIO_E8_Write,
        .read = GPIO_E8_Read_Output,
    },
    {
        .index = 52,
        .log = "电磁阀5",
        .init = GPIO_E9_Init,
        .write = GPIO_E9_Write,
        .read = GPIO_E9_Read_Output,
    },
    {
        .index = 53,
        .log = "电磁阀6",
        .init = GPIO_E10_Init,
        .write = GPIO_E10_Write,
        .read = GPIO_E10_Read_Output,
    },
    {
        .index = 84,
        .log = "配水泵电源",
        .init = GPIO_E11_Init,
        .write = GPIO_E11_Write,
        .read = GPIO_E11_Read_Output,
    },
    {
        .index = 85,
        .log = "河水净化增压泵电源",
        .init = GPIO_E12_Init,
        .write = GPIO_E12_Write,
        .read = GPIO_E12_Read_Output,
    },
    {
        .index = 86,
        .log = "搅拌电机电源",
        .init = GPIO_E13_Init,
        .write = GPIO_E13_Write,
        .read = GPIO_E13_Read_Output,
    },
    {
        .index = 87,
        .log = "絮凝剂进样泵电源",
        .init = GPIO_E0_Init,
        .write = GPIO_E0_Write,
        .read = GPIO_E0_Read_Output,
    },
    {
        .index = 88,
        .log = "隔膜泵电源",
        .init = GPIO_E1_Init,
        .write = GPIO_E1_Write,
        .read = GPIO_E1_Read_Output,
    },

    // IO状态
    {
        .index = 112,
        .log = "1路河水净化液位传感器",
        .init = GPIO_D8_Init,
        .write = NULL,
        .read = GPIO_D8_Read_Input,
    },
    {
        .index = 113,
        .log = "2路河水净化液位传感器",
        .init = GPIO_D9_Init,
        .write = NULL,
        .read = GPIO_D9_Read_Input,
    },
    {
        .index = 114,
        .log = "废水桶传感器报警",
        .init = GPIO_D10_Init,
        .write = NULL,
        .read = GPIO_D10_Read_Input,
    },
    {
        .index = 115,
        .log = "废液桶传感器报警",
        .init = GPIO_D11_Init,
        .write = NULL,
        .read = GPIO_D11_Read_Input,
    },
    {
        .index = 116,
        .log = "水流开关报警",
        .init = GPIO_C13_Init,
        .write = NULL,
        .read = GPIO_C13_Read_Input,
    },
    {
        .index = 117,
        .log = "液位1开关报警识别",
        .init = GPIO_C14_Init,
        .write = NULL,
        .read = GPIO_C14_Read_Input,
    },
    {
        .index = 118,
        .log = "液位2开关报警识别",
        .init = GPIO_C15_Init,
        .write = NULL,
        .read = GPIO_C15_Read_Input,
    },
    {
        .index = 119,
        .log = "液位3开关报警识别",
        .init = GPIO_F0_Init,
        .write = NULL,
        .read = GPIO_F0_Read_Input,
    },
    {
        .index = 128,
        .log = "2通电动球阀1关闭感应",
        .init = GPIO_D15_Init,
        .write = NULL,
        .read = GPIO_D15_Read_Input,
    },
    {
        .index = 129,
        .log = "2通电动球阀1开启感应",
        .init = GPIO_C6_Init,
        .write = NULL,
        .read = GPIO_C6_Read_Input,
    },
    {
        .index = 130,
        .log = "2通电动球阀2关闭感应",
        .init = GPIO_C7_Init,
        .write = NULL,
        .read = GPIO_C7_Read_Input,
    },
    {
        .index = 131,
        .log = "2通电动球阀2开启感应",
        .init = GPIO_C8_Init,
        .write = NULL,
        .read = GPIO_C8_Read_Input,
    },
    {
        .index = 132,
        .log = "2通电动球阀3关闭感应",
        .init = GPIO_C9_Init,
        .write = NULL,
        .read = GPIO_C9_Read_Input,
    },
    {
        .index = 133,
        .log = "2通电动球阀3开启感应",
        .init = GPIO_A8_Init,
        .write = NULL,
        .read = GPIO_A8_Read_Input,
    },
    {
        .index = 134,
        .log = "2通电动球阀4关闭感应",
        .init = GPIO_A11_Init,
        .write = NULL,
        .read = GPIO_A11_Read_Input,
    },
    {
        .index = 135,
        .log = "2通电动球阀4开启感应",
        .init = GPIO_A12_Init,
        .write = NULL,
        .read = GPIO_A12_Read_Input,
    },
    {
        .index = 136,
        .log = "2通电动球阀5关闭感应",
        .init = GPIO_A15_Init,
        .write = NULL,
        .read = GPIO_A15_Read_Input,
    },
    {
        .index = 137,
        .log = "2通电动球阀5开启感应",
        .init = GPIO_C10_Init,
        .write = NULL,
        .read = GPIO_C10_Read_Input,
    },
    {
        .index = 138,
        .log = "2通电动球阀6关闭感应",
        .init = GPIO_C11_Init,
        .write = NULL,
        .read = GPIO_C11_Read_Input,
    },
    {
        .index = 139,
        .log = "2通电动球阀6开启感应",
        .init = GPIO_C12_Init,
        .write = NULL,
        .read = GPIO_C12_Read_Input,
    },
    {
        .index = 140,
        .log = "2通电动球阀7关闭感应",
        .init = GPIO_D0_Init,
        .write = NULL,
        .read = GPIO_D0_Read_Input,
    },
    {
        .index = 141,
        .log = "2通电动球阀7开启感应",
        .init = GPIO_D1_Init,
        .write = NULL,
        .read = GPIO_D1_Read_Input,
    },
    {
        .index = 142,
        .log = "2通电动球阀8关闭感应",
        .init = GPIO_D2_Init,
        .write = NULL,
        .read = GPIO_D2_Read_Input,
    },
    {
        .index = 143,
        .log = "2通电动球阀8开启感应",
        .init = GPIO_D3_Init,
        .write = NULL,
        .read = GPIO_D3_Read_Input,
    },
    {
        .index = 144,
        .log = "3通电动球阀1-G感应",
        .init = GPIO_D4_Init,
        .write = NULL,
        .read = GPIO_D4_Read_Input,
    },
    {
        .index = 145,
        .log = "3通电动球阀1-R感应",
        .init = GPIO_D5_Init,
        .write = NULL,
        .read = GPIO_D5_Read_Input,
    },
    {
        .index = 146,
        .log = "3通电动球阀2-G感应",
        .init = GPIO_D6_Init,
        .write = NULL,
        .read = GPIO_D6_Read_Input,
    },
    {
        .index = 147,
        .log = "3通电动球阀2-R感应",
        .init = GPIO_D7_Init,
        .write = NULL,
        .read = GPIO_D7_Read_Input,
    },
    {
        .index = 148,
        .log = "3通电动球阀3-G感应",
        .init = GPIO_B3_Init,
        .write = NULL,
        .read = GPIO_B3_Read_Input,
    },
    {
        .index = 149,
        .log = "3通电动球阀3-R感应",
        .init = GPIO_B4_Init,
        .write = NULL,
        .read = GPIO_B4_Read_Input,
    },
    {
        .index = 150,
        .log = "3通电动球阀4-G感应",
        .init = GPIO_B5_Init,
        .write = NULL,
        .read = GPIO_B5_Read_Input,
    },
    {
        .index = 151,
        .log = "3通电动球阀4-R感应",
        .init = GPIO_B6_Init,
        .write = NULL,
        .read = GPIO_B6_Read_Input,
    },
};

void Port_Init(void)
{
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

bool Port_Read(int index, u8 *opt)
{
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
