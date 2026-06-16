#include "fault.h"
#include "stdint.h"
#include "shell.h"
#include "section.h"
static uint32_t fault_bit = 0;
static uint32_t alarm_bit = 0;

#if (SHELL_ON_OFF == 1)
REG_SHELL_VAR(fault_bit, fault_bit, SHELL_UINT32, 0xFFFFFFFFu, 0u, NULL, SHELL_STA_NULL)
#endif
void RAMFUNC fault_set_fault(FAULT_E fault)
{
    fault_bit |= (1 << fault);
}

void RAMFUNC fault_clear_fault(FAULT_E fault)
{
    fault_bit &= ~(1 << fault);
}

void fault_clear_all_fault(void)
{
    fault_bit = 0;
}

FAULT_E fault_get_fault(void)
{
    for (uint32_t i = 0; i < FAULT_MAX; i++)
    {
        if (fault_bit & (1 << i))
        {
            return (FAULT_E)(i + 1);
        }
    }
    return (FAULT_E)0;
}

uint8_t  fault_get_fault_bit(FAULT_E fault)
{
    if (fault_bit & (1 << fault))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


void fault_clear(void)
{
    static uint32_t fault_ocp_cnt = 0;
    
    if(fault_get_fault_bit(FAULT_OCP))
    {
        if(++fault_ocp_cnt >= 100)
        {
            fault_ocp_cnt = 0;
            fault_clear_fault(FAULT_OCP);
        }
    }
    else
    {
        fault_ocp_cnt = 0;
    }

    static uint32_t fault_fvs48_scp_cnt = 0;

    if(fault_get_fault_bit(FAULT_FVS48_SCP))
    {
        if(++fault_fvs48_scp_cnt >= 300)
        {
            fault_fvs48_scp_cnt = 0;
            fault_clear_fault(FAULT_FVS48_SCP);
        }
    }
    else
    {
        fault_fvs48_scp_cnt = 0;
    }

    static uint32_t fault_rvs12_scp_cnt = 0;

    if(fault_get_fault_bit(FAULT_RVS12_SCP))
    {
        if(++fault_rvs12_scp_cnt >= 300)
        {
            fault_rvs12_scp_cnt = 0;
            fault_clear_fault(FAULT_RVS12_SCP);
        }
    }
    else
    {
        fault_rvs12_scp_cnt = 0;
    }
}
REG_TASK(10,fault_clear)

uint32_t fault_get_all_fault(void)
{
    return fault_bit;
}

void fault_set_alarm(ALARM_E alarm)
{
    alarm_bit |= (1 << alarm);
}

void fault_clear_alarm(ALARM_E alarm)
{
    alarm_bit &= ~(1 << alarm);
}

void fault_clear_all_alarm(void)
{
    alarm_bit = 0;
}

//ALARM_E fault_get_alarm(void)
//{
//    for (uint32_t i = 0; i < ALARM_MAX; i++)
//    {
//        if (alarm_bit & (1 << i))
//        {
//            return i + 1;
//        }
//    }
//    return 0;
//}

uint32_t fault_get_all_alarm(void)
{
    return alarm_bit;
}

uint8_t  fault_get_alarm_bit(ALARM_E alarm)
{
    if (alarm_bit & (1 << alarm))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
