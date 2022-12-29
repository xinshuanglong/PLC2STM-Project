//==============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHT2x Sample Code (V1.2)
// File      :  SHT2x.c
// Author    :  MST
// Controller:  NEC V850/SG3 (uPD70F3740)
// Compiler  :  IAR compiler for V850 (3.50A)
// Brief     :  Sensor layer. Functions for sensor access
//==============================================================================

//---------- Includes ----------------------------------------------------------
#include "bsp_include.h"

static SemaphoreHandle_t SHT2XOptSem = NULL;

u8 SHT2X_Init(void)
{
    u8 error = 0;

    IIC1_Init();

    error |= SHT2x_SoftReset();
    error |= SHT2X_Config_Resolutions(SHT2x_RES_11_11BIT);

    SHT2XOptSem = xSemaphoreCreateBinary();
    xSemaphoreGive(SHT2XOptSem); //释放信号量

    return error;
}

//  CRC
u16 POLYNOMIAL = 0x131; //P(x)=x^8+x^5+x^4+1 = 100110001

//==============================================================================
u8 SHT2x_CheckCrc(u8 data[], u8 nbrOfBytes, u8 checksum)
//==============================================================================
{
    u8 crc = 0;
    u8 byteCtr;
    //calculates 8-Bit checksum with given polynomial
    for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);
        for (u8 bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }
    if (crc != checksum)
        return CHECKSUM_ERROR;
    else
        return 0;
}

//===========================================================================
u8 SHT2x_ReadUserRegister(u8 *pRegisterValue)
//===========================================================================
{
    u8 checksum;  //variable for checksum byte
    u8 error = 0; //variable for error code

    IIC1_Start();

    IIC1_Send_Byte(I2C_ADR_W);
    error |= IIC1_Wait_Ack();

    IIC1_Send_Byte(USER_REG_R);
    error |= IIC1_Wait_Ack();

    IIC1_Start();

    IIC1_Send_Byte(I2C_ADR_R);
    error |= IIC1_Wait_Ack();

    *pRegisterValue = IIC1_Read_Byte(1);

    checksum = IIC1_Read_Byte(0);

    error |= SHT2x_CheckCrc(pRegisterValue, 1, checksum);

    IIC1_Stop();

    return error;
}

//===========================================================================
u8 SHT2x_WriteUserRegister(u8 *pRegisterValue)
//===========================================================================
{
    u8 error = 0; //variable for error code

    IIC1_Start();

    IIC1_Send_Byte(I2C_ADR_W);
    error |= IIC1_Wait_Ack();

    IIC1_Send_Byte(USER_REG_W);
    error |= IIC1_Wait_Ack();

    IIC1_Send_Byte(*pRegisterValue);
    error |= IIC1_Wait_Ack();

    IIC1_Stop();
    return error;
}

u8 SHT2X_Config_Resolutions(etSHT2xResolution resolution)
{
    u8 error = 0;
    u8 userRegister;

    // --- Set Resolution e.g. RH 10bit, Temp 13bit ---
    error |= SHT2x_ReadUserRegister(&userRegister); //get actual user reg
    userRegister = (userRegister & ~SHT2x_RES_MASK) | resolution;
    error |= SHT2x_WriteUserRegister(&userRegister); //write changed user reg

    return error;
}

//===========================================================================
//在11bit:11bit模式下,15msRH转化完成
//在11bit:11bit模式下,11msTempeture转化完成
u8 SHT2x_MeasurePoll(etSHT2xMeasureType eSHT2xMeasureType, nt16 *pMeasurand)
//===========================================================================
{
    u8 checksum;  //checksum
    u8 data[2];   //data array for checksum verification
    u8 error = 0; //error variable
    u8 sht20OutTimeCount = 0;

    IIC1_Start();

    IIC1_Send_Byte(I2C_ADR_W); // I2C Adr
    error |= IIC1_Wait_Ack();

    switch (eSHT2xMeasureType)
    {
    case HUMIDITY:
        IIC1_Send_Byte(TRIG_RH_MEASUREMENT_POLL);
        error |= IIC1_Wait_Ack();
        break;

    case TEMP:
        IIC1_Send_Byte(TRIG_T_MEASUREMENT_POLL);
        error |= IIC1_Wait_Ack();
        break;
    default:
        break;
    }

    if (!error)
    {
        delay_us(20);
        IIC1_Stop();

        sht20OutTimeCount = 20;
        while (sht20OutTimeCount--)
        {
            delay_ms(10);
            IIC1_Start();
            IIC1_Send_Byte(I2C_ADR_R);
            if (IIC1_Wait_Ack() == 0)
                break;
            else
                IIC1_Stop();
        }
    }

    if (sht20OutTimeCount == 0)
        return BUSY_ERROR;

    //-- read two data bytes and one checksum byte --
    pMeasurand->s16.u8ValH = data[0] = IIC1_Read_Byte(1);
    pMeasurand->s16.u8ValL = data[1] = IIC1_Read_Byte(1);
    checksum = IIC1_Read_Byte(0);

    //-- verify checksum --
    error |= SHT2x_CheckCrc(data, 2, checksum);

    IIC1_Stop();

    return error;
}

