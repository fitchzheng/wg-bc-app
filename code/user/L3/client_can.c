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
    return (uint16_t)(((uint16_t)data[offset + 1] << 8) | data[offset]);
}

static void can_put_u16_be(uint8_t *data, uint8_t offset, uint16_t value)
{
    data[offset] = (uint8_t)(value & 0xff);
    data[offset + 1] = (uint8_t)((value >> 8) & 0xff);
}

static uint16_t can_centi_to_deci(uint16_t value)
{
    return (uint16_t)((value + 5U) / 10U);
}


static uint8_t can_write_register_u16(uint16_t addr, uint16_t value)
{
    uint8_t reg_data[2];

    set_uint16(reg_data, value);
    return wg_com_v2_write_registers(addr, 1U, reg_data);
}

static uint8_t can_write_registers_u16(uint16_t addr, uint16_t value0, uint16_t value1)
{
    uint8_t reg_data[4];

    set_uint16(&reg_data[0], value0);
    set_uint16(&reg_data[2], value1);
    return wg_com_v2_write_registers(addr, 2U, reg_data);
}

static uint8_t can_write_registers3_u16(uint16_t addr, uint16_t value0, uint16_t value1, uint16_t value2)
{
    uint8_t reg_data[6];

    set_uint16(&reg_data[0], value0);
    set_uint16(&reg_data[2], value1);
    set_uint16(&reg_data[4], value2);
    return wg_com_v2_write_registers(addr, 3U, reg_data);
}
static uint16_t can_read_u16_field(void *field)
{
    return get_uint16((uint8_t *)field);
}

static uint16_t can_field_register_addr(void *base, void *field, uint16_t base_addr)
{
    return (uint16_t)(base_addr + (((uint8_t *)field - (uint8_t *)base) / 2U));
}

static uint8_t can_write_param_u16(void *field, uint16_t value)
{
    return can_write_register_u16(can_field_register_addr(&wg_com_v2_param, field, WG_COM_V2_PARAM_ADDR), value);
}

static uint16_t can_deci_to_centi(uint16_t value)
{
    if(value > 6553U)
    {
        return 65535U;
    }
    return (uint16_t)(value * 10U);
}

