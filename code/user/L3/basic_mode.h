#ifndef __BASIC_MODE_H
#define __BASIC_MODE_H

#include "stdint.h"
#include "get_com_data.h"

// 基础模式
#define BASIC_BAT_SYS_12V_MAX_OUT_VOLT         60.00f
#define BASIC_BAT_SYS_12V_MIN_OUT_VOLT         10.00f
#define BASIC_BAT_SYS_12V_DEFAULT_OUT_VOLT     12.00f
#define BASIC_BAT_SYS_12V_MAX_CHURR_VOLT       85.00f
#define BASIC_BAT_SYS_12V_MIN_CHURR_VOLT       1.00f
#define BASIC_BAT_SYS_12V_DEFAULT_CHURR_VOLT   80.00f
#define BASIC_BAT_SYS_12V_MAX_POWER_VOLT       5000
#define BASIC_BAT_SYS_12V_MIN_POWER_VOLT       50
#define BASIC_BAT_SYS_12V_DEFAULT_POWER_VOLT   1200
#define BASIC_BAT_SYS_12V_OPEN_VOLT_A          13.60f
#define BASIC_BAT_SYS_12V_CLOSE_VOLT_A         13.00f
#define BASIC_BAT_SYS_12V_VEER_VOLT_A          12.00f
#define BASIC_BAT_SYS_12V_OPEN_VOLT_B          12.50f
#define BASIC_BAT_SYS_12V_CLOSE_VOLT_B         12.00f
#define BASIC_BAT_SYS_12V_SET_LED_CHAR_CURR    (BASIC_BAT_SYS_12V_DEFAULT_CHURR_VOLT*0.10f)
#define BASIC_BAT_SYS_12V_SET_LED_FULL_CURR    (BASIC_BAT_SYS_12V_SET_LED_CHAR_CURR - 1.00f)
#define BASIC_BAT_SYS_SET_UVLO                  10.00f
#define BASIC_BAT_SYS_SET_UVLORECOVER           11.00f
#define BASIC_BAT_SYS_SET_OVP                   62.00f
#define BASIC_BAT_SYS_SET_OVPRECOVER            61.00f

typedef enum
{
    eBASIC_SYS_12V,
    eBASIC_SYS_16V,
    eBASIC_SYS_24V,
    eBASIC_SYS_36V,
    eBASIC_SYS_48V,
    eBASIC_SYS_VOLT_MAX,
} BASIC_SYS_VOLT_STATE_E;

typedef struct
{
    float OutVoltMax;            // 输出最大电压
    float OutVoltMin;            // 输出最小电压
    float OutVoltDefault;        // 输出电压默认值
    float OutCurrMax;            // 输出最大电流
    float OutCurrMin;            // 输出最小电流
    float OutCurrDefault;        // 输出电流默认值
    float OpenVoltA;             // 自动模式下正向开电压
    float CloseVoltA;            // 自动模式下正向关闭电压
    float VeerVoltA;             // 自动模式下正向转向电压
    float OpenVoltB;             // 自动模式下反向开电压
    float CloseVoltB;            // 自动模式下反向关闭电压
    float SetChargLedCurr;
    float SetFullLedCurr;
    uint16_t OutPowerMax;        // 输出最大功率
    uint16_t OutPowerMin;        // 输出最小功率
    uint16_t OutPowerDefault;    // 输出功率默认值

} BASIC_MODE_CONFIG_T;
void init_basic_mode_parameter(void);
void basic_mode_run(charge_state_data_t *basic_charge_data);
#endif

