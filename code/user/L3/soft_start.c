#include "soft_start.h"
#include "get_com_data.h"

float curr_soft_start(float curr_lmt,float curr_tar,uint16_t time)
{
    float step_curr = curr_tar / time;
    float ret_curr = 0.0f;
    if(time != 0)
    {
        if((curr_lmt + step_curr) < curr_tar)
        {
            ret_curr = curr_lmt + step_curr;
            charge_state_data.soft_start_flag = 0;
        }
        else
        {
            ret_curr = curr_tar;
            charge_state_data.soft_start_flag = 1;
        }
    }
    else
    {
        charge_state_data.soft_start_flag = 1;
        ret_curr = curr_tar;
    }
    
    return (ret_curr < 1) ? (ret_curr = 1) : (ret_curr);
}