void Get_CAN_Communications_Content (uint32_t can_id, uint8_t *data, uint8_t len)
{
    // 提取 DGN
    uint16_t data_val = 0;
    if((can_id != 0x600) || (len != 8)){return;}
    if((data[1] == 0x22) || (data[1] == 0x88)){
        switch(data[0]){
            case A_VOLT_CURR_POWER:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_realtime_data.InpVolt));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_realtime_data.InpCurr));
                can_put_u16_be(data, 6, can_read_u16_field(&wg_com_v2_realtime_data.InpCurrPower));
                break;
            case B_VOLT_CURR_POWER:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_realtime_data.OutVolt));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_realtime_data.OutCurr));
                can_put_u16_be(data, 6, can_read_u16_field(&wg_com_v2_realtime_data.OutCurrPower));
                break;
            case TEMP_READINGS:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_realtime_data.InsideTemp));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_realtime_data.OutsideTemp));
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;
            case POWER_CHARGING_MODE:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_realtime_data.PowerMode));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_realtime_data.ChargMode));
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;
            case FAULT_ALARM_SIGNALS:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_realtime_data.FaultSign));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_realtime_data.AlarmSign));
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;
            case AB_COMPEN_TEMP:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_realtime_data.CompensationVoltA));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_realtime_data.CompensationVoltB));
                can_put_u16_be(data, 6, can_read_u16_field(&wg_com_v2_realtime_data.Temp2));
                break;
            case CHARGING_STATUS:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_realtime_data.StateCharge));
                data[4] = 0;                                                                                // 保留
                data[5] = 0;                                                                                // 保留
                data[6] = 0;                                                                                // 保留
                data[7] = 0;                                                                                // 保留
                break;

            case FACTORY_RESET:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    (void)can_write_register_u16(0x0400U, data_val);                         // 恢复出厂设置
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.FactoryReset));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case FACTORY_SAVE:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    (void)can_write_register_u16(0x040BU, data_val);
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.ResetFactoryData));
                    data[4] = 0;
                    data[5] = 0;
                    data[6] = 0;
                    data[7] = 0;
                }
                break;
            case ZERO_CURR_CALIBRATION:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    (void)can_write_register_u16(0x040AU, data_val);
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.ZeroCurrCalibration));
                    data[4] = 0;
                    data[5] = 0;
                    data[6] = 0;
                    data[7] = 0;
                }
                break;
            case MODE_CONTROL_STATE:
                if(data[1] == 0x88){
                    uint16_t direction = can_get_u16_be(data, 2);
                    uint16_t mppt_switch = can_get_u16_be(data, 4);
                    uint16_t sleep_mode = can_get_u16_be(data, 6);
                    uint16_t old_power_mode = can_read_u16_field(&wg_com_v2_ctrl.SetPowerMode);
                    uint16_t old_mppt_switch = can_read_u16_field(&wg_com_v2_ctrl.MpptSwitch);

                    if(mppt_switch == 1U)
                    {
                        direction = 1U;
                    }
                    if((old_power_mode == eSET_STANDARD_MODE) && (old_mppt_switch == 1U) && (mppt_switch == 0U))
                    {
                        direction = 1U;
                    }
                    if(old_power_mode != eSET_BAT_MODE)
                    {
                        sleep_mode = 0U;
                    }

                    (void)can_write_registers3_u16(0x040CU, direction, mppt_switch, sleep_mode);
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.BatModeFR));
                    can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_ctrl.MpptSwitch));
                    can_put_u16_be(data, 6, can_read_u16_field(&wg_com_v2_ctrl.SleepModeOnOff));
                }
                break;
            case POWER_STATUS:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    (void)can_write_register_u16(0x0401U, data_val);                           // 开关机状态
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.PowerOnOff));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case POWER_MODE:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    if(data_val < eSET_MODE_MAX)
                                        {
                        (void)can_write_register_u16(0x0402U, data_val);                         // 电源模式
                    }
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.SetPowerMode));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case CHARGING_MODE:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    (void)can_write_register_u16(0x0403U, data_val);                         // 充电模式
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.SetChargMode));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case A_BATTERY_TYPE:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    (void)can_write_register_u16(0x0404U, data_val);                          // A端电池类型，高位类型，低位电压
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.InpBatyType));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case B_BATTERY_TYPE:
                if(data[1] == 0x88){
                    data_val = can_get_u16_be(data, 2);
                    (void)can_write_register_u16(0x0405U, data_val);                          // B端电池类型，高位类型，低位电压
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.OutBatyType));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case BOOT_TIME_DELAY:
                if(data[1] == 0x88){
                    (void)can_write_registers_u16(0x0406U,
                                                  can_get_u16_be(data, 2),
                                                  can_get_u16_be(data, 4));
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.SetBootTimeA));
                    can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_ctrl.SetBootTimeB));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case SOFT_START_TIME_DELAY:
                if(data[1] == 0x88){
                    (void)can_write_registers_u16(0x0408U,
                                                  can_get_u16_be(data, 2),
                                                  can_get_u16_be(data, 4));
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_ctrl.SetOnCurrStartTimeA));
                    can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_ctrl.SetOnCurrStartTimeB));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_VOLTAGE:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpVolt, can_get_u16_be(data, 2));                        // 设置A端电压
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetInpVolt));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_CURRENT:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpCurr, can_get_u16_be(data, 2));                        // A端电流
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetInpCurr));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_POWER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpCurrPower, can_get_u16_be(data, 2));                     // A端功率
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetInpCurrPower));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_VOLT:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutVolt, can_get_u16_be(data, 2));                        // B端电压
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetOutVolt));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_CURR:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutCurr, can_get_u16_be(data, 2));                        // B端电流
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetOutCurr));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_POWER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutCurrPower, can_get_u16_be(data, 2));                     // B端功率
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetOutCurrPower));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_UNDER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpUvlo, can_deci_to_centi(can_get_u16_be(data, 2)));                        // A端欠压保护
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetInpUvlo)));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_UNDER_R:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpUvloRecover, can_deci_to_centi(can_get_u16_be(data, 2)));                 // A端欠压保护恢复
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetInpUvloRecover)));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_OVER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpOVP, can_deci_to_centi(can_get_u16_be(data, 2)));                         // A端过压保护
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetInpOVP)));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_OVER_R:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpOVPRecover, can_deci_to_centi(can_get_u16_be(data, 2)));                  // A端过压保护恢复
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetInpOVPRecover)));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_UNDER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutUvlo, can_deci_to_centi(can_get_u16_be(data, 2)));                        // B端欠压保护
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetOutUvlo)));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_UNDER_R:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutUvloRecover, can_deci_to_centi(can_get_u16_be(data, 2)));                 // B端欠压保护恢复
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetOutUvloRecover)));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_OVER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutOVP, can_deci_to_centi(can_get_u16_be(data, 2)));                         // B端过压保护
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetOutOVP)));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_OVER_R:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutOVPRecover, can_deci_to_centi(can_get_u16_be(data, 2)));                  // B端过压保护恢复
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetOutOVPRecover)));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case OVER_TEMPERATURE:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInsideTemp, can_get_u16_be(data, 2));                       // 内部温度
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutsideTemp, can_get_u16_be(data, 4));                      // 外部温度
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetInsideTemp));
                    can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_param.SetOutsideTemp));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_CHARGING_LIGHT:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpChargLedCurr, can_deci_to_centi(can_get_u16_be(data, 2)));                // A端充电指示灯电流
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpFullLedCurr, can_deci_to_centi(can_get_u16_be(data, 4)));                 // A端充满指示灯电流
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetInpChargLedCurr)));
                    can_put_u16_be(data, 4, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetInpFullLedCurr)));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_CHARGING_LIGHT:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutChargLedCurr, can_deci_to_centi(can_get_u16_be(data, 2)));                // B端充电指示灯电流
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutFullLedCurr, can_deci_to_centi(can_get_u16_be(data, 4)));                 // B端充满指示灯电流
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetOutChargLedCurr)));
                    can_put_u16_be(data, 4, can_centi_to_deci(can_read_u16_field(&wg_com_v2_param.SetOutFullLedCurr)));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_FORWARD_OPEN:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.AuotForwardOpenVoltA, can_get_u16_be(data, 2));              // 自动模式正向A端开启电压
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.AuotForwardOpenVoltA));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_FORWARD_VEER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.AuotForwardVeerVoltA, can_get_u16_be(data, 2));              // 自动模式正向转向A电压
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.AuotForwardVeerVoltA));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_FORWARD_SHUT:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.AuotForwardShutVoltA, can_get_u16_be(data, 2));              // 自动模式正向A端关闭电压
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.AuotForwardShutVoltA));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_REVERSE_OPEN:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.AuotReverseOpenVoltB, can_get_u16_be(data, 2));              // 自动模式反向B端开启电压
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.AuotReverseOpenVoltB));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_REVERSE_SHUT:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.AuotReverseShutVoltB, can_get_u16_be(data, 2));              // 自动模式反向B端关闭电压
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.AuotReverseShutVoltB));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case SET_TEMP_INNER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetTemp2, can_get_u16_be(data, 2));                            // 内部温度
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetTemp2));
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



