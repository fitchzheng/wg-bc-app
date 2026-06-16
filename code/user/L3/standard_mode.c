#include "standard_mode.h"
#include "get_com_data.h"
#include "wg_com_v2.h"
#include "soft_start.h"

standard_charge_status_t standard_charge_status;

const STANDARD_MODE_CONFIG_T Bat_Standard_Sys_Volt_Config[eSTANDARD_SYS_VOLT_MAX] = {
    [eSTANDARD_SYS_12V] = {
                    .OutVoltMax      = STANDARD_BAT_SYS_12V_MAX_OUT_VOLT,
                    .OutVoltMin      = STANDARD_BAT_SYS_12V_MIN_OUT_VOLT,
                    .OutVoltDefault  = STANDARD_BAT_SYS_12V_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = STANDARD_BAT_SYS_12V_MAX_CHURR_VOLT,
                    .OutCurrMin      = STANDARD_BAT_SYS_12V_MIN_CHURR_VOLT,
                    .OutCurrDefault  = STANDARD_BAT_SYS_12V_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = STANDARD_BAT_SYS_12V_MAX_POWER_VOLT,
                    .OutPowerMin     = STANDARD_BAT_SYS_12V_MIN_POWER_VOLT,
                    .OutPowerDefault = STANDARD_BAT_SYS_12V_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = STANDARD_BAT_SYS_12V_OPEN_VOLT_A,
                    .CloseVoltA      = STANDARD_BAT_SYS_12V_CLOSE_VOLT_A,
                    .VeerVoltA       = STANDARD_BAT_SYS_12V_VEER_VOLT_A,
                    .OpenVoltB       = STANDARD_BAT_SYS_12V_OPEN_VOLT_B,
                    .CloseVoltB      = STANDARD_BAT_SYS_12V_CLOSE_VOLT_B,
                    .SetChargLedCurr = STANDARD_BAT_SYS_12V_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = STANDARD_BAT_SYS_12V_SET_LED_FULL_CURR,
    },
    [eSTANDARD_SYS_16V] = {
                    .OutVoltMax      = STANDARD_BAT_SYS_16V_MAX_OUT_VOLT,
                    .OutVoltMin      = STANDARD_BAT_SYS_16V_MIN_OUT_VOLT,
                    .OutVoltDefault  = STANDARD_BAT_SYS_16V_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = STANDARD_BAT_SYS_16V_MAX_CHURR_VOLT,
                    .OutCurrMin      = STANDARD_BAT_SYS_16V_MIN_CHURR_VOLT,
                    .OutCurrDefault  = STANDARD_BAT_SYS_16V_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = STANDARD_BAT_SYS_16V_MAX_POWER_VOLT,
                    .OutPowerMin     = STANDARD_BAT_SYS_16V_MIN_POWER_VOLT,
                    .OutPowerDefault = STANDARD_BAT_SYS_16V_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = STANDARD_BAT_SYS_16V_OPEN_VOLT_A,
                    .CloseVoltA      = STANDARD_BAT_SYS_16V_CLOSE_VOLT_A,
                    .VeerVoltA       = STANDARD_BAT_SYS_16V_VEER_VOLT_A,
                    .OpenVoltB       = STANDARD_BAT_SYS_16V_OPEN_VOLT_B,
                    .CloseVoltB      = STANDARD_BAT_SYS_16V_CLOSE_VOLT_B,
                    .SetChargLedCurr = STANDARD_BAT_SYS_16V_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = STANDARD_BAT_SYS_16V_SET_LED_FULL_CURR,
    },
    [eSTANDARD_SYS_24V] = {
                    .OutVoltMax      = STANDARD_BAT_SYS_24V_MAX_OUT_VOLT,
                    .OutVoltMin      = STANDARD_BAT_SYS_24V_MIN_OUT_VOLT,
                    .OutVoltDefault  = STANDARD_BAT_SYS_24V_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = STANDARD_BAT_SYS_24V_MAX_CHURR_VOLT,
                    .OutCurrMin      = STANDARD_BAT_SYS_24V_MIN_CHURR_VOLT,
                    .OutCurrDefault  = STANDARD_BAT_SYS_24V_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = STANDARD_BAT_SYS_24V_MAX_POWER_VOLT,
                    .OutPowerMin     = STANDARD_BAT_SYS_24V_MIN_POWER_VOLT,
                    .OutPowerDefault = STANDARD_BAT_SYS_24V_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = STANDARD_BAT_SYS_24V_OPEN_VOLT_A,
                    .CloseVoltA      = STANDARD_BAT_SYS_24V_CLOSE_VOLT_A,
                    .VeerVoltA       = STANDARD_BAT_SYS_24V_VEER_VOLT_A,
                    .OpenVoltB       = STANDARD_BAT_SYS_24V_OPEN_VOLT_B,
                    .CloseVoltB      = STANDARD_BAT_SYS_24V_CLOSE_VOLT_B,
                    .SetChargLedCurr = STANDARD_BAT_SYS_24V_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = STANDARD_BAT_SYS_24V_SET_LED_FULL_CURR,
    },
    [eSTANDARD_SYS_36V] = {
                    .OutVoltMax      = STANDARD_BAT_SYS_36V_MAX_OUT_VOLT,
                    .OutVoltMin      = STANDARD_BAT_SYS_36V_MIN_OUT_VOLT,
                    .OutVoltDefault  = STANDARD_BAT_SYS_36V_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = STANDARD_BAT_SYS_36V_MAX_CHURR_VOLT,
                    .OutCurrMin      = STANDARD_BAT_SYS_36V_MIN_CHURR_VOLT,
                    .OutCurrDefault  = STANDARD_BAT_SYS_36V_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = STANDARD_BAT_SYS_36V_MAX_POWER_VOLT,
                    .OutPowerMin     = STANDARD_BAT_SYS_36V_MIN_POWER_VOLT,
                    .OutPowerDefault = STANDARD_BAT_SYS_36V_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = STANDARD_BAT_SYS_36V_OPEN_VOLT_A,
                    .CloseVoltA      = STANDARD_BAT_SYS_36V_CLOSE_VOLT_A,
                    .VeerVoltA       = STANDARD_BAT_SYS_36V_VEER_VOLT_A,
                    .OpenVoltB       = STANDARD_BAT_SYS_36V_OPEN_VOLT_B,
                    .CloseVoltB      = STANDARD_BAT_SYS_36V_CLOSE_VOLT_B,
                    .SetChargLedCurr = STANDARD_BAT_SYS_36V_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = STANDARD_BAT_SYS_36V_SET_LED_FULL_CURR,
    },
    [eSTANDARD_SYS_48V] = {
                    .OutVoltMax      = STANDARD_BAT_SYS_48V_MAX_OUT_VOLT,
                    .OutVoltMin      = STANDARD_BAT_SYS_48V_MIN_OUT_VOLT,
                    .OutVoltDefault  = STANDARD_BAT_SYS_48V_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = STANDARD_BAT_SYS_48V_MAX_CHURR_VOLT,
                    .OutCurrMin      = STANDARD_BAT_SYS_48V_MIN_CHURR_VOLT,
                    .OutCurrDefault  = STANDARD_BAT_SYS_48V_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = STANDARD_BAT_SYS_48V_MAX_POWER_VOLT,
                    .OutPowerMin     = STANDARD_BAT_SYS_48V_MIN_POWER_VOLT,
                    .OutPowerDefault = STANDARD_BAT_SYS_48V_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = STANDARD_BAT_SYS_48V_OPEN_VOLT_A,
                    .CloseVoltA      = STANDARD_BAT_SYS_48V_CLOSE_VOLT_A,
                    .VeerVoltA       = STANDARD_BAT_SYS_48V_VEER_VOLT_A,
                    .OpenVoltB       = STANDARD_BAT_SYS_48V_OPEN_VOLT_B,
                    .CloseVoltB      = STANDARD_BAT_SYS_48V_CLOSE_VOLT_B,
                    .SetChargLedCurr = STANDARD_BAT_SYS_48V_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = STANDARD_BAT_SYS_48V_SET_LED_FULL_CURR,
    },
};

