#include "client_can.h"
#include "section.h"
#include "bsp_can.h"
#include "get_com_data.h"
#include "wg_com_v2.h"
#include "gpio.h"

void send_data(void)
{
    uint8_t data[8];
    data[0] = 0;
    data[1] = 1;
    data[2] = 2;
    data[3] = 3;
    data[4] = 4;
    data[5] = 5;
    data[6] = 6;
    data[7] = 7;
    bsp_can_tx(0x580, data);        
    
}

//REG_TASK(500, send_data)

static uint16_t can_get_u16_be(const uint8_t *data, uint8_t offset)
{
    return (uint16_t)(((uint16_t)data[offset] << 8) | data[offset + 1]);
}

static void can_put_u16_be(uint8_t *data, uint8_t offset, uint16_t value)
{
    data[offset] = (uint8_t)((value >> 8) & 0xff);
    data[offset + 1] = (uint8_t)(value & 0xff);
}

static uint16_t can_centi_to_deci(uint16_t value)
{
    return (uint16_t)((value + 5U) / 10U);
}

void Get_CAN_Communications_Content (uint32_t can_id, uint8_t *data, uint8_t len)
{
    // 提取 DGN
    uint16_t data_val = 0;
    float f_data_val = 0;
    if((can_id != 0x600) || (len != 8)){return;}
    if((data[1] == 0x22) || (data[1] == 0x88)){
        switch(data[0]){
            case A_VOLT_CURR_POWER:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_realtime_data.InpVolt));
                can_put_u16_be(data, 4, (uint16_t)(wg_com_v2_realtime_data.InpCurr));
                can_put_u16_be(data, 6, (uint16_t)(wg_com_v2_realtime_data.InpCurrPower));
                break;
            case B_VOLT_CURR_POWER:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_realtime_data.OutVolt));
                can_put_u16_be(data, 4, (uint16_t)(wg_com_v2_realtime_data.OutCurr));
                can_put_u16_be(data, 6, (uint16_t)(wg_com_v2_realtime_data.OutCurrPower));
                break;
            case TEMP_READINGS:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_realtime_data.InsideTemp));
                can_put_u16_be(data, 4, (uint16_t)(wg_com_v2_realtime_data.OutsideTemp));
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;
            case POWER_CHARGING_MODE:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_realtime_data.PowerMode));
                can_put_u16_be(data, 4, (uint16_t)(wg_com_v2_realtime_data.ChargMode));
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;
            case FAULT_ALARM_SIGNALS:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_realtime_data.FaultSign));
                can_put_u16_be(data, 4, (uint16_t)(wg_com_v2_realtime_data.AlarmSign));
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;
            case AB_COMPEN_TEMP:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_realtime_data.CompensationVoltA));
                can_put_u16_be(data, 4, (uint16_t)(wg_com_v2_realtime_data.CompensationVoltB));
                can_put_u16_be(data, 6, (uint16_t)(wg_com_v2_realtime_data.Temp2));
                break;
            case CHARGING_STATUS:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_realtime_data.StateCharge));
                data[4] = 0;                                                                                // 保留
                data[5] = 0;                                                                                // 保留
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;

            case FACTORY_RESET:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.FactoryReset);                         // 恢复出厂设置
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_ctrl.FactoryReset));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case POWER_STATUS:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.PowerOnOff);                           // 开关机状�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_ctrl.PowerOnOff));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case POWER_MODE:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    if((data_val == eSET_BAT_MODE)  // 电池模式
                    || (data_val == eMPPT_MODE))
                    {
                        WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetPowerMode);                         // 电源模式
                    }
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_ctrl.SetPowerMode));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case CHARGING_MODE:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetChargMode);                         // 充电模式
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_ctrl.SetChargMode));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case A_BATTERY_TYPE:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.InpBatyType);                          // A端电池类�?�?位类型，�?位电�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_ctrl.InpBatyType));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case B_BATTERY_TYPE:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.OutBatyType);                          // B端电池类�?�?位类型，�?位电�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_ctrl.OutBatyType));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case BOOT_TIME_DELAY:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetBootTimeA);                         // A端开机时�?
                    data_val = can_get_u16_be(data, 4);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetBootTimeB);                         // B端开机时�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_ctrl.SetBootTimeA));
                    can_put_u16_be(data, 4, (uint16_t)(wg_com_v2_ctrl.SetBootTimeB));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case SOFT_START_TIME_DELAY:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetOnCurrStartTimeA);                  // 408: A端开机电流软起动时间
                    data_val = can_get_u16_be(data, 4);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetOnCurrStartTimeB);                  // 409: B端开机电流软起动时间
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_ctrl.SetOnCurrStartTimeA));
                    can_put_u16_be(data, 4, (uint16_t)(wg_com_v2_ctrl.SetOnCurrStartTimeB));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_VOLTAGE:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpVolt);                        // 设置A端电�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.SetInpVolt));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_CURRENT:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpCurr);                        // A端电�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.SetInpCurr));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_POWER:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_param.SetInpCurrPower);                     // A端功�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.SetInpCurrPower));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_VOLT:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutVolt);                        // B端电�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.SetOutVolt));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_CURR:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutCurr);                        // B端电�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.SetOutCurr));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_POWER:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_param.SetOutCurrPower);                     // B端功�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.SetOutCurrPower));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_UNDER:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpUvlo);                        // A端欠压保�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci((uint16_t)wg_com_v2_param.SetInpUvlo));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_UNDER_R:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpUvloRecover);                 // A端欠压保护恢�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci((uint16_t)wg_com_v2_param.SetInpUvloRecover));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_OVER:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpOVP);                         // A端过压保�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci((uint16_t)wg_com_v2_param.SetInpOVP));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_OVER_R:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpOVPRecover);                  // A端过压保护恢�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci((uint16_t)wg_com_v2_param.SetInpOVPRecover));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_UNDER:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutUvlo);                        // B端欠压保�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci((uint16_t)wg_com_v2_param.SetOutUvlo));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_UNDER_R:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutUvloRecover);                 // B端欠压保�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci((uint16_t)wg_com_v2_param.SetOutUvloRecover));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_OVER:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutOVP);                         // B端欠压保�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci((uint16_t)wg_com_v2_param.SetOutOVP));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_OVER_R:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutOVPRecover);                  // B端欠压保�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci((uint16_t)wg_com_v2_param.SetOutOVPRecover));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case OVER_TEMPERATURE:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_param.SetInsideTemp);                       // 内部温度
                    data_val = can_get_u16_be(data, 4);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_param.SetOutsideTemp);                      // 外部温度
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.SetInsideTemp));
                    can_put_u16_be(data, 4, (uint16_t)(wg_com_v2_param.SetOutsideTemp));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_CHARGING_LIGHT:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpChargLedCurr);                // A端充电指示灯电流
                    f_data_val = (can_get_u16_be(data, 4))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpFullLedCurr);                 // A端充满指示灯电流
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci((uint16_t)wg_com_v2_param.SetInpChargLedCurr));
                    can_put_u16_be(data, 4, can_centi_to_deci((uint16_t)wg_com_v2_param.SetInpFullLedCurr));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_CHARGING_LIGHT:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutChargLedCurr);                // B端充电指示灯电流
                    f_data_val = (can_get_u16_be(data, 4))/10.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutFullLedCurr);                 // B端充满指示灯电流
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci((uint16_t)wg_com_v2_param.SetOutChargLedCurr));
                    can_put_u16_be(data, 4, can_centi_to_deci((uint16_t)wg_com_v2_param.SetOutFullLedCurr));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_FORWARD_OPEN:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.AuotForwardOpenVoltA);              // 自动模式正向A端开启电�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.AuotForwardOpenVoltA));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_FORWARD_VEER:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.AuotForwardVeerVoltA);              // 自动模式正向转向A电压
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.AuotForwardVeerVoltA));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_FORWARD_SHUT:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.AuotForwardShutVoltA);              // 自动模式正向A端关闭电�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.AuotForwardShutVoltA));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_REVERSE_OPEN:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.AuotReverseOpenVoltB);              // 自动模式反向B端开启电�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.AuotReverseOpenVoltB));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_REVERSE_SHUT:
                if(data[1] == 0x88){
                    f_data_val = (can_get_u16_be(data, 2))/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.AuotReverseShutVoltB);              // 自动模式反向B端关闭电�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.AuotReverseShutVoltB));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case SET_TEMP_INNER:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_param.SetTemp2);                            // 内部温度
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, (uint16_t)(wg_com_v2_param.SetTemp2));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case MOSFET_CONTROL_300A:
                if(data[1] == 0x88){
                    if((data[2] == 0x11)&&(data[3] == 0x11))
                    {
                        mos_on_off_G300(1);
                    }else if((data[2] == 0x00)&&(data[3] == 0x00)){
                        mos_on_off_G300(0);
                    }
                }else if(data[1] == 0x22){
                    if(mos_g300_flag == 1)
                    {
                        data[2] = 0x11;
                        data[3] = 0x11;
                        data[4] = 0;                                                                            // 保留
                        data[5] = 0;                                                                            // 保留
                        data[6] = 0;                                                                            // 保留
                        data[7] = 0;                                                                            // 保留
                    }else{
                        data[2] = 0;                                                                            // 保留
                        data[3] = 0;                                                                            // 保留
                        data[4] = 0;                                                                            // 保留
                        data[5] = 0;                                                                            // 保留
                        data[6] = 0;                                                                            // 保留
                        data[7] = 0;                                                                            // 保留
                    }
                }
                break;
            case MOSFET_CONTROL_150A:
                if(data[1] == 0x88){
                    if((data[2] == 0x11)&&(data[3] == 0x11))
                    {
                        mos_on_off_G150(1);
                    }else if((data[2] == 0x00)&&(data[3] == 0x00)){
                        mos_on_off_G150(0);
                    }
                }else if(data[1] == 0x22){
                    if(mos_g150_flag == 1)
                    {
                        data[2] = 0x11;
                        data[3] = 0x11;
                        data[4] = 0;                                                                            // 保留
                        data[5] = 0;                                                                            // 保留
                        data[6] = 0;                                                                            // 保留
                        data[7] = 0;                                                                            // 保留
                    }else{
                        data[2] = 0;                                                                            // 保留
                        data[3] = 0;                                                                            // 保留
                        data[4] = 0;                                                                            // 保留
                        data[5] = 0;                                                                            // 保留
                        data[6] = 0;                                                                            // 保留
                        data[7] = 0;                                                                            // 保留
                    }
                }
                break;
            default:
                return;
        }
        bsp_can_tx(0x580, data);          
    }
}



