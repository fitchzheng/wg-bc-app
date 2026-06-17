#ifndef __FAULT_H
#define __FAULT_H

#include "stdint.h"

typedef enum
{
    FAULT_FVS48_SCP,
    FAULT_FVS48_OVP,
    FAULT_FVS48_UVP,
    FAULT_RVS12_SCP,
    FAULT_RVS12_OVP,
    FAULT_RVS12_UVP,
    FAULT_OTP,
    FAULT_TEMP2_OTP,
    FAULT_INSIDE_OTP,
    FAULT_OUTSIDE_OTP,
    FAULT_AUX_IS_ERR,
    FAULT_ACC_UVP,
    FAULT_RTM_UVP,
    FAULT_OCP = 16,
    FAULT_ADDR_IS_ERR,
    FAULT_FAST_UVP,
    FAULT_FUNCTION,
    FAULT_SOFT_STOP,
    FAULT_MAX,
} FAULT_E;

typedef enum
{
    ALARM_A_REDUCE_PWR,
    ALARM_B_REDUCE_PWR,
    ALARM_PWR_IS_OFF,
    ALARM_PG_IS_OFF,
    ALARM_RTM_IS_OFF,
    ALARM_AUTOSYS_NO_SYSTEM,
    ALARM_A_INSIDE_REDUCE_PWR = 16,
    ALARM_B_INSIDE_REDUCE_PWR,
    ALARM_A_OUTSIDE_REDUCE_PWR,
    ALARM_B_OUTSIDE_REDUCE_PWR,
    ALARM_INSIDE_OTP,
    ALARM_OUTSIDE_OTP,
    ALARM_TEMP2_OTP,
    ALARM_MAX,
} ALARM_E;

void fault_set_fault(FAULT_E fault);

void fault_clear_fault(FAULT_E fault);

void fault_clear_all_fault(void);

FAULT_E fault_get_fault(void);

uint8_t fault_get_fault_bit(FAULT_E fault);

uint32_t fault_get_all_fault(void);

void fault_set_alarm(ALARM_E alarm);

void fault_clear_alarm(ALARM_E alarm);

void fault_clear_all_alarm(void);

uint32_t fault_get_all_alarm(void);

uint8_t  fault_get_alarm_bit(ALARM_E alarm);
#endif
