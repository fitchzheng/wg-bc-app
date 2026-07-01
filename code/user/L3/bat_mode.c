#include "bat_mode.h"
#include "get_com_data.h"
#include "wg_com_v2.h"
#include "soft_start.h"
#include "adc.h"
#include "rvc_message_handler.h"
#include "eeprom_cfg.h"

static uint32_t FloatChargingDelay = 0;
static uint32_t PrechargeDelay = 0;
static uint32_t ChargeCVDelay = 0;
static uint32_t ChargeCCDelay = 0;
static uint32_t ClearChargeFlagDelay = 0;
static uint32_t LiBatFullDelay = 0;
static uint32_t CurrentReached90PercentDelay = 0;
static uint32_t PowerReached90PercentDelay = 0; 


const BAT_MODE_CONFIG_T Bat_Sys_Volt_Config[eBAT_SYS_VOLT_MAX][eBAT_TYPE_MAX] = {
    [eBAT_SYS_12V][eBAT_TYPE_AGM] = {
                    .OutVoltMax      = BAT_SYS_12V_AGM_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_12V_AGM_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_12V_AGM_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_12V_AGM_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_12V_AGM_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_12V_AGM_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_12V_AGM_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_12V_AGM_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_12V_AGM_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_12V_AGM_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_12V_AGM_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_12V_AGM_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_12V_AGM_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_12V_AGM_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_12V_AGM_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_12V_AGM_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_12V_AGM_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_12V_AGM_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_12V_AGM_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_12V_AGM_SET_OVPRECOVER,
    },
    [eBAT_SYS_12V][eBAT_TYPE_GEL] = {
                    .OutVoltMax      = BAT_SYS_12V_AGM_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_12V_AGM_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_12V_AGM_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_12V_AGM_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_12V_AGM_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_12V_AGM_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_12V_AGM_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_12V_AGM_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_12V_AGM_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_12V_AGM_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_12V_AGM_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_12V_AGM_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_12V_AGM_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_12V_AGM_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_12V_AGM_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_12V_AGM_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_12V_AGM_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_12V_AGM_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_12V_AGM_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_12V_AGM_SET_OVPRECOVER,
    },
    [eBAT_SYS_12V][eBAT_TYPE_LFP] = {
                    .OutVoltMax      = BAT_SYS_12V_LFP_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_12V_LFP_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_12V_LFP_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_12V_LFP_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_12V_LFP_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_12V_LFP_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_12V_LFP_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_12V_LFP_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_12V_LFP_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_12V_LFP_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_12V_LFP_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_12V_LFP_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_12V_LFP_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_12V_LFP_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_12V_LFP_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_12V_LFP_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_12V_LFP_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_12V_LFP_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_12V_LFP_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_12V_LFP_SET_OVPRECOVER,
    },
    [eBAT_SYS_12V][eBAT_TYPE_NMC] = {
                    .OutVoltMax      = BAT_SYS_12V_NMC_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_12V_NMC_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_12V_NMC_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_12V_NMC_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_12V_NMC_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_12V_NMC_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_12V_NMC_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_12V_NMC_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_12V_NMC_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_12V_NMC_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_12V_NMC_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_12V_NMC_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_12V_NMC_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_12V_NMC_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_12V_NMC_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_12V_NMC_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_12V_NMC_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_12V_NMC_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_12V_NMC_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_12V_NMC_SET_OVPRECOVER,
    },
    [eBAT_SYS_16V][eBAT_TYPE_AGM] = {
                    .OutVoltMax      = BAT_SYS_16V_AGM_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_16V_AGM_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_16V_AGM_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_16V_AGM_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_16V_AGM_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_16V_AGM_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_16V_AGM_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_16V_AGM_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_16V_AGM_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_16V_AGM_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_16V_AGM_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_16V_AGM_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_16V_AGM_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_16V_AGM_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_16V_AGM_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_16V_AGM_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_16V_AGM_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_16V_AGM_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_16V_AGM_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_16V_AGM_SET_OVPRECOVER,
    },
    [eBAT_SYS_16V][eBAT_TYPE_GEL] = {
                    .OutVoltMax      = BAT_SYS_16V_AGM_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_16V_AGM_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_16V_AGM_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_16V_AGM_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_16V_AGM_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_16V_AGM_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_16V_AGM_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_16V_AGM_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_16V_AGM_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_16V_AGM_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_16V_AGM_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_16V_AGM_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_16V_AGM_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_16V_AGM_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_16V_AGM_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_16V_AGM_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_16V_AGM_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_16V_AGM_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_16V_AGM_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_16V_AGM_SET_OVPRECOVER,
    },
    [eBAT_SYS_16V][eBAT_TYPE_LFP] = {
                    .OutVoltMax      = BAT_SYS_16V_LFP_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_16V_LFP_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_16V_LFP_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_16V_LFP_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_16V_LFP_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_16V_LFP_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_16V_LFP_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_16V_LFP_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_16V_LFP_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_16V_LFP_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_16V_LFP_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_16V_LFP_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_16V_LFP_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_16V_LFP_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_16V_LFP_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_16V_LFP_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_16V_LFP_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_16V_LFP_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_16V_LFP_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_16V_LFP_SET_OVPRECOVER,
    },
    [eBAT_SYS_16V][eBAT_TYPE_NMC] = {
                    .OutVoltMax      = BAT_SYS_16V_NMC_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_16V_NMC_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_16V_NMC_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_16V_NMC_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_16V_NMC_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_16V_NMC_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_16V_NMC_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_16V_NMC_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_16V_NMC_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_16V_NMC_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_16V_NMC_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_16V_NMC_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_16V_NMC_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_16V_NMC_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_16V_NMC_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_16V_NMC_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_16V_NMC_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_16V_NMC_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_16V_NMC_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_16V_NMC_SET_OVPRECOVER,
    },
    [eBAT_SYS_24V][eBAT_TYPE_AGM] = {
                    .OutVoltMax      = BAT_SYS_24V_AGM_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_24V_AGM_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_24V_AGM_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_24V_AGM_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_24V_AGM_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_24V_AGM_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_24V_AGM_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_24V_AGM_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_24V_AGM_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_24V_AGM_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_24V_AGM_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_24V_AGM_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_24V_AGM_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_24V_AGM_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_24V_AGM_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_24V_AGM_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_24V_AGM_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_24V_AGM_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_24V_AGM_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_24V_AGM_SET_OVPRECOVER,
    },
    [eBAT_SYS_24V][eBAT_TYPE_GEL] = {
                    .OutVoltMax      = BAT_SYS_24V_AGM_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_24V_AGM_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_24V_AGM_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_24V_AGM_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_24V_AGM_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_24V_AGM_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_24V_AGM_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_24V_AGM_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_24V_AGM_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_24V_AGM_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_24V_AGM_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_24V_AGM_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_24V_AGM_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_24V_AGM_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_24V_AGM_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_24V_AGM_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_24V_AGM_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_24V_AGM_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_24V_AGM_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_24V_AGM_SET_OVPRECOVER,
    },
    [eBAT_SYS_24V][eBAT_TYPE_LFP] = {
                    .OutVoltMax      = BAT_SYS_24V_LFP_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_24V_LFP_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_24V_LFP_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_24V_LFP_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_24V_LFP_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_24V_LFP_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_24V_LFP_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_24V_LFP_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_24V_LFP_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_24V_LFP_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_24V_LFP_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_24V_LFP_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_24V_LFP_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_24V_LFP_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_24V_LFP_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_24V_LFP_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_24V_LFP_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_24V_LFP_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_24V_LFP_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_24V_LFP_SET_OVPRECOVER,
    },
    [eBAT_SYS_24V][eBAT_TYPE_NMC] = {
                    .OutVoltMax      = BAT_SYS_24V_NMC_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_24V_NMC_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_24V_NMC_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_24V_NMC_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_24V_NMC_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_24V_NMC_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_24V_NMC_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_24V_NMC_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_24V_NMC_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_24V_NMC_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_24V_NMC_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_24V_NMC_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_24V_NMC_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_24V_NMC_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_24V_NMC_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_24V_NMC_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_24V_NMC_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_24V_NMC_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_24V_NMC_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_24V_NMC_SET_OVPRECOVER,
    },
    [eBAT_SYS_36V][eBAT_TYPE_AGM] = {
                    .OutVoltMax      = BAT_SYS_36V_AGM_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_36V_AGM_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_36V_AGM_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_36V_AGM_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_36V_AGM_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_36V_AGM_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_36V_AGM_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_36V_AGM_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_36V_AGM_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_36V_AGM_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_36V_AGM_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_36V_AGM_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_36V_AGM_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_36V_AGM_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_36V_AGM_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_36V_AGM_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_36V_AGM_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_36V_AGM_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_36V_AGM_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_36V_AGM_SET_OVPRECOVER,
    },
    [eBAT_SYS_36V][eBAT_TYPE_GEL] = {
                    .OutVoltMax      = BAT_SYS_36V_AGM_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_36V_AGM_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_36V_AGM_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_36V_AGM_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_36V_AGM_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_36V_AGM_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_36V_AGM_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_36V_AGM_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_36V_AGM_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_36V_AGM_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_36V_AGM_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_36V_AGM_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_36V_AGM_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_36V_AGM_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_36V_AGM_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_36V_AGM_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_36V_AGM_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_36V_AGM_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_36V_AGM_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_36V_AGM_SET_OVPRECOVER,
    },
    [eBAT_SYS_36V][eBAT_TYPE_LFP] = {
                    .OutVoltMax      = BAT_SYS_36V_LFP_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_36V_LFP_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_36V_LFP_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_36V_LFP_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_36V_LFP_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_36V_LFP_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_36V_LFP_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_36V_LFP_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_36V_LFP_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_36V_LFP_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_36V_LFP_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_36V_LFP_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_36V_LFP_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_36V_LFP_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_36V_LFP_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_36V_LFP_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_36V_LFP_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_36V_LFP_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_36V_LFP_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_36V_LFP_SET_OVPRECOVER,
    },
    [eBAT_SYS_36V][eBAT_TYPE_NMC] = {
                    .OutVoltMax      = BAT_SYS_36V_NMC_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_36V_NMC_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_36V_NMC_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_36V_NMC_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_36V_NMC_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_36V_NMC_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_36V_NMC_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_36V_NMC_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_36V_NMC_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_36V_NMC_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_36V_NMC_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_36V_NMC_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_36V_NMC_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_36V_NMC_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_36V_NMC_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_36V_NMC_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_36V_NMC_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_36V_NMC_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_36V_NMC_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_36V_NMC_SET_OVPRECOVER,
    },
    [eBAT_SYS_48V][eBAT_TYPE_AGM] = {
                    .OutVoltMax      = BAT_SYS_48V_AGM_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_48V_AGM_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_48V_AGM_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_48V_AGM_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_48V_AGM_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_48V_AGM_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_48V_AGM_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_48V_AGM_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_48V_AGM_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_48V_AGM_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_48V_AGM_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_48V_AGM_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_48V_AGM_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_48V_AGM_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_48V_AGM_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_48V_AGM_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_48V_AGM_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_48V_AGM_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_48V_AGM_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_48V_AGM_SET_OVPRECOVER,
    },
    [eBAT_SYS_48V][eBAT_TYPE_GEL] = {
                    .OutVoltMax      = BAT_SYS_48V_AGM_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_48V_AGM_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_48V_AGM_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_48V_AGM_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_48V_AGM_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_48V_AGM_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_48V_AGM_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_48V_AGM_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_48V_AGM_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_48V_AGM_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_48V_AGM_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_48V_AGM_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_48V_AGM_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_48V_AGM_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_48V_AGM_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_48V_AGM_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_48V_AGM_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_48V_AGM_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_48V_AGM_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_48V_AGM_SET_OVPRECOVER,
    },
    [eBAT_SYS_48V][eBAT_TYPE_LFP] = {
                    .OutVoltMax      = BAT_SYS_48V_LFP_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_48V_LFP_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_48V_LFP_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_48V_LFP_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_48V_LFP_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_48V_LFP_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_48V_LFP_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_48V_LFP_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_48V_LFP_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_48V_LFP_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_48V_LFP_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_48V_LFP_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_48V_LFP_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_48V_LFP_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_48V_LFP_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_48V_LFP_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_48V_LFP_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_48V_LFP_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_48V_LFP_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_48V_LFP_SET_OVPRECOVER,
    },
    [eBAT_SYS_48V][eBAT_TYPE_NMC] = {
                    .OutVoltMax      = BAT_SYS_48V_NMC_MAX_OUT_VOLT,
                    .OutVoltMin      = BAT_SYS_48V_NMC_MIN_OUT_VOLT,
                    .OutVoltDefault  = BAT_SYS_48V_NMC_DEFAULT_OUT_VOLT,
                    .OutCurrMax      = BAT_SYS_48V_NMC_MAX_CHURR_VOLT,
                    .OutCurrMin      = BAT_SYS_48V_NMC_MIN_CHURR_VOLT,
                    .OutCurrDefault  = BAT_SYS_48V_NMC_DEFAULT_CHURR_VOLT,
                    .OutPowerMax     = BAT_SYS_48V_NMC_MAX_POWER_VOLT,
                    .OutPowerMin     = BAT_SYS_48V_NMC_MIN_POWER_VOLT,
                    .OutPowerDefault = BAT_SYS_48V_NMC_DEFAULT_POWER_VOLT,
                    .OpenVoltA       = BAT_SYS_48V_NMC_OPEN_VOLT_A,
                    .CloseVoltA      = BAT_SYS_48V_NMC_CLOSE_VOLT_A,
                    .VeerVoltA       = BAT_SYS_48V_NMC_VEER_VOLT_A,
                    .OpenVoltB       = BAT_SYS_48V_NMC_OPEN_VOLT_B,
                    .CloseVoltB      = BAT_SYS_48V_NMC_CLOSE_VOLT_B,
                    .SetChargLedCurr = BAT_SYS_48V_NMC_SET_LED_CHAR_CURR,
                    .SetFullLedCurr  = BAT_SYS_48V_NMC_SET_LED_FULL_CURR,
                    .SetUvlo         = BAT_SYS_48V_NMC_SET_UVLO,
                    .SetUvloRecover  = BAT_SYS_48V_NMC_SET_UVLORECOVER,
                    .SetOVP          = BAT_SYS_48V_NMC_SET_OVP,
                    .SetOVPRecover   = BAT_SYS_48V_NMC_SET_OVPRECOVER,
    },
};

