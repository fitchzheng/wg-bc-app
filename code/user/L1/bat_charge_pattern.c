#include "bat_charge_pattern.h"
#include "wg_com_v2.h"
#include "adc_check.h"
#include "adc.h"
#include "gpio.h"
#include "bsp_gpio.h"
#include "get_com_data.h"
static ADC_CHECK_ADDRS_E check_state = ADDRS_IDLE;
static uint16_t StateCharge = eFORWARD;
void auto_charge_mode(void)
{
    static uint8_t StateModelFlag;
    if(StateModelFlag == 1){
        if(get_wg_com_v2_data.com_realtime_data.InpVolt > get_wg_com_v2_data.com_param.AuotForwardOpenVoltA){	
            StateModelFlag = 0;
        }
        check_state = ADDRS_BACKWARD;
        StateCharge = eAUTO_BACKWARD;
    }else if(StateModelFlag == 0){
        if((get_wg_com_v2_data.com_realtime_data.InpVolt < get_wg_com_v2_data.com_param.AuotForwardVeerVoltA)
        && (get_wg_com_v2_data.com_realtime_data.InpVolt > get_wg_com_v2_data.com_param.SetInpUvlo)
        && (get_wg_com_v2_data.com_realtime_data.OutVolt > get_wg_com_v2_data.com_param.AuotReverseOpenVoltB)){	 	  
            StateModelFlag = 1;
        }
        check_state = ADDRS_FORWARD;
        StateCharge = eAUTO_FORWARD;
    }
}

void forward_charg_mode(void)
{
    if(get_wg_com_v2_data.BatModeFRState == 2){
        check_state = ADDRS_BACKWARD;
        StateCharge = eBACKWARD;
    }else{
        check_state = ADDRS_FORWARD;
        StateCharge = eFORWARD;
    }
}

void backward_charg_mode(void)
{
    if(get_wg_com_v2_data.BatModeFRState == 1){
        check_state = ADDRS_FORWARD;
        StateCharge = eFORWARD;
    }else{
        check_state = ADDRS_BACKWARD;
        StateCharge = eBACKWARD;
    }
}

void manual_charg_mode(void)
{
    float Rmt_Volt = adc_get_rmtvs();
    float Acc_Volt = adc_get_accvs();
    
    if(get_wg_com_v2_data.BatModeFRState == 1){
        check_state = ADDRS_FORWARD;
        StateCharge = eMANUAL_FORWARD;
    }
    else if(get_wg_com_v2_data.BatModeFRState == 2)
    {
        check_state = ADDRS_BACKWARD;
        StateCharge = eMANUAL_BACKWARD;
    }
    else
    {
        if(Acc_Volt > ACC_VEER_VOUL)
        {
            check_state = ADDRS_FORWARD;
            StateCharge = eMANUAL_FORWARD;
        }
        else if((Acc_Volt < ACC_EXIT_VEER_VOUL)&&(Rmt_Volt > RTM_VEER_VOUL))
        {
            check_state = ADDRS_BACKWARD;
            StateCharge = eMANUAL_BACKWARD;
        }
    }
}

void get_protect_data(void);
void GetChargeState(uint16_t ChargMode)
{
    float Rmt_Volt = adc_get_rmtvs();
    float Acc_Volt = adc_get_accvs();

    switch(ChargMode){
        case eSET_FORWARD:                            // 正向
            forward_charg_mode();
            break;
        case eSET_BACKWARD:                           // 反向
            backward_charg_mode();
            break;
        case eSET_AUTO_MODE:                               // 自动
            auto_charge_mode();
            break;
        case eSET_MANUAL_MODE:                             // 手动
            manual_charg_mode();
            break;
        case eSET_PG_CUSTOM_MODE:                         // 外设
            if(get_key_pg_val() == 0){
                auto_charge_mode();
            }else{
                manual_charg_mode();
            }
            break;
    }
    get_protect_data();
    WG_COM_V2_SET_DATA_UINT(StateCharge, wg_com_v2_realtime_data.ChargMode);
}

uint16_t get_check_state_data(void)
{
    return (uint16_t)check_state;
}

uint8_t gpio_get_pg_en(void)
{
    uint8_t temp = 0;
    if(get_wg_com_v2_data.com_ctrl.SetChargMode == eSET_PG_CUSTOM_MODE)
    {
        temp = 1;
    }else{
        bsp_gpio_get_bit(PIN_PG_EN, &temp);
    }
    return temp;
}



