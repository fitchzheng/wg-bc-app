#ifndef __BAT_MODE_H
#define __BAT_MODE_H

#include "stdint.h"
#include "get_com_data.h"

#define TIME_CNT_BAT_TYPE_200MS_IN_10MS (20)
#define TIME_CNT_BAT_TYPE_1M_IN_10MS (60*100)

// 电池模式
#define BAT_SYS_12V_AGM_MAX_OUT_VOLT                    15.00f
#define BAT_SYS_12V_AGM_MIN_OUT_VOLT                    10.00f
#define BAT_SYS_12V_AGM_DEFAULT_OUT_VOLT                14.40f
#define BAT_SYS_12V_AGM_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_12V_AGM_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_12V_AGM_DEFAULT_CHURR_VOLT              100.00f
#define BAT_SYS_12V_AGM_MAX_POWER_VOLT                  1500
#define BAT_SYS_12V_AGM_MIN_POWER_VOLT                  50
#define BAT_SYS_12V_AGM_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_12V_AGM_OPEN_VOLT_A                     13.60f
#define BAT_SYS_12V_AGM_CLOSE_VOLT_A                    13.00f
#define BAT_SYS_12V_AGM_VEER_VOLT_A                     12.00f
#define BAT_SYS_12V_AGM_OPEN_VOLT_B                     12.50f
#define BAT_SYS_12V_AGM_CLOSE_VOLT_B                    12.00f
#define BAT_SYS_12V_AGM_SET_LED_CHAR_CURR               (BAT_SYS_12V_AGM_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_12V_AGM_SET_LED_FULL_CURR               (BAT_SYS_12V_AGM_SET_LED_CHAR_CURR-0.5f)
#define BAT_SYS_12V_AGM_SET_UVLO                        12.00f
#define BAT_SYS_12V_AGM_SET_UVLORECOVER                 13.00f
#define BAT_SYS_12V_AGM_SET_OVP                         (BAT_SYS_12V_AGM_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_12V_AGM_SET_OVPRECOVER                  (BAT_SYS_12V_AGM_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_16V_AGM_MAX_OUT_VOLT                    20.00f
#define BAT_SYS_16V_AGM_MIN_OUT_VOLT                    10.00f
#define BAT_SYS_16V_AGM_DEFAULT_OUT_VOLT                18.20f
#define BAT_SYS_16V_AGM_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_16V_AGM_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_16V_AGM_DEFAULT_CHURR_VOLT              80.00f
#define BAT_SYS_16V_AGM_MAX_POWER_VOLT                  1500
#define BAT_SYS_16V_AGM_MIN_POWER_VOLT                  50
#define BAT_SYS_16V_AGM_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_16V_AGM_OPEN_VOLT_A                     17.00f
#define BAT_SYS_16V_AGM_CLOSE_VOLT_A                    16.50f
#define BAT_SYS_16V_AGM_VEER_VOLT_A                     16.00f
#define BAT_SYS_16V_AGM_OPEN_VOLT_B                     16.00f
#define BAT_SYS_16V_AGM_CLOSE_VOLT_B                    15.00f
#define BAT_SYS_16V_AGM_SET_LED_CHAR_CURR               (BAT_SYS_16V_AGM_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_16V_AGM_SET_LED_FULL_CURR               (BAT_SYS_16V_AGM_SET_LED_CHAR_CURR-0.5f)
#define BAT_SYS_16V_AGM_SET_UVLO                        15.00f
#define BAT_SYS_16V_AGM_SET_UVLORECOVER                 16.00f
#define BAT_SYS_16V_AGM_SET_OVP                         (BAT_SYS_16V_AGM_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_16V_AGM_SET_OVPRECOVER                  (BAT_SYS_16V_AGM_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_24V_AGM_MAX_OUT_VOLT                    30.00f
#define BAT_SYS_24V_AGM_MIN_OUT_VOLT                    15.00f
#define BAT_SYS_24V_AGM_DEFAULT_OUT_VOLT                28.80f
#define BAT_SYS_24V_AGM_MAX_CHURR_VOLT                  55.00f
#define BAT_SYS_24V_AGM_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_24V_AGM_DEFAULT_CHURR_VOLT              50.00f
#define BAT_SYS_24V_AGM_MAX_POWER_VOLT                  1500
#define BAT_SYS_24V_AGM_MIN_POWER_VOLT                  50
#define BAT_SYS_24V_AGM_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_24V_AGM_OPEN_VOLT_A                     27.00f
#define BAT_SYS_24V_AGM_CLOSE_VOLT_A                    26.00f
#define BAT_SYS_24V_AGM_VEER_VOLT_A                     25.00f
#define BAT_SYS_24V_AGM_OPEN_VOLT_B                     25.00f
#define BAT_SYS_24V_AGM_CLOSE_VOLT_B                    24.00f
#define BAT_SYS_24V_AGM_SET_LED_CHAR_CURR               (BAT_SYS_24V_AGM_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_24V_AGM_SET_LED_FULL_CURR               (BAT_SYS_24V_AGM_SET_LED_CHAR_CURR-0.5f)
#define BAT_SYS_24V_AGM_SET_UVLO                        24.00f
#define BAT_SYS_24V_AGM_SET_UVLORECOVER                 25.00f
#define BAT_SYS_24V_AGM_SET_OVP                         (BAT_SYS_24V_AGM_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_24V_AGM_SET_OVPRECOVER                  (BAT_SYS_24V_AGM_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_36V_AGM_MAX_OUT_VOLT                    45.00f
#define BAT_SYS_36V_AGM_MIN_OUT_VOLT                    30.00f
#define BAT_SYS_36V_AGM_DEFAULT_OUT_VOLT                43.80f
#define BAT_SYS_36V_AGM_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_36V_AGM_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_36V_AGM_DEFAULT_CHURR_VOLT              35.00f
#define BAT_SYS_36V_AGM_MAX_POWER_VOLT                  1500
#define BAT_SYS_36V_AGM_MIN_POWER_VOLT                  50
#define BAT_SYS_36V_AGM_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_36V_AGM_OPEN_VOLT_A                     40.00f
#define BAT_SYS_36V_AGM_CLOSE_VOLT_A                    39.00f
#define BAT_SYS_36V_AGM_VEER_VOLT_A                     38.00f
#define BAT_SYS_36V_AGM_OPEN_VOLT_B                     37.00f
#define BAT_SYS_36V_AGM_CLOSE_VOLT_B                    36.00f
#define BAT_SYS_36V_AGM_SET_LED_CHAR_CURR               (BAT_SYS_36V_AGM_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_36V_AGM_SET_LED_FULL_CURR               (BAT_SYS_36V_AGM_SET_LED_CHAR_CURR-0.5f)
#define BAT_SYS_36V_AGM_SET_UVLO                        36.00f
#define BAT_SYS_36V_AGM_SET_UVLORECOVER                 37.00f
#define BAT_SYS_36V_AGM_SET_OVP                         (BAT_SYS_36V_AGM_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_36V_AGM_SET_OVPRECOVER                  (BAT_SYS_36V_AGM_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_48V_AGM_MAX_OUT_VOLT                    60.00f
#define BAT_SYS_48V_AGM_MIN_OUT_VOLT                    45.00f
#define BAT_SYS_48V_AGM_DEFAULT_OUT_VOLT                58.40f
#define BAT_SYS_48V_AGM_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_48V_AGM_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_48V_AGM_DEFAULT_CHURR_VOLT              25.00f
#define BAT_SYS_48V_AGM_MAX_POWER_VOLT                  1500
#define BAT_SYS_48V_AGM_MIN_POWER_VOLT                  50
#define BAT_SYS_48V_AGM_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_48V_AGM_OPEN_VOLT_A                     54.40f
#define BAT_SYS_48V_AGM_CLOSE_VOLT_A                    52.00f
#define BAT_SYS_48V_AGM_VEER_VOLT_A                     48.00f
#define BAT_SYS_48V_AGM_OPEN_VOLT_B                     50.00f
#define BAT_SYS_48V_AGM_CLOSE_VOLT_B                    48.00f
#define BAT_SYS_48V_AGM_SET_LED_CHAR_CURR               (BAT_SYS_48V_AGM_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_48V_AGM_SET_LED_FULL_CURR               (BAT_SYS_48V_AGM_SET_LED_CHAR_CURR-0.5f)
#define BAT_SYS_48V_AGM_SET_UVLO                        48.00f
#define BAT_SYS_48V_AGM_SET_UVLORECOVER                 49.00f
#define BAT_SYS_48V_AGM_SET_OVP                         (BAT_SYS_48V_AGM_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_48V_AGM_SET_OVPRECOVER                  (BAT_SYS_48V_AGM_DEFAULT_OUT_VOLT+1.0f)

