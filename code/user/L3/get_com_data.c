#include "get_com_data.h"
#include "wg_com_v2.h"
#include "section.h"

get_wg_com_v2_data_t get_wg_com_v2_data;    // 获取WG_COM_V2数据
charge_state_data_t  charge_state_data;     // charge state data
state_control_data_t State_Control_Data;    // state control data
static uint8_t force_update_parameter = 0;
static uint8_t power_mode_changed_update = 0;
static uint8_t out_baty_type_changed_update = 0;
static void ovp_uvp_rande_limt(void);
static uint16_t wg_raw_u16(void *data)
{
    return get_uint16((uint8_t *)data);
}

static int16_t wg_raw_i16(void *data)
{
    return get_int16((uint8_t *)data);
}

static float wg_raw_u16_scale(void *data, float scale)
{
    return (float)wg_raw_u16(data) * scale;
}
static void normalize_note_non_mppt_if_changed(uint16_t power_mode, uint16_t bat_mode_fr, uint16_t sleep_mode)
{
    static uint8_t cached_valid = 0U;
    static uint16_t cached_power_mode = 0xFFFFU;
    static uint16_t cached_bat_mode_fr = 0xFFFFU;
    static uint16_t cached_sleep_mode = 0xFFFFU;
    static uint16_t cached_type_a = 0xFFFFU;
    static uint16_t cached_type_b = 0xFFFFU;
    static uint16_t cached_boot_a = 0xFFFFU;
    static uint16_t cached_boot_b = 0xFFFFU;
    static uint16_t cached_soft_a = 0xFFFFU;
    static uint16_t cached_soft_b = 0xFFFFU;
    uint16_t type_a = 0U;
    uint16_t type_b = 0U;
    uint16_t boot_a = 0U;
    uint16_t boot_b = 0U;
    uint16_t soft_a = 0U;
    uint16_t soft_b = 0U;

    if((power_mode >= eSET_MODE_MAX) || (power_mode == eMPPT_MODE))
    {
        return;
    }

    if(power_mode != eSET_BAT_MODE)
    {
        sleep_mode = 0U;
    }
    else
    {
        type_a = wg_raw_u16(&wg_com_v2_ctrl.InpBatyType);
        type_b = wg_raw_u16(&wg_com_v2_ctrl.OutBatyType);
        boot_a = wg_raw_u16(&wg_com_v2_ctrl.SetBootTimeA);
        boot_b = wg_raw_u16(&wg_com_v2_ctrl.SetBootTimeB);
        soft_a = wg_raw_u16(&wg_com_v2_ctrl.SetOnCurrStartTimeA);
        soft_b = wg_raw_u16(&wg_com_v2_ctrl.SetOnCurrStartTimeB);
    }

    if((cached_valid == 0U) ||
       (cached_power_mode != power_mode) ||
       (cached_bat_mode_fr != bat_mode_fr) ||
       (cached_sleep_mode != sleep_mode) ||
       (cached_type_a != type_a) ||
       (cached_type_b != type_b) ||
       (cached_boot_a != boot_a) ||
       (cached_boot_b != boot_b) ||
       (cached_soft_a != soft_a) ||
       (cached_soft_b != soft_b))
    {
        wg_com_v2_note_non_mppt_control_state(power_mode, bat_mode_fr);
        cached_valid = 1U;
        cached_power_mode = power_mode;
        cached_bat_mode_fr = bat_mode_fr;
        cached_sleep_mode = sleep_mode;
        cached_type_a = type_a;
        cached_type_b = type_b;
        cached_boot_a = boot_a;
        cached_boot_b = boot_b;
        cached_soft_a = soft_a;
        cached_soft_b = soft_b;
    }
}

