#include "bsp_include.h"

extern QueueHandle_t MeasureQueue;

volatile _ReportStatus gReportStatus;

u8 REGForGB[1024]; // 测量结果   国标

// 国标命令处理
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

// 将时间,工作模式,告警等数据更新至modbus数组中，以便上位机访问
void RefreshREG(void)
{
    u8 alarm = 0, error = 0;

    memset((void *)&gReportStatus, 0, sizeof(gReportStatus));

    // 状态告警区
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
    "空闲",
    "水样测量",
    "标样核查",
    "零点核查",
    "跨度核查",
    "空白测试",
    "平行样测试",
    "加标回收",
    "空白校准",
    "标液校准",
    "初始化",
};

const char ControCmdChangeRangeCHS[16] = "量程切换";
const char ControCmdOneStepCHS[16] = "单步调试";
const char ControCmdSimpleTestCHS[16] = "复合调试";

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

const char UnknownCmdCHS[16] = "未知命令";

const char StartCmdCHS[][16] = {
    "GB命令",
    "Debug命令",
    "屏幕操作",
    "串口命令",
    "定点模式",
    "间隔模式",
    "屏幕校准操作",
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
    "GB命令",
    "Debug命令",
    "屏幕操作",
    "串口命令",
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
    {no_alarm, "无"},
    {lr_alarm, "缺试剂"},
    {lw_alarm, "缺水样"},
    {lp_alarm, "缺纯水"},
    {ls_alarm, "缺标液"},
    {weeping_alarm, "漏液"},
    {cali_alarm, "标定异常"},
    {hot_alarm, "加热"},
    {uplimit_alarm, "超上限"},
    {downlimit_alarm, "超下限"},
    {other_alarm, "其他"},
    {range_switch_alarm, "量程切换"},
    {time_alarm, "时间未校准"},
    {sd_alarm, "SD卡异常"},
    {null_flow_alarm, "缺流程"},
    {error_flash_alarm, "芯片存储异常"},
    {null_formal_alarm, "缺公式"},

    {valve_commuication_alarm, "旋转阀通信异常"},
    {spec_commuication_alarm, "光谱仪通信异常"},
    {pump_commuication_alarm, "柱塞泵通信异常"},
};

char *GetAlarmCHS(uint8_t code)
{
    memset(AlarmCHSBuffer, 0, sizeof(AlarmCHSBuffer));
    for (int i = 0; i < sizeof(AlarmErrorCHS) / sizeof(AlarmErrorCHS[0]); i++)
    {
        if (code == AlarmErrorCHS[i].code)
        {
            memcpy(AlarmCHSBuffer, AlarmErrorCHS[i].CHS, sizeof(AlarmCHSBuffer));
            strcat(AlarmCHSBuffer, "告警");
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
            strcat(ErrorCHSBuffer, "故障");
            return ErrorCHSBuffer;
        }
    }

    sprintf(ErrorCHSBuffer, "%d", code);
    return ErrorCHSBuffer;
}