void bat_a_arguments_limi(void)
{
    float verify_data = 0.00f;
    uint16_t BatTypeA = get_wg_com_v2_data.com_ctrl.InpBatyType;
    if(((BatTypeA&0xff00)>>8) == eBAT_AUTOSYS){
		BatTypeA = (eBAT_TYPE_AGM << 8) | (BatTypeA&0x00FF);
	}else if(((BatTypeA&0xff00)>>8) >= eBAT_TYPE_MAX){
        return;
    }

    if((BatTypeA&0x00FF) < eBAT_SYS_VOLT_MAX)
    {
        verify_data = get_wg_com_v2_data.com_param.SetInpVolt;
        LIMIT_MAX_MIN(Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutVoltMax,Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutVoltMin,Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutVoltDefault,get_wg_com_v2_data.com_param.SetInpVolt);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetInpVolt) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpVolt, wg_com_v2_param.SetInpVolt);}
        verify_data = get_wg_com_v2_data.com_param.SetInpCurr;
        LIMIT_MAX_MIN(Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutCurrMax,Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutCurrMin,Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutCurrDefault,get_wg_com_v2_data.com_param.SetInpCurr);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetInpCurr) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpCurr, wg_com_v2_param.SetInpCurr);}
        verify_data = get_wg_com_v2_data.com_param.SetInpCurrPower;
        LIMIT_MAX_MIN(Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutPowerMax,Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutPowerMin,Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutPowerDefault,get_wg_com_v2_data.com_param.SetInpCurrPower);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetInpCurrPower) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetInpCurrPower, wg_com_v2_param.SetInpCurrPower);}
        verify_data = get_wg_com_v2_data.com_param.AuotForwardOpenVoltA;
        LIMIT_MAX_MIN((Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OpenVoltA+2.0f),(Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OpenVoltA-2.0f),Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OpenVoltA,get_wg_com_v2_data.com_param.AuotForwardOpenVoltA);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.AuotForwardOpenVoltA) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.AuotForwardOpenVoltA, wg_com_v2_param.AuotForwardOpenVoltA);}
        verify_data = get_wg_com_v2_data.com_param.AuotForwardShutVoltA;
        LIMIT_MAX_MIN((Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].CloseVoltA+2.0f),(Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].CloseVoltA-2.0f),Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].CloseVoltA,get_wg_com_v2_data.com_param.AuotForwardShutVoltA);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.AuotForwardShutVoltA) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.AuotForwardShutVoltA, wg_com_v2_param.AuotForwardShutVoltA);}
        verify_data = get_wg_com_v2_data.com_param.AuotForwardVeerVoltA;
        LIMIT_MAX_MIN((Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].VeerVoltA+2.0f),(Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].VeerVoltA-2.0f),Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].VeerVoltA,get_wg_com_v2_data.com_param.AuotForwardVeerVoltA);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.AuotForwardVeerVoltA) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.AuotForwardVeerVoltA, wg_com_v2_param.AuotForwardVeerVoltA);}
        if((get_wg_com_v2_data.com_param.AuotForwardOpenVoltA <= get_wg_com_v2_data.com_param.AuotForwardShutVoltA) ||
           (get_wg_com_v2_data.com_param.AuotForwardShutVoltA <= get_wg_com_v2_data.com_param.AuotForwardVeerVoltA))
        {
            get_wg_com_v2_data.com_param.AuotForwardOpenVoltA = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OpenVoltA;
            get_wg_com_v2_data.com_param.AuotForwardShutVoltA = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].CloseVoltA;
            get_wg_com_v2_data.com_param.AuotForwardVeerVoltA = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].VeerVoltA;
            WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.AuotForwardOpenVoltA, wg_com_v2_param.AuotForwardOpenVoltA);
            WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.AuotForwardShutVoltA, wg_com_v2_param.AuotForwardShutVoltA);
            WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.AuotForwardVeerVoltA, wg_com_v2_param.AuotForwardVeerVoltA);
        }
    }
}