static void normalize_exclusive_power_mode(void)
{
    uint16_t power_mode = wg_raw_u16(&wg_com_v2_ctrl.SetPowerMode);
    uint16_t bat_mode_fr = wg_raw_u16(&wg_com_v2_ctrl.BatModeFR);
    uint16_t mppt_switch = wg_raw_u16(&wg_com_v2_ctrl.MpptSwitch);
    uint16_t sleep_mode = wg_raw_u16(&wg_com_v2_ctrl.SleepModeOnOff);
    uint16_t boot_time_b = wg_raw_u16(&wg_com_v2_ctrl.SetBootTimeB);
    uint16_t soft_start_a = wg_raw_u16(&wg_com_v2_ctrl.SetOnCurrStartTimeA);
    uint16_t soft_start_b = wg_raw_u16(&wg_com_v2_ctrl.SetOnCurrStartTimeB);

    if(mppt_switch == 1U)
    {
        if((power_mode != eMPPT_MODE) ||
           (bat_mode_fr != 1U) ||
           (sleep_mode != 0U))
        {
            wg_com_v2_enter_mppt_control_state();
            request_update_parameter();
        }
        if(boot_time_b != 0U)
        {
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetBootTimeB);
            request_update_parameter();
        }
        if(soft_start_a != 0U)
        {
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetOnCurrStartTimeA);
            request_update_parameter();
        }
    }
    else if(power_mode == eMPPT_MODE)
    {
        wg_com_v2_exit_mppt_control_state();
        request_update_parameter();
    }
    else if(power_mode == eSET_STANDARD_MODE)
    {
        if(bat_mode_fr != 1U)
        {
            WG_COM_V2_SET_DATA_UINT(1, wg_com_v2_ctrl.BatModeFR);
            bat_mode_fr = 1U;
        }
        if(soft_start_a != 0U)
        {
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetOnCurrStartTimeA);
        }
        if(soft_start_b != 0U)
        {
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetOnCurrStartTimeB);
        }
        normalize_note_non_mppt_if_changed(power_mode, 1U, 0U);
    }
    else
    {
        normalize_note_non_mppt_if_changed(power_mode, bat_mode_fr, sleep_mode);
    }

    power_mode = wg_raw_u16(&wg_com_v2_ctrl.SetPowerMode);
    sleep_mode = wg_raw_u16(&wg_com_v2_ctrl.SleepModeOnOff);

    if((power_mode != eSET_BAT_MODE) &&
       (sleep_mode != 0U))
    {
        WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SleepModeOnOff);
    }
}
void get_wg_com_data_fast_rum(void)
{
    normalize_exclusive_power_mode();

    get_wg_com_v2_data.BatModeFRState = wg_raw_u16(&wg_com_v2_ctrl.BatModeFR);

    get_wg_com_v2_data.com_realtime_data.InpVolt = wg_raw_u16_scale(&wg_com_v2_realtime_data.InpVolt, 0.01f);
    get_wg_com_v2_data.com_realtime_data.InpCurr = wg_raw_u16_scale(&wg_com_v2_realtime_data.InpCurr, 0.01f);
    get_wg_com_v2_data.com_realtime_data.InpCurrPower = wg_raw_u16(&wg_com_v2_realtime_data.InpCurrPower);
    get_wg_com_v2_data.com_realtime_data.OutVolt = wg_raw_u16_scale(&wg_com_v2_realtime_data.OutVolt, 0.01f);
    get_wg_com_v2_data.com_realtime_data.OutCurr = wg_raw_u16_scale(&wg_com_v2_realtime_data.OutCurr, 0.01f);
    get_wg_com_v2_data.com_realtime_data.OutCurrPower = wg_raw_u16(&wg_com_v2_realtime_data.OutCurrPower);
    get_wg_com_v2_data.com_realtime_data.InsideTemp = wg_raw_i16(&wg_com_v2_realtime_data.InsideTemp);
    get_wg_com_v2_data.com_realtime_data.OutsideTemp = wg_raw_i16(&wg_com_v2_realtime_data.OutsideTemp);
    get_wg_com_v2_data.com_realtime_data.PowerMode = wg_raw_u16(&wg_com_v2_realtime_data.PowerMode);
    get_wg_com_v2_data.com_realtime_data.ChargMode = wg_raw_u16(&wg_com_v2_realtime_data.ChargMode);
    get_wg_com_v2_data.com_realtime_data.FaultSign = wg_raw_u16(&wg_com_v2_realtime_data.FaultSign);
    get_wg_com_v2_data.com_realtime_data.AlarmSign = wg_raw_u16(&wg_com_v2_realtime_data.AlarmSign);
    get_wg_com_v2_data.com_realtime_data.CompensationVoltA = wg_raw_u16_scale(&wg_com_v2_realtime_data.CompensationVoltA, 0.01f);
    get_wg_com_v2_data.com_realtime_data.CompensationVoltB = wg_raw_u16_scale(&wg_com_v2_realtime_data.CompensationVoltB, 0.01f);
    get_wg_com_v2_data.com_realtime_data.Temp2 = wg_raw_i16(&wg_com_v2_realtime_data.Temp2);
    get_wg_com_v2_data.com_realtime_data.StateCharge = wg_raw_u16(&wg_com_v2_realtime_data.StateCharge);
    get_wg_com_v2_data.com_realtime_data.ADDVolt = wg_raw_u16_scale(&wg_com_v2_realtime_data.ADDVolt, 0.01f);

    get_wg_com_v2_data.com_ctrl.FactoryReset = wg_raw_u16(&wg_com_v2_ctrl.FactoryReset);
    get_wg_com_v2_data.com_ctrl.PowerOnOff = wg_raw_u16(&wg_com_v2_ctrl.PowerOnOff);
    get_wg_com_v2_data.com_ctrl.SetPowerMode = wg_raw_u16(&wg_com_v2_ctrl.SetPowerMode);
    get_wg_com_v2_data.com_ctrl.SetChargMode = wg_raw_u16(&wg_com_v2_ctrl.SetChargMode);
    get_wg_com_v2_data.com_ctrl.InpBatyType = wg_raw_u16(&wg_com_v2_ctrl.InpBatyType);
    get_wg_com_v2_data.com_ctrl.OutBatyType = wg_raw_u16(&wg_com_v2_ctrl.OutBatyType);
    get_wg_com_v2_data.com_ctrl.SetBootTimeA = wg_raw_u16(&wg_com_v2_ctrl.SetBootTimeA);
    get_wg_com_v2_data.com_ctrl.SetBootTimeB = wg_raw_u16(&wg_com_v2_ctrl.SetBootTimeB);
    get_wg_com_v2_data.com_ctrl.SetOnCurrStartTimeA = wg_raw_u16(&wg_com_v2_ctrl.SetOnCurrStartTimeA);
    get_wg_com_v2_data.com_ctrl.SetOnCurrStartTimeB = wg_raw_u16(&wg_com_v2_ctrl.SetOnCurrStartTimeB);
    get_wg_com_v2_data.com_ctrl.ZeroCurrCalibration = wg_raw_u16(&wg_com_v2_ctrl.ZeroCurrCalibration);
    get_wg_com_v2_data.com_ctrl.ResetFactoryData = wg_raw_u16(&wg_com_v2_ctrl.ResetFactoryData);
    get_wg_com_v2_data.com_ctrl.BatModeFR = wg_raw_u16(&wg_com_v2_ctrl.BatModeFR);
    get_wg_com_v2_data.com_ctrl.MpptSwitch = wg_raw_u16(&wg_com_v2_ctrl.MpptSwitch);
    get_wg_com_v2_data.com_ctrl.SleepModeOnOff = wg_raw_u16(&wg_com_v2_ctrl.SleepModeOnOff);

    if(get_wg_com_v2_data.com_realtime_data.PowerMode != get_wg_com_v2_data.com_ctrl.SetPowerMode)
    {
        WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SetPowerMode, wg_com_v2_realtime_data.PowerMode);
    }
}

