#include "bat_charge_pattern.h"
#include "wg_com_v2.h"
#include "adc_check.h"
#include "adc.h"
#include "gpio.h"
#include "bsp_gpio.h"
#include "get_com_data.h"
#include "eeprom_cfg.h"
static ADC_CHECK_ADDRS_E check_state = ADDRS_IDLE;
static uint16_t StateCharge = eFORWARD;
static uint8_t reverse_timeout_lock = 0U;
static uint8_t manual_direction_monitor_init = 0U;
static uint16_t manual_last_bat_mode_fr = 0U;
static uint8_t manual_last_acc_direction = 0U;
static uint8_t manual_active_direction = 0U;
static uint8_t manual_applied_direction = 0U;
static uint8_t manual_last_report_direction = 0U;
static uint8_t acc_reverse_enter_event = 0U;
static uint8_t manual_reverse_exit_delay_seconds = 0U;
static uint8_t manual_last_acc_reverse_request = 0U;

#define MANUAL_REVERSE_EXIT_DELAY_SECONDS 10U

#define DIR_MON_STAGE_INIT        8U
#define DIR_MON_STAGE_ACC_CHANGE  9U
#define DIR_MON_STAGE_40C_CHANGE  10U
#define DIR_MON_STAGE_APPLY       11U
#define DIR_MON_STAGE_EXIT_DELAY  13U
#define DIR_MON_DBG(stage, mode, profile, result, value) app_debug_event_push(APP_DBG_EVT_REVERSE_TIMER, APP_DBG_AREA_P02, (stage), (mode), (profile), (result), (value))

static void manual_sync_acc_direction_to_control(uint8_t acc_direction)
{
    if((acc_direction == eADDRS_FORWARD) || (acc_direction == eADDRS_BACKWARD))
    {
        wg_com_v2_set_bat_mode_fr_runtime(acc_direction);
        manual_last_bat_mode_fr = acc_direction;
    }
}

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

uint8_t bat_charge_acc_reverse_request_active(void)
{
    float Rmt_Volt = adc_get_rmtvs();
    float Acc_Volt = adc_get_accvs();

    return ((Acc_Volt < ACC_EXIT_VEER_VOUL) && (Rmt_Volt > RTM_VEER_VOUL)) ? 1U : 0U;
}

static uint8_t bat_charge_get_acc_direction_request(void)
{
    float Rmt_Volt = adc_get_rmtvs();
    float Acc_Volt = adc_get_accvs();

    if(Acc_Volt > ACC_VEER_VOUL)
    {
        return eADDRS_FORWARD;
    }
    if((Acc_Volt < ACC_EXIT_VEER_VOUL) && (Rmt_Volt > RTM_VEER_VOUL))
    {
        return eADDRS_BACKWARD;
    }
    return 0U;
}

uint8_t bat_charge_consume_acc_reverse_enter_event(void)
{
    uint8_t event = acc_reverse_enter_event;
    acc_reverse_enter_event = 0U;
    return event;
}

uint8_t bat_charge_reverse_timeout_lock_active(void)
{
    return reverse_timeout_lock;
}

void bat_charge_set_reverse_timeout_lock(uint8_t lock)
{
    reverse_timeout_lock = (lock != 0U) ? 1U : 0U;
}

