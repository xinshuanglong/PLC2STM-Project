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

    if (strcasecmp((const char *)data, "AT:ENTER") == 0)
    {
        debugLevel = 1;
        printf("Enter Debug Mode\r\n");
    }

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

        if (reg_addr >= 0x1000 && reg_addr < 0x1200) // ����
        {
            RefreshREG();
            temp = (reg_addr - 0x1000) * 2;
            for (i = 0; i < reg_count * 2; i++)
            {
                backData[index++] = REGForGB[temp + i];
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
        if (reg_addr == 0x03)
        {
            debugLevel = (data[7] << 8) + data[8];
        }
        else if (reg_addr == 0x1200)
        { //(����)
            u8 mode = (data[8] << 8) + data[7];
            modBusError = (ModBusError)DoGBCmd(mode, data + 9, reg_count * 2 - 2);
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

static int WaitReply(USART_TypeDef *uart, uint8_t maddr, uint8_t cmd, uint8_t *backData, int waitTime);

SemaphoreHandle_t ModBusMasterRxSem = NULL; // �ź���
SemaphoreHandle_t ModBusMasterOptMutex = NULL;
#define MODBUS_BUFFER_SIZE 1024
static uint8_t ModBusRxBuffer[MODBUS_BUFFER_SIZE];
static uint8_t ModBusTxBuffer[MODBUS_BUFFER_SIZE];

void ModBus_Master_Init(void)
{
    ModBusMasterOptMutex = xSemaphoreCreateMutex();
    ModBusMasterRxSem = xSemaphoreCreateBinary();
}

/*
 * ��������ModBus_Master_03
 * ����  : ��������ģʽ,����Э��ִ�в���������������
 * ����  ��
 * ���  : �������ݳ��� -1:�豸û�ظ�
 */
int ModBus_Master_03(USART_TypeDef *uart, uint8_t maddr, uint16_t reg, uint16_t count, uint8_t *backData)
{
    int replyCount = 0;
    xSemaphoreTake(ModBusMasterOptMutex, portMAX_DELAY);

    if (count == 0)
        return 0;
    int regCount = (count + 1) / 2;
    int index = 0;

    ModBusTxBuffer[index++] = maddr;
    ModBusTxBuffer[index++] = 0x03;
    ModBusTxBuffer[index++] = reg >> 8;
    ModBusTxBuffer[index++] = reg & 0xFF;
    ModBusTxBuffer[index++] = regCount >> 8;
    ModBusTxBuffer[index++] = regCount & 0xFF;
    uint16_t crc = CRC16(ModBusTxBuffer, index);
    ModBusTxBuffer[index++] = crc & 0xFF;
    ModBusTxBuffer[index++] = crc >> 8;

    xSemaphoreTake(ModBusMasterRxSem, 0);
    Uart_Get(uart, ModBusRxBuffer, MODBUS_BUFFER_SIZE);
    Uart_Send(uart, ModBusTxBuffer, index);

    replyCount = WaitReply(uart, maddr, 0x03, backData, 3000);

    xSemaphoreGive(ModBusMasterOptMutex);

    return replyCount;
}

/*
 * ��������ModBus_Master_16
 * ����  : ��������ģʽ,����Э��ִ�в���������������
 * ����  ��
 * ���  : 0:ʧ��  ��0:�ɹ� -1:�豸û�ظ�
 */
int ModBus_Master_16(USART_TypeDef *uart, uint8_t maddr, uint16_t reg, uint8_t *data, uint16_t count)
{
    int replyCount = 0;
    xSemaphoreTake(ModBusMasterOptMutex, portMAX_DELAY);

    if (count == 0)
        return 0;
    count = (count + 1) / 2 * 2;
    int index = 0;

    ModBusTxBuffer[index++] = maddr;
    ModBusTxBuffer[index++] = 0x10;
    ModBusTxBuffer[index++] = reg >> 8;
    ModBusTxBuffer[index++] = reg & 0xFF;
    ModBusTxBuffer[index++] = (count / 2) >> 8;
    ModBusTxBuffer[index++] = (count / 2) & 0xFF;
    ModBusTxBuffer[index++] = count;

    memcpy(&ModBusTxBuffer[index], data, count);
    index += count;
    uint16_t crc = CRC16(ModBusTxBuffer, index);
    ModBusTxBuffer[index++] = crc & 0xFF;
    ModBusTxBuffer[index++] = crc >> 8;

    xSemaphoreTake(ModBusMasterRxSem, 0);
    Uart_Get(uart, ModBusRxBuffer, MODBUS_BUFFER_SIZE);
    Uart_Send(uart, ModBusTxBuffer, index);

    replyCount = WaitReply(uart, maddr, 0x10, NULL, 1000);

    xSemaphoreGive(ModBusMasterOptMutex);

    return replyCount;
}

static int WaitReply(USART_TypeDef *uart, uint8_t maddr, uint8_t cmd, uint8_t *backData, int waitTime)
{
    if (xSemaphoreTake(ModBusMasterRxSem, waitTime) == pdTRUE)
    {
        uint16_t count = Uart_Get(uart, ModBusRxBuffer, MODBUS_BUFFER_SIZE);
        if (count < 5 || ModBusRxBuffer[0] != maddr || (ModBusRxBuffer[1] & 0x80) != 0)
            return -3;
        uint16_t crc = CRC16(ModBusRxBuffer, count - 2);
        if (crc != ((ModBusRxBuffer[count - 1] << 8) | ModBusRxBuffer[count - 2]))
            return -2;

        if (backData != NULL)
            memcpy(backData, &ModBusRxBuffer[3], count - 5);

        return count - 5;
    }
    else
    {
        return -1;
    }
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