void get_wg_com_data_rum(void)
{
    normalize_exclusive_power_mode();
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.BatModeFRState, wg_com_v2_ctrl.BatModeFR);                                                // 上位机设置正反向状态    // P01实时数据区
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.InpVolt,wg_com_v2_realtime_data.InpVolt);                               // A端电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.InpCurr,wg_com_v2_realtime_data.InpCurr);                               // A端电流
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.InpCurrPower,wg_com_v2_realtime_data.InpCurrPower);                     // A端功率
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.OutVolt,wg_com_v2_realtime_data.OutVolt);                               // B端电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.OutCurr,wg_com_v2_realtime_data.OutCurr);                               // B端电流
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.OutCurrPower,wg_com_v2_realtime_data.OutCurrPower);                     // B端功率
    WG_COM_V2_GET_DATA_INT(get_wg_com_v2_data.com_realtime_data.InsideTemp,wg_com_v2_realtime_data.InsideTemp);                          // 内部温度
    WG_COM_V2_GET_DATA_INT(get_wg_com_v2_data.com_realtime_data.OutsideTemp,wg_com_v2_realtime_data.OutsideTemp);                        // 外部温度
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.PowerMode,wg_com_v2_realtime_data.PowerMode);                           // 电源模式
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.ChargMode,wg_com_v2_realtime_data.ChargMode);                           // 充电模式
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.FaultSign,wg_com_v2_realtime_data.FaultSign);                           // 故障信号
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.AlarmSign,wg_com_v2_realtime_data.AlarmSign);                           // 告警信号
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.CompensationVoltA,wg_com_v2_realtime_data.CompensationVoltA);           // A端补偿
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.CompensationVoltB,wg_com_v2_realtime_data.CompensationVoltB);           // B端补偿
    WG_COM_V2_GET_DATA_INT(get_wg_com_v2_data.com_realtime_data.Temp2,wg_com_v2_realtime_data.Temp2);                                    // 器件温度2
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.StateCharge,wg_com_v2_realtime_data.StateCharge);                       // 充电状态
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_realtime_data.ADDVolt,wg_com_v2_realtime_data.ADDVolt);                               // ADD辅源电压
    // P02控制设置
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.FactoryReset,wg_com_v2_ctrl.FactoryReset);                                       // 恢复出厂设置
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.PowerOnOff,wg_com_v2_ctrl.PowerOnOff);                                           // 开关机状态
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SetPowerMode,wg_com_v2_ctrl.SetPowerMode);                                       // 电源模式
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SetChargMode,wg_com_v2_ctrl.SetChargMode);                                       // 充电模式
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.InpBatyType,wg_com_v2_ctrl.InpBatyType);                                         // A端电池类型，高位类型，低位电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.OutBatyType,wg_com_v2_ctrl.OutBatyType);                                         // B端电池类型，高位类型，低位电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SetBootTimeA,wg_com_v2_ctrl.SetBootTimeA);                                       // A端开机时间
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SetBootTimeB,wg_com_v2_ctrl.SetBootTimeB);                                       // B端开机时间
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SetOnCurrStartTimeA,wg_com_v2_ctrl.SetOnCurrStartTimeA);                         // A端开机电流软起动时间
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SetOnCurrStartTimeB,wg_com_v2_ctrl.SetOnCurrStartTimeB);                         // B端开机电流软起动时间
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.ZeroCurrCalibration,wg_com_v2_ctrl.ZeroCurrCalibration);                         // 端零电流校准
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.ResetFactoryData,wg_com_v2_ctrl.ResetFactoryData);                               // 恢复厂家数据
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.BatModeFR,wg_com_v2_ctrl.BatModeFR);                                             // 电池模式正反向切换
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.MpptSwitch,wg_com_v2_ctrl.MpptSwitch);                                           // 40D MPPT mode switch
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SleepModeOnOff,wg_com_v2_ctrl.SleepModeOnOff);                                   // 40E sleep mode switch
    // P03设置参数区
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpVolt,wg_com_v2_param.SetInpVolt);                                         // A端电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpCurr,wg_com_v2_param.SetInpCurr);                                         // A端电流
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpCurrPower,wg_com_v2_param.SetInpCurrPower);                               // A端功率
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutVolt,wg_com_v2_param.SetOutVolt);                                         // B端电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutCurr,wg_com_v2_param.SetOutCurr);                                         // B端电流
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutCurrPower,wg_com_v2_param.SetOutCurrPower);                               // B端功率
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpUvlo,wg_com_v2_param.SetInpUvlo);                                         // A端欠压保护
    
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpUvloRecover,wg_com_v2_param.SetInpUvloRecover);                           // A端欠压保护恢复
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpOVP,wg_com_v2_param.SetInpOVP);                                           // A端过压保护
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpOVPRecover,wg_com_v2_param.SetInpOVPRecover);                             // A端过压保护恢复
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutUvlo,wg_com_v2_param.SetOutUvlo);                                         // B端欠压保护
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutUvloRecover,wg_com_v2_param.SetOutUvloRecover);                           // B端欠压保护恢复
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutOVP,wg_com_v2_param.SetOutOVP);                                           // B端过压保护
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutOVPRecover,wg_com_v2_param.SetOutOVPRecover);                             // B端过压保护恢复
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetInsideTemp,wg_com_v2_param.SetInsideTemp);                                   // 内部温度
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutsideTemp,wg_com_v2_param.SetOutsideTemp);                                 // 外部温度
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpChargLedCurr,wg_com_v2_param.SetInpChargLedCurr);                         // A端充电指示灯电流
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpFullLedCurr,wg_com_v2_param.SetInpFullLedCurr);                           // A端充满指示灯电流
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutChargLedCurr,wg_com_v2_param.SetOutChargLedCurr);                         // B端充电指示灯电流
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutFullLedCurr,wg_com_v2_param.SetOutFullLedCurr);                           // B端充满指示灯电流
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.AuotForwardOpenVoltA,wg_com_v2_param.AuotForwardOpenVoltA);                     // 自动模式正向A端开启电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.AuotForwardVeerVoltA,wg_com_v2_param.AuotForwardVeerVoltA);                     // 自动模式正向转向A电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.AuotForwardShutVoltA,wg_com_v2_param.AuotForwardShutVoltA);                     // 自动模式正向A端关闭电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.AuotReverseOpenVoltB,wg_com_v2_param.AuotReverseOpenVoltB);                     // 自动模式反向B端开启电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.AuotReverseShutVoltB,wg_com_v2_param.AuotReverseShutVoltB);                     // 自动模式反向B端关闭电压
    WG_COM_V2_GET_DATA_UINT(get_wg_com_v2_data.com_param.SetTemp2,wg_com_v2_param.SetTemp2);                                             // 内部温度
    if(get_wg_com_v2_data.com_realtime_data.PowerMode != get_wg_com_v2_data.com_ctrl.SetPowerMode)
    {
        WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SetPowerMode, wg_com_v2_realtime_data.PowerMode);
    }
    ovp_uvp_rande_limt();
}

