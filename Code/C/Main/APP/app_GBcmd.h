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
  uint16_t runTime;             // �����̿�ʼ������,ֹͣʱ������ʱ��
  char describe[8];             // ��ǰ����ʣ��ʱ��,�Լ������¶ȵ���Ϣ
  char flowDescribe[64];        // ��ǰ����������������
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
  one_step_test = 0xFE, // ��������
  simple_test = 0xFF,   // ���ϵ���
} ControCmd;

typedef enum
{
  StopGB = 0,
  StopDebug,
  StopScreen,
  StopUart,
  StopError,
  StopUnknown = 0xFF, // δ֪
} StopMode;           // ֹͣ����

typedef enum
{
  StartGB = 0,
  StartDebug,
  StartScreen,
  StartUart,
  StartPeriod,          // ����
  StartTiming,          // ���
  StartScreenCalibrate, // У׼
  StartUnknown = 0xFF,  // δ֪
} StartMode;            // ֹͣ����

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
} ControMode; // ���궨��

typedef enum
{
  no_alarm,                // �޸澯
  lr_alarm,                // ȱ�Լ��澯
  lw_alarm,                // ȱˮ���澯
  lp_alarm,                // ȱ��ˮ�澯
  ls_alarm,                // ȱ��Һ�澯
  weeping_alarm,           // ©Һ�澯
  cali_alarm,              // �궨�쳣�澯
  hot_alarm = 8,           // ���ȸ澯
  uplimit_alarm = 10,      // �����޸澯
  downlimit_alarm,         // �����޸澯
  other_alarm,             // �����澯
  range_switch_alarm = 15, // �����л��澯
  time_alarm = 34,         // ʱ��δУ׼�澯
  sd_alarm = 35,           // SD���쳣�澯
  null_flow_alarm = 36,    // ȱ���̸澯
  error_flash_alarm = 37,  // оƬ�洢�쳣�澯
  null_formal_alarm = 38,  // ȱ��ʽ�澯

  shuiliu_alarm = 48,     // ˮ���쳣
  moto_switch_alarm = 49, // ��������л�
  yiji_yewei_alarm = 50,  // һ��Һλ��ʱ�쳣
  erji_yewei_alarm = 51,  // ������Һλ��ʱ�쳣
  sanji_yewei_alarm = 52, // ������Һλ��ʱ�쳣

  yiji_yewei_bushui_alarm = 53,  // һ��Һλ��ʱ�쳣(��ˮ)
  erji_yewei_bushui_alarm = 54,  // ������Һλ��ʱ�쳣(��ˮ)
  sanji_yewei_bushui_alarm = 55, // ������Һλ��ʱ�쳣(��ˮ)

  sub_commuication_alarm = 255, // ����ͨ���쳣�澯
} AlarmInfo;

typedef enum
{
  ug_L = 0,
  mg_L = 1,
  ppm = 2,
  mg_m3 = 3,
  cm = 4,
  percent = 7,    //%
  celsius_C = 17, // ��C
} Unit;

#define DataId_N "N"     // ����
#define DataId_N_ra "ra" // �ӱ��������
#define DataId_N_pt "pt" // ƽ������������
#define DataId_T "T"     // ������           ���Ũ�ȳ�������������
#define DataId_L "L"     // ������           ���Ũ�ȳ��������޻�С�ڼ����
#define DataId_D "D"     // ��������
#define DataId_F "F"     // ����ͨ�Ź���        �������ݲɼ�ʧ��
#define DataId_B "B"     // ��������          �������ߣ�����ͨ��������
#define DataId_lr "lr"   // ȱ�Լ�
#define DataId_lp "lp"   // ȱ��ˮ
#define DataId_lw "lw"   // ȱˮ��
#define DataId_ls "ls"   // ȱ����

extern volatile _ReportStatus gReportStatus;

extern u8 REGForGB[1024]; // �������   ����

u8 DoGBCmd(u16 mode, u8 *data, u8 datacount);
void RefreshREG(void);

char *GetControCmdCHS(uint8_t cmd);
char *GetStartCmdCHS(uint8_t cmd);
char *GetStopCmdCHS(uint8_t cmd);
char *GetAlarmCHS(uint8_t code);
char *GetErrorCHS(uint8_t code);

#endif
