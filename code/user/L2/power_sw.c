#include "power_sw.h"
#include "gpio.h"
#include "wg_com_v2.h"
#include "section.h"
#include "bat_charge_pattern.h"
static uint8_t power_is_on = 0;

void power_sw_run(void)
{
    uint8_t soft_is_off = 0;
    uint8_t soft_is_on = 0;
//    uint8_t hard_is_on = 0;
    static uint8_t soft_is_on_last = 0;
//    static uint8_t hard_is_on_last = 0;

    // 获取当前状态
    WG_COM_V2_GET_DATA_UINT(soft_is_off, wg_com_v2_ctrl.PowerOnOff);
    if ((gpio_get_pg_en() == 0) &&
        (soft_is_off == 0))
    {
        // 硬件开关关闭时，软件开关被强制关闭
       // soft_is_off = 1;
       // WG_COM_V2_SET_DATA(soft_is_off, wg_com_v2_ctrl.PowerOnOff);
    }
    soft_is_on = soft_is_off ? 0 : 1;
//    hard_is_on = gpio_get_pg_en() ? 0 : 1;

    // 检测边沿
    uint8_t soft_falling = (soft_is_on_last == 1) && (soft_is_on == 0);
    uint8_t soft_rising = (soft_is_on_last == 0) && (soft_is_on == 1);
//    uint8_t hard_falling = (hard_is_on_last == 0) && (hard_is_on == 1);
//    uint8_t hard_rising = (hard_is_on_last == 1) && (hard_is_on == 0);

    // 优先级处理：先检查下降沿
    if (soft_falling/* ||
        hard_falling*/)
    {
        power_is_on = 0; // 下降沿优先，强制关机
    }
    else if (/*(*/soft_rising/* ||
              hard_rising) */&&
             (power_is_on == 0))
    {
        power_is_on = 1; // 只有在当前关机状态才允许开机
    }

    // 更新输出
//    uint8_t power_is_off = power_is_on ? 0 : 1;
//    WG_COM_V2_SET_DATA(power_is_off, wg_com_v2_ctrl.PowerOnOff);

    // 更新历史状态
    soft_is_on_last = soft_is_on;
//    hard_is_on_last = hard_is_on;
}
REG_TASK(1, power_sw_run);

uint8_t power_sw_get_power_is_on(void)
{
    return power_is_on;
}


