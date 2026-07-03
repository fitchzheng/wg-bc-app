#include "basic_mode.h"
#include "wg_com_v2.h"
#include "get_com_data.h"
#include "soft_start.h"
#include "eeprom_cfg.h"

const BASIC_MODE_CONFIG_T basic_Sys_Volt_Config[1] = {
    [eBASIC_SYS_12V] = {
                    .OutVoltMax      = BASIC_BAT_SYS_12V_MAX_OUT_VOLT,
                    .OutVoltMin      = BASIC_BAT_SYS_12V_MIN_OUT_VOLT,
                    .OutVoltDefault  = BASIC_BAT_SYS_12V_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BASIC_BAT_SYS_12V_MAX_CHURR_VOLT,
                    .OutCurrMin      = BASIC_BAT_SYS_12V_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BASIC_BAT_SYS_12V_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BASIC_BAT_SYS_12V_MAX_POWER_VOLT,
                    .OutPowerMin     = BASIC_BAT_SYS_12V_MIN_POWER_VOLT,
                    .OutPowerDefault = BASIC_BAT_SYS_12V_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BASIC_BAT_SYS_12V_OPEN_VOLT_A,
                    .CloseVoltA      = BASIC_BAT_SYS_12V_CLOSE_VOLT_A,
                    .VeerVoltA       = BASIC_BAT_SYS_12V_VEER_VOLT_A,
                    .OpenVoltB       = BASIC_BAT_SYS_12V_OPEN_VOLT_B,
                    .CloseVoltB      = BASIC_BAT_SYS_12V_CLOSE_VOLT_B,
                    .SetChargLedCurr = BASIC_BAT_SYS_12V_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BASIC_BAT_SYS_12V_SET_LED_FULL_CURR,
    },
};

