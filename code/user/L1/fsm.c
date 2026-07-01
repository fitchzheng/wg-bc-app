#include "fsm.h"
#include "section.h"
#include "bsp_gpio.h"
#include "stdint.h"
#include "my_math.h"
#include "adc_check.h"
#include "adc.h"
#include "gpio.h"
#include "ctrl_app.h"
#include "fault.h"
#include "shell.h"
#include "data_com.h"
#include "power_sw.h"
#include "wg_com_v2.h"
#include "get_com_data.h"
#include "bat_charge_pattern.h"
#include "protect.h"
#include "stop_dormant.h"
uint8_t force_ccm = 0;
static uint8_t direction_restart_pending = 0;
static uint8_t mode_restart_pending = 0;
#define FAST_RESTART_VOLTAGE_SYNC_TIME TIME_CNT_20MS_IN_1MS
#define MODE_RESTART_HOLD_TIME TIME_CNT_500MS_IN_1MS
static uint16_t fast_restart_voltage_sync_cnt = 0;
static uint16_t mode_restart_hold_cnt = 0;
#define PG_ALARM_CONFIRM_TIME TIME_CNT_500MS_IN_1MS
// 状态枚举
typedef enum
{
    STATE_INIT = 1,
    STATE_IDLE,
    STATE_OPEN_AUX,
    STATE_MODE_DETECT,
    STATE_SET_FORWARD,
    STATE_SET_BACKWARD,
    STATE_SET_BIDIRECTIONAL,
    STATE_RUN,
    STATE_STOP,
    STATE_PWC_STOP,
    STATE_FAULT,
} fsm_states_t;

enum
{
    ev_init_is_ok = 1,
    ev_idle_to_chg,
    ev_aux_is_ok,
    ev_is_forward,
    ev_is_backward,
    ev_is_bidirectional,
    ev_goto_run,
    ev_stop,
    ev_pwc_stop,
    ev_idle,
    ev_fault,
};

static uint32_t fsm_ev = 0;

static uint32_t init_delay = 0;

static float fvs48_lmt = 0;
static float rvs12_lmt = 0;
static float ihv_lmt = 0;
static float ilv_lmt = 0;
extern uint8_t stop_time_flag;
extern uint8_t sleep_report_state_flag;
uint8_t pwc_stop_flag = 0;
extern uint8_t stop_soft;
static void init_in(void)
{
    gpio_set_auxoff(0);
    pwc_stop_flag = 0;
    stop_time_flag = 0;
    sleep_report_state_flag = 0;
    init_delay = 0;
    fault_clear_all_fault();
	get_key_pg_state();
}

static void init_exe(void)
{
    if (init_delay >= (TIME_CNT_200MS_IN_1MS))
    {
        fsm_ev = ev_init_is_ok;
        float rvs12 = get_show_rvs12_show();
        float fvs48 = get_show_fvs48_show();
        GetChargeState(get_wg_com_v2_data.com_ctrl.SetChargMode);
        if(get_check_state_data() == ADDRS_FORWARD){
            if(fvs48 < get_wg_com_v2_data.com_param.SetInpUvloRecover)
            {
                fault_set_fault(FAULT_FVS48_UVP);
            }
            if(fvs48 > get_wg_com_v2_data.com_param.SetInpOVPRecover)
            {
                fault_set_fault(FAULT_FVS48_OVP);
            }
            if(fvs48 < 7.5f){
                gpio_set_dsg(0);
                gpio_set_chg(0);
            }else if(fvs48 > 8.0f){
                gpio_set_dsg(1);
                gpio_set_chg(1);  
            }
        }else if(get_check_state_data() == ADDRS_BACKWARD){
            if(rvs12 < get_wg_com_v2_data.com_param.SetOutUvloRecover)
            {
                fault_set_fault(FAULT_RVS12_UVP);
            }
            if(rvs12 > get_wg_com_v2_data.com_param.SetOutOVPRecover)
            {
                fault_set_fault(FAULT_RVS12_OVP);
            }
            if(rvs12 < 7.5f){
                gpio_set_dsg(0);
                gpio_set_chg(0);
            }else if(rvs12 > 8.0f){
                gpio_set_dsg(1);
                gpio_set_chg(1);  
            }
        }
    }
    else
    {
        init_delay++;
    }
}