// LFP
#define BAT_SYS_12V_LFP_MAX_OUT_VOLT                    15.00f
#define BAT_SYS_12V_LFP_MIN_OUT_VOLT                    10.00f
#define BAT_SYS_12V_LFP_DEFAULT_OUT_VOLT                14.60f
#define BAT_SYS_12V_LFP_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_12V_LFP_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_12V_LFP_DEFAULT_CHURR_VOLT              100.00f
#define BAT_SYS_12V_LFP_MAX_POWER_VOLT                  1500
#define BAT_SYS_12V_LFP_MIN_POWER_VOLT                  50
#define BAT_SYS_12V_LFP_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_12V_LFP_OPEN_VOLT_A                     13.60f
#define BAT_SYS_12V_LFP_CLOSE_VOLT_A                    13.00f
#define BAT_SYS_12V_LFP_VEER_VOLT_A                     12.00f
#define BAT_SYS_12V_LFP_OPEN_VOLT_B                     12.50f
#define BAT_SYS_12V_LFP_CLOSE_VOLT_B                    12.00f
#define BAT_SYS_12V_LFP_SET_LED_CHAR_CURR               (BAT_SYS_12V_LFP_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_12V_LFP_SET_LED_FULL_CURR               (BAT_SYS_12V_LFP_DEFAULT_CHURR_VOLT-0.5f)
#define BAT_SYS_12V_LFP_SET_UVLO                        12.00f
#define BAT_SYS_12V_LFP_SET_UVLORECOVER                 13.00f
#define BAT_SYS_12V_LFP_SET_OVP                         (BAT_SYS_12V_LFP_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_12V_LFP_SET_OVPRECOVER                  (BAT_SYS_12V_LFP_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_16V_LFP_MAX_OUT_VOLT                    20.00f
#define BAT_SYS_16V_LFP_MIN_OUT_VOLT                    10.00f
#define BAT_SYS_16V_LFP_DEFAULT_OUT_VOLT                18.20f
#define BAT_SYS_16V_LFP_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_16V_LFP_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_16V_LFP_DEFAULT_CHURR_VOLT              80.00f
#define BAT_SYS_16V_LFP_MAX_POWER_VOLT                  1500
#define BAT_SYS_16V_LFP_MIN_POWER_VOLT                  50
#define BAT_SYS_16V_LFP_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_16V_LFP_OPEN_VOLT_A                     17.00f
#define BAT_SYS_16V_LFP_CLOSE_VOLT_A                    16.50f
#define BAT_SYS_16V_LFP_VEER_VOLT_A                     16.00f
#define BAT_SYS_16V_LFP_OPEN_VOLT_B                     16.00f
#define BAT_SYS_16V_LFP_CLOSE_VOLT_B                    15.00f
#define BAT_SYS_16V_LFP_SET_LED_CHAR_CURR               (BAT_SYS_16V_LFP_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_16V_LFP_SET_LED_FULL_CURR               (BAT_SYS_16V_LFP_SET_LED_CHAR_CURR-0.5f)
#define BAT_SYS_16V_LFP_SET_UVLO                        15.00f
#define BAT_SYS_16V_LFP_SET_UVLORECOVER                 16.00f
#define BAT_SYS_16V_LFP_SET_OVP                         (BAT_SYS_16V_LFP_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_16V_LFP_SET_OVPRECOVER                  (BAT_SYS_16V_LFP_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_24V_LFP_MAX_OUT_VOLT                    30.00f
#define BAT_SYS_24V_LFP_MIN_OUT_VOLT                    15.00f
#define BAT_SYS_24V_LFP_DEFAULT_OUT_VOLT                29.20f
#define BAT_SYS_24V_LFP_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_24V_LFP_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_24V_LFP_DEFAULT_CHURR_VOLT              50.00f
#define BAT_SYS_24V_LFP_MAX_POWER_VOLT                  1500
#define BAT_SYS_24V_LFP_MIN_POWER_VOLT                  50
#define BAT_SYS_24V_LFP_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_24V_LFP_OPEN_VOLT_A                     27.00f
#define BAT_SYS_24V_LFP_CLOSE_VOLT_A                    26.00f
#define BAT_SYS_24V_LFP_VEER_VOLT_A                     25.00f
#define BAT_SYS_24V_LFP_OPEN_VOLT_B                     25.00f
#define BAT_SYS_24V_LFP_CLOSE_VOLT_B                    24.00f
#define BAT_SYS_24V_LFP_SET_LED_CHAR_CURR               (BAT_SYS_24V_LFP_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_24V_LFP_SET_LED_FULL_CURR               (BAT_SYS_24V_LFP_SET_LED_CHAR_CURR-0.5f)
#define BAT_SYS_24V_LFP_SET_UVLO                        24.00f
#define BAT_SYS_24V_LFP_SET_UVLORECOVER                 25.00f
#define BAT_SYS_24V_LFP_SET_OVP                         (BAT_SYS_24V_LFP_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_24V_LFP_SET_OVPRECOVER                  (BAT_SYS_24V_LFP_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_36V_LFP_MAX_OUT_VOLT                    45.00f
#define BAT_SYS_36V_LFP_MIN_OUT_VOLT                    30.00f
#define BAT_SYS_36V_LFP_DEFAULT_OUT_VOLT                43.80f
#define BAT_SYS_36V_LFP_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_36V_LFP_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_36V_LFP_DEFAULT_CHURR_VOLT              35.00f
#define BAT_SYS_36V_LFP_MAX_POWER_VOLT                  1500
#define BAT_SYS_36V_LFP_MIN_POWER_VOLT                  50
#define BAT_SYS_36V_LFP_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_36V_LFP_OPEN_VOLT_A                     40.00f
#define BAT_SYS_36V_LFP_CLOSE_VOLT_A                    39.00f
#define BAT_SYS_36V_LFP_VEER_VOLT_A                     38.00f
#define BAT_SYS_36V_LFP_OPEN_VOLT_B                     37.00f
#define BAT_SYS_36V_LFP_CLOSE_VOLT_B                    36.00f
#define BAT_SYS_36V_LFP_SET_LED_CHAR_CURR               (BAT_SYS_36V_LFP_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_36V_LFP_SET_LED_FULL_CURR               (BAT_SYS_36V_LFP_SET_LED_CHAR_CURR-0.5f)
#define BAT_SYS_36V_LFP_SET_UVLO                        36.00f
#define BAT_SYS_36V_LFP_SET_UVLORECOVER                 37.00f
#define BAT_SYS_36V_LFP_SET_OVP                         (BAT_SYS_36V_LFP_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_36V_LFP_SET_OVPRECOVER                  (BAT_SYS_36V_LFP_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_48V_LFP_MAX_OUT_VOLT                    60.00f
#define BAT_SYS_48V_LFP_MIN_OUT_VOLT                    40.00f
#define BAT_SYS_48V_LFP_DEFAULT_OUT_VOLT                58.40f
#define BAT_SYS_48V_LFP_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_48V_LFP_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_48V_LFP_DEFAULT_CHURR_VOLT              25.00f
#define BAT_SYS_48V_LFP_MAX_POWER_VOLT                  1500
#define BAT_SYS_48V_LFP_MIN_POWER_VOLT                  50
#define BAT_SYS_48V_LFP_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_48V_LFP_OPEN_VOLT_A                     54.40f
#define BAT_SYS_48V_LFP_CLOSE_VOLT_A                    52.00f
#define BAT_SYS_48V_LFP_VEER_VOLT_A                     48.00f
#define BAT_SYS_48V_LFP_OPEN_VOLT_B                     50.00f
#define BAT_SYS_48V_LFP_CLOSE_VOLT_B                    48.00f
#define BAT_SYS_48V_LFP_SET_LED_CHAR_CURR               (BAT_SYS_48V_LFP_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_48V_LFP_SET_LED_FULL_CURR               (BAT_SYS_48V_LFP_SET_LED_CHAR_CURR-0.5f)
#define BAT_SYS_48V_LFP_SET_UVLO                        48.00f
#define BAT_SYS_48V_LFP_SET_UVLORECOVER                 49.00f
#define BAT_SYS_48V_LFP_SET_OVP                         (BAT_SYS_48V_LFP_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_48V_LFP_SET_OVPRECOVER                  (BAT_SYS_48V_LFP_DEFAULT_OUT_VOLT+1.0f)

