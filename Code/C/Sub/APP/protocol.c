#include "bsp_include.h"

static uint16_t CRC16(uint8_t *ptr, uint16_t len);

/*
 * ��������ModBus_Protocol
 * ����  : ����Э��ִ�в���������������
 * ����  ��data---�������ݣ� len---���ݳ��ȣ�backData---��������
 * ���  : �������ݳ���
 */
uint16_t ModBus_Protocol(uint8_t *data, uint16_t len, uint8_t *backData)
{
    uint8_t deviceAddr = 0;
    uint16_t crc;
    uint8_t cmd;
    uint16_t index = 0;
    uint16_t reg_addr, reg_addr_end;
    uint16_t reg_count;
    uint8_t res = 0;
    u16 temp;
    u16 i;

    ModBusError modBusError = ModBusNoError;

    if (data[0] != DEVICE_ADDR) // �Ƿ��豸��ַ
        return 0;

    crc = CRC16(data, len - 2);
    if (crc != ((data[len - 1] << 8) | data[len - 2]))
        return 0;

    deviceAddr = backData[index++] = data[0];
    cmd = backData[index++] = data[1];

    reg_addr = (data[2] << 8) + data[3];
    reg_count = (data[4] << 8) + data[5]; // Word Ϊ��λ
    reg_addr_end = reg_addr + reg_count - 1;

    switch (cmd)
    {
    case 0x03:

        backData[index++] = reg_count * 2; // char Ϊ��λ

        if (reg_addr >= 0x2600 && reg_addr < 0x3000) // ����
        {
            uint8_t *p = (uint8_t *)&gBoardOpt;
            temp = (reg_addr - 0x2600) * 2;
            for (i = 0; i < reg_count * 2; i++)
            {
                backData[index++] = p[temp + i];
            }
        }
        else if (reg_addr == 0xFF00)
        {
            uint32_t otaStatus = 0;
            backData[index++] = otaStatus >> 8;
            backData[index++] = otaStatus;
        }
        else if (reg_addr == 0xFF02)
        { // �̼���
            Flash_Read(FLASH_SOFTWARE_VERSION_ADDR, backData + 3, reg_count << 2);
            index = 3 + reg_count * 2;
        }
        else if (reg_addr == 0xFF04 && reg_count == 0x0002)
        { // ��ȡ�̼���С
            _W2Byte softwareLength;
            Flash_Read(FLASH_SOFTWARE_LENGTH_ADDR, softwareLength.b, 4);
            softwareLength.w2 += (4 * 1024 - 4);
            backData[index++] = softwareLength.b[0];
            backData[index++] = softwareLength.b[1];
            backData[index++] = softwareLength.b[2];
            backData[index++] = softwareLength.b[3];
        }
        else
        {
            modBusError = ModBusDataAddrIllegal;
        }

        break;

    case 0x10:
        if (reg_addr >= 0x2600 && reg_addr < 0x3000) // ����
        {
            uint8_t *p = (uint8_t *)&gBoardOpt;
            temp = (reg_addr - 0x2600) * 2;
            for (i = 0; i < reg_count * 2; i++)
            {
                p[temp + i] = data[7 + i];
            }
        }
        else if (reg_addr == 0xFF00 && reg_count == 0x0001)
        { // дOTA״̬(�൱��дCRC)
            uint32_t otaStatus = 0;
            otaStatus = (data[7] << 8) + data[8];
            if (otaStatus != 0)
            {
                Flash_Write(FLASH_SOFTWARE_CRC_ADDR, (u8 *)&otaStatus, 4);
            }
            gSysReboot = 1;
        }
        else
        {
            modBusError = ModBusDataAddrIllegal;
        }

        if (modBusError == ModBusNoError)
        {
            // ���ƼĴ�����ַ��Ĵ�������
            memcpy(&backData[index], &data[index], 2 * 2);
            index += 2 * 2;
        }

        break;

    default:
        modBusError = ModBusFunIllegal;
        break;
    }

    if (modBusError != ModBusNoError)
    {
        backData[1] |= 0x80;
        backData[2] = modBusError;
        index = 3;
    }

    crc = CRC16(backData, index);
    backData[index++] = crc;
    backData[index++] = crc >> 8;

    return index;
}

/*
 * ��������CRC16
 * ����  : ����CRC16
 * ����  ��aData---���ݣ� aSize---���ݳ���
 * ���  : У��ֵ
 */
static uint16_t CRC16(uint8_t *ptr, uint16_t len)
{
    unsigned char i;
    unsigned short crc = 0xFFFF;
    if (len == 0)
    {
        len = 1;
    }
    while (len--)
    {
        crc ^= *ptr;
        for (i = 0; i < 8; i++)
        {
            if (crc & 1)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
        ptr++;
    }
    return (crc);
}
