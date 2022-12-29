#include "bsp_include.h"

extern QueueHandle_t MeasureQueue;

volatile _ReportStatus gReportStatus;

u8 REGForGB[1024]; // �������   ����

// ���������
u8 DoGBCmd(u16 mode, u8 *data, u8 datacount)
{
    u8 res = 0;
    u8 modBusError = ModBusNoError;

    switch (mode)
    {
    case standard_test:
    case sample_test:
    case zero_test:
    case span_test:
    case blank_test:
    case parallel_test:
    case recovery_test:
    case blank_calibration_test:
    case sample_calibration_test:
        printf("[%d]_test\r\n", mode);
        if (gWorkStatus != idle)
        {
            modBusError = ModBusDeviceBusy;
            break;
        }

        break;

    case InitOrClean:
        printf("InitOrClean\r\n");
        break;

    case StopMeasure:
        printf("StopMeasureAll\r\n");
        Measure_Stop(StopGB);
        break;

    case SystemReboot:
        printf("SystemReboot\r\n");
        gSysReboot = 1;
        break;

    case CalibrateTime:
        printf("CalibrateTime\r\n");
        {
            _Time timeTemp;
            Data_TO_RTC_Time(&timeTemp, data);
            res = RTC_Set_Time(Time_Format_BCD, timeTemp);
            if (res == 0)
                modBusError = ModBusDeviceFault;
        }
        break;

    default:
        modBusError = ModBusFunIllegal;
        break;
    }

    return modBusError;
}

// ��ʱ��,����ģʽ,�澯�����ݸ�����modbus�����У��Ա���λ������
void RefreshREG(void)
{
    u8 alarm = 0, error = 0;

    memset((void *)&gReportStatus, 0, sizeof(gReportStatus));

    // ״̬�澯��
    RTC_Time_To_Data(gtTimeBdc, (uint8_t *)gReportStatus.date);

    gReportStatus.status = gWorkStatus;
    gReportStatus.mode = controlledMode;

    gReportStatus.log = gtPara.step;

    gReportStatus.version = VERSION_CODE;

    gReportStatus.instrument_temperature = 0;
    gReportStatus.instrument_humidity = 0;

    Alarm_Get(1, &alarm, &error);
    gReportStatus.alarm = alarm;
    gReportStatus.error = error;

    memcpy(REGForGB + REG_STATE_INDEX, (const void *)&gReportStatus, sizeof(gReportStatus));
}

const char ControCmdCHS[][16] = {
    "����",
    "ˮ������",
    "�����˲�",
    "���˲�",
    "��Ⱥ˲�",
    "�հײ���",
    "ƽ��������",
    "�ӱ����",
    "�հ�У׼",
    "��ҺУ׼",
    "��ʼ��",
};

const char ControCmdChangeRangeCHS[16] = "�����л�";
const char ControCmdOneStepCHS[16] = "��������";
const char ControCmdSimpleTestCHS[16] = "���ϵ���";

static char ControCmdTemp[16] = {0};
char *GetControCmdCHS(uint8_t cmd)
{
    if (cmd < sizeof(ControCmdCHS) / 16)
    {
        return (char *)ControCmdCHS[cmd];
    }

    if (cmd == change_range_test)
        return (char *)ControCmdChangeRangeCHS;
    if (cmd == one_step_test)
        return (char *)ControCmdOneStepCHS;
    if (cmd == simple_test)
        return (char *)ControCmdSimpleTestCHS;

    memset(ControCmdTemp, 0, 16);
    sprintf(ControCmdTemp, "FID[%d]", cmd);
    return ControCmdTemp;
}

const char UnknownCmdCHS[16] = "δ֪����";

const char StartCmdCHS[][16] = {
    "GB����",
    "Debug����",
    "��Ļ����",
    "��������",
    "����ģʽ",
    "���ģʽ",
    "��ĻУ׼����",
};
char *GetStartCmdCHS(uint8_t cmd)
{
    if (cmd < sizeof(StartCmdCHS) / 16)
    {
        return (char *)StartCmdCHS[cmd];
    }
    else
    {
        return (char *)UnknownCmdCHS;
    }
}

const char StopCmdCHS[][16] = {
    "GB����",
    "Debug����",
    "��Ļ����",
    "��������",
};
char *GetStopCmdCHS(uint8_t cmd)
{
    if (cmd < sizeof(StopCmdCHS) / 16)
    {
        return (char *)StopCmdCHS[cmd];
    }
    else
    {
        return (char *)UnknownCmdCHS;
    }
}

typedef struct
{
    uint8_t code;
    char CHS[24];
} _AlarmErrorCHS;

char AlarmCHSBuffer[24];
char ErrorCHSBuffer[24];

const _AlarmErrorCHS AlarmErrorCHS[] = {
    {no_alarm, "��"},
    {lr_alarm, "ȱ�Լ�"},
    {lw_alarm, "ȱˮ��"},
    {lp_alarm, "ȱ��ˮ"},
    {ls_alarm, "ȱ��Һ"},
    {weeping_alarm, "©Һ"},
    {cali_alarm, "�궨�쳣"},
    {hot_alarm, "����"},
    {uplimit_alarm, "������"},
    {downlimit_alarm, "������"},
    {other_alarm, "����"},
    {range_switch_alarm, "�����л�"},
    {time_alarm, "ʱ��δУ׼"},
    {sd_alarm, "SD���쳣"},
    {null_flow_alarm, "ȱ����"},
    {error_flash_alarm, "оƬ�洢�쳣"},
    {null_formal_alarm, "ȱ��ʽ"},

    {valve_commuication_alarm, "��ת��ͨ���쳣"},
    {spec_commuication_alarm, "������ͨ���쳣"},
    {pump_commuication_alarm, "������ͨ���쳣"},
};

char *GetAlarmCHS(uint8_t code)
{
    memset(AlarmCHSBuffer, 0, sizeof(AlarmCHSBuffer));
    for (int i = 0; i < sizeof(AlarmErrorCHS) / sizeof(AlarmErrorCHS[0]); i++)
    {
        if (code == AlarmErrorCHS[i].code)
        {
            memcpy(AlarmCHSBuffer, AlarmErrorCHS[i].CHS, sizeof(AlarmCHSBuffer));
            strcat(AlarmCHSBuffer, "�澯");
            return AlarmCHSBuffer;
        }
    }

    sprintf(AlarmCHSBuffer, "%d", code);
    return AlarmCHSBuffer;
}

char *GetErrorCHS(uint8_t code)
{
    memset(ErrorCHSBuffer, 0, sizeof(ErrorCHSBuffer));
    for (int i = 0; i < sizeof(AlarmErrorCHS) / sizeof(AlarmErrorCHS[0]); i++)
    {
        if (code == AlarmErrorCHS[i].code)
        {
            memcpy(ErrorCHSBuffer, AlarmErrorCHS[i].CHS, sizeof(ErrorCHSBuffer));
            strcat(ErrorCHSBuffer, "����");
            return ErrorCHSBuffer;
        }
    }

    sprintf(ErrorCHSBuffer, "%d", code);
    return ErrorCHSBuffer;
}
