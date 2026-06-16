#ifndef __STANDARD_MODE_H
#define __STANDARD_MODE_H

#include "stdint.h"
#include "get_com_data.h"

// 标准模式
#define STANDARD_BAT_SYS_12V_MAX_OUT_VOLT         15.00f
#define STANDARD_BAT_SYS_12V_MIN_OUT_VOLT         10.00f
#define STANDARD_BAT_SYS_12V_DEFAULT_OUT_VOLT     12.00f
#define STANDARD_BAT_SYS_12V_MAX_CHURR_VOLT       105.00f
#define STANDARD_BAT_SYS_12V_MIN_CHURR_VOLT       1.00f
#define STANDARD_BAT_SYS_12V_DEFAULT_CHURR_VOLT   102.00f
#define STANDARD_BAT_SYS_12V_MAX_POWER_VOLT       1500
#define STANDARD_BAT_SYS_12V_MIN_POWER_VOLT       10
#define STANDARD_BAT_SYS_12V_DEFAULT_POWER_VOLT   1500
#define STANDARD_BAT_SYS_12V_OPEN_VOLT_A          13.60f
#define STANDARD_BAT_SYS_12V_CLOSE_VOLT_A         13.00f
#define STANDARD_BAT_SYS_12V_VEER_VOLT_A          12.00f
#define STANDARD_BAT_SYS_12V_OPEN_VOLT_B          12.50f
#define STANDARD_BAT_SYS_12V_CLOSE_VOLT_B         12.00f
#define STANDARD_BAT_SYS_12V_SET_LED_CHAR_CURR    (STANDARD_BAT_SYS_12V_DEFAULT_CHURR_VOLT*0.95f)
#define STANDARD_BAT_SYS_12V_SET_LED_FULL_CURR    (STANDARD_BAT_SYS_12V_SET_LED_CHAR_CURR - 1.00f)


#define STANDARD_BAT_SYS_16V_MAX_OUT_VOLT         20.00f
#define STANDARD_BAT_SYS_16V_MIN_OUT_VOLT         15.00f
#define STANDARD_BAT_SYS_16V_DEFAULT_OUT_VOLT     16.00f
#define STANDARD_BAT_SYS_16V_MAX_CHURR_VOLT       95.00f
#define STANDARD_BAT_SYS_16V_MIN_CHURR_VOLT       1.00f
#define STANDARD_BAT_SYS_16V_DEFAULT_CHURR_VOLT   92.00f
#define STANDARD_BAT_SYS_16V_MAX_POWER_VOLT       1500
#define STANDARD_BAT_SYS_16V_MIN_POWER_VOLT       10
#define STANDARD_BAT_SYS_16V_DEFAULT_POWER_VOLT   1500
#define STANDARD_BAT_SYS_16V_OPEN_VOLT_A          17.00f
#define STANDARD_BAT_SYS_16V_CLOSE_VOLT_A         16.50f
#define STANDARD_BAT_SYS_16V_VEER_VOLT_A          16.00f
#define STANDARD_BAT_SYS_16V_OPEN_VOLT_B          16.00f
#define STANDARD_BAT_SYS_16V_CLOSE_VOLT_B         15.00f
#define STANDARD_BAT_SYS_16V_SET_LED_CHAR_CURR    (STANDARD_BAT_SYS_16V_DEFAULT_CHURR_VOLT*0.95f)
#define STANDARD_BAT_SYS_16V_SET_LED_FULL_CURR    (STANDARD_BAT_SYS_16V_DEFAULT_CHURR_VOLT - 1.00f)


#define STANDARD_BAT_SYS_24V_MAX_OUT_VOLT         30.00f
#define STANDARD_BAT_SYS_24V_MIN_OUT_VOLT         20.00f
#define STANDARD_BAT_SYS_24V_DEFAULT_OUT_VOLT     24.00f
#define STANDARD_BAT_SYS_24V_MAX_CHURR_VOLT       65.00f
#define STANDARD_BAT_SYS_24V_MIN_CHURR_VOLT       1.00f
#define STANDARD_BAT_SYS_24V_DEFAULT_CHURR_VOLT   62.00f
#define STANDARD_BAT_SYS_24V_MAX_POWER_VOLT       1500
#define STANDARD_BAT_SYS_24V_MIN_POWER_VOLT       10
#define STANDARD_BAT_SYS_24V_DEFAULT_POWER_VOLT   1500
#define STANDARD_BAT_SYS_24V_OPEN_VOLT_A          27.00f
#define STANDARD_BAT_SYS_24V_CLOSE_VOLT_A         26.00f
#define STANDARD_BAT_SYS_24V_VEER_VOLT_A          25.00f
#define STANDARD_BAT_SYS_24V_OPEN_VOLT_B          25.00f
#define STANDARD_BAT_SYS_24V_CLOSE_VOLT_B         24.00f
#define STANDARD_BAT_SYS_24V_SET_LED_CHAR_CURR    (STANDARD_BAT_SYS_24V_DEFAULT_CHURR_VOLT*0.95f)
#define STANDARD_BAT_SYS_24V_SET_LED_FULL_CURR    (STANDARD_BAT_SYS_24V_DEFAULT_CHURR_VOLT - 1.00f)