static uint32_t init_chk(uint32_t fsm_ev)
{
    if (fsm_ev == ev_init_is_ok)
    {
        return STATE_IDLE;
    }
    return 0;
}

static void init_out(void)
{
}

static void idle_in(void)
{
    charge_state_data.Boot_Time_Delay.SetBootTimeFlag = 0;
    charge_state_data.Boot_Time_Delay.BootTimeDelay = 0;
    if((direction_restart_pending != 0) || (mode_restart_pending != 0))
    {
        charge_state_data.soft_start_flag = 0;
        ihv_lmt = 1.0f;
        ilv_lmt = 1.0f;
        set_ihv_lmt_soft_curr(1.0f);
        set_ilv_lmt_soft_curr(1.0f);
        fast_restart_voltage_sync_cnt = FAST_RESTART_VOLTAGE_SYNC_TIME;
        mode_restart_hold_cnt = MODE_RESTART_HOLD_TIME;
        direction_restart_pending = 0;
        mode_restart_pending = 0;
    }
}

static uint8_t pwr_is_on = 0;
#if (SHELL_ON_OFF == 1)
REG_SHELL_VAR(pwr_is_on, pwr_is_on, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
#endif
static ADC_CHECK_ADDRS_E state_flag = ADDRS_IDLE;
static void idle_exe(void)
{
    pwr_is_on = power_sw_get_power_is_on();
    
    if(state_flag != adc_check_get_addrs_state())
    {
        state_flag = adc_check_get_addrs_state();
        charge_state_data.Boot_Time_Delay.SetBootTimeFlag = 0;
        charge_state_data.Boot_Time_Delay.BootTimeDelay = 0;
    }

    if(mode_restart_hold_cnt != 0)
    {
        mode_restart_hold_cnt--;
        charge_state_data.Boot_Time_Delay.SetBootTimeFlag = 0;
        charge_state_data.Boot_Time_Delay.BootTimeDelay = 0;
    }

    if ((adc_check_get_addrs_state() != ADDRS_IDLE) &&
        (adc_check_get_aux_is_ok() == 1) &&
        ((adc_check_get_fvs48_is_ok() == 1) ||
         (adc_check_get_rvs12_is_ok() == 1))&&
         (pwr_is_on == 1) &&
         (charge_state_data.protect_data.over_temp_protect == 0)&&
         (fault_get_alarm_bit(ALARM_PG_IS_OFF) == 0)            &&
         (mode_restart_hold_cnt == 0)                            &&
         (charge_state_data.bat_state.LithiumBatOnOff == 0)		&&
		 (get_key_pg_val()== 1))
    {
        if(charge_state_data.Boot_Time_Delay.SetBootTimeFlag == 1)
        {
            fsm_ev = ev_idle_to_chg;
        }
    }else{
        charge_state_data.Boot_Time_Delay.BootTimeDelay = 0;
    }

    if(fault_get_fault() != 0)
    {
        set_charge_state_mode(eFAULT_CHARGE);
    }
    else if(charge_state_data.bat_state.LithiumBatOnOff == 1)
    {
        set_charge_state_mode(eSTOP_CHARGE);
    }else{
        set_charge_state_mode(eIDIE_CHARGE);
    }
    if(sleep_report_state_flag == 1)
    {
        set_charge_state_mode(eSTOP_CHARGE);
    }
    charge_state_data.soft_close_flag = 0;
}

static uint32_t idle_chk(uint32_t fsm_ev)
{
    if (fsm_ev == ev_idle_to_chg)
    {
        return STATE_OPEN_AUX;
    }
    else if (fsm_ev == ev_fault)
    {
        return STATE_FAULT;
    }
    return 0;
}

static void idle_out(void)
{
}

static uint32_t aux_abnormal_cnt = 0;
static uint32_t aux_normal_cnt = 0;

static void open_aux_in(void)
{
    aux_abnormal_cnt = 0;
    aux_normal_cnt = 0;

}

static void open_aux_exe(void)
{
    if (adc_check_get_aux_is_ok() == 1)
    {
        aux_normal_cnt++;
    }

    if (aux_normal_cnt > 1)
    {
        fsm_ev = ev_aux_is_ok;
        fault_clear_fault(FAULT_AUX_IS_ERR);
    }

    aux_abnormal_cnt++;
    if (aux_abnormal_cnt > TIME_CNT_3S_IN_1MS)
    {
        aux_abnormal_cnt = 0;
        fsm_ev = ev_fault;
        fault_set_fault(FAULT_AUX_IS_ERR);
    }
}

static uint32_t open_aux_chk(uint32_t fsm_ev)
{
    if (fsm_ev == ev_aux_is_ok)
    {
        return STATE_MODE_DETECT;
    }
    else if (fsm_ev == ev_fault)
    {
        return STATE_FAULT;
    }
    return 0;
}

static void open_aux_out(void)
{
    
}

static uint32_t addrs_abnormal_cnt = 0;

static void mode_detect_in(void)
{
    addrs_abnormal_cnt = 0;
}

static void mode_detect_exe(void)
{
    if (adc_check_get_addrs_state() == ADDRS_FORWARD)
    {
        fsm_ev = ev_is_forward;
    }
    else if (adc_check_get_addrs_state() == ADDRS_BACKWARD)
    {
        fsm_ev = ev_is_backward;
    }
    else if (adc_check_get_addrs_state() == ADDRS_BIDIRECTIONAL)
    {
        fsm_ev = ev_is_bidirectional;
    }

    addrs_abnormal_cnt++;

    if (addrs_abnormal_cnt > TIME_CNT_3S_IN_1MS)
    {
        addrs_abnormal_cnt = 0;
        fsm_ev = ev_fault;
        fault_set_fault(FAULT_ADDR_IS_ERR);
    }
}

static uint32_t mode_detect_chk(uint32_t fsm_ev)
{
    if (fsm_ev == ev_is_forward)
    {
        return STATE_SET_FORWARD;
    }
    else if (fsm_ev == ev_is_backward)
    {
        return STATE_SET_BACKWARD;
    }
    else if (fsm_ev == ev_is_bidirectional)
    {
        return STATE_SET_BIDIRECTIONAL;
    }
    else if (fsm_ev == ev_fault)
    {
        return STATE_FAULT;
    }
    return 0;
}

static void mode_detect_out(void)
{
    ctrl_app_set_fvs48_lmt(adc_get_fvs48());
    ctrl_app_set_rvs12_lmt(adc_get_rvs12());
}
static void set_forward_in(void)
{
    ctrl_app_buck_boost_dc_init(CTRL_FORWARD);
    force_ccm = 0;
}

static void set_forward_exe(void)
{
    fsm_ev = ev_goto_run;
}

static uint32_t set_forward_chk(uint32_t fsm_ev)
{
    if (fsm_ev == ev_goto_run)
    {
        return STATE_RUN;
    }
    else if (fsm_ev == ev_fault)
    {
        return STATE_FAULT;
    }
    return 0;
}

static void set_forward_out(void)
{
}

static void set_backward_in(void)
{
    ctrl_app_buck_boost_dc_init(CTRL_BACKWARD);
    force_ccm = 0;
}

static void set_backward_exe(void)
{
    fsm_ev = ev_goto_run;
}

static uint32_t set_backward_chk(uint32_t fsm_ev)
{
    if (fsm_ev == ev_goto_run)
    {
        return STATE_RUN;
    }
    else if (fsm_ev == ev_fault)
    {
        return STATE_FAULT;
    }
    return 0;
}

static void set_backward_out(void)
{
}

static void set_bidirectional_in(void)
{
    ctrl_app_buck_boost_dc_init(CTRL_BIDIRECTIONAL);
    force_ccm = 0;
}

static void set_bidirectional_exe(void)
{
    fsm_ev = ev_goto_run;
}

static uint32_t set_bidirectional_chk(uint32_t fsm_ev)
{
    if (fsm_ev == ev_goto_run)
    {
        return STATE_RUN;
    }
    else if (fsm_ev == ev_fault)
    {
        return STATE_FAULT;
    }
    return 0;
}

static void set_bidirectional_out(void)
{
}

ADC_CHECK_ADDRS_E addrs_state_now = ADDRS_IDLE;
static STATE_CONTROL_PARAMETER_T control_parameter;
static uint16_t fsm_get_active_boot_time(ADC_CHECK_ADDRS_E addrs_state)
{
    if(addrs_state == ADDRS_BACKWARD)
    {
        return get_wg_com_v2_data.com_ctrl.SetBootTimeB;
    }
    else if(addrs_state == ADDRS_FORWARD)
    {
        return get_wg_com_v2_data.com_ctrl.SetBootTimeA;
    }
    return 0;
}
static void run_in(void)
{
    fvs48_lmt = adc_get_fvs48();
    rvs12_lmt = adc_get_rvs12();
    addrs_state_now = adc_check_get_addrs_state();
    control_parameter.InpBatyType  = get_wg_com_v2_data.com_ctrl.InpBatyType;
    control_parameter.OutBatyType  = get_wg_com_v2_data.com_ctrl.OutBatyType;
    control_parameter.SetChargMode = get_wg_com_v2_data.com_ctrl.SetChargMode;
    control_parameter.SetPowerMode = get_wg_com_v2_data.com_ctrl.SetPowerMode;
    control_parameter.MpptSwitch  = get_wg_com_v2_data.com_ctrl.MpptSwitch;

    if (addrs_state_now == ADDRS_BACKWARD)
    {
        charge_state_data.ActiveOnCurrStartTime = get_wg_com_v2_data.com_ctrl.SetOnCurrStartTimeA;
    }
    else if (addrs_state_now == ADDRS_FORWARD)
    {
        charge_state_data.ActiveOnCurrStartTime = get_wg_com_v2_data.com_ctrl.SetOnCurrStartTimeB;
    }
    else
    {
        charge_state_data.ActiveOnCurrStartTime = 0;
    }

    if(fsm_get_active_boot_time(addrs_state_now) <= 1)
    {
        fast_restart_voltage_sync_cnt = FAST_RESTART_VOLTAGE_SYNC_TIME;
    }
}

static void run_exe(void)
{
    ADC_CHECK_ADDRS_E current_addrs_state = adc_check_get_addrs_state();

    force_ccm = 0;
    pwr_is_on = power_sw_get_power_is_on();
    BAT_CHARGE_MODE_E bat_charge_mode;
    if ((current_addrs_state == addrs_state_now)                                    &&
       ((adc_check_get_fvs48_is_ok() == 1)                                          ||
        (adc_check_get_rvs12_is_ok() == 1))                                         &&
        (pwr_is_on == 1)                                                            &&
        (charge_state_data.bat_state.LithiumBatOnOff == 0)                          &&
        (control_parameter.InpBatyType  == get_wg_com_v2_data.com_ctrl.InpBatyType) &&
        (control_parameter.OutBatyType  == get_wg_com_v2_data.com_ctrl.OutBatyType) &&
        (control_parameter.SetChargMode == get_wg_com_v2_data.com_ctrl.SetChargMode)&&
        (control_parameter.SetPowerMode == get_wg_com_v2_data.com_ctrl.SetPowerMode)&&
        (control_parameter.MpptSwitch  == get_wg_com_v2_data.com_ctrl.MpptSwitch) &&
        (charge_state_data.protect_data.over_temp_protect == 0)                     &&
        (ctrl_app_get_is_run() == 1)                                                &&
        (fault_get_alarm_bit(ALARM_PG_IS_OFF) == 0)                                 &&
        (stop_time_flag == 0))
    {
        (get_volt_low_fault() == 1) ? (bat_charge_mode = eFAULT_CHARGE) : (bat_charge_mode = (BAT_CHARGE_MODE_E)charge_state_data.SetCharState);
        set_charge_state_mode(bat_charge_mode);
        if(sleep_report_state_flag == 1)
        {
            set_charge_state_mode(eSTOP_CHARGE);
        }
    }
    else
    {
        if((current_addrs_state != addrs_state_now) && (current_addrs_state != ADDRS_IDLE))
        {
            direction_restart_pending = 1;
        }
        if((control_parameter.SetPowerMode != get_wg_com_v2_data.com_ctrl.SetPowerMode) ||
           (control_parameter.MpptSwitch  != get_wg_com_v2_data.com_ctrl.MpptSwitch)  ||
           (control_parameter.InpBatyType != get_wg_com_v2_data.com_ctrl.InpBatyType) ||
           (control_parameter.OutBatyType != get_wg_com_v2_data.com_ctrl.OutBatyType))
        {
            mode_restart_pending = 1;
        }
        if(charge_state_data.bat_state.LithiumBatOnOff == 1)
        {
            charge_state_data.SetCharState = eSTOP_CHARGE;
            set_charge_state_mode(eSTOP_CHARGE);
        }
        fsm_ev = ev_stop;
    }

	charge_state_data.soft_close_flag = 0;
    if(fast_restart_voltage_sync_cnt != 0)
    {
        fvs48_lmt = data_com_get_fvs48_lmt();
        rvs12_lmt = data_com_get_rvs12_lmt();
        fast_restart_voltage_sync_cnt--;
    }
    else
    {
        RAMP(fvs48_lmt, data_com_get_fvs48_lmt(), data_com_get_fvs48_lmt() / 200.0f);
        RAMP(rvs12_lmt, data_com_get_rvs12_lmt(), data_com_get_rvs12_lmt() / 200.0f);
    }
    ctrl_app_set_fvs48_lmt(fvs48_lmt);
    ctrl_app_set_rvs12_lmt(rvs12_lmt);
}

static uint32_t run_chk(uint32_t fsm_ev)
{
    if (fsm_ev == ev_stop)
    {
        return STATE_STOP;
    }
    else if (fsm_ev == ev_fault)
    {
        return STATE_FAULT;
    }
    return 0;
}

static void run_out(void)
{
//    ctrl_app_disable();
}
#include "gpio.h"
static void stop_in(void)
{
    ihv_lmt = fabsf(adc_get_ihv());
    ilv_lmt = fabsf(adc_get_ilv());
    charge_state_data.soft_close_flag = 1;
}
static void stop_exe(void)
{
if(addrs_state_now == ADDRS_FORWARD)
    {
        RAMP(rvs12_lmt, 1.0f, 0.01f);
        RAMP(ilv_lmt, 1.0f, 0.01f);
    }
    else
    {
        RAMP(fvs48_lmt, 1.0f, 0.01f);
        RAMP(ihv_lmt, 1.0f, 0.01f);
    }
    ctrl_app_set_fvs48_lmt(fvs48_lmt);
    ctrl_app_set_rvs12_lmt(rvs12_lmt);
    if(((((addrs_state_now == ADDRS_FORWARD) && (ilv_lmt < 2.0f))   || ((addrs_state_now == ADDRS_BACKWARD) && (ihv_lmt   < 2.0f)))||(stop_soft == 1)) ||
       ((((addrs_state_now == ADDRS_FORWARD) && (rvs12_lmt < 2.0f)) || ((addrs_state_now == ADDRS_BACKWARD) && (fvs48_lmt < 2.0f)))||(stop_soft == 1)))
    {
        if(stop_time_flag == 1)
        {
            fsm_ev = ev_pwc_stop;
        }
        else
        {
            fsm_ev = ev_idle;
        }
        force_ccm = 0;
    }else{
        force_ccm = 1;
    }
}

static uint32_t stop_chk(uint32_t fsm_ev)
{
    if(fsm_ev == ev_pwc_stop)
    {
        return STATE_PWC_STOP;
    }
    else if (fsm_ev == ev_idle)
    {
        return STATE_IDLE;
    }
    else if (fsm_ev == ev_fault)
    {
        return STATE_FAULT;
    }
    return 0;
}

static void stop_out(void)
{
    ctrl_app_disable();
    charge_state_data.soft_close_flag = 0;
}


static uint32_t pwc_stop_delay = 0; 
static void pwc_stop_in(void)
{
    pwc_stop_delay = 0;
}

static void pwc_stop_exe(void)
{
    if(stop_time_flag == 1)
    {
        if (pwc_stop_delay >= TIME_CNT_1S_IN_1MS)
        {
            fsm_ev = ev_idle;
            sleep_low_power_commit();
            pwc_stop_flag = 1;
        }
        else
        {
            pwc_stop_delay++;
        }
    }
    else
    {

        fsm_ev = ev_idle;
    }
    
}

static uint32_t pwc_stop_chk(uint32_t fsm_ev)
{
    if (fsm_ev == ev_idle)
    {
        return STATE_IDLE;
    }
    else if (fsm_ev == ev_fault)
    {
        return STATE_FAULT;
    }
    return 0;
}

static void pwc_stop_out(void)
{
}










static void fault_in(void)
{
    ctrl_app_disable();
}

static void fault_exe(void)
{
    if (adc_check_get_aux_is_ok() == 1)
    {
        fault_clear_fault(FAULT_AUX_IS_ERR);
        fsm_ev = ev_idle;
    }
}

static uint32_t fault_chk(uint32_t fsm_ev)
{
    if (fsm_ev == ev_idle)
    {
        return STATE_IDLE;
    }
    return 0;
}

static void fault_out(void)
{
}

// 注册状态机
REG_FSM(my_fsm, STATE_INIT, fsm_ev,
        FSM_ENTRY(STATE_INIT, init_in, init_exe, init_chk, init_out),
        FSM_ENTRY(STATE_IDLE, idle_in, idle_exe, idle_chk, idle_out),
        FSM_ENTRY(STATE_OPEN_AUX, open_aux_in, open_aux_exe, open_aux_chk, open_aux_out),
        FSM_ENTRY(STATE_MODE_DETECT, mode_detect_in, mode_detect_exe, mode_detect_chk, mode_detect_out),
        FSM_ENTRY(STATE_SET_FORWARD, set_forward_in, set_forward_exe, set_forward_chk, set_forward_out),
        FSM_ENTRY(STATE_SET_BACKWARD, set_backward_in, set_backward_exe, set_backward_chk, set_backward_out),
        FSM_ENTRY(STATE_SET_BIDIRECTIONAL, set_bidirectional_in, set_bidirectional_exe, set_bidirectional_chk, set_bidirectional_out),
        FSM_ENTRY(STATE_RUN, run_in, run_exe, run_chk, run_out),
        FSM_ENTRY(STATE_STOP, stop_in, stop_exe, stop_chk, stop_out),
        FSM_ENTRY(STATE_PWC_STOP, pwc_stop_in, pwc_stop_exe, pwc_stop_chk, pwc_stop_out),
        FSM_ENTRY(STATE_FAULT, fault_in, fault_exe, fault_chk, fault_out), )

static uint8_t fsm_state = 0;
#if (SHELL_ON_OFF == 1)
REG_SHELL_VAR(fsm_state, fsm_state, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
#endif

static uint16_t Get_soft_is_off_State(void)
{
    uint16_t soft_is_off_State;
    WG_COM_V2_GET_DATA_UINT(soft_is_off_State, wg_com_v2_ctrl.PowerOnOff);
    return soft_is_off_State;
}

static void fsm_run(void)
{
    static uint32_t pg_alarm_low_cnt = 0;
    if (fault_get_fault())
    {
        fsm_ev = ev_fault;
    }
    fsm_state = FSM_GET_STATE(my_fsm);

    if(Get_soft_is_off_State() == 1){
        fault_set_alarm(ALARM_PWR_IS_OFF);
    }
    else
    {
        fault_clear_alarm(ALARM_PWR_IS_OFF);
    }

    if((get_key_pg_val() == 0)&&(get_wg_com_v2_data.com_ctrl.SetChargMode != eSET_PG_CUSTOM_MODE))
    {
        if(pg_alarm_low_cnt >= PG_ALARM_CONFIRM_TIME)
        {
            fault_set_alarm(ALARM_PG_IS_OFF);
        }
        else
        {
            pg_alarm_low_cnt++;
            fault_clear_alarm(ALARM_PG_IS_OFF);
        }
    }
    else
    {
        pg_alarm_low_cnt = 0;
        fault_clear_alarm(ALARM_PG_IS_OFF);
    }
}

REG_TASK(1, fsm_run)

uint8_t RAMFUNC fsm_get_fsm_state_is_fault(void)
{
    return (fsm_state == STATE_FAULT) ? 1 : 0;
}