uint8_t float_equal(float x, float y) {
    if(fabsf(x - y) < 1e-6f)
    {
        return 0;   // 使用单精度 fabsf 更高效
    }
    return 1;
}

uint8_t updated_parameter(void)
{
    uint8_t power_mode_changed =
        (State_Control_Data.SetPowerMode != get_wg_com_v2_data.com_ctrl.SetPowerMode);
    uint8_t out_baty_type_changed =
        (State_Control_Data.OutBatyType != get_wg_com_v2_data.com_ctrl.OutBatyType);

    if((force_update_parameter != 0) ||
       (power_mode_changed != 0)  || 
       (State_Control_Data.SetChargMode != get_wg_com_v2_data.com_ctrl.SetChargMode)  || 
       (State_Control_Data.InpBatyType  != get_wg_com_v2_data.com_ctrl.InpBatyType)   || 
       (out_baty_type_changed != 0))
    {
        power_mode_changed_update = power_mode_changed;
        out_baty_type_changed_update = out_baty_type_changed;
        force_update_parameter = 0;
        State_Control_Data.SetPowerMode = get_wg_com_v2_data.com_ctrl.SetPowerMode;
        State_Control_Data.SetChargMode = get_wg_com_v2_data.com_ctrl.SetChargMode;
        State_Control_Data.InpBatyType  = get_wg_com_v2_data.com_ctrl.InpBatyType;
        State_Control_Data.OutBatyType  = get_wg_com_v2_data.com_ctrl.OutBatyType;
        return 1;
    }
    power_mode_changed_update = 0;
    out_baty_type_changed_update = 0;
    return 0;
    
}