void standard_mode_run(charge_state_data_t *standard_charge_data)
{
    float verify_data = 0.00f;
    uint16_t BatTypeB = get_wg_com_v2_data.com_ctrl.OutBatyType;
    
    if(updated_parameter())
    {
        init_standard_mode_parameter();
    }

    if((BatTypeB&0x00FF) >= eSTANDARD_SYS_VOLT_MAX)
    {
        BatTypeB = (eBAT_DCDC<<8) + eSYS_12V; 
        get_wg_com_v2_data.com_ctrl.OutBatyType = (eBAT_DCDC<<8) + eSYS_12V;
        WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_ctrl.OutBatyType,  wg_com_v2_ctrl.OutBatyType);
    }
    
    if((get_wg_com_v2_data.com_ctrl.SetChargMode != eSET_FORWARD)                 ||
       (get_wg_com_v2_data.com_ctrl.InpBatyType  != ((eBAT_DCDC<<8)+eSYS_10_50V)) ||
       (((BatTypeB&0xFF00)>>8) != eBAT_DCDC))
    {
        get_wg_com_v2_data.com_ctrl.SetChargMode = eSET_FORWARD;
        get_wg_com_v2_data.com_ctrl.InpBatyType = ((eBAT_DCDC<<8)+eSYS_10_50V);
        get_wg_com_v2_data.com_ctrl.OutBatyType = ((BatTypeB&0x00FF)+(eBAT_DCDC<<8));
        WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_ctrl.SetChargMode, wg_com_v2_ctrl.SetChargMode);
        WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_ctrl.InpBatyType,  wg_com_v2_ctrl.InpBatyType);
        WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_ctrl.OutBatyType,  wg_com_v2_ctrl.OutBatyType);
    }