void bat_b_arguments_limi(void)
{
    float verify_data = 0.00f;
    uint16_t BatTypeB = get_wg_com_v2_data.com_ctrl.OutBatyType;
    if(((BatTypeB&0xff00)>>8) == eBAT_AUTOSYS){
		BatTypeB = (eBAT_TYPE_AGM << 8) | (BatTypeB&0x00FF);
	}else if(((BatTypeB&0xff00)>>8) >= eBAT_TYPE_MAX){
        return;
    }

    if((BatTypeB&0x00FF) < eBAT_SYS_VOLT_MAX)
    {
        verify_data = get_wg_com_v2_data.com_param.SetOutVolt;
        LIMIT_MAX_MIN(Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutVoltMax,Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutVoltMin,Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutVoltDefault,get_wg_com_v2_data.com_param.SetOutVolt);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetOutVolt) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutVolt, wg_com_v2_param.SetOutVolt);}
        verify_data = get_wg_com_v2_data.com_param.SetOutCurr;
        LIMIT_MAX_MIN(Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutCurrMax,Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutCurrMin,Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutCurrDefault,get_wg_com_v2_data.com_param.SetOutCurr);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetOutCurr) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutCurr, wg_com_v2_param.SetOutCurr);}
        verify_data = get_wg_com_v2_data.com_param.SetOutCurrPower;
        LIMIT_MAX_MIN(Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutPowerMax,Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutPowerMin,Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutPowerDefault,get_wg_com_v2_data.com_param.SetOutCurrPower);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.SetOutCurrPower) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.SetOutCurrPower, wg_com_v2_param.SetOutCurrPower);}
        verify_data = get_wg_com_v2_data.com_param.AuotReverseOpenVoltB;
        LIMIT_MAX_MIN((Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OpenVoltB+2.0f),(Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OpenVoltB-2.0f),Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OpenVoltB,get_wg_com_v2_data.com_param.AuotReverseOpenVoltB);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.AuotReverseOpenVoltB) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.AuotReverseOpenVoltB, wg_com_v2_param.AuotReverseOpenVoltB);}
        verify_data = get_wg_com_v2_data.com_param.AuotReverseShutVoltB;
        LIMIT_MAX_MIN((Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].CloseVoltB+2.0f),(Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].CloseVoltB-2.0f),Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].CloseVoltB,get_wg_com_v2_data.com_param.AuotReverseShutVoltB);
        if(float_equal(verify_data,get_wg_com_v2_data.com_param.AuotReverseShutVoltB) == 1){WG_COM_V2_SET_DATA_UINT(get_wg_com_v2_data.com_param.AuotReverseShutVoltB, wg_com_v2_param.AuotReverseShutVoltB);}
    }
}