//===========================================================================
u8 SHT2x_SoftReset(void)
//===========================================================================
{
    u8 error = 0; //error variable

    IIC1_Start();

    IIC1_Send_Byte(I2C_ADR_W); // I2C Adr
    error |= IIC1_Wait_Ack();

    IIC1_Send_Byte(SOFT_RESET); // Command
    error |= IIC1_Wait_Ack();

    IIC1_Stop();

    delay_ms(15); // wait till sensor has restarted

    return error;
}

//==============================================================================
float SHT2x_CalcRH(u16 u16sRH)
//==============================================================================
{
    float humidityRH; // variable for result

    u16sRH &= ~0x0003; // clear bits [1..0] (status bits)
    //-- calculate relative humidity [%RH] --

    humidityRH = -6.0 + 0.00190735 * (double)u16sRH; // RH= -6 + 125 * SRH/2^16
    return humidityRH;
}

//==============================================================================
float SHT2x_CalcTemperatureC(u16 u16sT)
//==============================================================================
{
    float temperatureC; // variable for result

    u16sT &= ~0x0003; // clear bits [1..0] (status bits)

    //-- calculate temperature [癈] --
    temperatureC = -46.85 + 0.00268127 * (double)u16sT; //T= -46.85 + 175.72 * ST/2^16
    return temperatureC;
}

//==============================================================================
u8 SHT2x_GetSerialNumber(u8 u8SerialNumber[])
//==============================================================================
{
    u8 error = 0; //error variable

    //Read from memory location 1
    IIC1_Start();

    IIC1_Send_Byte(I2C_ADR_W); //I2C address
    error |= IIC1_Wait_Ack();

    IIC1_Send_Byte(0xFA); //Command for readout on-chip memory
    error |= IIC1_Wait_Ack();

    IIC1_Send_Byte(0x0F); //on-chip memory address
    error |= IIC1_Wait_Ack();

    IIC1_Start();

    IIC1_Send_Byte(I2C_ADR_R); //I2C address
    error |= IIC1_Wait_Ack();

    u8SerialNumber[5] = IIC1_Read_Byte(1); //Read SNB_3
    IIC1_Read_Byte(1);                     //Read CRC SNB_3 (CRC is not analyzed)
    u8SerialNumber[4] = IIC1_Read_Byte(1); //Read SNB_2
    IIC1_Read_Byte(1);                     //Read CRC SNB_2 (CRC is not analyzed)
    u8SerialNumber[3] = IIC1_Read_Byte(1); //Read SNB_1
    IIC1_Read_Byte(1);                     //Read CRC SNB_1 (CRC is not analyzed)
    u8SerialNumber[2] = IIC1_Read_Byte(1); //Read SNB_0
    IIC1_Read_Byte(0);                     //Read CRC SNB_0 (CRC is not analyzed)

    IIC1_Stop();

    //Read from memory location 2
    IIC1_Start();

    IIC1_Send_Byte(I2C_ADR_W); //I2C address
    error |= IIC1_Wait_Ack();

    IIC1_Send_Byte(0xFC); //Command for readout on-chip memory
    error |= IIC1_Wait_Ack();

    IIC1_Send_Byte(0xC9); //on-chip memory address
    error |= IIC1_Wait_Ack();
    IIC1_Start();

    IIC1_Send_Byte(I2C_ADR_R); //I2C address
    error |= IIC1_Wait_Ack();

    u8SerialNumber[1] = IIC1_Read_Byte(1); //Read SNC_1
    u8SerialNumber[0] = IIC1_Read_Byte(1); //Read SNC_0
    IIC1_Read_Byte(1);                     //Read CRC SNC0/1 (CRC is not analyzed)
    u8SerialNumber[7] = IIC1_Read_Byte(1); //Read SNA_1
    u8SerialNumber[6] = IIC1_Read_Byte(1); //Read SNA_0
    IIC1_Read_Byte(0);                     //Read CRC SNA0/1 (CRC is not analyzed)

    IIC1_Stop();

    return error;
}

//返回值0,成功
//其他值失败
u8 SHT2x_GetTemperatureC(float *temperatureC)
{
    u8 error = 0;
    nt16 pMeasurand;

    error = SHT2x_MeasurePoll(TEMP, &pMeasurand);
    if (!error)
    {
        *temperatureC = SHT2x_CalcTemperatureC(pMeasurand.u16Val);
    }

    return error;
}

//返回值0,成功
//其他值失败
u8 SHT2x_GetHumidity(float *humidity)
{
    u8 error = 0;
    nt16 pMeasurand;

    error = SHT2x_MeasurePoll(HUMIDITY, &pMeasurand);
    if (!error)
    {
        *humidity = SHT2x_CalcRH(pMeasurand.u16Val);
    }

    return error;
}

u8 SHT2x_GetTempCAndHumidity(float *temperatureC, float *humidity)
{
    u8 error = 0;

    xSemaphoreTake(SHT2XOptSem, 1000); //阻止多线程访问
    error |= SHT2x_GetTemperatureC(temperatureC);
    error |= SHT2x_GetHumidity(humidity);
    xSemaphoreGive(SHT2XOptSem); //阻止多线程访问

    return error;
}