// NMC
#define BAT_SYS_12V_NMC_MAX_OUT_VOLT                    15.00f
#define BAT_SYS_12V_NMC_MIN_OUT_VOLT                    10.00f
#define BAT_SYS_12V_NMC_DEFAULT_OUT_VOLT                12.60f
#define BAT_SYS_12V_NMC_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_12V_NMC_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_12V_NMC_DEFAULT_CHURR_VOLT              100.00f
#define BAT_SYS_12V_NMC_MAX_POWER_VOLT                  1500
#define BAT_SYS_12V_NMC_MIN_POWER_VOLT                  50
#define BAT_SYS_12V_NMC_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_12V_NMC_OPEN_VOLT_A                     13.60f
#define BAT_SYS_12V_NMC_CLOSE_VOLT_A                    13.00f
#define BAT_SYS_12V_NMC_VEER_VOLT_A                     12.00f
#define BAT_SYS_12V_NMC_OPEN_VOLT_B                     12.50f
#define BAT_SYS_12V_NMC_CLOSE_VOLT_B                    12.00f
#define BAT_SYS_12V_NMC_SET_LED_CHAR_CURR               (BAT_SYS_12V_NMC_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_12V_NMC_SET_LED_FULL_CURR               (BAT_SYS_12V_NMC_DEFAULT_CHURR_VOLT-0.5f)
#define BAT_SYS_12V_NMC_SET_UVLO                        10.00f
#define BAT_SYS_12V_NMC_SET_UVLORECOVER                 11.00f
#define BAT_SYS_12V_NMC_SET_OVP                         (BAT_SYS_12V_NMC_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_12V_NMC_SET_OVPRECOVER                  (BAT_SYS_12V_NMC_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_16V_NMC_MAX_OUT_VOLT                    20.00f
#define BAT_SYS_16V_NMC_MIN_OUT_VOLT                    10.00f
#define BAT_SYS_16V_NMC_DEFAULT_OUT_VOLT                16.80f
#define BAT_SYS_16V_NMC_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_16V_NMC_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_16V_NMC_DEFAULT_CHURR_VOLT              80.00f
#define BAT_SYS_16V_NMC_MAX_POWER_VOLT                  1500
#define BAT_SYS_16V_NMC_MIN_POWER_VOLT                  50
#define BAT_SYS_16V_NMC_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_16V_NMC_OPEN_VOLT_A                     17.00f
#define BAT_SYS_16V_NMC_CLOSE_VOLT_A                    16.50f
#define BAT_SYS_16V_NMC_VEER_VOLT_A                     16.00f
#define BAT_SYS_16V_NMC_OPEN_VOLT_B                     16.00f
#define BAT_SYS_16V_NMC_CLOSE_VOLT_B                    15.00f
#define BAT_SYS_16V_NMC_SET_LED_CHAR_CURR               (BAT_SYS_16V_NMC_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_16V_NMC_SET_LED_FULL_CURR               (BAT_SYS_16V_NMC_DEFAULT_CHURR_VOLT-0.5f)
#define BAT_SYS_16V_NMC_SET_UVLO                        13.00f
#define BAT_SYS_16V_NMC_SET_UVLORECOVER                 14.00f
#define BAT_SYS_16V_NMC_SET_OVP                         (BAT_SYS_16V_NMC_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_16V_NMC_SET_OVPRECOVER                  (BAT_SYS_16V_NMC_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_24V_NMC_MAX_OUT_VOLT                    30.00f
#define BAT_SYS_24V_NMC_MIN_OUT_VOLT                    15.00f
#define BAT_SYS_24V_NMC_DEFAULT_OUT_VOLT                25.20f
#define BAT_SYS_24V_NMC_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_24V_NMC_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_24V_NMC_DEFAULT_CHURR_VOLT              50.00f
#define BAT_SYS_24V_NMC_MAX_POWER_VOLT                  1500
#define BAT_SYS_24V_NMC_MIN_POWER_VOLT                  50
#define BAT_SYS_24V_NMC_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_24V_NMC_OPEN_VOLT_A                     27.00f
#define BAT_SYS_24V_NMC_CLOSE_VOLT_A                    26.00f
#define BAT_SYS_24V_NMC_VEER_VOLT_A                     25.00f
#define BAT_SYS_24V_NMC_OPEN_VOLT_B                     25.00f
#define BAT_SYS_24V_NMC_CLOSE_VOLT_B                    24.00f
#define BAT_SYS_24V_NMC_SET_LED_CHAR_CURR               (BAT_SYS_24V_NMC_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_24V_NMC_SET_LED_FULL_CURR               (BAT_SYS_24V_NMC_DEFAULT_CHURR_VOLT-0.5f)
#define BAT_SYS_24V_NMC_SET_UVLO                        20.00f
#define BAT_SYS_24V_NMC_SET_UVLORECOVER                 21.00f
#define BAT_SYS_24V_NMC_SET_OVP                         (BAT_SYS_24V_NMC_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_24V_NMC_SET_OVPRECOVER                  (BAT_SYS_24V_NMC_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_36V_NMC_MAX_OUT_VOLT                    45.00f
#define BAT_SYS_36V_NMC_MIN_OUT_VOLT                    30.00f
#define BAT_SYS_36V_NMC_DEFAULT_OUT_VOLT                42.00f
#define BAT_SYS_36V_NMC_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_36V_NMC_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_36V_NMC_DEFAULT_CHURR_VOLT              35.00f
#define BAT_SYS_36V_NMC_MAX_POWER_VOLT                  1500
#define BAT_SYS_36V_NMC_MIN_POWER_VOLT                  50
#define BAT_SYS_36V_NMC_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_36V_NMC_OPEN_VOLT_A                     40.00f
#define BAT_SYS_36V_NMC_CLOSE_VOLT_A                    38.00f
#define BAT_SYS_36V_NMC_VEER_VOLT_A                     39.00f
#define BAT_SYS_36V_NMC_OPEN_VOLT_B                     37.00f
#define BAT_SYS_36V_NMC_CLOSE_VOLT_B                    36.00f
#define BAT_SYS_36V_NMC_SET_LED_CHAR_CURR               (BAT_SYS_36V_NMC_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_36V_NMC_SET_LED_FULL_CURR               (BAT_SYS_36V_NMC_DEFAULT_CHURR_VOLT-0.5f)
#define BAT_SYS_36V_NMC_SET_UVLO                        33.00f
#define BAT_SYS_36V_NMC_SET_UVLORECOVER                 34.00f
#define BAT_SYS_36V_NMC_SET_OVP                         (BAT_SYS_36V_NMC_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_36V_NMC_SET_OVPRECOVER                  (BAT_SYS_36V_NMC_DEFAULT_OUT_VOLT+1.0f)

