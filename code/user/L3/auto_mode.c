#include "auto_mode.h"
#include "wg_com_v2.h"
#include "get_com_data.h"
#include "soft_start.h"
//static uint8_t AutosteererOnOff;
//static uint8_t StateModelFlag;
//void auto_charge_mode(void)
//{
//    if(StateModelFlag == 1){
//        if(get_wg_com_v2_data.com_realtime_data.InpVolt > get_wg_com_v2_data.com_param.AuotForwardOpenVoltA){	
//            StateModelFlag = 0;
//        }
//    }else if(StateModelFlag == 0){
//        if((get_wg_com_v2_data.com_realtime_data.InpVolt < get_wg_com_v2_data.com_param.AuotForwardVeerVoltA)
//        && (get_wg_com_v2_data.com_realtime_data.InpVolt > get_wg_com_v2_data.com_param.SetInpUvlo)
//        && (get_wg_com_v2_data.com_realtime_data.OutVolt > get_wg_com_v2_data.com_param.AuotReverseOpenVoltB)){	 	  
//            StateModelFlag = 1;
//        }
//    }
//}


