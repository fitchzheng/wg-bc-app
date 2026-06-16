#include "temp_derate.h"
#include "get_com_data.h"
#include "section.h"
#include "fault.h"

static float temp_derate_curr = 0;

void temp_current(temp_derate_item_t *derate)
{
    if(derate->enable == 1)
    {
        if((derate->temp > (derate->set_temp-13))&&(derate->temp < derate->set_temp))
        {
            derate->temp_current = derate->current - ((derate->temp - (derate->set_temp-13))*0.02f*derate->current);
        }
        else if(derate->temp >= derate->set_temp)
        {
            derate->temp_current = derate->current - (13*0.02f*derate->current);
        }
        else
        {
            derate->temp_current = derate->current - (0.02f*derate->current);
        }
    }
    else
    {
        derate->temp_current = derate->current;
    }
}

enum
{
    DERATE_TEMP2_DETECTION,
    DERATE_INSIDE_DETECTION,
    DERATE_OUTSIDE_DETECTION,
    DERATE_MAX,
};
temp_derate_item_t protect_protections[DERATE_MAX];
void temp_derate_run (void)
{
    
    int16_t Temp2Vaul = 0,InsideVaul = 0,OutsideVaul = 0;
    int16_t SetTemp2Value = 0,SetInsideValue = 0,SetOutsideValue = 0;

    Temp2Vaul = get_wg_com_v2_data.com_realtime_data.Temp2;
    SetTemp2Value = get_wg_com_v2_data.com_param.SetTemp2;
    InsideVaul = get_wg_com_v2_data.com_realtime_data.InsideTemp;
    SetInsideValue = get_wg_com_v2_data.com_param.SetInsideTemp;
    OutsideVaul = get_wg_com_v2_data.com_realtime_data.OutsideTemp;
    SetOutsideValue = get_wg_com_v2_data.com_param.SetOutsideTemp; 

    protect_protections[DERATE_TEMP2_DETECTION].temp = Temp2Vaul;//tp3
    protect_protections[DERATE_TEMP2_DETECTION].set_temp = SetTemp2Value;
    protect_protections[DERATE_TEMP2_DETECTION].current = charge_state_data.ActualOutCurr;

    protect_protections[DERATE_INSIDE_DETECTION].temp = InsideVaul;
    protect_protections[DERATE_INSIDE_DETECTION].set_temp = SetInsideValue;//tp1
    protect_protections[DERATE_INSIDE_DETECTION].current = charge_state_data.ActualOutCurr;

    protect_protections[DERATE_OUTSIDE_DETECTION].temp = OutsideVaul;
    protect_protections[DERATE_OUTSIDE_DETECTION].set_temp = SetOutsideValue;//tp2
    protect_protections[DERATE_OUTSIDE_DETECTION].current = charge_state_data.ActualOutCurr;

   if(charge_state_data.check_state == eADDRS_BACKWARD)
    {
        protect_protections[DERATE_TEMP2_DETECTION].enable = fault_get_alarm_bit(ALARM_A_REDUCE_PWR);
        protect_protections[DERATE_INSIDE_DETECTION].enable = fault_get_alarm_bit(ALARM_A_INSIDE_REDUCE_PWR);
        protect_protections[DERATE_OUTSIDE_DETECTION].enable = fault_get_alarm_bit(ALARM_A_OUTSIDE_REDUCE_PWR);
    }
    else if(charge_state_data.check_state == eADDRS_FORWARD)
    {
        protect_protections[DERATE_TEMP2_DETECTION].enable = fault_get_alarm_bit(ALARM_B_REDUCE_PWR);
        protect_protections[DERATE_INSIDE_DETECTION].enable = fault_get_alarm_bit(ALARM_B_INSIDE_REDUCE_PWR);
        protect_protections[DERATE_OUTSIDE_DETECTION].enable = fault_get_alarm_bit(ALARM_B_OUTSIDE_REDUCE_PWR);
    }

    for (uint16_t i = 0; i < DERATE_MAX; i++)
    {
        temp_current(&protect_protections[i]);
    }

     (protect_protections[DERATE_TEMP2_DETECTION].temp_current  > protect_protections[DERATE_INSIDE_DETECTION].temp_current) ? 
    ((protect_protections[DERATE_INSIDE_DETECTION].temp_current > protect_protections[DERATE_OUTSIDE_DETECTION].temp_current)) ? (temp_derate_curr = protect_protections[DERATE_OUTSIDE_DETECTION].temp_current) : (temp_derate_curr = protect_protections[DERATE_INSIDE_DETECTION].temp_current): 
    ((protect_protections[DERATE_TEMP2_DETECTION].temp_current  > protect_protections[DERATE_OUTSIDE_DETECTION].temp_current)) ? (temp_derate_curr = protect_protections[DERATE_OUTSIDE_DETECTION].temp_current) : (temp_derate_curr = protect_protections[DERATE_TEMP2_DETECTION].temp_current);
}

REG_TASK(20, temp_derate_run)

void get_temp_derate_curr(void)
{
    charge_state_data.temp_derate_curr = temp_derate_curr;
}