#define BAT_SYS_48V_NMC_MAX_OUT_VOLT                    60.00f
#define BAT_SYS_48V_NMC_MIN_OUT_VOLT                    40.00f
#define BAT_SYS_48V_NMC_DEFAULT_OUT_VOLT                56.40f
#define BAT_SYS_48V_NMC_MAX_CHURR_VOLT                  100.00f
#define BAT_SYS_48V_NMC_MIN_CHURR_VOLT                  1.00f
#define BAT_SYS_48V_NMC_DEFAULT_CHURR_VOLT              25.00f
#define BAT_SYS_48V_NMC_MAX_POWER_VOLT                  1500
#define BAT_SYS_48V_NMC_MIN_POWER_VOLT                  50
#define BAT_SYS_48V_NMC_DEFAULT_POWER_VOLT              1500
#define BAT_SYS_48V_NMC_OPEN_VOLT_A                     54.40f
#define BAT_SYS_48V_NMC_CLOSE_VOLT_A                    52.00f
#define BAT_SYS_48V_NMC_VEER_VOLT_A                     48.00f
#define BAT_SYS_48V_NMC_OPEN_VOLT_B                     50.00f
#define BAT_SYS_48V_NMC_CLOSE_VOLT_B                    48.00f
#define BAT_SYS_48V_NMC_SET_LED_CHAR_CURR               (BAT_SYS_48V_NMC_DEFAULT_CHURR_VOLT*0.15f)
#define BAT_SYS_48V_NMC_SET_LED_FULL_CURR               (BAT_SYS_48V_NMC_DEFAULT_CHURR_VOLT-0.5f)
#define BAT_SYS_48V_NMC_SET_UVLO                        43.00f
#define BAT_SYS_48V_NMC_SET_UVLORECOVER                 44.00f
#define BAT_SYS_48V_NMC_SET_OVP                         (BAT_SYS_48V_NMC_DEFAULT_OUT_VOLT+2.0f)
#define BAT_SYS_48V_NMC_SET_OVPRECOVER                  (BAT_SYS_48V_NMC_DEFAULT_OUT_VOLT+1.0f)