void bat_mode_run(void)
{
    uint8_t param_updated;

    param_updated = updated_parameter();

    if(param_updated)
    {
        init_bat_mode_parameter();
    }

	bat_a_arguments_limi();
	bat_b_arguments_limi();
    (void)eeprom_autosys_runtime_update();

    #if(CAN_ON_OFF != 2) 
    if((get_wg_com_v2_data.com_ctrl.SetChargMode != eSET_AUTO_MODE)   &&
       (get_wg_com_v2_data.com_ctrl.SetChargMode != eSET_MANUAL_MODE))
    {
        WG_COM_V2_SET_DATA_UINT(eSET_MANUAL_MODE, wg_com_v2_ctrl.SetChargMode);
    }
    #endif

    BattChargingCurve(&charge_state_data,1);
}

void init_bat_mode_parameter(void)
{
    uint16_t BatSetOutCurrPower = 0;
    uint16_t BatSetInpCurrPower = 0;
    float BatSetOutVolt = 0.00f;
    float BatSetInpVolt = 0.00f;
    float BatSetOutCurr = 0.00f;
    float BatSetInpCurr = 0.00f;

    float AuotOpenVoltA  = 0.00f;
    float AuotVeerVoltA  = 0.00f;
    float AuotCloseVoltA = 0.00f;
    float AuotOpenVoltB  = 0.00f;
    float AuotCloseVoltB = 0.00f;

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

    uint16_t BatTypeA = get_wg_com_v2_data.com_ctrl.InpBatyType;
    uint16_t BatTypeB = get_wg_com_v2_data.com_ctrl.OutBatyType;

    if(eeprom_apply_battery_mode_profiles())
    {
        return;
    }

    if(((BatTypeA&0xff00)>>8) >= eBAT_TYPE_MAX)
    {
        BatTypeA = (eBAT_TYPE_LFP << 8) | (BatTypeA&0x00FF);
    }
    if(((BatTypeB&0xff00)>>8) >= eBAT_TYPE_MAX)
    {
        BatTypeB = (eBAT_TYPE_LFP << 8) | (BatTypeB&0x00FF);
    }

    if((BatTypeB&0x00FF) < eBAT_SYS_VOLT_MAX)
    {
        BatSetOutVolt      = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutVoltDefault;
        BatSetOutCurr      = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutCurrDefault;
        BatSetOutCurrPower = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutPowerDefault;
        AuotOpenVoltB      = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OpenVoltB;
        AuotCloseVoltB     = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].CloseVoltB;
        SetOutChargCurr    = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetChargLedCurr;
        SetOutFullCurr     = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetFullLedCurr;
        SetUvloB           = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetUvlo;
        SetUvloRecoverB    = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetUvloRecover;
        SetOVPB            = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetOVP;
        SetOVPRecoverB     = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetOVPRecover;
        WG_COM_V2_SET_DATA_UINT(BatSetOutVolt, wg_com_v2_param.SetOutVolt);
        WG_COM_V2_SET_DATA_UINT(BatSetOutCurr, wg_com_v2_param.SetOutCurr);
        WG_COM_V2_SET_DATA_UINT(BatSetOutCurrPower, wg_com_v2_param.SetOutCurrPower);
        WG_COM_V2_SET_DATA_UINT(AuotOpenVoltB, wg_com_v2_param.AuotReverseOpenVoltB);
        WG_COM_V2_SET_DATA_UINT(AuotCloseVoltB, wg_com_v2_param.AuotReverseShutVoltB);
        WG_COM_V2_SET_DATA_UINT(SetOutChargCurr, wg_com_v2_param.SetOutChargLedCurr);
        WG_COM_V2_SET_DATA_UINT(SetOutFullCurr, wg_com_v2_param.SetOutFullLedCurr);
        WG_COM_V2_SET_DATA_UINT(SetUvloB, wg_com_v2_param.SetOutUvlo);
        WG_COM_V2_SET_DATA_UINT(SetUvloRecoverB, wg_com_v2_param.SetOutUvloRecover);
        WG_COM_V2_SET_DATA_UINT(SetOVPB, wg_com_v2_param.SetOutOVP);
        WG_COM_V2_SET_DATA_UINT(SetOVPRecoverB, wg_com_v2_param.SetOutOVPRecover);
    }

    if((BatTypeA&0x00FF) < eBAT_SYS_VOLT_MAX)
    {
        BatSetInpVolt      = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutVoltDefault;
        BatSetInpCurr      = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutCurrDefault;
        BatSetInpCurrPower = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OutPowerDefault;
        AuotOpenVoltA      = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].OpenVoltA;
        AuotVeerVoltA      = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].VeerVoltA;
        AuotCloseVoltA     = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].CloseVoltA;
        SetInpChargCurr    = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].SetChargLedCurr;
        SetInpFullCurr     = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].SetFullLedCurr;
        SetUvloA           = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].SetUvlo;
        SetUvloRecoverA    = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].SetUvloRecover;
        SetOVPA            = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].SetOVP;
        SetOVPRecoverA     = Bat_Sys_Volt_Config[(BatTypeA&0x00FF)][((BatTypeA&0xff00)>>8)].SetOVPRecover;
        WG_COM_V2_SET_DATA_UINT(BatSetInpVolt, wg_com_v2_param.SetInpVolt);
        WG_COM_V2_SET_DATA_UINT(BatSetInpCurr, wg_com_v2_param.SetInpCurr);
        WG_COM_V2_SET_DATA_UINT(BatSetInpCurrPower, wg_com_v2_param.SetInpCurrPower);
        WG_COM_V2_SET_DATA_UINT(AuotOpenVoltA, wg_com_v2_param.AuotForwardOpenVoltA);
        WG_COM_V2_SET_DATA_UINT(AuotVeerVoltA, wg_com_v2_param.AuotForwardVeerVoltA);
        WG_COM_V2_SET_DATA_UINT(AuotCloseVoltA, wg_com_v2_param.AuotForwardShutVoltA);
        WG_COM_V2_SET_DATA_UINT(SetInpChargCurr, wg_com_v2_param.SetInpChargLedCurr);
        WG_COM_V2_SET_DATA_UINT(SetInpFullCurr, wg_com_v2_param.SetInpFullLedCurr);
        WG_COM_V2_SET_DATA_UINT(SetUvloA, wg_com_v2_param.SetInpUvlo);
        WG_COM_V2_SET_DATA_UINT(SetUvloRecoverA, wg_com_v2_param.SetInpUvloRecover);
        WG_COM_V2_SET_DATA_UINT(SetOVPA, wg_com_v2_param.SetInpOVP);
        WG_COM_V2_SET_DATA_UINT(SetOVPRecoverA, wg_com_v2_param.SetInpOVPRecover);
    }
}