#define STANDARD_BAT_SYS_36V_MAX_OUT_VOLT         45.00f
#define STANDARD_BAT_SYS_36V_MIN_OUT_VOLT         30.00f
#define STANDARD_BAT_SYS_36V_DEFAULT_OUT_VOLT     36.00f
#define STANDARD_BAT_SYS_36V_MAX_CHURR_VOLT       45.00f
#define STANDARD_BAT_SYS_36V_MIN_CHURR_VOLT       1.00f
#define STANDARD_BAT_SYS_36V_DEFAULT_CHURR_VOLT   42.00f
#define STANDARD_BAT_SYS_36V_MAX_POWER_VOLT       1500
#define STANDARD_BAT_SYS_36V_MIN_POWER_VOLT       10
#define STANDARD_BAT_SYS_36V_DEFAULT_POWER_VOLT   1500
#define STANDARD_BAT_SYS_36V_OPEN_VOLT_A          40.00f
#define STANDARD_BAT_SYS_36V_CLOSE_VOLT_A         39.00f
#define STANDARD_BAT_SYS_36V_VEER_VOLT_A          38.00f
#define STANDARD_BAT_SYS_36V_OPEN_VOLT_B          37.00f
#define STANDARD_BAT_SYS_36V_CLOSE_VOLT_B         36.00f
#define STANDARD_BAT_SYS_36V_SET_LED_CHAR_CURR    (STANDARD_BAT_SYS_36V_DEFAULT_CHURR_VOLT*0.95f)
#define STANDARD_BAT_SYS_36V_SET_LED_FULL_CURR    (STANDARD_BAT_SYS_36V_DEFAULT_CHURR_VOLT - 1.00f)

#define STANDARD_BAT_SYS_48V_MAX_OUT_VOLT         60.00f
#define STANDARD_BAT_SYS_48V_MIN_OUT_VOLT         45.00f
#define STANDARD_BAT_SYS_48V_DEFAULT_OUT_VOLT     48.00f
#define STANDARD_BAT_SYS_48V_MAX_CHURR_VOLT       35.00f
#define STANDARD_BAT_SYS_48V_MIN_CHURR_VOLT       1.00f
#define STANDARD_BAT_SYS_48V_DEFAULT_CHURR_VOLT   32.00f
#define STANDARD_BAT_SYS_48V_MAX_POWER_VOLT       1500
#define STANDARD_BAT_SYS_48V_MIN_POWER_VOLT       10
#define STANDARD_BAT_SYS_48V_DEFAULT_POWER_VOLT   1500
#define STANDARD_BAT_SYS_48V_OPEN_VOLT_A          54.40f
#define STANDARD_BAT_SYS_48V_CLOSE_VOLT_A         52.00f
#define STANDARD_BAT_SYS_48V_VEER_VOLT_A          48.00f
#define STANDARD_BAT_SYS_48V_OPEN_VOLT_B          50.00f
#define STANDARD_BAT_SYS_48V_CLOSE_VOLT_B         48.00f
#define STANDARD_BAT_SYS_48V_SET_LED_CHAR_CURR    (STANDARD_BAT_SYS_48V_DEFAULT_CHURR_VOLT*0.95f)
#define STANDARD_BAT_SYS_48V_SET_LED_FULL_CURR    (STANDARD_BAT_SYS_48V_DEFAULT_CHURR_VOLT - 1.00f)

#define STANDARD_BAT_SYS_SET_UVLO                 10.00f
#define STANDARD_BAT_SYS_SET_UVLORECOVER          11.00f
#define STANDARD_BAT_SYS_SET_OVP                  62.00f
#define STANDARD_BAT_SYS_SET_OVPRECOVER           61.00f


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

} STANDARD_MODE_CONFIG_T;

typedef struct
{
    float InpVoltLmt;
    float OutVoltLmt;
    uint16_t PowerMode;
    uint16_t BatTypeA;
    uint16_t BatTypeB;
    uint16_t BootTime;
    uint16_t CurrStartTime;
    uint16_t Power;
    uint16_t ChargeState;
    uint16_t SetCharState;
    uint16_t OutPower;
    uint16_t SetOutPower;
    uint16_t SetInpPower;
    uint16_t OutBatType;
    uint16_t InpBatType;
    float PrechargeVolt;
    float ChargeVolt;
    float OutVolt;
    float OutCurr;
    float SetOutVolt;
    float SetOutCurr;
    float SetInpVolt;
    float SetInpCurr;
    float FullCurr;
    float ChargCurr;
    float ActualOutVolt;
    float ActualOutCurr;
} standard_charge_status_t;

typedef enum
{
    eSTANDARD_SYS_12V,
    eSTANDARD_SYS_16V,
    eSTANDARD_SYS_24V,
    eSTANDARD_SYS_36V,
    eSTANDARD_SYS_48V,
    eSTANDARD_SYS_VOLT_MAX,
} STANDARD_SYS_VOLT_STATE_E;

void standard_mode_run(charge_state_data_t *standard_charge_data);
void init_standard_mode_parameter(void);
#endif