uint8_t consume_power_mode_changed_update(void)
{
    uint8_t changed = power_mode_changed_update;
    power_mode_changed_update = 0;
    return changed;
}

uint8_t consume_out_baty_type_changed_update(void)
{
    uint8_t changed = out_baty_type_changed_update;
    out_baty_type_changed_update = 0;
    return changed;
}

void request_update_parameter(void)
{
    force_update_parameter = 1;
}

float RAMFUNC Get_Set_Out_Curr_Value_Lmt(void)
{
    return charge_state_data.set_out_lmt_curr;
}

//REG_TASK(30, get_wg_com_data_rum)


void set_charge_state_mode(BAT_CHARGE_MODE_E state)
{
    static uint16_t current_state = 0;
    uint16_t target_state = (uint16_t)state;
    uint16_t stored_state = 0;

    WG_COM_V2_GET_DATA_UINT(stored_state, wg_com_v2_realtime_data.StateCharge);
    if((current_state != target_state) || (stored_state != target_state))
    {
        current_state = target_state;
        WG_COM_V2_SET_DATA_UINT(current_state, wg_com_v2_realtime_data.StateCharge);
    }
}

uint16_t Get_Charge_State(void)
{
    return ((charge_state_data.OutBatyType>>8)&0xff);
}

static void ovp_uvp_rande_limt(void)
{
    float OutUvlo = get_wg_com_v2_data.com_param.SetOutUvlo;
    float OutUvloRecover = get_wg_com_v2_data.com_param.SetOutUvloRecover;
    float OutOVP  = get_wg_com_v2_data.com_param.SetOutOVP;
    float OutOVPRecover = get_wg_com_v2_data.com_param.SetOutOVPRecover;
    float InpUvlo = get_wg_com_v2_data.com_param.SetInpUvlo;
    float InpUvloRecover = get_wg_com_v2_data.com_param.SetInpUvloRecover;
    float InpOVP  = get_wg_com_v2_data.com_param.SetInpOVP;
    float InpOVPRecover  = get_wg_com_v2_data.com_param.SetInpOVPRecover;

    float InpChargLedCurr = get_wg_com_v2_data.com_param.SetInpChargLedCurr;
    float InpFullLedCurr  = get_wg_com_v2_data.com_param.SetInpFullLedCurr;
    float OutChargLedCurr = get_wg_com_v2_data.com_param.SetOutChargLedCurr;
    float OutFullLedCurr  = get_wg_com_v2_data.com_param.SetOutFullLedCurr;

    float protect_unit = 0;
    wg_com_v2_data_lmt_map_t protect_lmt_map;
    protect_unit = get_unit_for_addr(&wg_com_v2_param.SetOutUvlo);
    protect_lmt_map = *get_lmt_for_addr(&wg_com_v2_param.SetOutUvlo);
    if((OutUvlo+4-(protect_unit/10*5)) > OutOVP)
    {
        if((OutUvlo - 4) < (protect_lmt_map.dn_lmt*protect_unit))
        {
            WG_COM_V2_SET_DATA_UINT((OutUvlo+4), wg_com_v2_param.SetOutOVP);
            WG_COM_V2_SET_DATA_UINT((OutUvlo+3.8f), wg_com_v2_param.SetOutOVPRecover);

        }
        else
        {
            WG_COM_V2_SET_DATA_UINT((OutOVP-4), wg_com_v2_param.SetOutUvlo);
            WG_COM_V2_SET_DATA_UINT((OutOVP-3.8f), wg_com_v2_param.SetOutUvloRecover);
        }
    }
    else
    {
        protect_unit = get_unit_for_addr(&wg_com_v2_param.SetOutUvlo);
        protect_lmt_map = *get_lmt_for_addr(&wg_com_v2_param.SetOutUvloRecover);
        if((OutUvlo+0.2f-(protect_unit/10*5)) > OutUvloRecover)
        {
            if((OutUvlo + 0.2f) < (protect_lmt_map.up_lmt*protect_unit))
            {
                WG_COM_V2_SET_DATA_UINT((OutUvlo+0.2f), wg_com_v2_param.SetOutUvloRecover);
            }
        }

        protect_unit = get_unit_for_addr(&wg_com_v2_param.SetOutOVP);
        protect_lmt_map = *get_lmt_for_addr(&wg_com_v2_param.SetOutOVPRecover);
        if((OutOVP-0.2f+(protect_unit/10*5)) < OutOVPRecover)
        {
            if((OutOVP - 0.2f) < (protect_lmt_map.up_lmt*protect_unit))
            {
                WG_COM_V2_SET_DATA_UINT((OutOVP-0.2f), wg_com_v2_param.SetOutOVPRecover);
            }
        }
    }

    protect_unit = get_unit_for_addr(&wg_com_v2_param.SetInpUvlo);
    protect_lmt_map = *get_lmt_for_addr(&wg_com_v2_param.SetInpUvlo);
    if((InpUvlo+4-(protect_unit/10*5)) > InpOVP)
    {
        if((InpUvlo - 4) < (protect_lmt_map.dn_lmt*protect_unit))
        {
            WG_COM_V2_SET_DATA_UINT((InpUvlo+4), wg_com_v2_param.SetInpOVP);
            WG_COM_V2_SET_DATA_UINT((InpUvlo+3.8f), wg_com_v2_param.SetInpOVPRecover);
        }
        else
        {
            WG_COM_V2_SET_DATA_UINT((InpOVP-4), wg_com_v2_param.SetInpUvlo);
            WG_COM_V2_SET_DATA_UINT((InpOVP-3.8f), wg_com_v2_param.SetInpUvloRecover);
        }
    }
    else
    {
        protect_unit = get_unit_for_addr(&wg_com_v2_param.SetInpUvlo);
        protect_lmt_map = *get_lmt_for_addr(&wg_com_v2_param.SetInpUvloRecover);
        if((InpUvlo+0.2f-(protect_unit/10*5)) > InpUvloRecover)
        {
            if((InpUvlo + 0.2f) < (protect_lmt_map.up_lmt*protect_unit))
            {
                WG_COM_V2_SET_DATA_UINT((InpUvlo+0.2f), wg_com_v2_param.SetInpUvloRecover);
            }
        }

        protect_unit = get_unit_for_addr(&wg_com_v2_param.SetInpOVP);
        protect_lmt_map = *get_lmt_for_addr(&wg_com_v2_param.SetInpOVPRecover);
        if((InpOVP-0.2f+(protect_unit/10*5)) < InpOVPRecover)
        {
            if((InpOVP - 0.2f) < (protect_lmt_map.up_lmt*protect_unit))
            {
                WG_COM_V2_SET_DATA_UINT((InpOVP-0.2f), wg_com_v2_param.SetInpOVPRecover);
            }
        }
    }

    protect_unit = get_unit_for_addr(&wg_com_v2_param.SetInpFullLedCurr);
    protect_lmt_map = *get_lmt_for_addr(&wg_com_v2_param.SetInpFullLedCurr);
    if((InpFullLedCurr+0.5f-(protect_unit/10*5)) > InpChargLedCurr)
    {
        if((InpFullLedCurr - 0.5f) < (protect_lmt_map.dn_lmt*protect_unit))
        {
            WG_COM_V2_SET_DATA_UINT((InpFullLedCurr+0.5f), wg_com_v2_param.SetInpChargLedCurr);
        }
        else
        {
            WG_COM_V2_SET_DATA_UINT((InpChargLedCurr-0.5f), wg_com_v2_param.SetInpFullLedCurr);
        }
    }

    protect_unit = get_unit_for_addr(&wg_com_v2_param.SetOutFullLedCurr);
    protect_lmt_map = *get_lmt_for_addr(&wg_com_v2_param.SetOutFullLedCurr);
    if((OutFullLedCurr+0.5f-(protect_unit/10*5)) > OutChargLedCurr)
    {
        if((OutFullLedCurr - 0.5f) < (protect_lmt_map.dn_lmt*protect_unit))
        {
            WG_COM_V2_SET_DATA_UINT((OutFullLedCurr+0.5f), wg_com_v2_param.SetOutChargLedCurr);
        }
        else
        {
            WG_COM_V2_SET_DATA_UINT((OutChargLedCurr-0.5f), wg_com_v2_param.SetOutFullLedCurr);
        }
    }
}

void boot_time_delay_run(void)
{
    static uint16_t TimeDelay = 0;
    if(charge_state_data.Boot_Time_Delay.SetBootTime <= 1)
    {
        charge_state_data.Boot_Time_Delay.SetBootTimeFlag = 1;
        return;
    }
    if(charge_state_data.Boot_Time_Delay.SetBootTimeFlag == 0)
    {
        if(++TimeDelay >= 10)
        {
            TimeDelay = 0;
            if(++charge_state_data.Boot_Time_Delay.BootTimeDelay > (charge_state_data.Boot_Time_Delay.SetBootTime-1))
            {
                charge_state_data.Boot_Time_Delay.BootTimeDelay = 0;
                charge_state_data.Boot_Time_Delay.SetBootTimeFlag = 1;
            }
        }
    }
    else
    {
        TimeDelay = 0;
    }
}

REG_TASK(100, boot_time_delay_run)