void BattChargingCurve(charge_state_data_t *bat_charge_data,uint8_t soft_flag)
{
    float OutVolt = 0.0f;               // 显示输出电压
    float OutCurr = 0.0f;               // 显示输出电流
    float ChargeVolt = 0.0f;            // 充电电压
    float PrechargeVolt = 0.0f;         // 预充电电压
    float SetOutChargLedCurr = 0.0f;    // 设置输出充电指示灯电流
    float SetOutFullLedCurr = 0.0f;     // 设置输出充满指示灯电流
    uint16_t OutPower = 0;              // 显示输出功率
    uint16_t SetOutCurrPower;           // 设置输出功率
    uint8_t bat_step = 0;
    static uint8_t record_bat_step = 0;
    static uint8_t exeion_step = 0;
    float UfuncOutVolt = 0.0f;               // 运算输出电压
    float UfuncOutCurr = 0.0f;               // 运算输出电流

    if(bat_charge_data->check_state == eADDRS_BACKWARD)
    {
//        OutVolt = fabsf(get_show_fvs48_show());
//        OutCurr = fabsf(get_show_ihv_show());
        OutVolt = get_wg_com_v2_data.com_realtime_data.InpVolt;     // A端电压
        OutCurr = get_wg_com_v2_data.com_realtime_data.InpCurr;     // A端电流
        SetOutCurrPower = get_wg_com_v2_data.com_param.SetInpCurrPower;
        SetOutChargLedCurr = get_wg_com_v2_data.com_param.SetInpChargLedCurr;
        SetOutFullLedCurr = get_wg_com_v2_data.com_param.SetInpFullLedCurr;
        bat_charge_data->SetOutVolt = get_wg_com_v2_data.com_param.SetInpVolt;
        bat_charge_data->SetInpCurr = get_wg_com_v2_data.com_param.SetOutCurr;
        bat_charge_data->SetOutCurr = get_wg_com_v2_data.com_param.SetInpCurr;
        bat_charge_data->fvs48_pwr_lmt = get_wg_com_v2_data.com_param.SetInpCurrPower;
        bat_charge_data->rvs12_pwr_lmt = get_wg_com_v2_data.com_param.SetOutCurrPower;
        bat_charge_data->OutBatyType = get_wg_com_v2_data.com_ctrl.InpBatyType;
        bat_charge_data->rvs12_lmt = get_wg_com_v2_data.com_param.SetOutUvlo+0.5f;
        if(((bat_charge_data->OutBatyType&0xff00)>>8) == eBAT_DCDC)
        {
            bat_charge_data->fvs48_lmt = bat_charge_data->SetOutVolt;
        }
        else
        {
            bat_charge_data->fvs48_lmt = bat_charge_data->ActualOutVolt;//get_wg_com_v2_data.com_param.SetInpVolt;
        }
        bat_charge_data->Boot_Time_Delay.SetBootTime = get_wg_com_v2_data.com_ctrl.SetBootTimeB;
        bat_charge_data->ilv_lmt = bat_charge_data->SetInpCurr;
        if(bat_charge_data->get_is_run == 1)
        {
            if(soft_flag == 1)
            {
                bat_charge_data->ihv_lmt = curr_soft_start(bat_charge_data->ihv_lmt,bat_charge_data->temp_derate_curr,(bat_charge_data->ActiveOnCurrStartTime*100));
            }else{
                bat_charge_data->ihv_lmt = curr_soft_start(bat_charge_data->ihv_lmt,bat_charge_data->temp_derate_curr,0);
            }
        }else{
            bat_charge_data->ihv_lmt = 1.0f;
        }
    }
    else if(bat_charge_data->check_state == eADDRS_FORWARD) 
    {
//        OutVolt = fabsf(get_show_rvs12_show());
//        OutCurr = fabsf(get_show_ilv_show());
        OutVolt = get_wg_com_v2_data.com_realtime_data.OutVolt;     // B端电压
        OutCurr = get_wg_com_v2_data.com_realtime_data.OutCurr;     // B端电流
        SetOutCurrPower = get_wg_com_v2_data.com_param.SetOutCurrPower;
        SetOutChargLedCurr = get_wg_com_v2_data.com_param.SetOutChargLedCurr;
        SetOutFullLedCurr = get_wg_com_v2_data.com_param.SetOutFullLedCurr;
        bat_charge_data->SetOutVolt = get_wg_com_v2_data.com_param.SetOutVolt;
        bat_charge_data->SetInpCurr = get_wg_com_v2_data.com_param.SetInpCurr;
        bat_charge_data->SetOutCurr = get_wg_com_v2_data.com_param.SetOutCurr;
        bat_charge_data->fvs48_pwr_lmt = get_wg_com_v2_data.com_param.SetInpCurrPower;
        bat_charge_data->rvs12_pwr_lmt = get_wg_com_v2_data.com_param.SetOutCurrPower;
        bat_charge_data->OutBatyType = get_wg_com_v2_data.com_ctrl.OutBatyType;
        if(((bat_charge_data->OutBatyType&0xff00)>>8) == eBAT_DCDC)
        {
            bat_charge_data->rvs12_lmt = bat_charge_data->SetOutVolt;
        }
        else
        {
            bat_charge_data->rvs12_lmt = bat_charge_data->ActualOutVolt;//get_wg_com_v2_data.com_param.SetOutVolt;
        }
        bat_charge_data->fvs48_lmt = get_wg_com_v2_data.com_param.SetInpUvlo+0.5f;
        bat_charge_data->Boot_Time_Delay.SetBootTime = get_wg_com_v2_data.com_ctrl.SetBootTimeA;
        bat_charge_data->ihv_lmt = bat_charge_data->SetInpCurr;
        if(bat_charge_data->get_is_run == 1)
        {
            if(soft_flag == 1)
            {
                bat_charge_data->ilv_lmt = curr_soft_start(bat_charge_data->ilv_lmt,bat_charge_data->temp_derate_curr,(bat_charge_data->ActiveOnCurrStartTime*100));
            }else{
                bat_charge_data->ilv_lmt = curr_soft_start(bat_charge_data->ilv_lmt,bat_charge_data->temp_derate_curr,0);
            }
        }else{
            bat_charge_data->ilv_lmt = 1.0f;
        }
    }

    UfuncOutVolt = 0.8f*UfuncOutVolt + (1-0.8f)*OutVolt;
    UfuncOutCurr = 0.8f*UfuncOutCurr + (1-0.8f)*OutCurr;
    OutPower = UfuncOutVolt * UfuncOutCurr;
    if(soft_flag == 1)
    {
        bat_charge_data->set_out_lmt_curr = (bat_charge_data->SetOutCurr * 0.1f);
    }else{
        bat_charge_data->set_out_lmt_curr = (bat_charge_data->SetOutCurr * 0.0002f);
    }
    ChargeVolt = bat_charge_data->SetOutVolt * 0.70f;
    PrechargeVolt = bat_charge_data->SetOutVolt * 0.65f;

    switch(Get_Charge_State())
    {
        case eBAT_LA_AGM:
        case eBAT_LA_GEL:
            bat_step = 1;
            break;
        case eBAT_LI_LFP:
        case eBAT_LI_NMC:
            bat_step = 2;
            break;
        case eSCAP:
            bat_step = 3;
            break;
        case eBAT_DCDC:
            bat_step = 4;
            break;
        default :
            bat_step = 1;
            break;
    }

    if(record_bat_step != bat_step)
    {
        record_bat_step = bat_step;
        exeion_step = 0;
    }else{
        exeion_step = bat_step;
    }

    switch(exeion_step){
        case 0:
            bat_charge_data->SetCharState = ePRE_CHARGE;
            FloatChargingDelay = 0;
            PrechargeDelay = 0;
            ChargeCVDelay = 0;
            ChargeCCDelay = 0;
            ClearChargeFlagDelay = 0;
            LiBatFullDelay = 0;
            CurrentReached90PercentDelay = 0;
            PowerReached90PercentDelay = 0;  

            bat_charge_data->bat_state.sucCurrentReached90Percent = 0;
            bat_charge_data->bat_state.sucPowerReached90Percent = 0;
            bat_charge_data->bat_state.LithiumBatOnOff = 0;
            break;
		case 1:  // 铅酸电池
			if(OutVolt >= ChargeVolt){ // 70%
				if(OutCurr < SetOutFullLedCurr){
                    if(++FloatChargingDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                    {
                        FloatChargingDelay = 0;
                        bat_charge_data->SetCharState = eFLOAT_CHARGE;    // 浮充电模式
                    }
                    ChargeCVDelay = 0;
                    ChargeCCDelay = 0;
				}else{
                    if((OutCurr > (bat_charge_data->SetOutCurr*0.95f))
                    || (OutVolt < (bat_charge_data->SetOutVolt*0.96f))){
                        if(++ChargeCCDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                        {
                            ChargeCCDelay = 0;
                            bat_charge_data->SetCharState = eCC_CHARGE;    // CC恒流充电
                        }
                        ChargeCVDelay = 0;
                    }else if((OutCurr < (bat_charge_data->SetOutCurr*0.95f))
                          && (OutCurr > SetOutChargLedCurr)
                          && (OutVolt > bat_charge_data->SetOutVolt*0.97f)){
                        if(++ChargeCVDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                        {
                            ChargeCVDelay = 0;
                            bat_charge_data->SetCharState = eCV_CHARGE;    // CV恒流充电
                        }
                        ChargeCCDelay = 0;
                    }else{
                        ChargeCVDelay = 0;
                        ChargeCCDelay = 0;
                    }
                    FloatChargingDelay = 0;
                }
                PrechargeDelay = 0;
			}else if(OutVolt <= PrechargeVolt){   // 65%
                if(++PrechargeDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                {
                    PrechargeDelay = 0;
                    bat_charge_data->SetCharState = ePRE_CHARGE;    // 预充电模式
                }
                FloatChargingDelay = 0;
                ChargeCVDelay = 0;
                ChargeCCDelay = 0;
			}else{
                FloatChargingDelay = 0;
                PrechargeDelay = 0;
                ChargeCVDelay = 0;
                ChargeCCDelay = 0;
			}

			switch(bat_charge_data->SetCharState)
			{
				case ePRE_CHARGE:
					bat_charge_data->ActualOutVolt = bat_charge_data->SetOutVolt;
					bat_charge_data->ActualOutCurr = bat_charge_data->SetOutCurr*0.20f;
					break;
				case eCC_CHARGE:
				case eCV_CHARGE:
					bat_charge_data->ActualOutVolt = bat_charge_data->SetOutVolt;
					bat_charge_data->ActualOutCurr = bat_charge_data->SetOutCurr;
					break;
				case eFLOAT_CHARGE:
					bat_charge_data->ActualOutVolt = bat_charge_data->SetOutVolt*0.92f;
					bat_charge_data->ActualOutCurr = bat_charge_data->SetOutCurr;
					break;
				default : 
					bat_charge_data->ActualOutVolt = bat_charge_data->SetOutVolt;
					bat_charge_data->ActualOutCurr = bat_charge_data->SetOutCurr*0.20f;
					break;
			}
            break;
		case 2:  // 锂电池
			// 判断电流是否曾经达到过设置值的 90%
			if (OutCurr >= (bat_charge_data->SetOutCurr * 0.90f)) { 
                if(++CurrentReached90PercentDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                {
                    CurrentReached90PercentDelay = 0;
                    bat_charge_data->bat_state.sucCurrentReached90Percent = 1;             // 电流达到或超过 90% 时，置位标志
                }
			}else{
                CurrentReached90PercentDelay = 0;
            }
			// 判断功率是否曾经达到过设置值的 90%
			if (OutPower >= (SetOutCurrPower * 0.90f)) {
                if(++PowerReached90PercentDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                {
                    PowerReached90PercentDelay = 0;
                    bat_charge_data->bat_state.sucPowerReached90Percent = 1;             // 功率达到或超过 90% 时，置位标志
                }
			}else{
                PowerReached90PercentDelay = 0;
            }
			// 如果电流曾经达到过 90% 并且电流小于 1.5A，且电压大于设置电压的 90%，则标记为充满
			if ((!bat_charge_data->bat_state.LithiumBatOnOff)                          // 没有充满 
			&& ((bat_charge_data->bat_state.sucCurrentReached90Percent)||(bat_charge_data->bat_state.sucPowerReached90Percent))                // 是否大电流充电过
			&&  (OutVolt > (bat_charge_data->SetOutVolt * 0.95f))    // 输出电压是否大于设置电压95%
			&&  (OutCurr < (bat_charge_data->SetOutCurr * 0.10f))    // 输出电流小于设置电流10%
            &&  (bat_charge_data->SetCharState == eFULL_CHARGE)){		
				if(++LiBatFullDelay >= TIME_CNT_BAT_TYPE_1M_IN_10MS)
				{
                    LiBatFullDelay = 0;
					bat_charge_data->bat_state.LithiumBatOnOff = 1; // 符合条件，标记充满
				}
                ChargeCVDelay = 0;
                ChargeCCDelay = 0;                
			}else{
				LiBatFullDelay = 0;
                if(OutVolt >= ChargeVolt){ // 70%
                    if((OutCurr > (bat_charge_data->SetOutCurr*0.95f))
                     ||(OutVolt < (bat_charge_data->SetOutVolt*0.96f))){
                        bat_charge_data->SetCharState = eCC_CHARGE;    // CC恒流充电
                    }else if((OutCurr < (bat_charge_data->SetOutCurr*0.95f))
                          && (OutCurr > SetOutChargLedCurr)
                          && (OutVolt > bat_charge_data->SetOutVolt*0.97f)){
                        bat_charge_data->SetCharState = eCV_CHARGE;    // CV恒流充电
                    }else if((OutCurr < SetOutFullLedCurr)
                          && (OutVolt > bat_charge_data->SetOutVolt*0.98f)){
                        bat_charge_data->SetCharState = eFULL_CHARGE;    // 电池充满
                    }

                    PrechargeDelay = 0;
                }else if(OutVolt <= PrechargeVolt){   // 65%
                    if(++PrechargeDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                    {
                        PrechargeDelay = 0;
                        bat_charge_data->SetCharState = ePRE_CHARGE;    // 预充电模式
                    }
                    ChargeCVDelay = 0;
                    ChargeCCDelay = 0;
                }else{
                    PrechargeDelay = 0;
                    ChargeCVDelay = 0;
                    ChargeCCDelay = 0;
                }
			}

			if(bat_charge_data->bat_state.LithiumBatOnOff)
			{	
				// 如果电压低于设置电压*0.93
				if (OutVolt < (bat_charge_data->SetOutVolt * 0.93f)) 
				{
                    if(++ClearChargeFlagDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                    {
                        ClearChargeFlagDelay = 0;
						bat_charge_data->bat_state.LithiumBatOnOff = 0; // 电压低于阈值，清除充满标记
						bat_charge_data->bat_state.sucCurrentReached90Percent = 0; // 清除电流标志位，重新判断
						bat_charge_data->bat_state.sucPowerReached90Percent = 0;
                    }
				}else{
                    ClearChargeFlagDelay = 0;
				}

				bat_charge_data->SetCharState = eSTOP_CHARGE;    // 充满停止

                PrechargeDelay = 0;
                ChargeCVDelay = 0;
                ChargeCCDelay = 0;
			}
            else
            {
                ClearChargeFlagDelay = 0;
            }

            switch(bat_charge_data->SetCharState)
            {
                case ePRE_CHARGE:
                    bat_charge_data->ActualOutVolt = bat_charge_data->SetOutVolt;
                    bat_charge_data->ActualOutCurr = bat_charge_data->SetOutCurr * 0.20f;
                    break;
                case eCC_CHARGE:
                case eCV_CHARGE:
                case eFULL_CHARGE:
                    bat_charge_data->ActualOutVolt = bat_charge_data->SetOutVolt;
                    bat_charge_data->ActualOutCurr = bat_charge_data->SetOutCurr;
                    break;
                default : 
                    bat_charge_data->ActualOutVolt = bat_charge_data->SetOutVolt;
                    bat_charge_data->ActualOutCurr = bat_charge_data->SetOutCurr * 0.20f;
                    break;
            }
            break;
		case 3:  // 超级电容
            if((OutCurr > (bat_charge_data->SetOutCurr*0.95f))
            || (OutVolt < (bat_charge_data->SetOutVolt*0.96f))){
                if(++ChargeCCDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                {
                    ChargeCCDelay = 0;
                    bat_charge_data->SetCharState = eCC_CHARGE;    // CC恒流充电
                }
                ChargeCVDelay = 0;
                LiBatFullDelay = 0;
            }else if((OutCurr < (bat_charge_data->SetOutCurr*0.95f))
                  && (OutVolt > bat_charge_data->SetOutVolt*0.97f)){
                if(++ChargeCVDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                {
                    ChargeCVDelay = 0;
                    bat_charge_data->SetCharState = eCV_CHARGE;    // CV恒流充电
                }
                ChargeCCDelay = 0;
                LiBatFullDelay = 0;
            }else if((OutCurr < SetOutFullLedCurr)
                  && (OutVolt > bat_charge_data->SetOutVolt*0.98f)){
                if(++LiBatFullDelay >= TIME_CNT_BAT_TYPE_200MS_IN_10MS)
                {
                    LiBatFullDelay = 0;
                    bat_charge_data->SetCharState = eFULL_CHARGE;  // 电池充满
                }
            }else{
                ChargeCVDelay = 0;
                ChargeCCDelay = 0;
                LiBatFullDelay = 0;
            }
            bat_charge_data->ActualOutVolt = bat_charge_data->SetOutVolt;
            bat_charge_data->ActualOutCurr = bat_charge_data->SetOutCurr;
            break;
		case 4:  // DCDC
            bat_charge_data->SetCharState = eCC_CHARGE;
            bat_charge_data->ActualOutVolt = bat_charge_data->SetOutVolt;
            bat_charge_data->ActualOutCurr = bat_charge_data->SetOutCurr;
            break;
    }
//    set_charge_state_mode(bat_charge_data->SetCharState);
}

void init_mppt_mode_parameter(void)
{
    uint16_t BatSetOutCurrPower = 0;
    uint16_t BatSetInpCurrPower = 0;
    float BatSetOutVolt = 0.00f;
    float BatSetInpVolt = 0.00f;
    float BatSetOutCurr = 0.00f;
    float BatSetInpCurr = 0.00f;

    float AuotOpenVoltA  = 0.00f;
    float AuotVeerVoltA  = 0.00f;
    float AuotCloseVoltA = 0.00f;
    float AuotOpenVoltB  = 0.00f;
    float AuotCloseVoltB = 0.00f;

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

    if(eeprom_apply_mppt_mode_profile())
    {
        return;
    }

    if(((BatTypeB&0xff00)>>8) >= eBAT_TYPE_MAX)
    {
        BatTypeB = (eBAT_TYPE_LFP << 8) | (BatTypeB&0x00FF);
    }

    if((BatTypeB&0x00FF) < eBAT_SYS_VOLT_MAX)
    {
        BatSetOutVolt      = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutVoltDefault;
        BatSetOutCurr      = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutCurrDefault;
        BatSetOutCurrPower = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].OutPowerDefault;
        SetOutChargCurr    = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetChargLedCurr;
        SetOutFullCurr     = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetFullLedCurr;
        SetUvloB           = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetUvlo;
        SetUvloRecoverB    = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetUvloRecover;
        SetOVPB            = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetOVP;
        SetOVPRecoverB     = Bat_Sys_Volt_Config[(BatTypeB&0x00FF)][((BatTypeB&0xff00)>>8)].SetOVPRecover;
        WG_COM_V2_SET_DATA_UINT(BatSetOutVolt, wg_com_v2_param.SetOutVolt);
        WG_COM_V2_SET_DATA_UINT(BatSetOutCurr, wg_com_v2_param.SetOutCurr);
        WG_COM_V2_SET_DATA_UINT(BatSetOutCurrPower, wg_com_v2_param.SetOutCurrPower);
        WG_COM_V2_SET_DATA_UINT(SetOutChargCurr, wg_com_v2_param.SetOutChargLedCurr);
        WG_COM_V2_SET_DATA_UINT(SetOutFullCurr, wg_com_v2_param.SetOutFullLedCurr);
        WG_COM_V2_SET_DATA_UINT(SetUvloB, wg_com_v2_param.SetOutUvlo);
        WG_COM_V2_SET_DATA_UINT(SetUvloRecoverB, wg_com_v2_param.SetOutUvloRecover);
        WG_COM_V2_SET_DATA_UINT(SetOVPB, wg_com_v2_param.SetOutOVP);
        WG_COM_V2_SET_DATA_UINT(SetOVPRecoverB, wg_com_v2_param.SetOutOVPRecover);
    }

    BatSetInpVolt      = 12.00f;
    BatSetInpCurr      = 85.00f;
    BatSetInpCurrPower = 1200.00f;                 
    AuotOpenVoltA      = 13.60f;
    AuotVeerVoltA      = 12.00f;
    AuotCloseVoltA     = 13.00f;
    AuotOpenVoltB      = 12.50f;
    AuotCloseVoltB     = 12.00f;
    
    SetInpChargCurr    = (BatSetInpCurr*0.15f);
    SetInpFullCurr     = (SetInpChargCurr-0.5f);
    SetUvloA           = 10.00f;
    SetUvloRecoverA    = 11.00f;
    SetOVPA            = 62.00f;
    SetOVPRecoverA     = 61.00f;
    WG_COM_V2_SET_DATA_UINT(BatSetInpVolt, wg_com_v2_param.SetInpVolt);
    WG_COM_V2_SET_DATA_UINT(BatSetInpCurr, wg_com_v2_param.SetInpCurr);
    WG_COM_V2_SET_DATA_UINT(BatSetInpCurrPower, wg_com_v2_param.SetInpCurrPower);
    WG_COM_V2_SET_DATA_UINT(AuotOpenVoltA, wg_com_v2_param.AuotForwardOpenVoltA);
    WG_COM_V2_SET_DATA_UINT(AuotVeerVoltA, wg_com_v2_param.AuotForwardVeerVoltA);
    WG_COM_V2_SET_DATA_UINT(AuotCloseVoltA, wg_com_v2_param.AuotForwardShutVoltA);
    WG_COM_V2_SET_DATA_UINT(AuotOpenVoltB, wg_com_v2_param.AuotReverseOpenVoltB);
    WG_COM_V2_SET_DATA_UINT(AuotCloseVoltB, wg_com_v2_param.AuotReverseShutVoltB);
    WG_COM_V2_SET_DATA_UINT(SetInpChargCurr, wg_com_v2_param.SetInpChargLedCurr);
    WG_COM_V2_SET_DATA_UINT(SetInpFullCurr, wg_com_v2_param.SetInpFullLedCurr);
    WG_COM_V2_SET_DATA_UINT(SetUvloA, wg_com_v2_param.SetInpUvlo);
    WG_COM_V2_SET_DATA_UINT(SetUvloRecoverA, wg_com_v2_param.SetInpUvloRecover);
    WG_COM_V2_SET_DATA_UINT(SetOVPA, wg_com_v2_param.SetInpOVP);
    WG_COM_V2_SET_DATA_UINT(SetOVPRecoverA, wg_com_v2_param.SetInpOVPRecover);
}