//    set_charge_state_mode(ePOWER_CHARGE);
    standard_charge_data->SetCharState = ePOWER_CHARGE;

    if((BatTypeB&0x00FF) < eSTANDARD_SYS_VOLT_MAX)
    {
        verify_data = get_wg_com_v2_data.com_param.SetOutVolt;
        LIMIT_MAX_MIN(Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutVoltMax,Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutVoltMin,Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutVoltDefault,get_wg_com_v2_data.com_param.SetOutVolt);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetOutVolt) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutVolt, wg_com_v2_param.SetOutVolt);}
        verify_data = get_wg_com_v2_data.com_param.SetOutCurr;
        LIMIT_MAX_MIN(Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutCurrMax,Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutCurrMin,Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutCurrDefault,get_wg_com_v2_data.com_param.SetOutCurr);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetOutCurr) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutCurr, wg_com_v2_param.SetOutCurr);}
        verify_data = get_wg_com_v2_data.com_param.SetOutCurrPower;
        LIMIT_MAX_MIN(Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutPowerMax,Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutPowerMin,Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutPowerDefault,get_wg_com_v2_data.com_param.SetOutCurrPower);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetOutCurrPower) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutCurrPower, wg_com_v2_param.SetOutCurrPower);}
    }

    if(standard_charge_data->check_state == eADDRS_BACKWARD)
    {
        standard_charge_data->SetOutVolt = get_wg_com_v2_data.com_param.SetInpVolt;
        standard_charge_data->SetInpCurr = get_wg_com_v2_data.com_param.SetOutCurr;
        standard_charge_data->SetOutCurr = get_wg_com_v2_data.com_param.SetInpCurr;
        standard_charge_data->fvs48_pwr_lmt = get_wg_com_v2_data.com_param.SetInpCurrPower;
        standard_charge_data->rvs12_pwr_lmt = get_wg_com_v2_data.com_param.SetOutCurrPower;
        standard_charge_data->OutBatyType = get_wg_com_v2_data.com_ctrl.InpBatyType;
        standard_charge_data->rvs12_lmt = get_wg_com_v2_data.com_param.SetOutUvlo+0.5f;
        standard_charge_data->fvs48_lmt = get_wg_com_v2_data.com_param.SetInpVolt;
        standard_charge_data->ilv_lmt = standard_charge_data->SetInpCurr;
        if(standard_charge_data->get_is_run == 1)
        {
            //standard_charge_data->ihv_lmt = standard_charge_data->temp_derate_curr;
            standard_charge_data->ihv_lmt = curr_soft_start(standard_charge_data->ihv_lmt,standard_charge_data->temp_derate_curr,80);
        }else{
            standard_charge_data->ihv_lmt = 1.0f;
        }
    }
    else if(standard_charge_data->check_state == eADDRS_FORWARD)
    {
        standard_charge_data->SetOutVolt = get_wg_com_v2_data.com_param.SetOutVolt;
        standard_charge_data->SetInpCurr=get_wg_com_v2_data.com_param.SetInpCurr;
        standard_charge_data->SetOutCurr=get_wg_com_v2_data.com_param.SetOutCurr;
        standard_charge_data->fvs48_pwr_lmt = get_wg_com_v2_data.com_param.SetInpCurrPower;
        standard_charge_data->rvs12_pwr_lmt = get_wg_com_v2_data.com_param.SetOutCurrPower;
        standard_charge_data->OutBatyType = get_wg_com_v2_data.com_ctrl.OutBatyType;
        standard_charge_data->rvs12_lmt = get_wg_com_v2_data.com_param.SetOutVolt;
        standard_charge_data->fvs48_lmt = get_wg_com_v2_data.com_param.SetInpUvlo+0.5f;
        
        standard_charge_data->ihv_lmt = standard_charge_data->SetInpCurr;
        if(standard_charge_data->get_is_run == 1)
        {
            standard_charge_data->ilv_lmt = curr_soft_start(standard_charge_data->ilv_lmt,standard_charge_data->temp_derate_curr,80);
            //standard_charge_data->ilv_lmt = standard_charge_data->temp_derate_curr;
        }else{
            standard_charge_data->ilv_lmt = 1.0f;
        }
    }
    standard_charge_data->ActualOutCurr = standard_charge_data->SetOutCurr;
    standard_charge_data->ActualOutVolt = standard_charge_data->SetOutVolt;
    standard_charge_data->set_out_lmt_curr = (standard_charge_data->SetOutCurr * 0.1f);
    standard_charge_data->Boot_Time_Delay.SetBootTime = 0;

    standard_charge_data->bat_state.sucCurrentReached90Percent = 0;
    standard_charge_data->bat_state.sucPowerReached90Percent = 0;
    standard_charge_data->bat_state.LithiumBatOnOff = 0;
}