void manual_charg_mode(void)
{
    uint16_t bat_mode_fr = get_wg_com_v2_data.BatModeFRState;
    uint8_t acc_direction = bat_charge_get_acc_direction_request();
    uint8_t acc_reverse_request = bat_charge_acc_reverse_request_active();

    if(manual_direction_monitor_init == 0U)
    {
        manual_direction_monitor_init = 1U;
        manual_last_bat_mode_fr = bat_mode_fr;
        manual_last_acc_direction = acc_direction;
        manual_last_acc_reverse_request = acc_reverse_request;
        if(acc_direction != 0U)
        {
            manual_sync_acc_direction_to_control(acc_direction);
            bat_mode_fr = acc_direction;
            manual_active_direction = acc_direction;
            if(acc_direction == eADDRS_BACKWARD)
            {
                acc_reverse_enter_event = 1U;
            }
        }
        else if((bat_mode_fr == eADDRS_FORWARD) || (bat_mode_fr == eADDRS_BACKWARD))
        {
            manual_active_direction = (uint8_t)bat_mode_fr;
        }
        else
        {
            manual_active_direction = eADDRS_FORWARD;
        }
        manual_applied_direction = manual_active_direction;
        manual_last_report_direction = manual_active_direction;
        DIR_MON_DBG(DIR_MON_STAGE_INIT, acc_direction, (uint8_t)bat_mode_fr, APP_DBG_RESULT_START, manual_active_direction);
    }
    else
    {
        if(bat_mode_fr != manual_last_bat_mode_fr)
        {
            manual_last_bat_mode_fr = bat_mode_fr;
            if((bat_mode_fr == eADDRS_FORWARD) || (bat_mode_fr == eADDRS_BACKWARD))
            {
                manual_active_direction = (uint8_t)bat_mode_fr;
            }
            else
            {
                manual_active_direction = (acc_direction != 0U) ? acc_direction : eADDRS_FORWARD;
            }
            DIR_MON_DBG(DIR_MON_STAGE_40C_CHANGE, acc_direction, (uint8_t)bat_mode_fr, APP_DBG_RESULT_START, manual_active_direction);
        }

        if(acc_direction != manual_last_acc_direction)
        {
            manual_last_acc_direction = acc_direction;
            manual_last_acc_reverse_request = acc_reverse_request;
            if(acc_direction != 0U)
            {
                manual_sync_acc_direction_to_control(acc_direction);
                bat_mode_fr = acc_direction;
                manual_active_direction = acc_direction;
                if(acc_direction == eADDRS_BACKWARD)
                {
                    acc_reverse_enter_event = 1U;
                }
            }
            else if(manual_active_direction == eADDRS_BACKWARD)
            {
                manual_sync_acc_direction_to_control(eADDRS_FORWARD);
                bat_mode_fr = eADDRS_FORWARD;
                manual_active_direction = eADDRS_FORWARD;
            }
            DIR_MON_DBG(DIR_MON_STAGE_ACC_CHANGE, acc_direction, (uint8_t)bat_mode_fr, APP_DBG_RESULT_START, manual_active_direction);
        }
        else if(acc_reverse_request != manual_last_acc_reverse_request)
        {
            manual_last_acc_reverse_request = acc_reverse_request;
            if(acc_reverse_request != 0U)
            {
                manual_sync_acc_direction_to_control(eADDRS_BACKWARD);
                bat_mode_fr = eADDRS_BACKWARD;
                manual_active_direction = eADDRS_BACKWARD;
                acc_reverse_enter_event = 1U;
            }
            else if(manual_active_direction == eADDRS_BACKWARD)
            {
                manual_sync_acc_direction_to_control(eADDRS_FORWARD);
                bat_mode_fr = eADDRS_FORWARD;
                manual_active_direction = eADDRS_FORWARD;
            }
            DIR_MON_DBG(DIR_MON_STAGE_ACC_CHANGE, acc_direction, (uint8_t)bat_mode_fr, APP_DBG_RESULT_START, manual_active_direction);
        }
    }

    if((reverse_timeout_lock != 0U) && (bat_charge_acc_reverse_request_active() != 0U))
    {
        manual_applied_direction = eADDRS_FORWARD;
        manual_reverse_exit_delay_seconds = 0U;
        check_state = ADDRS_FORWARD;
        StateCharge = eMANUAL_FORWARD;
    }
    else
    {
        if(manual_active_direction == eADDRS_BACKWARD)
        {
            manual_applied_direction = eADDRS_BACKWARD;
            manual_reverse_exit_delay_seconds = 0U;
        }
        else if(manual_applied_direction != eADDRS_BACKWARD)
        {
            manual_applied_direction = eADDRS_FORWARD;
        }

        if(manual_applied_direction == eADDRS_BACKWARD)
        {
            check_state = ADDRS_BACKWARD;
            StateCharge = eMANUAL_BACKWARD;
        }
        else
        {
            check_state = ADDRS_FORWARD;
            StateCharge = eMANUAL_FORWARD;
        }
    }

    if(manual_last_report_direction != manual_active_direction)
    {
        manual_last_report_direction = manual_active_direction;
        DIR_MON_DBG(DIR_MON_STAGE_APPLY, manual_last_acc_direction, (uint8_t)bat_mode_fr, APP_DBG_RESULT_OK, manual_active_direction);
    }
}

static void manual_reverse_exit_delay_task(void)
{
    if(manual_applied_direction != eADDRS_BACKWARD)
    {
        manual_reverse_exit_delay_seconds = 0U;
        return;
    }

    if(manual_active_direction == eADDRS_BACKWARD)
    {
        manual_reverse_exit_delay_seconds = 0U;
        return;
    }

    if(manual_reverse_exit_delay_seconds == 0U)
    {
        manual_reverse_exit_delay_seconds = MANUAL_REVERSE_EXIT_DELAY_SECONDS;
        DIR_MON_DBG(DIR_MON_STAGE_EXIT_DELAY, manual_last_acc_direction, manual_active_direction, APP_DBG_RESULT_START, manual_reverse_exit_delay_seconds);
        return;
    }

    manual_reverse_exit_delay_seconds--;
    if(manual_reverse_exit_delay_seconds == 0U)
    {
        manual_applied_direction = eADDRS_FORWARD;
        DIR_MON_DBG(DIR_MON_STAGE_EXIT_DELAY, manual_last_acc_direction, manual_active_direction, APP_DBG_RESULT_OK, 0U);
    }
}

REG_TASK(1000, manual_reverse_exit_delay_task)

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

    switch(StateCharge)
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


