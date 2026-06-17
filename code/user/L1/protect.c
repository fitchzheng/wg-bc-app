#include "adc.h"
#include "fault.h"
#include "protect.h"
#include "section.h"
#include "ctrl_app.h"
#include <math.h>
#include "shell.h"
#include "my_math.h"
#include "wg_com_v2.h"
#include "stdbool.h"
#include "adc_check.h"
#include "fsm.h"
#include "gpio.h"
#include "ctrl_app.h"
#include "get_com_data.h"
#include "bat_charge_pattern.h"
// 使用静态变量限制作用域并提高访问速度
static float ila_val = 0.0f;
static float ilb_val = 0.0f;
static uint32_t il_ocp_cnt = 0;

#if (APP_DEBUG_FEATURES == 1)
static float ila_val_obs = 0.0f;
static float ilb_val_obs = 0.0f;

REG_SHELL_VAR(ila_val_obs, ila_val_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ilb_val_obs, ilb_val_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
#endif

extern void gpio_set_auxoff(uint8_t val);
void protect_fast(void)
{
    // 1. 快速获取ADC值 - 直接使用寄存器访问方式如果可能
    ila_val = adc_get_ila();
    ilb_val = adc_get_ilb();

    // 2. 使用整数比较和位操作加速条件判断
    if ((fabsf(ila_val) > IL_OCP_THRESHOLD) |
        (fabsf(ilb_val) > IL_OCP_THRESHOLD))
    {
        // 3. 使用饱和计数器防止溢出
        if (il_ocp_cnt < IL_OCP_TIME)
        {
            il_ocp_cnt++;
        }

        // 4. 使用提前判断减少分支预测失败
        if (il_ocp_cnt >= IL_OCP_TIME)
        {
            il_ocp_cnt = 0;
            fault_set_fault(FAULT_OCP);
            ((ctrl_app_get_ctrl_mode() == CTRL_BACKWARD) ? fault_set_fault(FAULT_FVS48_SCP) : fault_set_fault(FAULT_RVS12_SCP));
            ctrl_app_disable();    // 禁用控制器
//            ila_val_obs = ila_val; // 记录异常值
//            ilb_val_obs = ilb_val; // 记录异常值
            return;                // 提前返回避免后续判断
        }
    }
    else
    {
        il_ocp_cnt = 0; // 重置计数器
    }

    const float fvs48_val = get_show_fvs48_show();
    const float rvs12_val = get_show_rvs12_show();

    const float vin_val = (ctrl_app_get_ctrl_mode() == CTRL_BACKWARD) ? rvs12_val : fvs48_val;

    static uint32_t uvp_cnt = 0;

    if ((vin_val < FAST_UVP_THRESHOLD) &&
        (ctrl_app_get_is_run()))
    {
        uvp_cnt++;
        if (uvp_cnt >= FAST_UVP_TIME)
        {
            uvp_cnt = FAST_UVP_TIME; // 防止溢出
            fault_set_fault(FAULT_FAST_UVP);
            ctrl_app_disable(); // 禁用控制器
        }
    }
    else
    {
        uvp_cnt = 0;
        if (fsm_get_fsm_state_is_fault())
        {
            fault_clear_fault(FAULT_FAST_UVP);
        }
    }
}

// 确保中断处理函数在快速内存区域
REG_INTERRUPT(0, protect_fast);

bool cmp_less(float a, float b) { return a < b; }    // 用于欠压
bool cmp_greater(float a, float b) { return a > b; } // 用于过压

void update_protect_item(protect_item_t *item)
{
    if (item->enable == 0)
    {
        if (item->counter)
        {
            item->counter--;
        }
        else
        {
            if (++item->counter_r >= item->RecoverTime)
            {
                item->flag = false;
                fault_clear_fault(item->fault);
            }
        }
        return;
    }

    if ((!item->flag)&&(!fault_get_fault_bit(item->fault)))
    {

        if (item->cmp_trigger(item->val, item->limit))
        {
            if (++item->counter >= item->TriggerTime)
            {
                item->flag = true;
                fault_set_fault(item->fault);
                item->counter_r = 0;
            }
        }
        else
        {
            item->counter = 0;
        }
    }
    else // 正在保护中
    {
        if (item->cmp_recover(item->val, item->recover))
        {
            if (++item->counter_r >= item->RecoverTime)
            {
                item->flag = false;
                item->counter = 0;
                fault_clear_fault(item->fault);
            }
        }
    }
}

void update_warning_item(warning_item_t *item)
{
    if (item->enable == 0)
    {
        if (item->counter)
        {
            item->counter--;
        }
        else
        {
            if (++item->counter_r >= item->RecoverTime)
            {
                item->flag = false;
                fault_clear_alarm(item->alarm);
            }
        }
        return;
    }

    if ((!item->flag)&&(!fault_get_alarm_bit(item->alarm)))
    {
        if (item->cmp_trigger(item->val, item->limit))
        {
            if (++item->counter >= item->TriggerTime)
            {
                item->flag = true;
                fault_set_alarm(item->alarm);
                item->counter = 0;
            }
        }
        else
        {
            item->counter = 0;
        }
    }
    else // 正在保护中
    {
        if (item->cmp_recover(item->val, item->recover))
        {
            if (++item->counter_r >= item->RecoverTime)
            {
                item->flag = false;
                item->counter_r = 0;
                fault_clear_alarm(item->alarm);
            }
        }
    }
}

enum
{
    PROTECT_FVS48_SCP,
    PROTECT_FVS48_UVP,
    PROTECT_FVS48_OVP,
    PROTECT_ACC_UVP,
    PROTECT_FVS12_SCP,
    PROTECT_RVS12_UVP,
    PROTECT_RVS12_OVP,
    PROTECT_RTM_UVP,
//    PROTECT_INSIDE_OTP,
//    PROTECT_OUTSIDE_OTP,
//    PROTECT_TEMP2_OTP,
    PROTECT_MAX,
};

static protect_item_t protections[PROTECT_MAX] = {
//    [PROTECT_FVS48_SCP] = {.fault = FAULT_FVS48_SCP, .cmp_trigger = cmp_less, .cmp_recover = cmp_greater, .enable = 0,.TriggerTime = PROTECT_SCP_TRIGGER_DELAY_COUNT,PROTECT_SCP_TRIGGER_R_DELAY_COUNT},
    [PROTECT_FVS48_UVP] = {.fault = FAULT_FVS48_UVP, .cmp_trigger = cmp_less, .cmp_recover = cmp_greater, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [PROTECT_FVS48_OVP] = {.fault = FAULT_FVS48_OVP, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [PROTECT_ACC_UVP  ] = {.fault = FAULT_ACC_UVP, .cmp_trigger = cmp_less, .cmp_recover = cmp_greater, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
//    [PROTECT_FVS12_SCP] = {.fault = FAULT_RVS12_SCP, .cmp_trigger = cmp_less, .cmp_recover = cmp_greater, .enable = 0,.TriggerTime = PROTECT_SCP_TRIGGER_DELAY_COUNT,PROTECT_SCP_TRIGGER_R_DELAY_COUNT},
    [PROTECT_RVS12_UVP] = {.fault = FAULT_RVS12_UVP, .cmp_trigger = cmp_less, .cmp_recover = cmp_greater, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [PROTECT_RVS12_OVP] = {.fault = FAULT_RVS12_OVP, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [PROTECT_RTM_UVP  ] = {.fault = FAULT_RTM_UVP, .cmp_trigger = cmp_less, .cmp_recover = cmp_greater, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
//    [PROTECT_INSIDE_OTP] = {.fault = FAULT_INSIDE_OTP, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 1,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
//    [PROTECT_OUTSIDE_OTP] = {.fault = FAULT_OUTSIDE_OTP, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 1,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
//    [PROTECT_TEMP2_OTP] = {.fault = FAULT_TEMP2_OTP, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 1,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
};


enum
{
    WARNING_A_REDUCE,
    WARNING_B_REDUCE,
    WARNING_A_INSIDE_REDUCE,
    WARNING_B_INSIDE_REDUCE,
    WARNING_A_OUTSIDE_REDUCE,
    WARNING_B_OUTSIDE_REDUCE,
    WARNING_INSIDE_OTP,
    WARNING_OUTSIDE_OTP,
    WARNING_TEMP2_OTP,
    WARNING_MAX,
};

static warning_item_t warningions[WARNING_MAX] = {
    [WARNING_A_REDUCE]         = {.alarm = ALARM_A_REDUCE_PWR, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [WARNING_B_REDUCE]         = {.alarm = ALARM_B_REDUCE_PWR, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [WARNING_A_INSIDE_REDUCE]  = {.alarm = ALARM_A_INSIDE_REDUCE_PWR, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [WARNING_B_INSIDE_REDUCE]  = {.alarm = ALARM_B_INSIDE_REDUCE_PWR, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [WARNING_A_OUTSIDE_REDUCE] = {.alarm = ALARM_A_OUTSIDE_REDUCE_PWR, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [WARNING_B_OUTSIDE_REDUCE] = {.alarm = ALARM_B_OUTSIDE_REDUCE_PWR, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [WARNING_INSIDE_OTP]       = {.alarm = ALARM_INSIDE_OTP, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [WARNING_OUTSIDE_OTP]      = {.alarm = ALARM_OUTSIDE_OTP, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
    [WARNING_TEMP2_OTP]        = {.alarm = ALARM_TEMP2_OTP, .cmp_trigger = cmp_greater, .cmp_recover = cmp_less, .enable = 0,.TriggerTime = PROTECT_TRIGGER_DELAY_COUNT,PROTECT_TRIGGER_R_DELAY_COUNT},
};
float rvs12_volt = 0.0f;
float rvs12_volt_r = 0.0f;
float fvs48_volt = 0.0f;
float fvs48_volt_r = 0.0f;
#if (SHELL_ON_OFF == 1)
REG_SHELL_VAR(rvs12_volt, rvs12_volt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(rvs12_volt_r, rvs12_volt_r, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(fvs48_volt, fvs48_volt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(fvs48_volt_r, fvs48_volt_r, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
#endif
static void protect_volt(void)
{

//    float fvs48_val = get_show_fvs48_show(); // 输入电压
//    float rvs12_val = get_show_rvs12_show(); // 输出电压
//    float vdd_8v_val = adc_get_vdd_8v();// vdd电压
    float Acc_val = adc_get_accvs();
    float Rtm_val = adc_get_rmtvs();

    // 从参数中获取阈值
//    float InpUvlo = 0.00f, InpUvloRecover = 0.00f, InpOVP = 0.00f, InpOVPRecover = 0.00f;
//    float OutUvlo = 0.00f, OutUvloRecover = 0.00f, OutOVP = 0.00f, OutOVPRecover = 0.00f;
//    float AuotVeerVolt = 0.00f;
//    float AuotOpenVoltA = 0.00f;
//    float AuotCloseVoltA = 0.00f;
//    float AuotOpenVoltB = 0.00f;
//    float AuotCloseVoltB = 0.00f;
//    float SetVolt = 0.00f;
//    float SetInpt = 0.00f;

//    SetVolt = get_wg_com_v2_data.com_param.SetOutVolt;
//    SetInpt = get_wg_com_v2_data.com_param.SetInpVolt;
//    InpUvlo = get_wg_com_v2_data.com_param.SetInpUvlo;
//    InpUvloRecover = get_wg_com_v2_data.com_param.SetInpUvloRecover;
//    InpOVP = get_wg_com_v2_data.com_param.SetInpOVP;
//    InpOVPRecover = get_wg_com_v2_data.com_param.SetInpOVPRecover;
//    OutUvlo = get_wg_com_v2_data.com_param.SetOutUvlo;
//    OutUvloRecover = get_wg_com_v2_data.com_param.SetOutUvloRecover;
//    OutOVP = get_wg_com_v2_data.com_param.SetOutOVP;
//    OutOVPRecover = get_wg_com_v2_data.com_param.SetOutOVPRecover;

//    AuotVeerVolt = get_wg_com_v2_data.com_param.AuotForwardVeerVoltA;
//    AuotOpenVoltA = get_wg_com_v2_data.com_param.AuotForwardOpenVoltA;
//    AuotCloseVoltA = get_wg_com_v2_data.com_param.AuotForwardShutVoltA;
//    AuotOpenVoltB = get_wg_com_v2_data.com_param.AuotReverseOpenVoltB;
//    AuotCloseVoltB = get_wg_com_v2_data.com_param.AuotReverseShutVoltB;

//    protections[PROTECT_FVS12_SCP].val = rvs12_val;
//    protections[PROTECT_FVS12_SCP].limit = 3.0f;
//    protections[PROTECT_FVS12_SCP].recover = 5.0f;
//    protections[PROTECT_FVS12_SCP].enable = (((get_check_state_data() == ADDRS_FORWARD)&&(Get_Soft_Finish_Flag() == 1)) ? 1 : 0);
//    
//    protections[PROTECT_FVS48_SCP].val = fvs48_val;
//    protections[PROTECT_FVS48_SCP].limit = 3.0f;
//    protections[PROTECT_FVS48_SCP].recover = 5.0f;
//    protections[PROTECT_FVS48_SCP].enable = (((get_check_state_data() == ADDRS_BACKWARD)&&(Get_Soft_Finish_Flag() == 1)) ? 1 : 0);

    protections[PROTECT_ACC_UVP  ].val = charge_state_data.protect_data.protect_item_acc.val;
    protections[PROTECT_ACC_UVP  ].limit = charge_state_data.protect_data.protect_item_acc.limit;
    protections[PROTECT_ACC_UVP  ].recover = charge_state_data.protect_data.protect_item_acc.recover;
    protections[PROTECT_ACC_UVP  ].enable =  charge_state_data.protect_data.protect_item_acc.enable;

    protections[PROTECT_RTM_UVP  ].val = charge_state_data.protect_data.protect_item_rtm.val;
    protections[PROTECT_RTM_UVP  ].limit = charge_state_data.protect_data.protect_item_rtm.limit;
    protections[PROTECT_RTM_UVP  ].recover = charge_state_data.protect_data.protect_item_rtm.recover;
    protections[PROTECT_RTM_UVP  ].enable = charge_state_data.protect_data.protect_item_rtm.enable;

    protections[PROTECT_FVS48_UVP].val = charge_state_data.protect_data.protect_item_fvs48_uvp.val;
    protections[PROTECT_FVS48_UVP].limit = charge_state_data.protect_data.protect_item_fvs48_uvp.limit;
    protections[PROTECT_FVS48_UVP].recover = charge_state_data.protect_data.protect_item_fvs48_uvp.recover;
    protections[PROTECT_FVS48_UVP].enable = charge_state_data.protect_data.protect_item_fvs48_uvp.enable;

    protections[PROTECT_FVS48_OVP].val = charge_state_data.protect_data.protect_item_fvs48_ovp.val;
    protections[PROTECT_FVS48_OVP].limit = charge_state_data.protect_data.protect_item_fvs48_ovp.limit;
    protections[PROTECT_FVS48_OVP].recover = charge_state_data.protect_data.protect_item_fvs48_ovp.recover;
    protections[PROTECT_FVS48_OVP].enable = charge_state_data.protect_data.protect_item_fvs48_ovp.enable;

    protections[PROTECT_RVS12_UVP].val = charge_state_data.protect_data.protect_item_rvs12_uvp.val;
    protections[PROTECT_RVS12_UVP].limit = charge_state_data.protect_data.protect_item_rvs12_uvp.limit;
    protections[PROTECT_RVS12_UVP].recover = charge_state_data.protect_data.protect_item_rvs12_uvp.recover;
    protections[PROTECT_RVS12_UVP].enable = charge_state_data.protect_data.protect_item_rvs12_uvp.enable;

    protections[PROTECT_RVS12_OVP].val = charge_state_data.protect_data.protect_item_rvs12_ovp.val;
    protections[PROTECT_RVS12_OVP].limit = charge_state_data.protect_data.protect_item_rvs12_ovp.limit;
    protections[PROTECT_RVS12_OVP].recover = charge_state_data.protect_data.protect_item_rvs12_ovp.recover;
    protections[PROTECT_RVS12_OVP].enable = charge_state_data.protect_data.protect_item_rvs12_ovp.enable;
}

void protect_temp(void)
{
    float inside_temp = adc_check_get_ntc1_temp();
    float outside_temp = adc_check_get_ntc2_temp();
    float Temp2_temp = adc_check_get_ntc3_temp();
    float InsideTemp, OutsideTemp,Temp2;
    float InsideTempRecover, OutsideTempRecover,SetTemp2Recover;
//    WG_COM_V2_GET_DATA_INT(InsideTemp, wg_com_v2_param.SetInsideTemp);
//    WG_COM_V2_GET_DATA_INT(OutsideTemp, wg_com_v2_param.SetOutsideTemp);
//    WG_COM_V2_GET_DATA_INT(Temp2, wg_com_v2_param.SetTemp2);
    InsideTemp = get_wg_com_v2_data.com_param.SetInsideTemp;
    OutsideTemp = get_wg_com_v2_data.com_param.SetOutsideTemp;
    Temp2 = get_wg_com_v2_data.com_param.SetTemp2;
    
    InsideTempRecover = InsideTemp - 10;//InsideTemp * 0.5f;
    OutsideTempRecover = OutsideTemp - 10;//OutsideTemp * 0.5f;
    SetTemp2Recover = Temp2 - 10;//Temp2 * 0.5f;
    
//    protections[PROTECT_INSIDE_OTP].val = inside_temp;
//    protections[PROTECT_INSIDE_OTP].limit = InsideTemp;
//    protections[PROTECT_INSIDE_OTP].recover = InsideTempRecover;
//    protections[PROTECT_INSIDE_OTP].enable = otp_enable;

    warningions[WARNING_A_INSIDE_REDUCE].val = inside_temp;
    warningions[WARNING_A_INSIDE_REDUCE].limit = InsideTempRecover;
    warningions[WARNING_A_INSIDE_REDUCE].recover = InsideTempRecover-3;
    warningions[WARNING_A_INSIDE_REDUCE].enable = ((get_check_state_data() == ADDRS_BACKWARD) ? 1 : 0);

    warningions[WARNING_B_INSIDE_REDUCE].val = inside_temp;
    warningions[WARNING_B_INSIDE_REDUCE].limit = InsideTempRecover;
    warningions[WARNING_B_INSIDE_REDUCE].recover = InsideTempRecover-3;
    warningions[WARNING_B_INSIDE_REDUCE].enable = ((get_check_state_data() == ADDRS_FORWARD) ? 1 : 0);

//    protections[PROTECT_OUTSIDE_OTP].val = outside_temp;
//    protections[PROTECT_OUTSIDE_OTP].limit = OutsideTemp;
//    protections[PROTECT_OUTSIDE_OTP].recover = OutsideTempRecover;
//    protections[PROTECT_OUTSIDE_OTP].enable = otp_enable;

    warningions[WARNING_A_OUTSIDE_REDUCE].val = outside_temp;
    warningions[WARNING_A_OUTSIDE_REDUCE].limit = OutsideTempRecover;
    warningions[WARNING_A_OUTSIDE_REDUCE].recover = OutsideTempRecover-3;
    warningions[WARNING_A_OUTSIDE_REDUCE].enable = ((get_check_state_data() == ADDRS_BACKWARD) ? 1 : 0);

    warningions[WARNING_B_OUTSIDE_REDUCE].val = outside_temp;
    warningions[WARNING_B_OUTSIDE_REDUCE].limit = OutsideTempRecover;
    warningions[WARNING_B_OUTSIDE_REDUCE].recover = OutsideTempRecover-3;
    warningions[WARNING_B_OUTSIDE_REDUCE].enable = ((get_check_state_data() == ADDRS_FORWARD) ? 1 : 0);

//    protections[PROTECT_TEMP2_OTP].val = Temp2_temp;
//    protections[PROTECT_TEMP2_OTP].limit = Temp2;
//    protections[PROTECT_TEMP2_OTP].recover = SetTemp2Recover;
//    protections[PROTECT_TEMP2_OTP].enable = otp_enable;
    
    warningions[WARNING_A_REDUCE].val = Temp2_temp;
    warningions[WARNING_A_REDUCE].limit = SetTemp2Recover;
    warningions[WARNING_A_REDUCE].recover = SetTemp2Recover-3;
    warningions[WARNING_A_REDUCE].enable = ((get_check_state_data() == ADDRS_BACKWARD) ? 1 : 0);
    
    warningions[WARNING_B_REDUCE].val = Temp2_temp;
    warningions[WARNING_B_REDUCE].limit = SetTemp2Recover;
    warningions[WARNING_B_REDUCE].recover = SetTemp2Recover-3;
    warningions[WARNING_B_REDUCE].enable = ((get_check_state_data() == ADDRS_FORWARD) ? 1 : 0);

    warningions[WARNING_INSIDE_OTP].val = inside_temp;
    warningions[WARNING_INSIDE_OTP].limit = InsideTemp;
    warningions[WARNING_INSIDE_OTP].recover = InsideTempRecover;
    warningions[WARNING_INSIDE_OTP].enable = 1;

    warningions[WARNING_OUTSIDE_OTP].val = outside_temp;
    warningions[WARNING_OUTSIDE_OTP].limit = OutsideTemp;
    warningions[WARNING_OUTSIDE_OTP].recover = OutsideTempRecover;
    warningions[WARNING_OUTSIDE_OTP].enable = 1;

    warningions[WARNING_TEMP2_OTP].val = Temp2_temp;
    warningions[WARNING_TEMP2_OTP].limit = Temp2;
    warningions[WARNING_TEMP2_OTP].recover = SetTemp2Recover;
    warningions[WARNING_TEMP2_OTP].enable = 1;

    if((fault_get_alarm_bit(ALARM_INSIDE_OTP)   == 0)    &&
       (fault_get_alarm_bit(ALARM_OUTSIDE_OTP)  == 0)    &&
       (fault_get_alarm_bit(ALARM_TEMP2_OTP)    == 0))
    {
        charge_state_data.protect_data.over_temp_protect = 0;
        fault_clear_fault(FAULT_INSIDE_OTP);
        fault_clear_fault(FAULT_OUTSIDE_OTP);
        fault_clear_fault(FAULT_TEMP2_OTP);
        fault_clear_fault(FAULT_OTP);
    }else{
        charge_state_data.protect_data.over_temp_protect = 1;
    }

    if((charge_state_data.get_is_run == 0) 
    && (charge_state_data.protect_data.over_temp_protect == 1))
    {
        if(fault_get_alarm_bit(ALARM_INSIDE_OTP))
        {
            fault_set_fault(FAULT_INSIDE_OTP);
        }else{
            fault_clear_fault(FAULT_INSIDE_OTP);
        }
        if(fault_get_alarm_bit(ALARM_OUTSIDE_OTP))
        {
            fault_set_fault(FAULT_OUTSIDE_OTP);
        }else{
            fault_clear_fault(FAULT_OUTSIDE_OTP);
        }
        if(fault_get_alarm_bit(ALARM_TEMP2_OTP))
        {
            fault_set_fault(FAULT_TEMP2_OTP);
        }else{
            fault_clear_fault(FAULT_TEMP2_OTP);
        }
        fault_set_fault(FAULT_OTP);
    }
}

extern uint8_t sleep_report_state_flag;

void protect_slow(void)
{
    protect_volt();
    protect_temp();

    for (int i = 0; i < PROTECT_MAX; i++)
    {
        update_protect_item(&protections[i]);
    }

    for (int i = 0; i < WARNING_MAX; i++)
    {
        update_warning_item(&warningions[i]);
    }

//    if (fault_get_fault_bit(FAULT_INSIDE_OTP) ||
//        fault_get_fault_bit(FAULT_OUTSIDE_OTP)||
//        fault_get_fault_bit(FAULT_TEMP2_OTP))
//    {
//        fault_set_fault(FAULT_OTP);
//    }
//    else
//    {
//        fault_clear_fault(FAULT_OTP);
//    }
    
    uint16_t FaultSign = fault_get_all_fault() & 0xFFFF;
    WG_COM_V2_SET_DATA_UINT(FaultSign, wg_com_v2_realtime_data.FaultSign);

    uint16_t AlarmSign = fault_get_all_alarm() & 0xFFFF;
    AlarmSign |= (fault_get_alarm_bit(ALARM_A_INSIDE_REDUCE_PWR) || fault_get_alarm_bit(ALARM_A_OUTSIDE_REDUCE_PWR));
    AlarmSign |= (uint16_t)((fault_get_alarm_bit(ALARM_B_INSIDE_REDUCE_PWR) || fault_get_alarm_bit(ALARM_B_OUTSIDE_REDUCE_PWR))<<1);
    if(sleep_report_state_flag == 1)
    {
        AlarmSign |= 0x8000U;
    }
    WG_COM_V2_SET_DATA_UINT(AlarmSign, wg_com_v2_realtime_data.AlarmSign);
}

REG_TASK(1, protect_slow)

static uint8_t chag_fault = 0;
void short_circuit_protection(void)
{
    float OutVolt = 0;
    uint8_t out_baty_type = (uint8_t)((charge_state_data.OutBatyType >> 8) & 0xFF);

    if(charge_state_data.check_state == eADDRS_BACKWARD)
    {
        OutVolt = get_wg_com_v2_data.com_realtime_data.InpVolt;
    }else if(charge_state_data.check_state == eADDRS_FORWARD){
        OutVolt = get_wg_com_v2_data.com_realtime_data.OutVolt;
    }

    if((out_baty_type != eSCAP)
    && (out_baty_type != eBAT_DCDC)
    && (charge_state_data.soft_start_flag == 1)
    && (charge_state_data.get_is_run == 1)
    && (OutVolt < charge_state_data.SetOutVolt*0.5f)){
        chag_fault = 1;
    }else{
        chag_fault = 0;
    }
}

uint8_t get_volt_low_fault(void)
{
    return chag_fault;
}

REG_TASK(10, short_circuit_protection)