void init_standard_mode_parameter(void)
{
    uint16_t BatSetOutCurrPower = 0;
    uint16_t BatSetInpCurrPower = 0;
    float BatSetOutVolt = 0.00f;
    float BatSetInpVolt = 0.00f;
    float BatSetOutCurr = 0.00f;
    float BatSetInpCurr = 0.00f;

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

    uint16_t BatTypeB = get_wg_com_v2_data.com_ctrl.OutBatyType;
    if((BatTypeB&0x00FF) >= eSTANDARD_SYS_VOLT_MAX){
        BatTypeB = (eBAT_DCDC<<8) + eSYS_12V;
    }

    BatSetOutVolt      = Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutVoltDefault;
    BatSetOutCurr      = Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutCurrDefault;
    BatSetOutCurrPower = Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].OutPowerDefault;

    SetUvloB           = STANDARD_BAT_SYS_SET_UVLO;
    SetUvloRecoverB    = STANDARD_BAT_SYS_SET_UVLORECOVER;
    SetOVPB            = STANDARD_BAT_SYS_SET_OVP;
    SetOVPRecoverB     = STANDARD_BAT_SYS_SET_OVPRECOVER;
    SetOutChargCurr    = Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].SetChargLedCurr;
    SetOutFullCurr     = Bat_Standard_Sys_Volt_Config[(BatTypeB&0x00FF)].SetFullLedCurr;

    BatSetInpVolt      = Bat_Standard_Sys_Volt_Config[eSTANDARD_SYS_12V].OutVoltDefault;
    BatSetInpCurr      = 125.00f;
    BatSetInpCurrPower = Bat_Standard_Sys_Volt_Config[eSTANDARD_SYS_12V].OutPowerDefault;
    SetUvloA           = STANDARD_BAT_SYS_SET_UVLO;
    SetUvloRecoverA    = STANDARD_BAT_SYS_SET_UVLORECOVER;
    SetOVPA            = STANDARD_BAT_SYS_SET_OVP;
    SetOVPRecoverA     = STANDARD_BAT_SYS_SET_OVPRECOVER;
    SetInpChargCurr    = Bat_Standard_Sys_Volt_Config[eSTANDARD_SYS_12V].SetChargLedCurr;
    SetInpFullCurr     = Bat_Standard_Sys_Volt_Config[eSTANDARD_SYS_12V].SetFullLedCurr;

    WG_COM_V2_SET_DATA_UINT(((eBAT_DCDC<<8) + eSYS_10_50V),  wg_com_v2_ctrl.InpBatyType);

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
}



