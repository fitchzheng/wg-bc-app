#include "charge_control.h"
#include "get_com_data.h"
#include "wg_com_v2.h"
#include "section.h"
#include "standard_mode.h"
#include "bat_mode.h"
#include "bat_charge_pattern.h"
#include "ctrl_app.h"
#include "basic_mode.h"
#include "temp_derate.h"
#include "fault.h"
#include "mppt.h"
void charge_control_run(void)
{
    uint16_t power_mode;
    get_wg_com_data_rum();
    charge_state_data.get_is_run = ctrl_app_get_is_run();
    charge_state_data.check_state = get_check_state_data();
    GetChargeState(get_wg_com_v2_data.com_ctrl.SetChargMode);
    get_temp_derate_curr();
    power_mode = get_wg_com_v2_data.com_ctrl.SetPowerMode;
    if(get_wg_com_v2_data.com_ctrl.MpptSwitch == 1)
    {
        power_mode = eMPPT_MODE;
    }
    else if(power_mode == eMPPT_MODE)
    {
        power_mode = eSET_BAT_MODE;
    }
    switch(power_mode)
    {
        case eSET_STANDARD_MODE:
            #if (STANDARD_MODE_RUN_ON_OFF == 1)
            standard_mode_run(&charge_state_data);
            #else
            fault_set_fault(FAULT_FUNCTION);
            charge_state_data.SetCharState = eSTOP_CHARGE;
            updated_parameter();
            #endif
            charge_state_data.mppt_mode_flag = 0;
            break;
        case eSET_CUSTOM_MODE:
            if(fault_get_fault_bit(FAULT_FUNCTION) == 1)
            {
                fault_clear_fault(FAULT_FUNCTION);
            }
            basic_mode_run(&charge_state_data);
            charge_state_data.mppt_mode_flag = 0;
            break;
        case eSET_BAT_MODE:
            #if (BAT_MODE_RUN_ON_OFF == 1)
            bat_mode_run();
            #else
            fault_set_fault(FAULT_FUNCTION);
            charge_state_data.SetCharState = eSTOP_CHARGE;
            updated_parameter();
            #endif
            charge_state_data.mppt_mode_flag = 0;
            break;
        case eMPPT_MODE:
            mppt_mode_run();
            charge_state_data.mppt_mode_flag = 1;
            break;
    	default:
            fault_set_fault(FAULT_FUNCTION); 
    		break;
    }
}


REG_TASK(10, charge_control_run)



