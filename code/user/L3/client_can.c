#include "client_can.h"
#include "section.h"
#include "bsp_can.h"
#include "get_com_data.h"
#include "wg_com_v2.h"
#include "gpio.h"
#include "eeprom_cfg.h"

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

static uint16_t can_get_battery_type_value(const uint8_t *data, uint8_t offset)
{
    return can_get_u16_be(data, offset);
}

static void can_put_u16_be(uint8_t *data, uint8_t offset, uint16_t value)
{
    data[offset] = (uint8_t)((value >> 8) & 0xff);
    data[offset + 1] = (uint8_t)(value & 0xff);
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
static uint16_t can_read_u16_field(const void *field)
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

#define CANSTACK_DEFAULT_ADDRESS 1U
#define CANSTACK_ADDRESS_MAX 31U

static uint8_t canstack_selected_address = CANSTACK_DEFAULT_ADDRESS;

static uint8_t canstack_normalize_address(uint16_t address)
{
    if(address > 255U)
    {
        address >>= 8U;
    }
    if((address == 0U) || (address > CANSTACK_ADDRESS_MAX))
    {
        return CANSTACK_DEFAULT_ADDRESS;
    }

    return (uint8_t)address;
}

static uint8_t canstack_current_address(void)
{
    uint16_t address = can_read_u16_field(&wg_com_v2_product_info.Address);

    return canstack_normalize_address(address);
}

static uint8_t canstack_target_is_selected(void)
{
    return canstack_current_address() == canstack_selected_address ? 1U : 0U;
}

static void canstack_handle_address_select(const uint8_t *data)
{
    uint16_t address = can_get_u16_be(data, 2);

    canstack_selected_address = canstack_normalize_address(address);
}

static uint8_t canstack_part_is_valid(uint8_t part)
{
    return (part < 4U) ? 1U : 0U;
}

static uint8_t canstack_read_text_byte(const uint16_t *src, uint8_t index)
{
    uint16_t word = can_read_u16_field(&src[index / 2U]);

    return ((index & 1U) == 0U) ? (uint8_t)((word >> 8) & 0xFFU) : (uint8_t)(word & 0xFFU);
}

static void canstack_write_text_byte(uint16_t *dst, uint8_t index, uint8_t value)
{
    uint16_t word = can_read_u16_field(&dst[index / 2U]);

    if((index & 1U) == 0U)
    {
        word = (uint16_t)((word & 0x00FFU) | ((uint16_t)value << 8));
    }
    else
    {
        word = (uint16_t)((word & 0xFF00U) | value);
    }
    WG_COM_V2_SET_DATA_UINT(word, dst[index / 2U]);
}

static void canstack_read_text_part(uint8_t *data, const uint16_t *src, uint8_t total_len)
{
    uint8_t part = data[2];
    uint8_t offset = (uint8_t)(part * 5U);
    uint8_t i;

    if((src == 0) || (canstack_part_is_valid(part) == 0U))
    {
        return;
    }

    for(i = 0U; i < 5U; i++)
    {
        uint8_t index = (uint8_t)(offset + i);
        data[3U + i] = (index < total_len) ? canstack_read_text_byte(src, index) : 0x20U;
    }
}

static void canstack_write_text_part(uint16_t *dst, uint8_t total_len, const uint8_t *data)
{
    uint8_t part = data[2];
    uint8_t offset = (uint8_t)(part * 5U);
    uint8_t i;

    if((dst == 0) || (canstack_part_is_valid(part) == 0U))
    {
        return;
    }

    for(i = 0U; i < 5U; i++)
    {
        uint8_t index = (uint8_t)(offset + i);
        if(index < total_len)
        {
            canstack_write_text_byte(dst, index, data[3U + i]);
        }
    }
}

static uint8_t canstack_is_last_text_part(uint8_t total_len, const uint8_t *data)
{
    uint8_t part_count;

    if((data == 0) || (total_len == 0U))
    {
        return 0U;
    }

    part_count = (uint8_t)(((uint16_t)total_len + 4U) / 5U);
    return (data[2] == (uint8_t)(part_count - 1U)) ? 1U : 0U;
}

static void canstack_commit_product_info(void)
{
    (void)eeprom_commit_current_pages_for_range(WG_COM_V2_PRUCUCT_INFO_ADDR,
                                                (uint16_t)(sizeof(wg_com_v2_product_info_t) / 2U));
}

static uint8_t canstack_calibration_item_to_addr(uint8_t item, uint16_t *addr)
{
    if((addr == 0) || (item > 11U))
    {
        return 0U;
    }

    *addr = (uint16_t)(WG_COM_V2_PARAM_ADDR + ((uint16_t)item * 2U));
    return 1U;
}

static uint16_t canstack_read_calibration_word(uint16_t addr)
{
    uint16_t offset = (uint16_t)(addr - WG_COM_V2_PARAM_ADDR);

    return get_uint16(((uint8_t *)&wg_com_v2_param) + ((uint32_t)offset * 2U));
}

static uint16_t canstack_get_calibration_current_value(uint8_t item)
{
    switch(item)
    {
        case 0U: return get_uint16((uint8_t *)&wg_com_v2_param.SetInpVolt);
        case 1U: return get_uint16((uint8_t *)&wg_com_v2_param.SetInpCurr);
        case 2U: return get_uint16((uint8_t *)&wg_com_v2_realtime_data.InpVolt);
        case 3U: return get_uint16((uint8_t *)&wg_com_v2_realtime_data.InpCurr);
        case 4U: return get_uint16((uint8_t *)&wg_com_v2_param.SetOutVolt);
        case 5U: return get_uint16((uint8_t *)&wg_com_v2_param.SetOutCurr);
        case 6U: return get_uint16((uint8_t *)&wg_com_v2_realtime_data.OutVolt);
        case 7U: return get_uint16((uint8_t *)&wg_com_v2_realtime_data.OutCurr);
        case 8U: return get_uint16((uint8_t *)&wg_com_v2_realtime_data.OutCurr);
        case 9U: return get_uint16((uint8_t *)&wg_com_v2_realtime_data.InpCurr);
        case 10U: return get_uint16((uint8_t *)&wg_com_v2_realtime_data.CompensationVoltA);
        case 11U: return get_uint16((uint8_t *)&wg_com_v2_realtime_data.CompensationVoltB);
        default: return 0xFFFFU;
    }
}

static void canstack_handle_calibration(uint8_t *data)
{
    uint8_t op = (data[1] == 0x88U) ? 1U : 0U;
    uint8_t item = data[2];
    uint8_t status = 0U;
    uint16_t addr = 0U;
    uint16_t value = 0U;
    uint16_t current_value = 0xFFFFU;

    if(canstack_calibration_item_to_addr(item, &addr) == 0U)
    {
        status = 1U;
    }
    else if(op == 1U)
    {
        value = can_get_u16_be(data, 3);
        if(can_write_register_u16(addr, value) == 0U)
        {
            status = 2U;
        }
    }

    if(status == 0U)
    {
        value = canstack_read_calibration_word(addr);
        current_value = canstack_get_calibration_current_value(item);
    }

    data[1] = op;
    data[2] = item;
    data[3] = status;
    can_put_u16_be(data, 4, value);
    can_put_u16_be(data, 6, current_value);
}

static void canstack_handle_app_debug(uint8_t *data)
{
#if (APP_DEBUG_EVENT_FEATURES == 1)
    uint16_t reg_offset = (uint16_t)(((uint16_t)data[2] * 5U) / 2U);
    uint8_t raw[6] = {0};
    uint8_t byte_offset = (uint8_t)(((uint16_t)data[2] * 5U) % 2U);

    app_debug_event_read_regs(reg_offset, 3U, raw);
    data[3] = raw[byte_offset + 0U];
    data[4] = raw[byte_offset + 1U];
    data[5] = raw[byte_offset + 2U];
    data[6] = raw[byte_offset + 3U];
    data[7] = raw[byte_offset + 4U];
#else
    data[3] = 0xFFU;
    data[4] = 0xFFU;
    data[5] = 0xFFU;
    data[6] = 0xFFU;
    data[7] = 0xFFU;
#endif
}
void Get_CAN_Communications_Content (uint32_t can_id, uint8_t *data, uint8_t len)
{
    // 提取 DGN
    uint16_t data_val = 0;
    if((can_id != 0x600) || (len != 8)){return;}
    if(data[0] == CANSTACK_ADDRESS_SELECT)
    {
        canstack_handle_address_select(data);
        return;
    }
    if(((data[0] == POWER_STATUS) && (data[1] == 0x22U)) == 0U)
    {
        if(canstack_target_is_selected() == 0U)
        {
            return;
        }
    }
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
                can_put_u16_be(data, 6, can_read_u16_field(&wg_com_v2_realtime_data.ADDVolt));
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
                    if((mppt_switch == 0U) && (old_power_mode == eSET_STANDARD_MODE))
                    {
                        direction = 1U;
                    }
                    if((mppt_switch != 0U) ||
                       (old_power_mode != eSET_BAT_MODE) ||
                       ((old_mppt_switch == 1U) && (mppt_switch == 0U)))
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
                    (void)can_write_register_u16(0x0401U, data_val);                           // 开关机状�?
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
                    data_val = can_get_battery_type_value(data, 2);
                    (void)can_write_register_u16(0x0404U, data_val);                          // A端电池类型，高位类型，低位电�?
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
                    data_val = can_get_battery_type_value(data, 2);
                    (void)can_write_register_u16(0x0405U, data_val);                          // B端电池类型，高位类型，低位电�?
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
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpVolt, can_get_u16_be(data, 2));                        // 设置A端电�?
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
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpCurr, can_get_u16_be(data, 2));                        // A端电�?
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
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpCurrPower, can_get_u16_be(data, 2));                     // A端功�?
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
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutVolt, can_get_u16_be(data, 2));                        // B端电�?
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
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutCurr, can_get_u16_be(data, 2));                        // B端电�?
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
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutCurrPower, can_get_u16_be(data, 2));                     // B端功�?
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
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpUvlo, can_get_u16_be(data, 2));                        // A端欠压保�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetInpUvlo));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_UNDER_R:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpUvloRecover, can_get_u16_be(data, 2));                 // A端欠压保护恢�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetInpUvloRecover));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_OVER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpOVP, can_get_u16_be(data, 2));                         // A端过压保�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetInpOVP));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_A_OVER_R:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpOVPRecover, can_get_u16_be(data, 2));                  // A端过压保护恢�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetInpOVPRecover));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_UNDER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutUvlo, can_get_u16_be(data, 2));                        // B端欠压保�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetOutUvlo));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_UNDER_R:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutUvloRecover, can_get_u16_be(data, 2));                 // B端欠压保护恢�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetOutUvloRecover));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_OVER:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutOVP, can_get_u16_be(data, 2));                         // B端过压保�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetOutOVP));
                    data[4] = 0;                                                                            // 保留
                    data[5] = 0;                                                                            // 保留
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_OVER_R:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutOVPRecover, can_get_u16_be(data, 2));                  // B端过压保护恢�?
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetOutOVPRecover));
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
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpChargLedCurr, can_get_u16_be(data, 2));                // A端充电指示灯电流
                    (void)can_write_param_u16(&wg_com_v2_param.SetInpFullLedCurr, can_get_u16_be(data, 4));                 // A端充满指示灯电流
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetInpChargLedCurr));
                    can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_param.SetInpFullLedCurr));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case TERMINAL_B_CHARGING_LIGHT:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutChargLedCurr, can_get_u16_be(data, 2));                // B端充电指示灯电流
                    (void)can_write_param_u16(&wg_com_v2_param.SetOutFullLedCurr, can_get_u16_be(data, 4));                 // B端充满指示灯电流
                }else if(data[1] == 0x22){
                    can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_param.SetOutChargLedCurr));
                    can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_param.SetOutFullLedCurr));
                    data[6] = 0;                                                                            // 保留
                    data[7] = 0;                                                                            // 保留
                }
                break;
            case AUTO_CHARGE_FORWARD_OPEN:
                if(data[1] == 0x88){
                    (void)can_write_param_u16(&wg_com_v2_param.AuotForwardOpenVoltA, can_get_u16_be(data, 2));              // 自动模式正向A端开启电�?
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
                    (void)can_write_param_u16(&wg_com_v2_param.AuotForwardShutVoltA, can_get_u16_be(data, 2));              // 自动模式正向A端关闭电�?
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
                    (void)can_write_param_u16(&wg_com_v2_param.AuotReverseOpenVoltB, can_get_u16_be(data, 2));              // 自动模式反向B端开启电�?
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
                    (void)can_write_param_u16(&wg_com_v2_param.AuotReverseShutVoltB, can_get_u16_be(data, 2));              // 自动模式反向B端关闭电�?
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
            case MFG_PROTOCOL_VERSION:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_product_info.ProtocolVersion[0]));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_product_info.ProtocolVersion[1]));
                data[6] = 0;
                data[7] = 0;
                break;
            case MFG_PRODUCT_TYPE:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_product_info.ProductType[0]));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_product_info.ProductType[1]));
                data[6] = 0;
                data[7] = 0;
                break;
            case MFG_HARDVER_VERSION:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_product_info.HardverVerzi[0]));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_product_info.HardverVerzi[1]));
                data[6] = 0;
                data[7] = 0;
                break;
            case MFG_SOFT_VERSION:
                if(data[1] != 0x22){return;}
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_product_info.SoftVersion[0]));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_product_info.SoftVersion[1]));
                data[6] = 0;
                data[7] = 0;
                break;
            case MFG_ADDRESS_SCENARIO:
                if(data[1] == 0x88){
                    WG_COM_V2_SET_DATA_UINT(canstack_normalize_address(can_get_u16_be(data, 2)), wg_com_v2_product_info.Address);
                    WG_COM_V2_SET_DATA_UINT(can_get_u16_be(data, 4), wg_com_v2_product_info.ApplicationScenarios);
                    canstack_selected_address = canstack_current_address();
                    canstack_commit_product_info();
                }
                can_put_u16_be(data, 2, canstack_current_address());
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_product_info.ApplicationScenarios));
                data[6] = 0;
                data[7] = 0;
                break;
            case MFG_CUSTOM_BT_NAME:
                if(data[1] == 0x88){
                    WG_COM_V2_SET_DATA_UINT(can_get_u16_be(data, 2), wg_com_v2_product_info.CustomizationVersion);
                    WG_COM_V2_SET_DATA_UINT(can_get_u16_be(data, 4), wg_com_v2_product_info.BtName);
                    canstack_commit_product_info();
                }
                can_put_u16_be(data, 2, can_read_u16_field(&wg_com_v2_product_info.CustomizationVersion));
                can_put_u16_be(data, 4, can_read_u16_field(&wg_com_v2_product_info.BtName));
                data[6] = 0;
                data[7] = 0;
                break;
            case MFG_SN_SERIAL_PART:
                if(data[1] == 0x88){
                    canstack_write_text_part(&wg_com_v2_product_info.SnSerial[0], 20U, data);
                    if(canstack_is_last_text_part(20U, data) != 0U)
                    {
                        canstack_commit_product_info();
                    }
                }
                canstack_read_text_part(data, &wg_com_v2_product_info.SnSerial[0], 20U);
                break;
            case MFG_PRODUCT_NAME_PART:
                if(data[1] == 0x88){
                    canstack_write_text_part(&wg_com_v2_product_info.ProductName[0], 20U, data);
                    if(canstack_is_last_text_part(20U, data) != 0U)
                    {
                        canstack_commit_product_info();
                    }
                }
                canstack_read_text_part(data, &wg_com_v2_product_info.ProductName[0], 20U);
                break;
            case MFG_MAC_ADDRESS_PART:
                if(data[1] == 0x88){
                    canstack_write_text_part(&wg_com_v2_product_info.MacAddress[0], 20U, data);
                    if(canstack_is_last_text_part(20U, data) != 0U)
                    {
                        canstack_commit_product_info();
                    }
                }
                canstack_read_text_part(data, &wg_com_v2_product_info.MacAddress[0], 20U);
                break;
            case APP_DEBUG_EVENT_PART:
                if(data[1] != 0x22){return;}
                canstack_handle_app_debug(data);
                break;
            case CANSTACK_CALIBRATION:
                canstack_handle_calibration(data);
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