typedef struct
{
    float OutVoltMax;         // 输出最大电压
    float OutVoltMin;         // 输出最小电压
    float OutVoltDefault;     // 输出电压默认值
    float OutCurrMax;         // 输出最大电流
    float OutCurrMin;         // 输出最小电流
    float OutCurrDefault;     // 输出电流默认值
    float OpenVoltA;          // 自动模式下正向开电压
    float CloseVoltA;         // 自动模式下正向关闭电压
    float VeerVoltA;          // 自动模式下正向转向电压
    float OpenVoltB;          // 自动模式下反向开电压
    float CloseVoltB;         // 自动模式下反向关闭电压
    float SetChargLedCurr;
    float SetFullLedCurr;
    float SetUvlo;
    float SetUvloRecover;
    float SetOVP;
    float SetOVPRecover;
    uint16_t OutPowerMax;        // 输出最大功率
    uint16_t OutPowerMin;        // 输出最小功率
    uint16_t OutPowerDefault; // 输出功率默认值

} BAT_MODE_CONFIG_T;

typedef enum
{
    eBAT_SYS_12V,
    eBAT_SYS_16V,
    eBAT_SYS_24V,
    eBAT_SYS_36V,
    eBAT_SYS_48V,
    eBAT_SYS_VOLT_MAX,
} BAT_SYS_VOLT_STATE_E;

typedef enum
{
    eBAT_TYPE_AGM,
    eBAT_TYPE_GEL,
    eBAT_TYPE_LFP,
    eBAT_TYPE_NMC,
    eBAT_TYPE_MAX,
} BAT_TYPE_SELECT_E;

void bat_mode_run(void);
void init_bat_mode_parameter(void);
void BattChargingCurve(charge_state_data_t *bat_charge_data,uint8_t soft_flag);
void bat_a_arguments_limi(void);
void bat_b_arguments_limi(void);
void init_mppt_mode_parameter(void);
extern const BAT_MODE_CONFIG_T Bat_Sys_Volt_Config[eBAT_SYS_VOLT_MAX][eBAT_TYPE_MAX];
#endif

