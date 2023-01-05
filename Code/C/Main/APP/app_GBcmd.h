#ifndef _APP_GB_H
#define _APP_GB_H

#define REG_MEASURE_INDEX (0x0 * 2)
#define REG_STATE_INDEX (0x80 * 2)

#pragma pack(1)
typedef struct
{
  u8 date[6];                   // 0
  u16 status;                   // 6
  u16 mode;                     // 8
  u16 alarm;                    // 10
  u16 error;                    // 12
  u16 log;                      // 14
  u16 version;                  // 16
  float instrument_temperature; //
  float instrument_humidity;    //
  uint16_t runTime;             // 从流程开始到现在,停止时代表总时间
  char describe[8];             // 当前步骤剩余时间,试剂量，温度等信息
  char flowDescribe[64];        // 当前步骤流程中文名字
} _ReportStatus;

#pragma pack()

typedef enum
{
  standard_test = 1,
  sample_test,
  zero_test,
  span_test,
  blank_test,
  parallel_test,
  recovery_test,
  blank_calibration_test = 8,
  sample_calibration_test = 9,
  InitOrClean = 10,
  StopMeasure = 11,
  SystemReboot = 12,
  CalibrateTime = 13,

  change_range_test = 0xF0,
  one_step_test = 0xFE, // 单步调试
  simple_test = 0xFF,   // 复合调试
} ControCmd;

typedef enum
{
  StopGB = 0,
  StopDebug,
  StopScreen,
  StopUart,
  StopError,
  StopUnknown = 0xFF, // 未知
} StopMode;           // 停止类型

typedef enum
{
  StartGB = 0,
  StartDebug,
  StartScreen,
  StartUart,
  StartPeriod,          // 定点
  StartTiming,          // 间隔
  StartScreenCalibrate, // 校准
  StartUnknown = 0xFF,  // 未知
} StartMode;            // 停止类型

typedef enum
{
  idle = 0,
  waterSampleTest = 1,
  blankCalibrate = 8,
  standardCalibrate = 9,
  initOrCleaning = 10,
} WorkStatus;

typedef enum
{
  continuousMode = 1,
  periodMode,
  timingMode,
  controlledMode,
  handMode,
} ControMode; // 国标定义

typedef enum
{
  no_alarm,                // 无告警
  lr_alarm,                // 缺试剂告警
  lw_alarm,                // 缺水样告警
  lp_alarm,                // 缺纯水告警
  ls_alarm,                // 缺标液告警
  weeping_alarm,           // 漏液告警
  cali_alarm,              // 标定异常告警
  hot_alarm = 8,           // 加热告警
  uplimit_alarm = 10,      // 超上限告警
  downlimit_alarm,         // 超下限告警
  other_alarm,             // 其他告警
  range_switch_alarm = 15, // 量程切换告警
  time_alarm = 34,         // 时间未校准告警
  sd_alarm = 35,           // SD卡异常告警
  null_flow_alarm = 36,    // 缺流程告警
  error_flash_alarm = 37,  // 芯片存储异常告警
  null_formal_alarm = 38,  // 缺公式告警

  shuiliu_alarm = 48,     // 水流异常
  moto_switch_alarm = 49, // 发生电机切换
  yiji_yewei_alarm = 50,  // 一级液位超时异常
  erji_yewei_alarm = 51,  // 二级级液位超时异常
  sanji_yewei_alarm = 52, // 三级级液位超时异常

  yiji_yewei_bushui_alarm = 53,  // 一级液位超时异常(补水)
  erji_yewei_bushui_alarm = 54,  // 二级级液位超时异常(补水)
  sanji_yewei_bushui_alarm = 55, // 三级级液位超时异常(补水)

  sub_commuication_alarm = 255, // 副板通信异常告警
} AlarmInfo;

typedef enum
{
  ug_L = 0,
  mg_L = 1,
  ppm = 2,
  mg_m3 = 3,
  cm = 4,
  percent = 7,    //%
  celsius_C = 17, // °C
} Unit;

#define DataId_N "N"     // 正常
#define DataId_N_ra "ra" // 加标回收正常
#define DataId_N_pt "pt" // 平行样测试正常
#define DataId_T "T"     // 超上限           监测浓度超仪器测量上限
#define DataId_L "L"     // 超下限           监测浓度超仪器下限或小于检出限
#define DataId_D "D"     // 仪器故障
#define DataId_F "F"     // 仪器通信故障        仪器数据采集失败
#define DataId_B "B"     // 仪器离线          仪器离线（数据通信正常）
#define DataId_lr "lr"   // 缺试剂
#define DataId_lp "lp"   // 缺纯水
#define DataId_lw "lw"   // 缺水样
#define DataId_ls "ls"   // 缺标样

extern volatile _ReportStatus gReportStatus;

extern u8 REGForGB[1024]; // 测量结果   国标

u8 DoGBCmd(u16 mode, u8 *data, u8 datacount);
void RefreshREG(void);

char *GetControCmdCHS(uint8_t cmd);
char *GetStartCmdCHS(uint8_t cmd);
char *GetStopCmdCHS(uint8_t cmd);
char *GetAlarmCHS(uint8_t code);
char *GetErrorCHS(uint8_t code);

#endif