void basic_mode_run(charge_state_data_t *basic_charge_data)
{
//    float verify_data = 0.00f;

    if(updated_parameter())
    {
        init_basic_mode_parameter();
    }
    
//    if((get_wg_com_v2_data.com_ctrl.SetChargMode != eSET_FORWARD)   && 
//       (get_wg_com_v2_data.com_ctrl.SetChargMode != eSET_BACKWARD))
//    {
//        get_wg_com_v2_data.com_ctrl.SetChargMode = eSET_FORWARD;
//        WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SetChargMode, wg_com_v2_ctrl.SetChargMode);
//    }

//    set_charge_state_mode(ePOWER_CHARGE);
    basic_charge_data->SetCharState = ePOWER_CHARGE;
    
//    verify_data = get_wg_com_v2_data.com_param.SetInpVolt;
//    LIMIT_MAX_MIN(basic_Sys_Volt_Config[eBASIC_SYS_12V].OutVoltMax,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutVoltMin,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutVoltDefault,get_wg_com_v2_data.com_param.SetInpVolt);
//    if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetInpVolt) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpVolt, wg_com_v2_param.SetInpVolt);}
//    verify_data = get_wg_com_v2_data.com_param.SetInpCurr;
//    LIMIT_MAX_MIN(basic_Sys_Volt_Config[eBASIC_SYS_12V].OutCurrMax,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutCurrMin,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutCurrDefault,get_wg_com_v2_data.com_param.SetInpCurr);
//    if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetInpCurr) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpCurr, wg_com_v2_param.SetInpCurr);}
//    verify_data = get_wg_com_v2_data.com_param.SetInpCurrPower;
//    LIMIT_MAX_MIN(basic_Sys_Volt_Config[eBASIC_SYS_12V].OutPowerMax,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutPowerMin,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutPowerDefault,get_wg_com_v2_data.com_param.SetInpCurrPower);
//    if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetInpCurrPower) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpCurrPower, wg_com_v2_param.SetInpCurrPower);}
//    
//    verify_data = get_wg_com_v2_data.com_param.SetOutVolt;
//    LIMIT_MAX_MIN(basic_Sys_Volt_Config[eBASIC_SYS_12V].OutVoltMax,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutVoltMin,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutVoltDefault,get_wg_com_v2_data.com_param.SetOutVolt);
//    if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetOutVolt) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutVolt, wg_com_v2_param.SetOutVolt);}
//    verify_data = get_wg_com_v2_data.com_param.SetOutCurr;
//    LIMIT_MAX_MIN(basic_Sys_Volt_Config[eBASIC_SYS_12V].OutCurrMax,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutCurrMin,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutCurrDefault,get_wg_com_v2_data.com_param.SetOutCurr);
//    if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetOutCurr) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutCurr, wg_com_v2_param.SetOutCurr);}
//    verify_data = get_wg_com_v2_data.com_param.SetOutCurrPower;
//    LIMIT_MAX_MIN(basic_Sys_Volt_Config[eBASIC_SYS_12V].OutPowerMax,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutPowerMin,basic_Sys_Volt_Config[eBASIC_SYS_12V].OutPowerDefault,get_wg_com_v2_data.com_param.SetOutCurrPower);
//    if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetOutCurrPower) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutCurrPower, wg_com_v2_param.SetOutCurrPower);}

    if(basic_charge_data->check_state == eADDRS_BACKWARD)
    {
        basic_charge_data->SetOutVolt = get_wg_com_v2_data.com_param.SetInpVolt;
        basic_charge_data->SetInpCurr = get_wg_com_v2_data.com_param.SetOutCurr;
        basic_charge_data->SetOutCurr = get_wg_com_v2_data.com_param.SetInpCurr;
        basic_charge_data->fvs48_pwr_lmt = get_wg_com_v2_data.com_param.SetInpCurrPower;
        basic_charge_data->rvs12_pwr_lmt = get_wg_com_v2_data.com_param.SetOutCurrPower;
        basic_charge_data->OutBatyType = get_wg_com_v2_data.com_ctrl.InpBatyType;
        basic_charge_data->rvs12_lmt = get_wg_com_v2_data.com_param.SetOutUvlo+0.5f;
        basic_charge_data->fvs48_lmt = get_wg_com_v2_data.com_param.SetInpVolt;
        basic_charge_data->Boot_Time_Delay.SetBootTime = get_wg_com_v2_data.com_ctrl.SetBootTimeB;
        basic_charge_data->ilv_lmt = basic_charge_data->SetInpCurr;
        if(basic_charge_data->get_is_run == 1)
        {
            if(basic_charge_data->ActiveOnCurrStartTime == 0)
            {
                basic_charge_data->ihv_lmt = curr_soft_start(basic_charge_data->ihv_lmt,basic_charge_data->temp_derate_curr,(10));
            }
            else
            {
                basic_charge_data->ihv_lmt = curr_soft_start(basic_charge_data->ihv_lmt,basic_charge_data->temp_derate_curr,(basic_charge_data->ActiveOnCurrStartTime*100));
            }
        }else{
            basic_charge_data->ihv_lmt = 1.0f;
        }
    }
    else if(basic_charge_data->check_state == eADDRS_FORWARD)
    {
        basic_charge_data->SetOutVolt = get_wg_com_v2_data.com_param.SetOutVolt;
        basic_charge_data->SetInpCurr=get_wg_com_v2_data.com_param.SetInpCurr;
        basic_charge_data->SetOutCurr=get_wg_com_v2_data.com_param.SetOutCurr;
        basic_charge_data->fvs48_pwr_lmt = get_wg_com_v2_data.com_param.SetInpCurrPower;
        basic_charge_data->rvs12_pwr_lmt = get_wg_com_v2_data.com_param.SetOutCurrPower;
        basic_charge_data->OutBatyType = get_wg_com_v2_data.com_ctrl.OutBatyType;
        basic_charge_data->rvs12_lmt = get_wg_com_v2_data.com_param.SetOutVolt;
        basic_charge_data->fvs48_lmt = get_wg_com_v2_data.com_param.SetInpUvlo+0.5f;
        basic_charge_data->Boot_Time_Delay.SetBootTime = get_wg_com_v2_data.com_ctrl.SetBootTimeA;
        basic_charge_data->ihv_lmt = basic_charge_data->SetInpCurr;
        if(basic_charge_data->get_is_run == 1)
        {
            if(basic_charge_data->ActiveOnCurrStartTime == 0)
            {
                basic_charge_data->ilv_lmt = curr_soft_start(basic_charge_data->ilv_lmt,basic_charge_data->temp_derate_curr,(10));
            }
            else
            {
                basic_charge_data->ilv_lmt = curr_soft_start(basic_charge_data->ilv_lmt,basic_charge_data->temp_derate_curr,(basic_charge_data->ActiveOnCurrStartTime*100));
            }
        }else{
            basic_charge_data->ilv_lmt = 1.0f;
        }
    }
    basic_charge_data->ActualOutCurr = basic_charge_data->SetOutCurr;
    basic_charge_data->ActualOutVolt = basic_charge_data->SetOutVolt;
    basic_charge_data->set_out_lmt_curr = (basic_charge_data->SetOutCurr * 0.1f);

    basic_charge_data->bat_state.sucCurrentReached90Percent = 0;
    basic_charge_data->bat_state.sucPowerReached90Percent = 0;
    basic_charge_data->bat_state.LithiumBatOnOff = 0;
}


