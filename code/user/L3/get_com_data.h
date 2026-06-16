#ifndef __GET_COM_DATA_H
#define __GET_COM_DATA_H

#include "stdint.h"
#include "my_math.h"

#define LIMIT_MAX_MIN(MAX,MIN,DEFAULT,Value)    if((Value > MAX)||(Value < MIN)){Value = DEFAULT;}

typedef enum
{
    eADDRS_IDLE,
    eADDRS_FORWARD,
    eADDRS_BACKWARD,
    eADDRS_MAX,
} STATE_CHARGE_ADDRS_E;

typedef enum
{
    eSET_STANDARD_MODE,             // 标准模式
    eSET_CUSTOM_MODE,               // 定制模式
    eSET_BAT_MODE,                  // 电池模式
    eMPPT_MODE,                     // MPPT模式
    eSET_MODE_MAX,
} MODE_STATE_E;

typedef enum
{
    eSET_FORWARD,          // 正向
    eSET_BACKWARD,         // 反向
    eSET_AUTO_MODE,        // 自动
    eSET_MANUAL_MODE,      // 手动
    eSET_PG_CUSTOM_MODE,   // 外设
    eSET_CHARG_MAX,
}SET_MODE_STATE_E;

typedef enum
{
    eFORWARD,
    eBACKWARD,
    eAUTO_FORWARD,
    eAUTO_BACKWARD,
    eMANUAL_FORWARD,
    eMANUAL_BACKWARD,
    ePG_AUTO_FORWARD,
    ePG_AUTO_BACKWARD,
    ePG_MANUAL_FORWARD,
    ePG_MANUAL_BACKWARD,
    eCHARG_MODE_MAX,
} MODE_STATE_DIRECTION_E;

typedef enum
{
    eBAT_LA_AGM,
    eBAT_LA_GEL,
    eBAT_LI_LFP,
    eBAT_LI_NMC,
    eBAT_DCDC,
    eSCAP,
    eBAT_MODE_TYPE_MAX,
} BAT_TYPE_ENUM_E;

typedef enum
{
    eSYS_12V,
    eSYS_16V,
    eSYS_24V,
    eSYS_36V,
    eSYS_48V,
    eSYS_60V,
    eSYS_72V,
    eSYS_10_50V,
    eSYS_VOLT_MAX,
} SYS_VOLT_STATE_E;

typedef enum
{
    ePOWER_CHARGE,
    ePRE_CHARGE,
    eCC_CHARGE,
    eCV_CHARGE,
    eFLOAT_CHARGE,
    eSTOP_CHARGE,
    eFULL_CHARGE,
    eIDIE_CHARGE,
    eFAULT_CHARGE,
} BAT_CHARGE_MODE_E;

#pragma pack(1)

typedef struct
{
    float InpVolt;      // A端电压
    float InpCurr;      // A端电流
    uint16_t InpCurrPower; // A端功率
    float OutVolt;      // B端电压
    float OutCurr;      // B端电流
    uint16_t OutCurrPower; // B端功率
    int16_t InsideTemp;   // 内部温度
    int16_t OutsideTemp;  // 外部温度
    uint16_t PowerMode;    // 电源模式
    uint16_t ChargMode;    // 充电模式
    uint16_t FaultSign;    // 故障信号
    uint16_t AlarmSign;    // 告警信号
    float CompensationVoltA; // A端补偿
    float CompensationVoltB; // B端补偿
    int16_t Temp2;                // 器件温度2
    uint16_t StateCharge;         // 充电状态  
    float ADDVolt;                // ADD辅源电压
} com_realtime_data_t;

typedef struct
{
    uint16_t FactoryReset;        // 400: 恢复出厂设置
    uint16_t PowerOnOff;          // 401: 开关机状态
    uint16_t SetPowerMode;        // 402: 电源模式
    uint16_t SetChargMode;        // 403: 充电模式
    uint16_t InpBatyType;         // 404: A端电池类型（高8位类型，低8位电压）
    uint16_t OutBatyType;         // 405: B端电池类型（高8位类型，低8位电压）
    uint16_t SetBootTimeA;        // 406: A端开机时间
    uint16_t SetBootTimeB;        // 407: B端开机时间
    uint16_t SetOnCurrStartTimeA; // 408: A端开机电流软起动时间
    uint16_t SetOnCurrStartTimeB; // 409: B端开机电流软起动时间
    uint16_t ZeroCurrCalibration; // 40A: 端零电流校准
    uint16_t ResetFactoryData;    // 40B: 恢复厂家数据
    uint16_t BatModeFR;           // 40C: 电池模式正反向切换
    uint16_t MpptSwitch;          // 40D: MPPT开关
    uint16_t SleepModeOnOff;
} com_ctrl_t;

