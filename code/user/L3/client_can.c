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
    bsp_rvc_can_tx(0x580, data, 8);
    
}

//REG_TASK(500, send_data)


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
                data[2] = ( wg_com_v2_realtime_data.InpVolt&0xff);                                          // A端电压
                data[3] = (((wg_com_v2_realtime_data.InpVolt)>>8)&0xff);                                    // A端电压
                data[4] = ( wg_com_v2_realtime_data.InpCurr&0xff);                                          // A端电流
                data[5] = (((wg_com_v2_realtime_data.InpCurr)>>8)&0xff);                                    // A端电流
                data[6] = ( wg_com_v2_realtime_data.InpCurrPower&0xff);                                     // A端功率
                data[7] = (((wg_com_v2_realtime_data.InpCurrPower)>>8)&0xff);                               // A端功率
                break;
            case B_VOLT_CURR_POWER:
                if(data[1] != 0x22){return;}
                data[2] = ( wg_com_v2_realtime_data.OutVolt&0xff);                                          // B端电压
                data[3] = (((wg_com_v2_realtime_data.OutVolt)>>8)&0xff);                                    // B端电压
                data[4] = ( wg_com_v2_realtime_data.OutCurr&0xff);                                          // B端电流
                data[5] = (((wg_com_v2_realtime_data.OutCurr)>>8)&0xff);                                    // B端电流
                data[6] = ( wg_com_v2_realtime_data.OutCurrPower&0xff);                                     // B端功率
                data[7] = (((wg_com_v2_realtime_data.OutCurrPower)>>8)&0xff);                               // B端功率
                break;
            case TEMP_READINGS:
                if(data[1] != 0x22){return;}
                data[2] = ( wg_com_v2_realtime_data.InsideTemp&0xff);                                       // 内部温度
                data[3] = (((wg_com_v2_realtime_data.InsideTemp)>>8)&0xff);                                 // 内部温度
                data[4] = ( wg_com_v2_realtime_data.OutsideTemp&0xff);                                      // 外部温度
                data[5] = (((wg_com_v2_realtime_data.OutsideTemp)>>8)&0xff);                                // 外部温度
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;
            case POWER_CHARGING_MODE:
                if(data[1] != 0x22){return;}
                data[2] = ( wg_com_v2_realtime_data.PowerMode&0xff);                                        // 电源模式
                data[3] = (((wg_com_v2_realtime_data.PowerMode)>>8)&0xff);                                  // 电源模式
                data[4] = ( wg_com_v2_realtime_data.ChargMode&0xff);                                        // 充电模式
                data[5] = (((wg_com_v2_realtime_data.ChargMode)>>8)&0xff);                                  // 充电模式
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;
            case FAULT_ALARM_SIGNALS:
                if(data[1] != 0x22){return;}
                data[2] = ( wg_com_v2_realtime_data.FaultSign&0xff);                                        // 故障信号
                data[3] = (((wg_com_v2_realtime_data.FaultSign)>>8)&0xff);                                  // 故障信号
                data[4] = ( wg_com_v2_realtime_data.AlarmSign&0xff);                                        // 告警信号
                data[5] = (((wg_com_v2_realtime_data.AlarmSign)>>8)&0xff);                                  // 告警信号
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;
            case AB_COMPEN_TEMP:
                if(data[1] != 0x22){return;}
                data[2] = ( wg_com_v2_realtime_data.CompensationVoltA&0xff);                                // A端补偿
                data[3] = (((wg_com_v2_realtime_data.CompensationVoltA)>>8)&0xff);                          // A端补偿
                data[4] = ( wg_com_v2_realtime_data.CompensationVoltB&0xff);                                // B端补偿
                data[5] = (((wg_com_v2_realtime_data.CompensationVoltB)>>8)&0xff);                          // B端补偿
                data[6] = ( wg_com_v2_realtime_data.Temp2&0xff);                                            // 器件温度2
                data[7] = (((wg_com_v2_realtime_data.Temp2)>>8)&0xff);                                      // 器件温度2                                                                               // 保留
                break;
            case CHARGING_STATUS:
                if(data[1] != 0x22){return;}
                data[2] = ( wg_com_v2_realtime_data.StateCharge&0xff);                                      // 充电状态
                data[3] = (((wg_com_v2_realtime_data.StateCharge)>>8)&0xff);                                // 充电状态
                data[4] = 0;                                                                                // 保留
                data[5] = 0;                                                                                // 保留
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;

            case FACTORY_RESET:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.FactoryReset);                         // 恢复出厂设置
                }else if(data[1] == 0x22){
                    data[2] = ( wg_com_v2_ctrl.FactoryReset&0xff);                                          // 恢复出厂设置
                    data[3] = (((wg_com_v2_ctrl.FactoryReset)>>8)&0xff);                                    // 恢复出厂设置
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case POWER_STATUS:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.PowerOnOff);                           // 开关机状态
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_ctrl.PowerOnOff&0xff);                                             // 开关机状态
                    data[3] = (((wg_com_v2_ctrl.PowerOnOff)>>8)&0xff);                                      // 开关机状态
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case POWER_MODE:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    if((data_val == eSET_BAT_MODE)  // 电池模式
                    || (data_val == eMPPT_MODE))
                    {
                        WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetPowerMode);                         // 电源模式
                    }
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_ctrl.SetPowerMode&0xff);                                           // 电源模式
                    data[3] = (((wg_com_v2_ctrl.SetPowerMode)>>8)&0xff);                                    // 电源模式
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case CHARGING_MODE:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetChargMode);                         // 充电模式
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_ctrl.SetChargMode&0xff);                                           // 充电模式
                    data[3] = (((wg_com_v2_ctrl.SetChargMode)>>8)&0xff);                                    // 充电模式
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case A_BATTERY_TYPE:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.InpBatyType);                          // A端电池类型(高8位类型，低8位电压)
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_ctrl.InpBatyType&0xff);                                            // A端电池类型(高8位类型，低8位电压)
                    data[3] = (((wg_com_v2_ctrl.InpBatyType)>>8)&0xff);                                     // A端电池类型(高8位类型，低8位电压)
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case B_BATTERY_TYPE:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.OutBatyType);                          // B端电池类型(高8位类型，低8位电压)
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_ctrl.OutBatyType&0xff);                                            // B端电池类型(高8位类型，低8位电压)
                    data[3] = (((wg_com_v2_ctrl.OutBatyType)>>8)&0xff);                                     // B端电池类型(高8位类型，低8位电压)
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case BOOT_TIME_DELAY:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetBootTimeA);                         // A端开机时间
                    data_val = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetBootTimeB);                         // B端开机时间
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_ctrl.SetBootTimeA&0xff);                                           // A端开机时间
                    data[3] = (((wg_com_v2_ctrl.SetBootTimeA)>>8)&0xff);                                    // A端开机时间
                    data[4] = (wg_com_v2_ctrl.SetBootTimeB&0xff);                                           // B端开机时间
                    data[5] = (((wg_com_v2_ctrl.SetBootTimeB)>>8)&0xff);                                    // B端开机时间
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case SOFT_START_TIME_DELAY:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetOnCurrStartTimeA);                  // 408: A端开机电流软起动时间
                    data_val = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_ctrl.SetOnCurrStartTimeB);                  // 409: B端开机电流软起动时间
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_ctrl.SetOnCurrStartTimeA&0xff);                                    // A端开机时间
                    data[3] = (((wg_com_v2_ctrl.SetOnCurrStartTimeA)>>8)&0xff);                             // A端开机时间
                    data[4] = (wg_com_v2_ctrl.SetOnCurrStartTimeB&0xff);                                    // B端开机时间
                    data[5] = (((wg_com_v2_ctrl.SetOnCurrStartTimeB)>>8)&0xff);                             // B端开机时间
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_VOLTAGE:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpVolt);                        // 设置A端电压
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetInpVolt&0xff);                                            // 设置A端电压
                    data[3] = (((wg_com_v2_param.SetInpVolt)>>8)&0xff);                                     // 设置A端电压
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_CURRENT:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpCurr);                        // A端电流
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetInpCurr&0xff);                                            // A端电流
                    data[3] = (((wg_com_v2_param.SetInpCurr)>>8)&0xff);                                     // A端电流
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_POWER:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_param.SetInpCurrPower);                     // A端功率
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetInpCurrPower&0xff);                                       // A端功率
                    data[3] = (((wg_com_v2_param.SetInpCurrPower)>>8)&0xff);                                // A端功率
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_VOLT:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutVolt);                        // B端电压
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetOutVolt&0xff);                                            // B端电压
                    data[3] = (((wg_com_v2_param.SetOutVolt)>>8)&0xff);                                     // B端电压
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_CURR:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutCurr);                        // B端电流
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetOutCurr&0xff);                                            // B端电流
                    data[3] = (((wg_com_v2_param.SetOutCurr)>>8)&0xff);                                     // B端电流
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_POWER:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_param.SetOutCurrPower);                     // B端功率
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetOutCurrPower&0xff);                                       // B端功率
                    data[3] = (((wg_com_v2_param.SetOutCurrPower)>>8)&0xff);                                // B端功率
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_UNDER:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpUvlo);                        // A端欠压保护
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetInpUvlo&0xff);                                            // A端欠压保护
                    data[3] = (((wg_com_v2_param.SetInpUvlo)>>8)&0xff);                                     // A端欠压保护
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_UNDER_R:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpUvloRecover);                 // A端欠压保护恢复
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetInpUvloRecover&0xff);                                     // A端欠压保护恢复
                    data[3] = (((wg_com_v2_param.SetInpUvloRecover)>>8)&0xff);                              // A端欠压保护恢复
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_OVER:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpOVP);                         // A端过压保护
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetInpOVP&0xff);                                             // A端过压保护
                    data[3] = (((wg_com_v2_param.SetInpOVP)>>8)&0xff);                                      // A端过压保护
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_OVER_R:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpOVPRecover);                  // A端过压保护恢复
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetInpOVPRecover&0xff);                                      // A端过压保护恢复
                    data[3] = (((wg_com_v2_param.SetInpOVPRecover)>>8)&0xff);                               // A端过压保护恢复
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_UNDER:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutUvlo);                        // B端欠压保护
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetOutUvlo&0xff);                                            // B端欠压保护
                    data[3] = (((wg_com_v2_param.SetOutUvlo)>>8)&0xff);                                     // B端欠压保护
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_UNDER_R:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutUvloRecover);                 // B端欠压保护
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetOutUvloRecover&0xff);                                     // B端欠压保护恢复
                    data[3] = (((wg_com_v2_param.SetOutUvloRecover)>>8)&0xff);                              // B端欠压保护恢复
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_OVER:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutOVP);                         // B端欠压保护
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetOutOVP&0xff);                                             // B端过压保护
                    data[3] = (((wg_com_v2_param.SetOutOVP)>>8)&0xff);                                      // B端过压保护
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_OVER_R:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutOVPRecover);                  // B端欠压保护
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetOutOVPRecover&0xff);                                      // B端过压保护恢复
                    data[3] = (((wg_com_v2_param.SetOutOVPRecover)>>8)&0xff);                               // B端过压保护恢复
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case OVER_TEMPERATURE:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_param.SetInsideTemp);                       // 内部温度
                    data_val = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_param.SetOutsideTemp);                      // 外部温度
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetInsideTemp&0xff);                                         // 内部温度
                    data[3] = (((wg_com_v2_param.SetInsideTemp)>>8)&0xff);                                  // 内部温度
                    data[4] = (wg_com_v2_param.SetOutsideTemp&0xff);                                        // 外部温度
                    data[5] = (((wg_com_v2_param.SetOutsideTemp)>>8)&0xff);                                 // 外部温度
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_CHARGING_LIGHT:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpChargLedCurr);                // A端充电指示灯电流
                    f_data_val = ((data[4]<<8)+data[5])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetInpFullLedCurr);                 // A端充满指示灯电流
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetInpChargLedCurr&0xff);                                    // A端充电指示灯电流
                    data[3] = (((wg_com_v2_param.SetInpChargLedCurr)>>8)&0xff);                             // A端充电指示灯电流
                    data[4] = (wg_com_v2_param.SetInpFullLedCurr&0xff);                                     // A端充满指示灯电流
                    data[5] = (((wg_com_v2_param.SetInpFullLedCurr)>>8)&0xff);                              // A端充满指示灯电流
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_CHARGING_LIGHT:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutChargLedCurr);                // B端充电指示灯电流
                    f_data_val = ((data[4]<<8)+data[5])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.SetOutFullLedCurr);                 // B端充满指示灯电流
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetOutChargLedCurr&0xff);                                    // B端充电指示灯电流
                    data[3] = (((wg_com_v2_param.SetOutChargLedCurr)>>8)&0xff);                             // B端充电指示灯电流
                    data[4] = (wg_com_v2_param.SetOutFullLedCurr&0xff);                                     // B端充满指示灯电流
                    data[5] = (((wg_com_v2_param.SetOutFullLedCurr)>>8)&0xff);                              // B端充满指示灯电流
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_FORWARD_OPEN:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.AuotForwardOpenVoltA);              // 自动模式正向A端开启电压
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.AuotForwardOpenVoltA&0xff);                                  // 自动模式正向A端开启电压
                    data[3] = (((wg_com_v2_param.AuotForwardOpenVoltA)>>8)&0xff);                           // 自动模式正向A端开启电压
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_FORWARD_VEER:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.AuotForwardVeerVoltA);              // 自动模式正向转向A电压
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.AuotForwardVeerVoltA&0xff);                                  // 自动模式正向转向A电压
                    data[3] = (((wg_com_v2_param.AuotForwardVeerVoltA)>>8)&0xff);                           // 自动模式正向转向A电压
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_FORWARD_SHUT:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.AuotForwardShutVoltA);              // 自动模式正向A端关闭电压
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.AuotForwardShutVoltA&0xff);                                  // 自动模式正向A端关闭电压
                    data[3] = (((wg_com_v2_param.AuotForwardShutVoltA)>>8)&0xff);                           // 自动模式正向A端关闭电压
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_REVERSE_OPEN:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.AuotReverseOpenVoltB);              // 自动模式反向B端开启电压
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.AuotReverseOpenVoltB&0xff);                                  // 自动模式反向B端开启电压
                    data[3] = (((wg_com_v2_param.AuotReverseOpenVoltB)>>8)&0xff);                           // 自动模式反向B端开启电压
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_REVERSE_SHUT:
                if(data[1] == 0x88){
                    f_data_val = ((data[2]<<8)+data[3])/100.00f;
                    WG_COM_V2_SET_DATA_UINT(f_data_val, wg_com_v2_param.AuotReverseShutVoltB);              // 自动模式反向B端关闭电压
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.AuotReverseShutVoltB&0xff);                                  // 自动模式反向B端关闭电压
                    data[3] = (((wg_com_v2_param.AuotReverseShutVoltB)>>8)&0xff);                           // 自动模式反向B端关闭电压
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case SET_TEMP_INNER:
                if(data[1] == 0x88){
                    data_val = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_val, wg_com_v2_param.SetTemp2);                            // 内部温度
                }else if(data[1] == 0x22){
                    data[2] = (wg_com_v2_param.SetTemp2&0xff);                                              // 内部温度
                    data[3] = (((wg_com_v2_param.SetTemp2)>>8)&0xff);                                       // 内部温度
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
        bsp_rvc_can_tx(0x580, data, len);
    }
}

