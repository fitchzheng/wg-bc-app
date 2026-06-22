#include "fan_drive.h"
#include "section.h"
#include "get_com_data.h"
#include "hc32_ll_tmr6.h"
#include "bsp_pwm.h"

#define MAX_POWER_TEMP    100.00f
#define MIN_POWER_TEMP    50.00f
void fan_drive_run(void)
{
    static int16_t Temp = 0;
    static uint8_t fan_onoff_flag = 0;
    static uint16_t fan_delay = 0;
    static float duty = 0;
    float fan_duty = 0;
    static uint8_t fan_flag = 0;
    
    
     Temp  = get_wg_com_v2_data.com_realtime_data.InsideTemp;
    (Temp <= get_wg_com_v2_data.com_realtime_data.OutsideTemp) ? (Temp = get_wg_com_v2_data.com_realtime_data.OutsideTemp) : (Temp);
    (Temp <= get_wg_com_v2_data.com_realtime_data.Temp2)       ? (Temp = get_wg_com_v2_data.com_realtime_data.Temp2)       : (Temp);
    
    if(Temp < (MIN_POWER_TEMP-5))
    {
        fan_onoff_flag = 0;
    }
    
    if(Temp >= MIN_POWER_TEMP){
        fan_onoff_flag = 1;
    }

    if(fan_onoff_flag == 1)
    {
        if(Temp >= MAX_POWER_TEMP){
            fan_duty = 1;
            // 100%
        }else if(Temp < MIN_POWER_TEMP){
            fan_duty = 0.5f;
            // 50%
        }else{
            fan_duty = (Temp/MAX_POWER_TEMP);
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
        bsp_pwm_set_tmr6_fan(duty);
    }
}

REG_TASK(100, fan_drive_run)


