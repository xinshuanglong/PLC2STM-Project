#include "bsp_include.h"

SemaphoreHandle_t DebugRxSem = NULL;

#define DEBUG_RX_SIZE 128
uint8_t DEBUG_RxBuffer[DEBUG_RX_SIZE];

u8 debugStask = 0, debugCpu = 0, debugADC = 0, debugIO = 0;

static int AT_CmdDeal(const char *cmd);
static uint16_t CRC16(uint8_t *ptr, uint16_t len);

/***********************************************/
// �����ջ��С
#define DEBUG_CMD_PARSE_TASK_STK_SIZE 512
// ������
TaskHandle_t DebugCmdParseTaskHandler;
// ������
void DebugCmdParseTask(void *pvParameters);
/***********************************************/

/***********************************************/
// �����ջ��С
#define DEBUG_PRINT_MSG_TASK_STK_SIZE 512
// ������
TaskHandle_t DebugPrintMsgTaskHandler;
// ������
void DebugPrintMsgTask(void *pvParameters);
/***********************************************/

void DebugCmdParseTaskCreat(void)
{
    DebugRxSem = xSemaphoreCreateBinary();

    xTaskCreate(DebugCmdParseTask,
                "DebugCmdParseTask",
                DEBUG_CMD_PARSE_TASK_STK_SIZE,
                NULL,
                DEBUG_CMD_PARSE_TASK_PRIO,
                DebugCmdParseTaskHandler);
}

void DebugPrintMsgTaskCreat(void)
{
    xTaskCreate(DebugPrintMsgTask,
                "DebugPrintMsgTask",
                DEBUG_PRINT_MSG_TASK_STK_SIZE,
                NULL,
                DEBUG_PRINT_MSG_TASK_PRIO,
                DebugPrintMsgTaskHandler);
}

char gCBuffer[2048]; // �������ʹ��

void DebugPrintMsgTask(void *pvParameters)
{
    while (1)
    {
        if (debugADC)
        {
            printf("VCC[%7.2f]\t", gVcc * 1000);
            for (int i = 0; i < 4; i++)
            {
                float mV = gAdcKfp[i] / gAdcRef * 1200;
                float mA = gAdcKfp[i] / 4096.0f * 20;
                printf("V%d.[%.2f][%.2fmV][%.2fmA] ", i, gAdcKfp[i], mV, mA);
            }
            printf("\r\n");
        }

        if (debugIO)
        {
            printf("IO0.[0x%04x] IO1.[0x%04x]\r\n", gIoStateInBit[0], gIoStateInBit[1]);
            for (int i = 0; i < IO_INDEX_COUNT; i++)
            {
                if (i != 0 && i % 16 == 0)
                    printf("\r\n");
                printf("%2d.[%d] ", i, gIoState[i]);
            }
            printf("\r\n");
        }

        if (debugStask)
        {
            vTaskList(gCBuffer);
            printf("%s\r\n", gCBuffer);
            // FreeRTOS�ڴ�
            printf("FreeHeapSize: %d\r\n", xPortGetFreeHeapSize());
        }
        if (debugCpu)
        {
            vTaskGetRunTimeStats(gCBuffer);
            printf("%s\r\n", gCBuffer);
        }

        vTaskDelay(250);
    }
}

static void CleanDebug(void)
{
    debugStask = 0;
    debugCpu = 0;
    debugADC = 0;
    debugIO = 0;
}

extern FATFS fs[1];
void DebugCmdParseTask(void *pvParameters)
{
    uint16_t crc = 0;
    uint16_t length = 0;

    while (1)
    {
        if (xSemaphoreTake(DebugRxSem, portMAX_DELAY) == pdPASS)
        {
            length = Uart_Get(DEBUG_USART, DEBUG_RxBuffer, DEBUG_RX_SIZE);

            // ����ǺϷ���MODBUSָ���������Debugģʽ
            if (length >= 5)
            {
                crc = CRC16(DEBUG_RxBuffer, length - 2);
                if (crc == ((DEBUG_RxBuffer[length - 1] << 8) | DEBUG_RxBuffer[length - 2]))
                {
                    printf("Quit Debug Mode\r\n");
                    debugLevel = 0;
                    CleanDebug();
                    continue;
                }
            }

            DEBUG_RxBuffer[length] = '\0'; // ���������

            printf("Get Cmd :%s\r\n", DEBUG_RxBuffer);

            if (strcasecmp((const char *)DEBUG_RxBuffer, "v") == 0)
            {
                printf("VERSION:%s\r\n", VERSION);
            }

            if (strcasecmp((const char *)DEBUG_RxBuffer, "h") == 0)
            {
                // FreeRTOS�ڴ�
                printf("FreeHeapSize: %d\r\n", xPortGetFreeHeapSize());
                // SD��ʣ��
                DWORD fre_clust = 0;
                FATFS *pfs = &fs[0];
                f_getfree("", &fre_clust, &pfs);
                DWORD TOT_SIZE = (pfs->n_fatent - 2) * pfs->csize / 2; // ������ ��λbyte
                DWORD FRE_SIZE = fre_clust * pfs->csize / 2;           // �������� ��λbyte
                printf("SD: Free [%d] bytes\tTotal [%d] bytes\tUsed [%.2f%%]\r\n", FRE_SIZE, TOT_SIZE, (float)FRE_SIZE / TOT_SIZE);
            }

            if (strcasecmp((const char *)DEBUG_RxBuffer, "t") == 0)
            {
                printf("[%04X-%02X-%02X ", gtTimeBdc.year + 0x2000, gtTimeBdc.month, gtTimeBdc.date);
                printf("%02X:%02X:%02X] ", gtTimeBdc.hour, gtTimeBdc.minute, gtTimeBdc.second);
                printf("\r\n");
            }

            if (strcasecmp((const char *)DEBUG_RxBuffer, "stack") == 0)
            {
                debugStask = 1;
            }

            if (strcasecmp((const char *)DEBUG_RxBuffer, "cpu") == 0)
            {
                debugCpu = 1;
            }

            if (strcasecmp((const char *)DEBUG_RxBuffer, "adc") == 0)
            {
                debugADC = 1;
            }

            if (strcasecmp((const char *)DEBUG_RxBuffer, "io") == 0)
            {
                debugIO = 1;
            }

            if (strcasecmp((const char *)DEBUG_RxBuffer, "q") == 0)
            {
                CleanDebug();
            }

            if (strcasecmp((const char *)DEBUG_RxBuffer, "reboot") == 0)
            {
                printf("reboot\r\n");
                SysReboot();
            }

            if (strncmp((const char *)DEBUG_RxBuffer, "AT:", 3) == 0)
            {
                AT_CmdDeal((const char *)DEBUG_RxBuffer);
            }
        }
    }
}

// ���سɹ�����������Ĳ���
// ����ֵ>0�ųɹ�(sscanf��ƥ�䷵��-1,ûƥ�䵽��������0)
static int AT_CmdDeal(const char *cmd)
{
    int res = 0;
    int para[12] = {0};

    res = sscanf(cmd, "AT:Run:%d,%d,%d", para, para + 1, para + 2);
    if (res == 3)
    {
        _Measure measure = {0};
        measure.fid = para[0];
        measure.isOnlyDuoCan = para[1];
        measure.isJiaBiao = para[2];

        Measure_Start(measure);
        goto end;
    }

    res = 0; // δ�ҵ�ƥ����

end:
    if (res <= 0)
        printf("AT CMD [Error]\r\n");
    return res;
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