typedef struct
{
    uint16_t InpVoltCalibrK;     // A端电压校准K值
    uint16_t InpVoltCalibrB;     // A端电压校准B值
    uint16_t InpCurrCalibrK;     // A端电流校准K值
    uint16_t InpCurrCalibrB;     // A端电流校准B值
    uint16_t InpShowVoltCalibrK; // A端显示电压校准K值
    uint16_t InpShowVoltCalibrB; // A端显示电压校准B值
    uint16_t InpShowCurrCalibrK; // A端显示电流校准K值
    uint16_t InpShowCurrCalibrB; // A端显示电流校准B值
    uint16_t OutVoltCalibrK;     // B端电压校准K值
    uint16_t OutVoltCalibrB;     // B端电压校准B值
    uint16_t OutCurrCalibrK;     // B端电流校准K值
    uint16_t OutCurrCalibrB;     // B端电流校准B值
    uint16_t OutShowVoltCalibrK; // B端显示电压校准K值
    uint16_t OutShowVoltCalibrB; // B端显示电压校准B值
    uint16_t OutShowCurrCalibrK; // B端显示电流校准K值
    uint16_t OutShowCurrCalibrB; // B端显示电流校准B值
    uint16_t BOutShowCurrCalibrK;// B端显示电流校准K值	
    uint16_t BOutShowCurrCalibrB;// B端显示电流校准B值	
    uint16_t AOutShowCurrCalibrK;// A端显示电流校准K值	
    uint16_t AOutShowCurrCalibrB;// A端显示电流校准B值	
    uint16_t VoltCompensationAK; // ACC/A端电压补偿校准K值	
    uint16_t VoltCompensationAB; // ACC/A端电压补偿校准B值	
    uint16_t VoltCompensationBK; // RTM/B端电压补偿校准K值	
    uint16_t VoltCompensationBB; // RTM/B端电压补偿校准B值	
    uint16_t Retain10[2];        // 保留
    float SetInpVolt;            // A端电压
    float SetInpCurr;            // A端电流
    uint16_t SetInpCurrPower;    // A端功率
    float SetOutVolt;            // B端电压
    float SetOutCurr;            // B端电流
    uint16_t SetOutCurrPower;    // B端功率
    float SetInpUvlo;            // A端欠压保护
    float SetInpUvloRecover;     // A端欠压保护恢复
    float SetInpOVP;             // A端过压保护
    float SetInpOVPRecover;      // A端过压保护恢复
    float SetOutUvlo;            // B端欠压保护
    float SetOutUvloRecover;     // B端欠压保护恢复
    float SetOutOVP;             // B端过压保护
    float SetOutOVPRecover;      // B端过压保护恢复
    int16_t SetInsideTemp;       // 内部温度
    int16_t SetOutsideTemp;      // 外部温度
    float SetInpChargLedCurr;    // A端充电指示灯电流
    float SetInpFullLedCurr;     // A端充满指示灯电流
    float SetOutChargLedCurr;    // B端充电指示灯电流
    float SetOutFullLedCurr;     // B端充满指示灯电流
    float AuotForwardOpenVoltA;  // 自动模式正向A端开启电压
    float AuotForwardVeerVoltA;  // 自动模式正向转向A电压
    float AuotForwardShutVoltA;  // 自动模式正向A端关闭电压
    float AuotReverseOpenVoltB;  // 自动模式反向B端开启电压
    float AuotReverseShutVoltB;  // 自动模式反向B端关闭电压
    int16_t SetTemp2;            // 内部温度
} com_param_t;

typedef struct
{
    uint16_t BatModeFRState;     // 上位机设置正反向状态
    // P01
    com_realtime_data_t com_realtime_data;
    // P02
    com_ctrl_t com_ctrl;
    // P03
    com_param_t com_param;
} get_wg_com_v2_data_t;

extern get_wg_com_v2_data_t get_wg_com_v2_data;

typedef struct
{
    uint8_t  sucCurrentReached90Percent;
    uint8_t  sucPowerReached90Percent;
    uint8_t  LithiumBatOnOff;
    uint8_t  Reserve;
}bat_state_t;

typedef struct
{
    float val;                         // 当前值
    float limit;                       // 触发阈值
    float recover;                     // 恢复阈值    
    uint8_t enable;                    // 使能
} protect_item_data_t;

typedef struct
{
    protect_item_data_t protect_item_acc;
    protect_item_data_t protect_item_rtm;
    protect_item_data_t protect_item_fvs48_uvp;
    protect_item_data_t protect_item_fvs48_ovp;
    protect_item_data_t protect_item_rvs12_uvp;
    protect_item_data_t protect_item_rvs12_ovp;
    uint16_t over_temp_protect;
}protect_data_t;

typedef struct
{
    uint8_t SetBootTimeFlag;     // 启动开机延时标志位
    uint8_t retain;              // 保留
    uint16_t BootTimeDelay;      // 开机延时计数
    uint16_t SetBootTime;        // 开机时间
}Boot_Time_Delay_t;

typedef struct
{
    uint8_t get_is_run;          // 获取是否允许启动输出电流
    uint8_t dysfunction_is_run;  // 异常功能
    uint8_t soft_start_flag;
    uint8_t soft_close_flag;
    uint8_t mppt_mode_flag;      // mppt模式
    uint8_t retain;
    uint16_t check_state;        // 输出状态
    uint16_t OutBatyType;        // 电池类型
    uint16_t fvs48_pwr_lmt;
    uint16_t rvs12_pwr_lmt;
    uint16_t SetCharState;       // 充电状态
    float SetInpCurr;            // 输入电流
    float SetOutVolt;            // 输出电压
    float SetOutCurr;            // 输出电流
    float ActualOutVolt;         // 真实输出电压
    float ActualOutCurr;         // 真实输出电流
    float set_out_lmt_curr;      // 最小输出电流
    float rvs12_lmt;
    float fvs48_lmt;
    float ilv_lmt;
    float ihv_lmt;
    float temp_derate_curr;      // 温度降额电流
    Boot_Time_Delay_t Boot_Time_Delay;
    bat_state_t bat_state;
    protect_data_t protect_data; // 保护
} charge_state_data_t;

extern charge_state_data_t charge_state_data;

typedef struct
{
    uint16_t SetPowerMode;
    uint16_t SetChargMode;
    uint16_t InpBatyType;
    uint16_t OutBatyType;
} state_control_data_t;

extern state_control_data_t State_Control_Data;



#pragma pack()
uint8_t updated_parameter(void);
uint16_t Get_Charge_State(void);
void set_charge_state_mode(BAT_CHARGE_MODE_E state);
void get_wg_com_data_rum(void);
uint8_t float_equal(float x, float y);
void get_wg_com_data_rum(void);
#endif