void get_protect_data(void)
{
    float fvs48_val = get_show_fvs48_show(); // 输入电压
    float rvs12_val = get_show_rvs12_show(); // 输出电压
    float Acc_val = adc_get_accvs();
    float Rtm_val = adc_get_rmtvs();

    float InpUvlo = 0.00f;
    float InpUvloRecover = 0.00f;
    float InpOVP = 0.00f;
    float InpOVPRecover = 0.00f;
    float OutUvlo = 0.00f;
    float OutUvloRecover = 0.00f;
    float OutOVP = 0.00f;
    float OutOVPRecover = 0.00f;
    float AuotOpenVoltA = 0.00f;
    float AuotCloseVoltA = 0.00f;
    float AuotOpenVoltB = 0.00f;
    float AuotCloseVoltB = 0.00f;
    float SetVolt = 0.00f;
    float SetInpt = 0.00f;

    SetVolt = get_wg_com_v2_data.com_param.SetOutVolt;
    SetInpt = get_wg_com_v2_data.com_param.SetInpVolt;
    InpUvlo = get_wg_com_v2_data.com_param.SetInpUvlo;
    InpUvloRecover = get_wg_com_v2_data.com_param.SetInpUvloRecover;
    InpOVP = get_wg_com_v2_data.com_param.SetInpOVP;
    InpOVPRecover = get_wg_com_v2_data.com_param.SetInpOVPRecover;
    OutUvlo = get_wg_com_v2_data.com_param.SetOutUvlo;
    OutUvloRecover = get_wg_com_v2_data.com_param.SetOutUvloRecover;
    OutOVP = get_wg_com_v2_data.com_param.SetOutOVP;
    OutOVPRecover = get_wg_com_v2_data.com_param.SetOutOVPRecover;

    AuotOpenVoltA = get_wg_com_v2_data.com_param.AuotForwardOpenVoltA;
    AuotCloseVoltA = get_wg_com_v2_data.com_param.AuotForwardShutVoltA;
    AuotOpenVoltB = get_wg_com_v2_data.com_param.AuotReverseOpenVoltB;
    AuotCloseVoltB = get_wg_com_v2_data.com_param.AuotReverseShutVoltB;

    charge_state_data.protect_data.protect_item_acc.val = Acc_val;
    charge_state_data.protect_data.protect_item_acc.limit = ACC_EXIT_VEER_VOUL;
    charge_state_data.protect_data.protect_item_acc.recover = ACC_VEER_VOUL;
    charge_state_data.protect_data.protect_item_acc.enable = (get_wg_com_v2_data.BatModeFRState == 0) ? ((get_check_state_data() == ADDRS_FORWARD) ? 1 : 0) : 0;

    charge_state_data.protect_data.protect_item_rtm.val = Rtm_val;
    charge_state_data.protect_data.protect_item_rtm.limit = RTM_EXIT_VEER_VOUL;
    charge_state_data.protect_data.protect_item_rtm.recover = RTM_VEER_VOUL;
    charge_state_data.protect_data.protect_item_rtm.enable = (get_wg_com_v2_data.BatModeFRState == 0) ? ((get_check_state_data() == ADDRS_BACKWARD) ? 1 : 0) : 0;

    switch(get_wg_com_v2_data.com_realtime_data.ChargMode)
    {
        case eFORWARD: // 正向
        case eMANUAL_FORWARD:
        case ePG_MANUAL_FORWARD:
            charge_state_data.protect_data.protect_item_fvs48_uvp.val = fvs48_val;
            charge_state_data.protect_data.protect_item_fvs48_uvp.limit = InpUvlo;
            charge_state_data.protect_data.protect_item_fvs48_uvp.recover = InpUvloRecover;
            charge_state_data.protect_data.protect_item_fvs48_uvp.enable = 1;
            charge_state_data.protect_data.protect_item_fvs48_ovp.val = fvs48_val;
            charge_state_data.protect_data.protect_item_fvs48_ovp.limit = InpOVP;
            charge_state_data.protect_data.protect_item_fvs48_ovp.recover = InpOVPRecover;
            charge_state_data.protect_data.protect_item_fvs48_ovp.enable = 1;
            charge_state_data.protect_data.protect_item_rvs12_uvp.val = rvs12_val;
            charge_state_data.protect_data.protect_item_rvs12_uvp.limit = OutUvlo;
            charge_state_data.protect_data.protect_item_rvs12_uvp.recover = OutUvloRecover;
            charge_state_data.protect_data.protect_item_rvs12_uvp.enable = 0;
            charge_state_data.protect_data.protect_item_rvs12_ovp.val = rvs12_val;
            charge_state_data.protect_data.protect_item_rvs12_ovp.limit = (SetVolt+2.0f);
            charge_state_data.protect_data.protect_item_rvs12_ovp.recover = SetVolt;
            charge_state_data.protect_data.protect_item_rvs12_ovp.enable = 1;
             break;
        case eBACKWARD: // 反向
        case eMANUAL_BACKWARD:
        case ePG_MANUAL_BACKWARD:
            charge_state_data.protect_data.protect_item_fvs48_uvp.val = fvs48_val;
            charge_state_data.protect_data.protect_item_fvs48_uvp.limit = InpUvlo;
            charge_state_data.protect_data.protect_item_fvs48_uvp.recover = InpUvloRecover;
            charge_state_data.protect_data.protect_item_fvs48_uvp.enable = 0;
            charge_state_data.protect_data.protect_item_fvs48_ovp.val = fvs48_val;
            charge_state_data.protect_data.protect_item_fvs48_ovp.limit = (SetInpt+2.0f);
            charge_state_data.protect_data.protect_item_fvs48_ovp.recover = SetInpt;
            charge_state_data.protect_data.protect_item_fvs48_ovp.enable = 1;
            charge_state_data.protect_data.protect_item_rvs12_uvp.val = rvs12_val;
            charge_state_data.protect_data.protect_item_rvs12_uvp.limit = OutUvlo;
            charge_state_data.protect_data.protect_item_rvs12_uvp.recover = OutUvloRecover;
            charge_state_data.protect_data.protect_item_rvs12_uvp.enable = 1;
            charge_state_data.protect_data.protect_item_rvs12_ovp.val = rvs12_val;
            charge_state_data.protect_data.protect_item_rvs12_ovp.limit = OutOVP;
            charge_state_data.protect_data.protect_item_rvs12_ovp.recover = OutOVPRecover;
            charge_state_data.protect_data.protect_item_rvs12_ovp.enable = 1;
            break;
        case eAUTO_FORWARD:
        case ePG_AUTO_FORWARD:
            charge_state_data.protect_data.protect_item_fvs48_uvp.val = fvs48_val;
            charge_state_data.protect_data.protect_item_fvs48_uvp.limit = (InpUvlo > AuotCloseVoltA) ? InpUvlo : AuotCloseVoltA;
            charge_state_data.protect_data.protect_item_fvs48_uvp.recover = (InpUvloRecover > AuotOpenVoltA) ? InpUvloRecover : AuotOpenVoltA;
            charge_state_data.protect_data.protect_item_fvs48_uvp.enable = 1;
            charge_state_data.protect_data.protect_item_fvs48_ovp.val = fvs48_val;
            charge_state_data.protect_data.protect_item_fvs48_ovp.limit = InpOVP;
            charge_state_data.protect_data.protect_item_fvs48_ovp.recover = InpOVPRecover;
            charge_state_data.protect_data.protect_item_fvs48_ovp.enable = 1;
            charge_state_data.protect_data.protect_item_rvs12_uvp.val = rvs12_val;
            charge_state_data.protect_data.protect_item_rvs12_uvp.limit = AuotCloseVoltB;
            charge_state_data.protect_data.protect_item_rvs12_uvp.recover = AuotOpenVoltB;
            charge_state_data.protect_data.protect_item_rvs12_uvp.enable = 1;
            charge_state_data.protect_data.protect_item_rvs12_ovp.val = rvs12_val;
            charge_state_data.protect_data.protect_item_rvs12_ovp.limit = (SetVolt+2.0f);
            charge_state_data.protect_data.protect_item_rvs12_ovp.recover = SetVolt;
            charge_state_data.protect_data.protect_item_rvs12_ovp.enable = 1;
            break;
        case eAUTO_BACKWARD:
        case ePG_AUTO_BACKWARD:
            charge_state_data.protect_data.protect_item_fvs48_uvp.val = fvs48_val;
            charge_state_data.protect_data.protect_item_fvs48_uvp.limit = InpUvlo;
            charge_state_data.protect_data.protect_item_fvs48_uvp.recover = InpUvloRecover;
            charge_state_data.protect_data.protect_item_fvs48_uvp.enable = 1;
            charge_state_data.protect_data.protect_item_fvs48_ovp.val = fvs48_val;
            charge_state_data.protect_data.protect_item_fvs48_ovp.limit = (SetInpt+2.0f);
            charge_state_data.protect_data.protect_item_fvs48_ovp.recover = SetInpt;
            charge_state_data.protect_data.protect_item_fvs48_ovp.enable = 1;
            charge_state_data.protect_data.protect_item_rvs12_uvp.val = rvs12_val;
            charge_state_data.protect_data.protect_item_rvs12_uvp.limit = (OutUvlo > AuotCloseVoltB) ? OutUvlo : AuotCloseVoltB;
            charge_state_data.protect_data.protect_item_rvs12_uvp.recover = (OutUvloRecover > AuotOpenVoltB) ? OutUvloRecover : AuotOpenVoltB;
            charge_state_data.protect_data.protect_item_rvs12_uvp.enable = 1;
            charge_state_data.protect_data.protect_item_rvs12_ovp.val = rvs12_val;
            charge_state_data.protect_data.protect_item_rvs12_ovp.limit = OutOVP;
            charge_state_data.protect_data.protect_item_rvs12_ovp.recover = OutOVPRecover;
            charge_state_data.protect_data.protect_item_rvs12_ovp.enable = 1;
            break;
    }
}