void init_basic_mode_parameter(void)
{
    uint16_t BatSetOutCurrPower = 0;
    uint16_t BatSetInpCurrPower = 0;
    float BatSetOutVolt = 0.00f;
    float BatSetInpVolt = 0.00f;
    float BatSetOutCurr = 0.00f;
    float BatSetInpCurr = 0.00f;

    float AuotOpenVoltA  = 0.00f;
    float AuotVeerVoltA  = 0.00f;
    float AuotCloseVoltA = 0.00f;
    float AuotOpenVoltB  = 0.00f;
    float AuotCloseVoltB = 0.00f;

    float SetUvloA        = 0.00f;
    float SetUvloRecoverA = 0.00f;
    float SetOVPA         = 0.00f;
    float SetOVPRecoverA  = 0.00f;
    float SetUvloB        = 0.00f;
    float SetUvloRecoverB = 0.00f;
    float SetOVPB         = 0.00f;
    float SetOVPRecoverB  = 0.00f;

    float SetInpChargCurr = 0.00f;
    float SetInpFullCurr  = 0.00f;
    float SetOutChargCurr = 0.00f;
    float SetOutFullCurr  = 0.00f;
    
    uint16_t SetBootTimeA = 0;
    uint16_t SetBootTimeB = 0;
    uint16_t SetOnCurrStartTimeA = 0;
    uint16_t SetOnCurrStartTimeB = 0;

    uint16_t BatTypeA;
    uint16_t BatTypeB;

    if((consume_power_mode_changed_update() == 0) && eeprom_apply_basic_mode_profile())
    {
        return;
    }

    WG_COM_V2_GET_DATA_UINT(SetBootTimeA, wg_com_v2_ctrl.SetBootTimeA);
    WG_COM_V2_GET_DATA_UINT(SetBootTimeB, wg_com_v2_ctrl.SetBootTimeB);
    WG_COM_V2_GET_DATA_UINT(SetOnCurrStartTimeA, wg_com_v2_ctrl.SetOnCurrStartTimeA);
    WG_COM_V2_GET_DATA_UINT(SetOnCurrStartTimeB, wg_com_v2_ctrl.SetOnCurrStartTimeB);

    BatTypeB = (eBAT_DCDC<<8) + eSYS_10_60V;
    BatTypeA = (eBAT_DCDC<<8) + eSYS_10_60V;
    if(get_wg_com_v2_data.com_realtime_data.PowerMode != State_Control_Data.SetPowerMode)
    {
        WG_COM_V2_SET_DATA_UINT(BatTypeA, wg_com_v2_ctrl.InpBatyType);
        WG_COM_V2_SET_DATA_UINT(BatTypeB, wg_com_v2_ctrl.OutBatyType);
    }

    BatSetOutVolt      = basic_Sys_Volt_Config[eBASIC_SYS_12V].OutVoltDefault;
    BatSetOutCurr      = basic_Sys_Volt_Config[eBASIC_SYS_12V].OutCurrDefault;
    BatSetOutCurrPower = basic_Sys_Volt_Config[eBASIC_SYS_12V].OutPowerDefault;
    AuotOpenVoltB      = basic_Sys_Volt_Config[eBASIC_SYS_12V].OpenVoltB;
    AuotCloseVoltB     = basic_Sys_Volt_Config[eBASIC_SYS_12V].CloseVoltB;
    SetOutChargCurr    = basic_Sys_Volt_Config[eBASIC_SYS_12V].SetChargLedCurr;
    SetOutFullCurr     = basic_Sys_Volt_Config[eBASIC_SYS_12V].SetFullLedCurr;
    SetUvloB           = BASIC_BAT_SYS_SET_UVLO;
    SetUvloRecoverB    = BASIC_BAT_SYS_SET_UVLORECOVER;
    SetOVPB            = BASIC_BAT_SYS_SET_OVP;
    SetOVPRecoverB     = BASIC_BAT_SYS_SET_OVPRECOVER;

    BatSetInpVolt      = basic_Sys_Volt_Config[eBASIC_SYS_12V].OutVoltDefault;
    BatSetInpCurr      = basic_Sys_Volt_Config[eBASIC_SYS_12V].OutCurrDefault;
    BatSetInpCurrPower = basic_Sys_Volt_Config[eBASIC_SYS_12V].OutPowerDefault;
    AuotOpenVoltA      = basic_Sys_Volt_Config[eBASIC_SYS_12V].OpenVoltA;
    AuotVeerVoltA      = basic_Sys_Volt_Config[eBASIC_SYS_12V].VeerVoltA;
    AuotCloseVoltA     = basic_Sys_Volt_Config[eBASIC_SYS_12V].CloseVoltA;
    SetInpChargCurr    = basic_Sys_Volt_Config[eBASIC_SYS_12V].SetChargLedCurr;
    SetInpFullCurr     = basic_Sys_Volt_Config[eBASIC_SYS_12V].SetFullLedCurr;
    SetUvloA           = BASIC_BAT_SYS_SET_UVLO;
    SetUvloRecoverA    = BASIC_BAT_SYS_SET_UVLORECOVER;
    SetOVPA            = BASIC_BAT_SYS_SET_OVP;
    SetOVPRecoverA     = BASIC_BAT_SYS_SET_OVPRECOVER;

    WG_COM_V2_SET_DATA_UINT(SetUvloA, wg_com_v2_param.SetInpUvlo);
    WG_COM_V2_SET_DATA_UINT(SetUvloRecoverA, wg_com_v2_param.SetInpUvloRecover);
    WG_COM_V2_SET_DATA_UINT(SetOVPA, wg_com_v2_param.SetInpOVP);
    WG_COM_V2_SET_DATA_UINT(SetOVPRecoverA, wg_com_v2_param.SetInpOVPRecover);
    WG_COM_V2_SET_DATA_UINT(SetUvloB, wg_com_v2_param.SetOutUvlo);
    WG_COM_V2_SET_DATA_UINT(SetUvloRecoverB, wg_com_v2_param.SetOutUvloRecover);
    WG_COM_V2_SET_DATA_UINT(SetOVPB, wg_com_v2_param.SetOutOVP);
    WG_COM_V2_SET_DATA_UINT(SetOVPRecoverB, wg_com_v2_param.SetOutOVPRecover);

    WG_COM_V2_SET_DATA_UINT(SetInpChargCurr, wg_com_v2_param.SetInpChargLedCurr);
    WG_COM_V2_SET_DATA_UINT(SetInpFullCurr, wg_com_v2_param.SetInpFullLedCurr);
    WG_COM_V2_SET_DATA_UINT(SetOutChargCurr, wg_com_v2_param.SetOutChargLedCurr);
    WG_COM_V2_SET_DATA_UINT(SetOutFullCurr, wg_com_v2_param.SetOutFullLedCurr);
    WG_COM_V2_SET_DATA_UINT(BatSetOutVolt, wg_com_v2_param.SetOutVolt);
    WG_COM_V2_SET_DATA_UINT(BatSetOutCurr, wg_com_v2_param.SetOutCurr);
    WG_COM_V2_SET_DATA_UINT(BatSetOutCurrPower, wg_com_v2_param.SetOutCurrPower);
    WG_COM_V2_SET_DATA_UINT(BatSetInpVolt, wg_com_v2_param.SetInpVolt);
    WG_COM_V2_SET_DATA_UINT(BatSetInpCurr, wg_com_v2_param.SetInpCurr);
    WG_COM_V2_SET_DATA_UINT(BatSetInpCurrPower, wg_com_v2_param.SetInpCurrPower);

    WG_COM_V2_SET_DATA_UINT(AuotOpenVoltA, wg_com_v2_param.AuotForwardOpenVoltA);
    WG_COM_V2_SET_DATA_UINT(AuotVeerVoltA, wg_com_v2_param.AuotForwardVeerVoltA);
    WG_COM_V2_SET_DATA_UINT(AuotCloseVoltA, wg_com_v2_param.AuotForwardShutVoltA);
    WG_COM_V2_SET_DATA_UINT(AuotOpenVoltB, wg_com_v2_param.AuotReverseOpenVoltB);
    WG_COM_V2_SET_DATA_UINT(AuotCloseVoltB, wg_com_v2_param.AuotReverseShutVoltB);

    WG_COM_V2_SET_DATA_UINT(SetBootTimeA, wg_com_v2_ctrl.SetBootTimeA);
    WG_COM_V2_SET_DATA_UINT(SetBootTimeB, wg_com_v2_ctrl.SetBootTimeB);
    WG_COM_V2_SET_DATA_UINT(SetOnCurrStartTimeA, wg_com_v2_ctrl.SetOnCurrStartTimeA);
    WG_COM_V2_SET_DATA_UINT(SetOnCurrStartTimeB, wg_com_v2_ctrl.SetOnCurrStartTimeB);

    (void)eeprom_commit_current_pages_for_range(WG_COM_V2_CTRL_ADDR,
                                                (uint16_t)(sizeof(wg_com_v2_ctrl_t) / 2U));
    (void)eeprom_commit_current_pages_for_range((uint16_t)(WG_COM_V2_PARAM_ADDR + (EEPROM_PARAM_CAL_SIZE / 2U)),
                                                (uint16_t)(EEPROM_PARAM_USER_SIZE / 2U));
}



