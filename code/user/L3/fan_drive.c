#include "fan_drive.h"
#include "section.h"
#include "get_com_data.h"
#include "hc32_ll_tmr6.h"
#include "bsp_pwm.h"

#define FAN_FULL_POWER_TEMP       100.00f
#define FAN_START_TEMP            60.00f
#define FAN_STOP_HYSTERESIS_TEMP  3.00f
#define FAN_START_DUTY            0.35f
#define FAN_FULL_DUTY             1.00f
#define FAN_POWER_ON_CHECK_TICKS       50U
#define FAN_POWER_ON_CHECK_HALF_TICKS  25U
#define FAN_POWER_ON_CHECK_MIN_DUTY    0.25f
#define FAN_POWER_ON_CHECK_MAX_DUTY    0.55f

static uint8_t fan_running_flag = 0;

uint8_t fan_drive_is_running(void)
{
    return fan_running_flag;
}

static float fan_drive_get_power_on_check_duty(uint16_t tick)
{
    float duty = FAN_POWER_ON_CHECK_MIN_DUTY;
    float ratio = 0.0f;

    if(tick < FAN_POWER_ON_CHECK_HALF_TICKS)
    {
        ratio = (float)tick / (float)(FAN_POWER_ON_CHECK_HALF_TICKS - 1U);
        duty = FAN_POWER_ON_CHECK_MIN_DUTY +
               ((FAN_POWER_ON_CHECK_MAX_DUTY - FAN_POWER_ON_CHECK_MIN_DUTY) * ratio);
    }else{
        ratio = (float)(tick - FAN_POWER_ON_CHECK_HALF_TICKS) /
                (float)(FAN_POWER_ON_CHECK_HALF_TICKS - 1U);
        duty = FAN_POWER_ON_CHECK_MAX_DUTY -
               ((FAN_POWER_ON_CHECK_MAX_DUTY - FAN_POWER_ON_CHECK_MIN_DUTY) * ratio);
    }

    return duty;
}

void fan_drive_run(void)
{
    static int16_t Temp = 0;
    static uint8_t fan_onoff_flag = 0;
    static uint16_t fan_delay = 0;
    static float duty = 0;
    float fan_duty = 0;
    static uint8_t fan_flag = 0;
    static uint16_t power_on_check_tick = 0U;
    static uint8_t power_on_check_finished = 0U;
    
    if(power_on_check_finished != 0U)
    {
        duty = 0.0f;
        fan_delay = 0U;
        fan_flag = 0U;
        fan_running_flag = 0U;
        power_on_check_finished = 0U;
        bsp_pwm_set_tmr6_fan(duty);
    }

    if(power_on_check_tick < FAN_POWER_ON_CHECK_TICKS)
    {
        duty = fan_drive_get_power_on_check_duty(power_on_check_tick);
        power_on_check_tick++;
        if(power_on_check_tick >= FAN_POWER_ON_CHECK_TICKS)
        {
            power_on_check_finished = 1U;
        }
        fan_running_flag = 1U;
        bsp_pwm_set_tmr6_fan(duty);
        return;
    }
    
     Temp  = get_wg_com_v2_data.com_realtime_data.InsideTemp;
    (Temp <= get_wg_com_v2_data.com_realtime_data.OutsideTemp) ? (Temp = get_wg_com_v2_data.com_realtime_data.OutsideTemp) : (Temp);
    (Temp <= get_wg_com_v2_data.com_realtime_data.Temp2)       ? (Temp = get_wg_com_v2_data.com_realtime_data.Temp2)       : (Temp);
    
    if(Temp <= (FAN_START_TEMP - FAN_STOP_HYSTERESIS_TEMP))
    {
        fan_onoff_flag = 0;
    }
    
    if(Temp >= FAN_START_TEMP){
        fan_onoff_flag = 1;
    }

    if(fan_onoff_flag == 1)
    {
        if(Temp >= FAN_FULL_POWER_TEMP){
            fan_duty = FAN_FULL_DUTY;
            // 100%
        }else if(Temp < FAN_START_TEMP){
            fan_duty = FAN_START_DUTY;
            // 35%
        }else{
            fan_duty = FAN_START_DUTY +
                       (((float)Temp - FAN_START_TEMP) *
                        (FAN_FULL_DUTY - FAN_START_DUTY) /
                        (FAN_FULL_POWER_TEMP - FAN_START_TEMP));
        }
    }else{
        // 0%
        fan_duty = 0;
    }
    if(++fan_delay >= 10)
    {
        fan_delay = 0;
        fan_flag = 1;
    }

    if(fan_flag == 1)
    {
        RAMP(duty,fan_duty,0.016f);
        fan_running_flag = (duty > 0.01f) ? 1U : 0U;
        bsp_pwm_set_tmr6_fan(duty);
    }
}

REG_TASK(100, fan_drive_run)


