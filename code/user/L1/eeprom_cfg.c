#include "eeprom_cfg.h"
#include "section.h"
#include "wg_com_v2.h"
#include "string.h"
#include "ctrl_app.h"
#include "stdbool.h"
#include "bsp_gpio.h"
#include "get_com_data.h"
#include "fault.h"
#include "bsp_iic.h"
#include "flash.h"
#include "bsp_flash.h"
#include "gpio.h"
#include "adc_check.h"
#include "adc.h"
#include "shell.h"
#include "bat_charge_pattern.h"
#include "get_com_data.h"
#include "charge_control.h"
#include "bat_mode.h"
#include "basic_mode.h"
eeprom_wg_com_v2_param_t eeprom_wg_com_v2_param;
eeprom_wg_com_v2_param_t eeprom_backup_wg_com_v2_param;
uint8_t eeprom_page_read_data[EE_24CXX_PAGE_SIZE];
static uint8_t eeprom_page_verify_data[EE_24CXX_PAGE_SIZE];
static uint8_t eeprom_current_param_valid = 1;
static uint8_t eeprom_write_p02_current(void);
static uint8_t eeprom_write_p03_user_current(void);
static uint8_t eeprom_battery_profile_reload_pending = 0;
#define EEPROM_PROFILE_RESERVED_DEFAULT 0xFFU
#define EEPROM_PROFILE_RESERVED_USER    0xA5U
#define EEPROM_PROFILE_TIME_MAX         180U
#define EEPROM_MPPT_RETURN_MAGIC        0x4D52U
#define EEPROM_MPPT_RETURN_VERSION      2U
#define EEPROM_FACTORY_FLASH_PAGE          (FLASH_LOGICAL_PAGE_COUNT - 1U)
#define EEPROM_FACTORY_FLASH_MAGIC         0x42434647UL
#define EEPROM_FACTORY_FLASH_VERSION       1UL
#define EEPROM_FACTORY_FLASH_HEADER_WORDS  4U
#define EEPROM_FACTORY_FLASH_PAYLOAD_SIZE  (4U * EE_24CXX_PAGE_SIZE)

#define EEPROM_FACTORY_STEP_POWER_OFF       1U
#define EEPROM_FACTORY_STEP_EXT_BACKUP      2U
#define EEPROM_FACTORY_STEP_INTERNAL_BACKUP 3U
#define EEPROM_FACTORY_STEP_PROFILE_RESET   4U
#define EEPROM_FACTORY_STEP_FINAL_P02       5U
#define EEPROM_FACTORY_STEP_POWER_ON        6U
#define EEPROM_FACTORY_SAVE_WAIT_TICKS      10U
#define EEPROM_FACTORY_RESTORE_STEP_POWER_OFF    11U
#define EEPROM_FACTORY_RESTORE_STEP_EXT_CHECK    12U
#define EEPROM_FACTORY_RESTORE_STEP_EXT_APPLY    13U
#define EEPROM_FACTORY_RESTORE_STEP_INT_CHECK    14U
#define EEPROM_FACTORY_RESTORE_STEP_APP_DEFAULT  15U
#define EEPROM_FACTORY_RESTORE_STEP_FINAL_COMMIT 16U
#define EEPROM_FACTORY_RESTORE_STEP_PROFILE_RESET 17U
#define EEPROM_FACTORY_RESTORE_STEP_POWER_ON     18U

typedef struct
{
    uint16_t magic;
    uint16_t version;
    uint16_t bat_type_a;
    uint16_t bat_type_b;
    uint16_t boot_time_a;
    uint16_t soft_start_b;
    uint16_t checksum;
} eeprom_mppt_return_battery_t;

#if (APP_DEBUG_EVENT_FEATURES == 1)

typedef struct
{
    uint8_t seq;
    uint8_t event;
    uint8_t area;
    uint8_t page;
    uint8_t mode;
    uint8_t profile;
    uint8_t result;
    uint8_t value;
} app_debug_event_t;

static app_debug_event_t app_debug_events[WG_COM_V2_APP_DEBUG_EVENT_COUNT];
static uint8_t app_debug_event_head = 0;
static uint8_t app_debug_event_seq = 0;
static uint8_t app_debug_suppress_page_write_events = 0;

static uint8_t app_debug_area_from_page(uint16_t page)
{
    if(page == EEPROM_PAGE_P00_CURRENT) return APP_DBG_AREA_P00;
    if(page == EEPROM_PAGE_P02_CURRENT) return APP_DBG_AREA_P02;
    if(page == EEPROM_PAGE_CAL_CURRENT) return APP_DBG_AREA_CAL;
    if(page == EEPROM_PAGE_PARAM_CURRENT) return APP_DBG_AREA_PARAM;
    if((page >= EEPROM_PAGE_P00_FACTORY) && (page <= EEPROM_PAGE_PARAM_FACTORY)) return APP_DBG_AREA_FACTORY;
    if((page >= EEPROM_PAGE_BAT_PROFILE_A_BASE) && (page < EEPROM_PAGE_MPPT_PROFILE_BASE)) return APP_DBG_AREA_BAT;
    if((page >= EEPROM_PAGE_MPPT_PROFILE_BASE) && (page < EEPROM_PAGE_EXT_PROFILE_BASE)) return APP_DBG_AREA_MPPT;
    return APP_DBG_AREA_EXT;
}

static uint8_t app_debug_page_from_addr(uint16_t addr)
{
    return (uint8_t)(addr / EE_24CXX_PAGE_SIZE);
}

void app_debug_event_push(uint8_t event,
                          uint8_t area,
                          uint8_t page,
                          uint8_t mode,
                          uint8_t profile,
                          uint8_t result,
                          uint8_t value)
{
    app_debug_event_t *slot = &app_debug_events[app_debug_event_head];

    slot->seq = ++app_debug_event_seq;
    slot->event = event;
    slot->area = area;
    slot->page = page;
    slot->mode = mode;
    slot->profile = profile;
    slot->result = result;
    slot->value = value;
    app_debug_event_head++;
    app_debug_event_head &= (WG_COM_V2_APP_DEBUG_EVENT_COUNT - 1U);
}

void app_debug_event_read_regs(uint16_t reg_offset, uint16_t reg_count, uint8_t *data)
{
    uint16_t byte_count = (uint16_t)(reg_count * 2U);
    uint16_t byte_index;

    if(data == NULL)
    {
        return;
    }

    memset(data, 0, byte_count);
    for(byte_index = 0; byte_index < byte_count; byte_index++)
    {
        uint16_t abs_byte = (uint16_t)(reg_offset * 2U + byte_index);
        uint8_t event_index = (uint8_t)(abs_byte / WG_COM_V2_APP_DEBUG_EVENT_SIZE);
        uint8_t field_index = (uint8_t)(abs_byte % WG_COM_V2_APP_DEBUG_EVENT_SIZE);
        uint8_t ring_index;
        const app_debug_event_t *slot;

        if(event_index >= WG_COM_V2_APP_DEBUG_EVENT_COUNT)
        {
            break;
        }

        ring_index = (uint8_t)((app_debug_event_head + event_index) & (WG_COM_V2_APP_DEBUG_EVENT_COUNT - 1U));
        slot = &app_debug_events[ring_index];
        switch(field_index)
        {
            case 0: data[byte_index] = slot->seq; break;
            case 1: data[byte_index] = slot->event; break;
            case 2: data[byte_index] = slot->area; break;
            case 3: data[byte_index] = slot->page; break;
            case 4: data[byte_index] = slot->mode; break;
            case 5: data[byte_index] = slot->profile; break;
            case 6: data[byte_index] = slot->result; break;
            default: data[byte_index] = slot->value; break;
        }
    }
}

#else

void app_debug_event_read_regs(uint16_t reg_offset, uint16_t reg_count, uint8_t *data)
{
    uint16_t byte_count = (uint16_t)(reg_count * 2U);

    (void)reg_offset;

    if(data == NULL)
    {
        return;
    }

    memset(data, 0, byte_count);
}

#endif
//uint8_t flash_buffer_data[4096] = {0};
// 初始�?         1word->2byte   起始0地址
// P00厂家数据�? 42word->84byte  起始地址2
// P02控制设置     12word->24byte  起始地址86
// P03设置参数�? 52word->104byte 起始地址110
// 安全减少计数器（防止下溢�?
uint16_t eeprom_profile_page_to_addr(uint16_t page)
{
    return (uint16_t)(page * EE_24CXX_PAGE_SIZE);
}

uint16_t eeprom_profile_calc_page(uint8_t port, uint8_t type_index, uint8_t volt_index)
{
    if(volt_index >= EEPROM_BAT_PROFILE_VOLT_COUNT)
    {
        return 0xFFFF;
    }

    if(port == EEPROM_PROFILE_PORT_A)
    {
        if(type_index >= EEPROM_BAT_PROFILE_A_TYPE_COUNT)
        {
            return 0xFFFF;
        }
        return (uint16_t)(EEPROM_PAGE_BAT_PROFILE_A_BASE +
                          (type_index * EEPROM_BAT_PROFILE_VOLT_COUNT) +
                          volt_index);
    }

    if(port == EEPROM_PROFILE_PORT_B)
    {
        if(type_index >= EEPROM_BAT_PROFILE_B_TYPE_COUNT)
        {
            return 0xFFFF;
        }
        return (uint16_t)(EEPROM_PAGE_BAT_PROFILE_B_BASE +
                          (type_index * EEPROM_BAT_PROFILE_VOLT_COUNT) +
                          volt_index);
    }

    if(port == EEPROM_PROFILE_PORT_MPPT)
    {
        if(type_index >= EEPROM_BAT_PROFILE_B_TYPE_COUNT)
        {
            return 0xFFFF;
        }
        return (uint16_t)(EEPROM_PAGE_BAT_PROFILE_B_BASE +
                          (type_index * EEPROM_BAT_PROFILE_VOLT_COUNT) +
                          volt_index);
    }

    return 0xFFFF;
}

uint16_t eeprom_profile_checksum16(const uint8_t *data, uint16_t len)
{
    uint16_t checksum = 0;
    uint16_t i;

    if(data == NULL)
    {
        return 0xFFFF;
    }

    for(i = 0; i < len; i++)
    {
        checksum = (uint16_t)(checksum + data[i]);
    }

    return checksum;
}

uint8_t eeprom_profile_header_is_valid(const eeprom_profile_header_t *header,
                                       const uint8_t *payload,
                                       uint16_t payload_len)
{
    if((header == NULL) || (payload == NULL))
    {
        return 0;
    }

    if(header->magic != EEPROM_PROFILE_MAGIC_VALID)
    {
        return 0;
    }

    if(header->version != EEPROM_PROFILE_VERSION)
    {
        return 0;
    }

    if(header->checksum != eeprom_profile_checksum16(payload, payload_len))
    {
        return 0;
    }

    return 1;
}

static uint8_t eeprom_page_is_blank(const uint8_t *data, uint16_t len)
{
    uint16_t i;

    if(data == NULL)
    {
        return 1;
    }

    for(i = 0; i < len; i++)
    {
        if(data[i] != 0xFF)
        {
            return 0;
        }
    }

    return 1;
}

static void eeprom_param_copy_cal_to_page(const wg_com_v2_param_t *param)
{
    memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&eeprom_page_read_data,
           (uint8_t *)param,
           EEPROM_PARAM_CAL_SIZE);
}

static void eeprom_param_copy_user_to_page(const wg_com_v2_param_t *param)
{
    memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&eeprom_page_read_data,
           ((uint8_t *)param) + EEPROM_PARAM_CAL_SIZE,
           EEPROM_PARAM_USER_SIZE);
}

static void eeprom_param_copy_cal_from_page(wg_com_v2_param_t *param)
{
    memcpy((uint8_t *)param,
           (uint8_t *)&eeprom_page_read_data,
           EEPROM_PARAM_CAL_SIZE);
}

static void eeprom_param_copy_user_from_page(wg_com_v2_param_t *param)
{
    memcpy(((uint8_t *)param) + EEPROM_PARAM_CAL_SIZE,
           (uint8_t *)&eeprom_page_read_data,
           EEPROM_PARAM_USER_SIZE);
}

static uint16_t eeprom_ctrl_get(const uint16_t *field)
{
    return get_uint16((uint8_t *)field);
}

static uint8_t eeprom_bat_type_is_valid(uint16_t value)
{
    uint16_t type = (value & 0xFF00U) >> 8;
    uint16_t sys = value & 0x00FFU;

    return ((type < eBAT_MODE_TYPE_MAX) && (sys < eSYS_VOLT_MAX)) ? 1 : 0;
}

static uint8_t eeprom_ctrl_is_valid(const wg_com_v2_ctrl_t *ctrl)
{
    if(ctrl == NULL)
    {
        return 0;
    }

    if(eeprom_ctrl_get(&ctrl->FactoryReset) > 1U) return 0;
    if(eeprom_ctrl_get(&ctrl->PowerOnOff) > 1U) return 0;
    if(eeprom_ctrl_get(&ctrl->SetPowerMode) >= eSET_MODE_MAX) return 0;
    if(eeprom_ctrl_get(&ctrl->SetChargMode) >= eSET_CHARG_MAX) return 0;
    if(!eeprom_bat_type_is_valid(eeprom_ctrl_get(&ctrl->InpBatyType))) return 0;
    if(!eeprom_bat_type_is_valid(eeprom_ctrl_get(&ctrl->OutBatyType))) return 0;
    if(eeprom_ctrl_get(&ctrl->ZeroCurrCalibration) > 1U) return 0;
    if(eeprom_ctrl_get(&ctrl->ResetFactoryData) > 1U) return 0;
    if(eeprom_ctrl_get(&ctrl->BatModeFR) >= eADDRS_MAX) return 0;
    if(eeprom_ctrl_get(&ctrl->MpptSwitch) > 1U) return 0;
    if(eeprom_ctrl_get(&ctrl->SleepModeOnOff) > 1U) return 0;
    return 1;
}

static void eeprom_ctrl_fill_safe_default(void)
{
    WG_COM_V2_SET_DATA_UINT(0, eeprom_wg_com_v2_param.wg_com_v2_ctrl.FactoryReset);
    WG_COM_V2_SET_DATA_UINT(0, eeprom_wg_com_v2_param.wg_com_v2_ctrl.PowerOnOff);
    WG_COM_V2_SET_DATA_UINT(eSET_CUSTOM_MODE, eeprom_wg_com_v2_param.wg_com_v2_ctrl.SetPowerMode);
    WG_COM_V2_SET_DATA_UINT(eSET_FORWARD, eeprom_wg_com_v2_param.wg_com_v2_ctrl.SetChargMode);
    WG_COM_V2_SET_DATA_UINT(((eBAT_DCDC << 8) | eSYS_10_60V), eeprom_wg_com_v2_param.wg_com_v2_ctrl.InpBatyType);
    WG_COM_V2_SET_DATA_UINT(((eBAT_DCDC << 8) | eSYS_10_60V), eeprom_wg_com_v2_param.wg_com_v2_ctrl.OutBatyType);
    WG_COM_V2_SET_DATA_UINT(0, eeprom_wg_com_v2_param.wg_com_v2_ctrl.SetBootTimeA);
    WG_COM_V2_SET_DATA_UINT(0, eeprom_wg_com_v2_param.wg_com_v2_ctrl.SetBootTimeB);
    WG_COM_V2_SET_DATA_UINT(0, eeprom_wg_com_v2_param.wg_com_v2_ctrl.SetOnCurrStartTimeA);
    WG_COM_V2_SET_DATA_UINT(0, eeprom_wg_com_v2_param.wg_com_v2_ctrl.SetOnCurrStartTimeB);
    WG_COM_V2_SET_DATA_UINT(0, eeprom_wg_com_v2_param.wg_com_v2_ctrl.ZeroCurrCalibration);
    WG_COM_V2_SET_DATA_UINT(0, eeprom_wg_com_v2_param.wg_com_v2_ctrl.ResetFactoryData);
    WG_COM_V2_SET_DATA_UINT(eADDRS_FORWARD, eeprom_wg_com_v2_param.wg_com_v2_ctrl.BatModeFR);
    WG_COM_V2_SET_DATA_UINT(0, eeprom_wg_com_v2_param.wg_com_v2_ctrl.MpptSwitch);
    WG_COM_V2_SET_DATA_UINT(0, eeprom_wg_com_v2_param.wg_com_v2_ctrl.SleepModeOnOff);
}

static uint16_t eeprom_ctrl_make_storable_bat_type(uint16_t runtime_value, uint16_t stored_value)
{
    uint16_t runtime_type = (runtime_value & 0xFF00U) >> 8;
    uint16_t runtime_sys = runtime_value & 0x00FFU;
    uint16_t stored_type = (stored_value & 0xFF00U) >> 8;
    uint16_t stored_sys = stored_value & 0x00FFU;

    if((runtime_type == eBAT_AUTOSYS) && (runtime_sys >= eSYS_VOLT_MAX))
    {
        if((stored_type == eBAT_AUTOSYS) && (stored_sys < eSYS_VOLT_MAX))
        {
            return stored_value;
        }

        return (uint16_t)((eBAT_AUTOSYS << 8) | eSYS_12V);
    }

    return runtime_value;
}

static void eeprom_ctrl_prepare_store_image(wg_com_v2_ctrl_t *dst,
                                            const wg_com_v2_ctrl_t *src,
                                            const wg_com_v2_ctrl_t *stored)
{
    uint16_t inp_stored = (uint16_t)((eBAT_AUTOSYS << 8) | eSYS_12V);
    uint16_t out_stored = (uint16_t)((eBAT_AUTOSYS << 8) | eSYS_12V);
    uint16_t inp_value;
    uint16_t out_value;

    memcpy((uint8_t *)dst, (uint8_t *)src, sizeof(*dst));
    if(stored != NULL)
    {
        inp_stored = eeprom_ctrl_get(&stored->InpBatyType);
        out_stored = eeprom_ctrl_get(&stored->OutBatyType);
    }

    inp_value = eeprom_ctrl_make_storable_bat_type(eeprom_ctrl_get(&dst->InpBatyType), inp_stored);
    out_value = eeprom_ctrl_make_storable_bat_type(eeprom_ctrl_get(&dst->OutBatyType), out_stored);

    WG_COM_V2_SET_DATA_UINT(inp_value, dst->InpBatyType);
    WG_COM_V2_SET_DATA_UINT(out_value, dst->OutBatyType);
    WG_COM_V2_SET_DATA_UINT(0, dst->FactoryReset);
    WG_COM_V2_SET_DATA_UINT(0, dst->PowerOnOff);
    WG_COM_V2_SET_DATA_UINT(0, dst->ZeroCurrCalibration);
    WG_COM_V2_SET_DATA_UINT(0, dst->ResetFactoryData);
}

static uint8_t eeprom_param_user_is_valid(const wg_com_v2_param_t *param)
{
    uint16_t in_volt;
    uint16_t out_volt;
    uint16_t in_curr;
    uint16_t out_curr;
    int16_t inside_temp;
    int16_t outside_temp;
    int16_t temp2;

    if(param == NULL)
    {
        return 0;
    }

    in_volt = get_uint16((uint8_t *)&param->SetInpVolt);
    out_volt = get_uint16((uint8_t *)&param->SetOutVolt);
    in_curr = get_uint16((uint8_t *)&param->SetInpCurr);
    out_curr = get_uint16((uint8_t *)&param->SetOutCurr);
    inside_temp = get_int16((uint8_t *)&param->SetInsideTemp);
    outside_temp = get_int16((uint8_t *)&param->SetOutsideTemp);
    temp2 = get_int16((uint8_t *)&param->SetTemp2);

    if((in_volt < 800U) || (in_volt > 8000U)) return 0;
    if((out_volt < 800U) || (out_volt > 8000U)) return 0;
    if((in_curr == 0U) || (in_curr > 20000U)) return 0;
    if((out_curr == 0U) || (out_curr > 20000U)) return 0;
    if((inside_temp < 50) || (inside_temp > 130)) return 0;
    if((outside_temp < 50) || (outside_temp > 130)) return 0;
    if((temp2 < 50) || (temp2 > 115)) return 0;
    return 1;
}

static uint8_t eeprom_param_fix_safe_temperatures(wg_com_v2_param_t *param)
{
    uint8_t fixed = 0;
    int16_t inside_temp;
    int16_t outside_temp;
    int16_t temp2;

    if(param == NULL)
    {
        return 0;
    }

    inside_temp = get_int16((uint8_t *)&param->SetInsideTemp);
    outside_temp = get_int16((uint8_t *)&param->SetOutsideTemp);
    temp2 = get_int16((uint8_t *)&param->SetTemp2);

    if((inside_temp < 50) || (inside_temp > 130))
    {
        WG_COM_V2_SET_DATA_INT(100.0f, param->SetInsideTemp);
        fixed = 1;
    }
    if((outside_temp < 50) || (outside_temp > 130))
    {
        WG_COM_V2_SET_DATA_INT(100.0f, param->SetOutsideTemp);
        fixed = 1;
    }
    if((temp2 < 50) || (temp2 > 115))
    {
        WG_COM_V2_SET_DATA_INT(100.0f, param->SetTemp2);
        fixed = 1;
    }

    return fixed;
}

static uint8_t eeprom_write_page_verify(uint16_t addr)
{
#if (APP_DEBUG_EVENT_FEATURES == 1)
    uint8_t page = app_debug_page_from_addr(addr);
    uint8_t area = app_debug_area_from_page(page);

    if(!app_debug_suppress_page_write_events)
    {
        app_debug_event_push(APP_DBG_EVT_WRITE_BEGIN, area, page,
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             0, APP_DBG_RESULT_START, 0);
    }
#endif
    IICx_Write_Byte(addr, (uint8_t *)&eeprom_page_read_data, sizeof(eeprom_page_read_data));
    IICx_Read_Byte(addr, (uint8_t *)&eeprom_page_verify_data, sizeof(eeprom_page_verify_data));

    if(memcmp((uint8_t *)&eeprom_page_read_data,
              (uint8_t *)&eeprom_page_verify_data,
              sizeof(eeprom_page_read_data)) != 0)
    {
#if (APP_DEBUG_EVENT_FEATURES == 1)
        if(!app_debug_suppress_page_write_events)
        {
            app_debug_event_push(APP_DBG_EVT_VERIFY_FAIL, area, page,
                                 (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                                 0, APP_DBG_RESULT_FAIL, 0);
        }
#endif
        return 0;
    }

#if (APP_DEBUG_EVENT_FEATURES == 1)
    if(!app_debug_suppress_page_write_events)
    {
        app_debug_event_push(APP_DBG_EVT_VERIFY_OK, area, page,
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             0, APP_DBG_RESULT_OK, 0);
    }
#endif
    return 1;
}

static uint16_t eeprom_mppt_return_checksum(const eeprom_mppt_return_battery_t *snapshot)
{
    if(snapshot == NULL)
    {
        return 0xFFFFU;
    }

    return (uint16_t)(snapshot->magic +
                      snapshot->version +
                      snapshot->bat_type_a +
                      snapshot->bat_type_b +
                      snapshot->boot_time_a +
                      snapshot->soft_start_b);
}

void eeprom_save_mppt_return_battery_types(uint16_t bat_type_a, uint16_t bat_type_b)
{
    eeprom_mppt_return_battery_t snapshot;
    eeprom_mppt_return_battery_t old_snapshot;
    uint16_t boot_time_a = 0;
    uint16_t soft_start_b = 0;

    if(!eeprom_bat_type_is_valid(bat_type_a) || !eeprom_bat_type_is_valid(bat_type_b))
    {
        return;
    }

    IICx_Read_Byte(eeprom_profile_page_to_addr(EEPROM_PAGE_MPPT_PROFILE_BASE),
                   (uint8_t *)eeprom_page_read_data,
                   sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&old_snapshot, (uint8_t *)eeprom_page_read_data, sizeof(old_snapshot));
    if((old_snapshot.magic == EEPROM_MPPT_RETURN_MAGIC) &&
       (old_snapshot.version == EEPROM_MPPT_RETURN_VERSION) &&
       (old_snapshot.checksum == eeprom_mppt_return_checksum(&old_snapshot)) &&
       (old_snapshot.boot_time_a <= EEPROM_PROFILE_TIME_MAX) &&
       (old_snapshot.soft_start_b <= EEPROM_PROFILE_TIME_MAX))
    {
        boot_time_a = old_snapshot.boot_time_a;
        soft_start_b = old_snapshot.soft_start_b;
    }

    snapshot.magic = EEPROM_MPPT_RETURN_MAGIC;
    snapshot.version = EEPROM_MPPT_RETURN_VERSION;
    snapshot.bat_type_a = bat_type_a;
    snapshot.bat_type_b = bat_type_b;
    snapshot.boot_time_a = boot_time_a;
    snapshot.soft_start_b = soft_start_b;
    snapshot.checksum = eeprom_mppt_return_checksum(&snapshot);

    memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&eeprom_page_read_data, (uint8_t *)&snapshot, sizeof(snapshot));
    (void)eeprom_write_page_verify(eeprom_profile_page_to_addr(EEPROM_PAGE_MPPT_PROFILE_BASE));
}

static uint8_t eeprom_save_mppt_timing(uint16_t boot_time_a, uint16_t soft_start_b)
{
    eeprom_mppt_return_battery_t snapshot;
    eeprom_mppt_return_battery_t old_snapshot;

    if((boot_time_a > EEPROM_PROFILE_TIME_MAX) || (soft_start_b > EEPROM_PROFILE_TIME_MAX))
    {
        return 0;
    }

    IICx_Read_Byte(eeprom_profile_page_to_addr(EEPROM_PAGE_MPPT_PROFILE_BASE),
                   (uint8_t *)eeprom_page_read_data,
                   sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&old_snapshot, (uint8_t *)eeprom_page_read_data, sizeof(old_snapshot));
    if((old_snapshot.magic == EEPROM_MPPT_RETURN_MAGIC) &&
       (old_snapshot.version == EEPROM_MPPT_RETURN_VERSION) &&
       (old_snapshot.checksum == eeprom_mppt_return_checksum(&old_snapshot)) &&
       eeprom_bat_type_is_valid(old_snapshot.bat_type_a) &&
       eeprom_bat_type_is_valid(old_snapshot.bat_type_b))
    {
        snapshot = old_snapshot;
    }
    else
    {
        snapshot.magic = EEPROM_MPPT_RETURN_MAGIC;
        snapshot.version = EEPROM_MPPT_RETURN_VERSION;
        snapshot.bat_type_a = get_wg_com_v2_data.com_ctrl.InpBatyType;
        snapshot.bat_type_b = get_wg_com_v2_data.com_ctrl.OutBatyType;
        if(!eeprom_bat_type_is_valid(snapshot.bat_type_a))
        {
            snapshot.bat_type_a = (uint16_t)((eBAT_LA_AGM << 8) | eSYS_12V);
        }
        if(!eeprom_bat_type_is_valid(snapshot.bat_type_b))
        {
            snapshot.bat_type_b = (uint16_t)((eBAT_LI_LFP << 8) | eSYS_16V);
        }
    }

    snapshot.boot_time_a = boot_time_a;
    snapshot.soft_start_b = soft_start_b;
    snapshot.checksum = eeprom_mppt_return_checksum(&snapshot);

    memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&eeprom_page_read_data, (uint8_t *)&snapshot, sizeof(snapshot));
    return eeprom_write_page_verify(eeprom_profile_page_to_addr(EEPROM_PAGE_MPPT_PROFILE_BASE));
}

static uint8_t eeprom_load_mppt_timing(uint16_t *boot_time_a, uint16_t *soft_start_b)
{
    eeprom_mppt_return_battery_t snapshot;

    if((boot_time_a == NULL) || (soft_start_b == NULL))
    {
        return 0;
    }

    IICx_Read_Byte(eeprom_profile_page_to_addr(EEPROM_PAGE_MPPT_PROFILE_BASE),
                   (uint8_t *)eeprom_page_read_data,
                   sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&snapshot, (uint8_t *)eeprom_page_read_data, sizeof(snapshot));

    if((snapshot.magic != EEPROM_MPPT_RETURN_MAGIC) ||
       (snapshot.version != EEPROM_MPPT_RETURN_VERSION) ||
       (snapshot.checksum != eeprom_mppt_return_checksum(&snapshot)) ||
       (snapshot.boot_time_a > EEPROM_PROFILE_TIME_MAX) ||
       (snapshot.soft_start_b > EEPROM_PROFILE_TIME_MAX))
    {
        return 0;
    }

    *boot_time_a = snapshot.boot_time_a;
    *soft_start_b = snapshot.soft_start_b;
    return 1;
}

uint8_t eeprom_load_mppt_return_battery_types(uint16_t *bat_type_a, uint16_t *bat_type_b)
{
    eeprom_mppt_return_battery_t snapshot;

    if((bat_type_a == NULL) || (bat_type_b == NULL))
    {
        return 0;
    }

    IICx_Read_Byte(eeprom_profile_page_to_addr(EEPROM_PAGE_MPPT_PROFILE_BASE),
                   (uint8_t *)eeprom_page_read_data,
                   sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&snapshot, (uint8_t *)eeprom_page_read_data, sizeof(snapshot));

    if((snapshot.magic != EEPROM_MPPT_RETURN_MAGIC) ||
       (snapshot.version != EEPROM_MPPT_RETURN_VERSION) ||
       (snapshot.checksum != eeprom_mppt_return_checksum(&snapshot)) ||
       !eeprom_bat_type_is_valid(snapshot.bat_type_a) ||
       !eeprom_bat_type_is_valid(snapshot.bat_type_b))
    {
        return 0;
    }

    *bat_type_a = snapshot.bat_type_a;
    *bat_type_b = snapshot.bat_type_b;
    return 1;
}
static void eeprom_profile_prepare_header(eeprom_profile_header_t *header,
                                          uint8_t port,
                                          uint8_t type,
                                          uint8_t sys,
                                          const uint8_t *payload,
                                          uint16_t payload_len,
                                          uint16_t magic,
                                          uint8_t reserved)
{
    header->magic = magic;
    header->version = EEPROM_PROFILE_VERSION;
    header->port = port;
    header->type = type;
    header->sys = sys;
    header->reserved = reserved;
    header->checksum = eeprom_profile_checksum16(payload, payload_len);
}

static uint8_t eeprom_profile_write_payload_ex(uint16_t addr,
                                               uint8_t port,
                                               uint8_t type,
                                               uint8_t sys,
                                               const uint8_t *payload,
                                               uint16_t payload_len,
                                               uint8_t reserved)
{
    eeprom_profile_header_t header;

    if((payload == NULL) ||
       ((sizeof(eeprom_profile_header_t) + payload_len) > sizeof(eeprom_page_read_data)))
    {
        return 0;
    }

    memset((uint8_t *)eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
    eeprom_profile_prepare_header(&header, port, type, sys, payload, payload_len, EEPROM_PROFILE_MAGIC_INVALID, reserved);
    memcpy((uint8_t *)eeprom_page_read_data, (uint8_t *)&header, sizeof(header));
    memcpy((uint8_t *)eeprom_page_read_data + sizeof(header), payload, payload_len);
    if(!eeprom_write_page_verify(addr))
    {
        return 0;
    }

    eeprom_profile_prepare_header(&header, port, type, sys, payload, payload_len, EEPROM_PROFILE_MAGIC_VALID, reserved);
    memcpy((uint8_t *)eeprom_page_read_data, (uint8_t *)&header, sizeof(header));
    return eeprom_write_page_verify(addr);
}

static uint8_t eeprom_profile_write_payload(uint16_t addr,
                                            uint8_t port,
                                            uint8_t type,
                                            uint8_t sys,
                                            const uint8_t *payload,
                                            uint16_t payload_len)
{
    return eeprom_profile_write_payload_ex(addr, port, type, sys, payload, payload_len, EEPROM_PROFILE_RESERVED_DEFAULT);
}

static uint8_t eeprom_profile_write_user_payload(uint16_t addr,
                                                 uint8_t port,
                                                 uint8_t type,
                                                 uint8_t sys,
                                                 const uint8_t *payload,
                                                 uint16_t payload_len)
{
    return eeprom_profile_write_payload_ex(addr, port, type, sys, payload, payload_len, EEPROM_PROFILE_RESERVED_USER);
}
static uint8_t eeprom_profile_read_payload(uint16_t addr,
                                           uint8_t port,
                                           uint8_t type,
                                           uint8_t sys,
                                           uint8_t *payload,
                                           uint16_t payload_len)
{
    eeprom_profile_header_t *header = NULL;
    uint8_t *stored_payload = NULL;

    if((payload == NULL) ||
       ((sizeof(eeprom_profile_header_t) + payload_len) > sizeof(eeprom_page_read_data)))
    {
        return 0;
    }

    IICx_Read_Byte(addr, (uint8_t *)eeprom_page_read_data, sizeof(eeprom_page_read_data));
    header = (eeprom_profile_header_t *)eeprom_page_read_data;
    stored_payload = ((uint8_t *)eeprom_page_read_data) + sizeof(eeprom_profile_header_t);

    if((header->port != port) || (header->type != type) || (header->sys != sys))
    {
        app_debug_event_push(APP_DBG_EVT_READ, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                             app_debug_page_from_addr(addr),
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             type, APP_DBG_RESULT_INVALID, sys);
        return 0;
    }

    if(!eeprom_profile_header_is_valid(header, stored_payload, payload_len))
    {
        uint16_t legacy_payload_len = (uint16_t)(sizeof(eeprom_system_profile_t) - (2U * sizeof(uint16_t)));
        if((payload_len == sizeof(eeprom_system_profile_t)) &&
           eeprom_profile_header_is_valid(header, stored_payload, legacy_payload_len))
        {
            eeprom_system_profile_t *profile = (eeprom_system_profile_t *)payload;
            memset(payload, 0xFF, payload_len);
            memcpy(payload, stored_payload, legacy_payload_len);
            if(port == EEPROM_PROFILE_PORT_A)
            {
                WG_COM_V2_GET_DATA_UINT(profile->SetBootTime, wg_com_v2_ctrl.SetBootTimeA);
                WG_COM_V2_GET_DATA_UINT(profile->SetOnCurrStartTime, wg_com_v2_ctrl.SetOnCurrStartTimeA);
            }
            else
            {
                WG_COM_V2_GET_DATA_UINT(profile->SetBootTime, wg_com_v2_ctrl.SetBootTimeB);
                WG_COM_V2_GET_DATA_UINT(profile->SetOnCurrStartTime, wg_com_v2_ctrl.SetOnCurrStartTimeB);
            }
            app_debug_event_push(APP_DBG_EVT_READ, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                                 app_debug_page_from_addr(addr),
                                 (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                                 type, APP_DBG_RESULT_RETRY, sys);
            return 1;
        }

        app_debug_event_push(APP_DBG_EVT_READ, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                             app_debug_page_from_addr(addr),
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             type, APP_DBG_RESULT_FAIL, sys);
        return 0;
    }

    memcpy(payload, stored_payload, payload_len);
    app_debug_event_push(APP_DBG_EVT_READ, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                         app_debug_page_from_addr(addr),
                         (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                         type, APP_DBG_RESULT_OK, sys);
    return 1;
}

static void eeprom_user_profile_capture(uint8_t *payload)
{
    memcpy(payload,
           ((uint8_t *)&wg_com_v2_param) + EEPROM_PARAM_CAL_SIZE,
           EEPROM_PARAM_USER_SIZE);
}

static void eeprom_prepare_factory_backup_cache(void)
{
    memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_product_info,
           (uint8_t *)&wg_com_v2_product_info,
           sizeof(wg_com_v2_product_info));

    memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
           (uint8_t *)&wg_com_v2_ctrl,
           sizeof(wg_com_v2_ctrl));

    eeprom_wg_com_v2_param.wg_com_v2_ctrl.FactoryReset = 0;
    eeprom_wg_com_v2_param.wg_com_v2_ctrl.PowerOnOff = 0;
    eeprom_wg_com_v2_param.wg_com_v2_ctrl.ZeroCurrCalibration = 0;
    eeprom_wg_com_v2_param.wg_com_v2_ctrl.ResetFactoryData = 0;

    memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param,
           (uint8_t *)&wg_com_v2_param,
           sizeof(wg_com_v2_param));

    memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param,
           (uint8_t *)&eeprom_wg_com_v2_param,
           sizeof(eeprom_backup_wg_com_v2_param));
}

static uint8_t eeprom_save_factory_backup_external_verified(void)
{
    eeprom_prepare_factory_backup_cache();

    memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&eeprom_page_read_data,
           (uint8_t *)&eeprom_backup_wg_com_v2_param,
           (sizeof(eeprom_backup_wg_com_v2_param.is_writed) +
            sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_product_info)));
    if(!eeprom_write_page_verify(P00_BACKUP_ADDR))
    {
        return 0;
    }

    memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&eeprom_page_read_data,
           (uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl,
           sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl));
    if(!eeprom_write_page_verify(P02_BACKUP_ADDR))
    {
        return 0;
    }

    eeprom_param_copy_cal_to_page(&eeprom_backup_wg_com_v2_param.wg_com_v2_param);
    if(!eeprom_write_page_verify(P03_BACKUP_1_ADDR))
    {
        return 0;
    }

    eeprom_param_copy_user_to_page(&eeprom_backup_wg_com_v2_param.wg_com_v2_param);
    if(!eeprom_write_page_verify(P03_BACKUP_ADDR))
    {
        return 0;
    }

    return 1;
}

static uint32_t eeprom_factory_backup_checksum(const uint8_t *data, uint16_t len)
{
    uint32_t checksum = 0xA55A5AA5UL;
    uint16_t i;

    for(i = 0; i < len; i++)
    {
        checksum = (checksum << 5) | (checksum >> 27);
        checksum += data[i];
        checksum ^= 0x10210305UL;
    }

    return checksum;
}

static uint32_t eeprom_factory_flash_base_addr(void)
{
    return FLASH_USER_BASE_ADDR + (EEPROM_FACTORY_FLASH_PAGE * FLASH_LOGICAL_PAGE_SIZE);
}

static void eeprom_factory_flash_program_word(uint32_t offset, uint32_t value)
{
    EFM_ProgramWord(eeprom_factory_flash_base_addr() + offset, value);
}

static uint32_t eeprom_factory_flash_read_word(uint32_t offset)
{
    return *(volatile uint32_t *)(eeprom_factory_flash_base_addr() + offset);
}

static uint32_t eeprom_factory_external_backup_checksum(void)
{
    uint32_t checksum = 0xA55A5AA5UL;
    uint16_t addrs[4] = {P00_BACKUP_ADDR, P02_BACKUP_ADDR, P03_BACKUP_1_ADDR, P03_BACKUP_ADDR};
    uint8_t i;
    uint16_t j;

    for(i = 0; i < 4U; i++)
    {
        IICx_Read_Byte(addrs[i], eeprom_page_read_data, EE_24CXX_PAGE_SIZE);
        for(j = 0; j < EE_24CXX_PAGE_SIZE; j++)
        {
            checksum = (checksum << 5) | (checksum >> 27);
            checksum += eeprom_page_read_data[j];
            checksum ^= 0x10210305UL;
        }
    }

    return checksum;
}

static uint32_t eeprom_factory_flash_payload_checksum(void)
{
    const uint8_t *payload = (const uint8_t *)(eeprom_factory_flash_base_addr() +
                                               (EEPROM_FACTORY_FLASH_HEADER_WORDS * sizeof(uint32_t)));

    return eeprom_factory_backup_checksum(payload, EEPROM_FACTORY_FLASH_PAYLOAD_SIZE);
}

static void eeprom_factory_flash_program_payload(void)
{
    uint16_t addrs[4] = {P00_BACKUP_ADDR, P02_BACKUP_ADDR, P03_BACKUP_1_ADDR, P03_BACKUP_ADDR};
    uint32_t flash_offset = EEPROM_FACTORY_FLASH_HEADER_WORDS * sizeof(uint32_t);
    uint8_t i;
    uint16_t j;

    for(i = 0; i < 4U; i++)
    {
        IICx_Read_Byte(addrs[i], eeprom_page_read_data, EE_24CXX_PAGE_SIZE);
        for(j = 0; j < EE_24CXX_PAGE_SIZE; j += 4U)
        {
            uint32_t word = ((uint32_t)eeprom_page_read_data[j]) |
                            ((uint32_t)eeprom_page_read_data[j + 1U] << 8) |
                            ((uint32_t)eeprom_page_read_data[j + 2U] << 16) |
                            ((uint32_t)eeprom_page_read_data[j + 3U] << 24);
            eeprom_factory_flash_program_word(flash_offset, word);
            flash_offset += 4U;
        }
    }
}

static uint8_t eeprom_save_factory_backup_internal_verified(void)
{
    uint32_t checksum = eeprom_factory_external_backup_checksum();

    app_debug_event_push(APP_DBG_EVT_WRITE_BEGIN, APP_DBG_AREA_FACTORY, 0xFE,
                         (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                         0, APP_DBG_RESULT_START, (uint8_t)EEPROM_FACTORY_FLASH_PAGE);

    if(!bsp_flash_erase_page(EEPROM_FACTORY_FLASH_PAGE))
    {
        app_debug_event_push(APP_DBG_EVT_VERIFY_FAIL, APP_DBG_AREA_FACTORY, 0xFE,
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             0, APP_DBG_RESULT_FAIL, (uint8_t)EEPROM_FACTORY_FLASH_PAGE);
        return 0;
    }

    EFM_REG_Unlock();
    EFM_FWMC_Cmd(ENABLE);
    (void)EFM_SingleSectorOperateCmd(EEPROM_FACTORY_FLASH_PAGE, ENABLE);

    eeprom_factory_flash_program_word(0U, EEPROM_FACTORY_FLASH_MAGIC);
    eeprom_factory_flash_program_word(4U, EEPROM_FACTORY_FLASH_VERSION);
    eeprom_factory_flash_program_word(8U, EEPROM_FACTORY_FLASH_PAYLOAD_SIZE);
    eeprom_factory_flash_program_word(12U, checksum);
    eeprom_factory_flash_program_payload();

    (void)EFM_SingleSectorOperateCmd(EEPROM_FACTORY_FLASH_PAGE, DISABLE);
    EFM_REG_Lock();

    if((eeprom_factory_flash_read_word(0U) != EEPROM_FACTORY_FLASH_MAGIC) ||
       (eeprom_factory_flash_read_word(4U) != EEPROM_FACTORY_FLASH_VERSION) ||
       (eeprom_factory_flash_read_word(8U) != EEPROM_FACTORY_FLASH_PAYLOAD_SIZE) ||
       (eeprom_factory_flash_read_word(12U) != checksum) ||
       (eeprom_factory_flash_payload_checksum() != checksum))
    {
        app_debug_event_push(APP_DBG_EVT_VERIFY_FAIL, APP_DBG_AREA_FACTORY, 0xFE,
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             0, APP_DBG_RESULT_FAIL, (uint8_t)EEPROM_FACTORY_FLASH_PAGE);
        return 0;
    }

    app_debug_event_push(APP_DBG_EVT_VERIFY_OK, APP_DBG_AREA_FACTORY, 0xFE,
                         (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                         0, APP_DBG_RESULT_OK, (uint8_t)EEPROM_FACTORY_FLASH_PAGE);
    return 1;
}

static void eeprom_factory_step_event(uint8_t step, uint8_t result, uint8_t value)
{
    app_debug_event_push(APP_DBG_EVT_FACTORY_STEP, APP_DBG_AREA_FACTORY, 0xFE,
                         (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                         step, result, value);
}

typedef enum
{
    EEPROM_FACTORY_SAVE_IDLE = 0,
    EEPROM_FACTORY_SAVE_POWER_OFF_WAIT,
    EEPROM_FACTORY_SAVE_RUN_BACKUP,
    EEPROM_FACTORY_SAVE_POWER_ON_WAIT
} eeprom_factory_save_state_t;

static eeprom_factory_save_state_t eeprom_factory_save_state = EEPROM_FACTORY_SAVE_IDLE;
static uint8_t eeprom_factory_save_wait_count = 0;
static uint8_t eeprom_write_all_current_pages(void);

typedef enum
{
    EEPROM_FACTORY_RESTORE_IDLE = 0,
    EEPROM_FACTORY_RESTORE_POWER_OFF_WAIT,
    EEPROM_FACTORY_RESTORE_RUN,
    EEPROM_FACTORY_RESTORE_POWER_ON_WAIT
} eeprom_factory_restore_state_t;

static eeprom_factory_restore_state_t eeprom_factory_restore_state = EEPROM_FACTORY_RESTORE_IDLE;
static uint8_t eeprom_factory_restore_wait_count = 0;

static void eeprom_factory_set_power(uint8_t power_off)
{
    WG_COM_V2_SET_DATA_UINT(power_off ? 1U : 0U, wg_com_v2_ctrl.PowerOnOff);
    charge_control_run();
    get_wg_com_data_rum();
}

static void eeprom_factory_save_abort(uint8_t step)
{
    uint16_t flag = 0;

    WG_COM_V2_SET_DATA_UINT(flag, wg_com_v2_ctrl.ResetFactoryData);
    eeprom_factory_set_power(1U);
    eeprom_factory_save_state = EEPROM_FACTORY_SAVE_IDLE;
    eeprom_factory_save_wait_count = 0;
    eeprom_factory_step_event(step, APP_DBG_RESULT_FAIL, 0);
}

static uint8_t eeprom_reset_battery_profile_pages_to_app_defaults(uint8_t factory_step)
{
    uint16_t page;
#if (APP_DEBUG_EVENT_FEATURES == 1)
    uint8_t old_suppress_page_write_events = app_debug_suppress_page_write_events;
#endif

    eeprom_factory_step_event(factory_step, APP_DBG_RESULT_START, 0);
    memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));

#if (APP_DEBUG_EVENT_FEATURES == 1)
    app_debug_suppress_page_write_events = 1;
#endif
    for(page = EEPROM_PAGE_BAT_PROFILE_A_BASE; page < EEPROM_PAGE_EXT_PROFILE_BASE; page++)
    {
        if(!eeprom_write_page_verify(eeprom_profile_page_to_addr(page)))
        {
#if (APP_DEBUG_EVENT_FEATURES == 1)
            app_debug_suppress_page_write_events = old_suppress_page_write_events;
#endif
            eeprom_factory_step_event(factory_step,
                                      APP_DBG_RESULT_FAIL,
                                      (uint8_t)page);
            return 0;
        }
    }
#if (APP_DEBUG_EVENT_FEATURES == 1)
    app_debug_suppress_page_write_events = old_suppress_page_write_events;
#endif

    eeprom_factory_step_event(factory_step,
                              APP_DBG_RESULT_OK,
                              (uint8_t)(EEPROM_PAGE_EXT_PROFILE_BASE - EEPROM_PAGE_BAT_PROFILE_A_BASE));
    return 1;
}

static void eeprom_factory_restore_sync_runtime(void)
{
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.SetPowerMode, wg_com_v2_ctrl.SetPowerMode);
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.SetChargMode, wg_com_v2_ctrl.SetChargMode);
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.InpBatyType, wg_com_v2_ctrl.InpBatyType);
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.OutBatyType, wg_com_v2_ctrl.OutBatyType);
    WG_COM_V2_SET_DATA_UINT(7, wg_com_v2_realtime_data.StateCharge);
    get_wg_com_data_rum();
    request_update_parameter();
}

static uint8_t eeprom_factory_restore_current_from_runtime_verified(void)
{
    uint8_t ok;
#if (APP_DEBUG_EVENT_FEATURES == 1)
    uint8_t old_suppress_page_write_events = app_debug_suppress_page_write_events;

    app_debug_suppress_page_write_events = 1;
#endif
    ok = eeprom_write_all_current_pages();
#if (APP_DEBUG_EVENT_FEATURES == 1)
    app_debug_suppress_page_write_events = old_suppress_page_write_events;
#endif

    if(!ok)
    {
        return 0;
    }

    eeprom_factory_restore_sync_runtime();
    return 1;
}

static uint8_t eeprom_factory_restore_external_to_runtime(void)
{
    IICx_Read_Byte(P00_BACKUP_ADDR, (uint8_t *)&eeprom_page_read_data, sizeof(eeprom_page_read_data));
    if((eeprom_page_read_data[1] != ((EEROM_INIT_DATA & 0xff00) >> 8)) ||
       (eeprom_page_read_data[0] !=  (EEROM_INIT_DATA & 0x00ff)))
    {
        return 0;
    }

    eeprom_wg_com_v2_param.is_writed = EEROM_INIT_DATA;
    memcpy((uint8_t *)&wg_com_v2_product_info,
           (uint8_t *)&eeprom_page_read_data[sizeof(eeprom_wg_com_v2_param.is_writed)],
           sizeof(wg_com_v2_product_info));

    IICx_Read_Byte(P02_BACKUP_ADDR, (uint8_t *)&eeprom_page_read_data, sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&wg_com_v2_ctrl,
           (uint8_t *)&eeprom_page_read_data,
           sizeof(wg_com_v2_ctrl));
    if(!eeprom_ctrl_is_valid(&wg_com_v2_ctrl))
    {
        return 0;
    }

    IICx_Read_Byte(P03_BACKUP_1_ADDR, (uint8_t *)&eeprom_page_read_data, sizeof(eeprom_page_read_data));
    eeprom_param_copy_cal_from_page(&wg_com_v2_param);

    IICx_Read_Byte(P03_BACKUP_ADDR, (uint8_t *)&eeprom_page_read_data, sizeof(eeprom_page_read_data));
    if(eeprom_page_is_blank((uint8_t *)&eeprom_page_read_data, EEPROM_PARAM_USER_SIZE))
    {
        return 0;
    }
    eeprom_param_copy_user_from_page(&wg_com_v2_param);
    if(!eeprom_param_user_is_valid(&wg_com_v2_param))
    {
        return 0;
    }

    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.FactoryReset);
    WG_COM_V2_SET_DATA_UINT(1, wg_com_v2_ctrl.PowerOnOff);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ZeroCurrCalibration);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ResetFactoryData);
    return 1;
}

static uint8_t eeprom_factory_restore_internal_to_runtime(void)
{
    const uint8_t *payload;

    if((eeprom_factory_flash_read_word(0U) != EEPROM_FACTORY_FLASH_MAGIC) ||
       (eeprom_factory_flash_read_word(4U) != EEPROM_FACTORY_FLASH_VERSION) ||
       (eeprom_factory_flash_read_word(8U) != EEPROM_FACTORY_FLASH_PAYLOAD_SIZE) ||
       (eeprom_factory_flash_payload_checksum() != eeprom_factory_flash_read_word(12U)))
    {
        return 0;
    }

    payload = (const uint8_t *)(eeprom_factory_flash_base_addr() +
                                (EEPROM_FACTORY_FLASH_HEADER_WORDS * sizeof(uint32_t)));

    memcpy((uint8_t *)&eeprom_page_read_data, payload, EE_24CXX_PAGE_SIZE);
    if((eeprom_page_read_data[1] != ((EEROM_INIT_DATA & 0xff00) >> 8)) ||
       (eeprom_page_read_data[0] !=  (EEROM_INIT_DATA & 0x00ff)))
    {
        return 0;
    }
    eeprom_wg_com_v2_param.is_writed = EEROM_INIT_DATA;
    memcpy((uint8_t *)&wg_com_v2_product_info,
           (uint8_t *)&eeprom_page_read_data[sizeof(eeprom_wg_com_v2_param.is_writed)],
           sizeof(wg_com_v2_product_info));

    memcpy((uint8_t *)&eeprom_page_read_data, payload + EE_24CXX_PAGE_SIZE, EE_24CXX_PAGE_SIZE);
    memcpy((uint8_t *)&wg_com_v2_ctrl,
           (uint8_t *)&eeprom_page_read_data,
           sizeof(wg_com_v2_ctrl));
    if(!eeprom_ctrl_is_valid(&wg_com_v2_ctrl))
    {
        return 0;
    }

    memcpy((uint8_t *)&eeprom_page_read_data, payload + (EE_24CXX_PAGE_SIZE * 2U), EE_24CXX_PAGE_SIZE);
    eeprom_param_copy_cal_from_page(&wg_com_v2_param);

    memcpy((uint8_t *)&eeprom_page_read_data, payload + (EE_24CXX_PAGE_SIZE * 3U), EE_24CXX_PAGE_SIZE);
    if(eeprom_page_is_blank((uint8_t *)&eeprom_page_read_data, EEPROM_PARAM_USER_SIZE))
    {
        return 0;
    }
    eeprom_param_copy_user_from_page(&wg_com_v2_param);
    if(!eeprom_param_user_is_valid(&wg_com_v2_param))
    {
        return 0;
    }

    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.FactoryReset);
    WG_COM_V2_SET_DATA_UINT(1, wg_com_v2_ctrl.PowerOnOff);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ZeroCurrCalibration);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ResetFactoryData);
    return 1;
}

static uint8_t eeprom_factory_restore_app_defaults_verified(void)
{
    eeprom_wg_com_v2_param.is_writed = EEROM_INIT_DATA;
    WG_COM_V2_SET_DATA_UINT((('V'<<8)+'1'), wg_com_v2_product_info.ProtocolVersion[0]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'1'), wg_com_v2_product_info.ProtocolVersion[1]);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_product_info.ProductType[0]);
    WG_COM_V2_SET_DATA_UINT(5, wg_com_v2_product_info.ProductType[1]);
    WG_COM_V2_SET_DATA_UINT((('V'<<8)+'1'), wg_com_v2_product_info.HardverVerzi[0]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'3'), wg_com_v2_product_info.HardverVerzi[1]);
    WG_COM_V2_SET_DATA_UINT((('V'<<8)+'2'), wg_com_v2_product_info.SoftVersion[0]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'1'), wg_com_v2_product_info.SoftVersion[1]);
    WG_COM_V2_SET_DATA_UINT((('W'<<8)+'G'), wg_com_v2_product_info.SnSerial[0]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'5'), wg_com_v2_product_info.SnSerial[1]);
    WG_COM_V2_SET_DATA_UINT((('-'<<8)+'2'), wg_com_v2_product_info.SnSerial[2]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'2'), wg_com_v2_product_info.SnSerial[3]);
    WG_COM_V2_SET_DATA_UINT((('5'<<8)+'0'), wg_com_v2_product_info.SnSerial[4]);
    WG_COM_V2_SET_DATA_UINT((('7'<<8)+'2'), wg_com_v2_product_info.SnSerial[5]);
    WG_COM_V2_SET_DATA_UINT((('2'<<8)+'-'), wg_com_v2_product_info.SnSerial[6]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'), wg_com_v2_product_info.SnSerial[7]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'), wg_com_v2_product_info.SnSerial[8]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'), wg_com_v2_product_info.SnSerial[9]);
    WG_COM_V2_SET_DATA_UINT((('W'<<8)+'G'), wg_com_v2_product_info.ProductName[0]);
    WG_COM_V2_SET_DATA_UINT((('-'<<8)+'B'), wg_com_v2_product_info.ProductName[1]);
    WG_COM_V2_SET_DATA_UINT((('C'<<8)+'1'), wg_com_v2_product_info.ProductName[2]);
    WG_COM_V2_SET_DATA_UINT((('5'<<8)+'0'), wg_com_v2_product_info.ProductName[3]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'M'), wg_com_v2_product_info.ProductName[4]);
    for(uint8_t i = 5; i < 10; i++)
    {
        WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '), wg_com_v2_product_info.ProductName[i]);
    }
    WG_COM_V2_SET_DATA_UINT(1, wg_com_v2_product_info.Address);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_product_info.ApplicationScenarios);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_product_info.CustomizationVersion);
    for(uint8_t i = 0; i < 10; i++)
    {
        WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '), wg_com_v2_product_info.MacAddress[i]);
    }
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_product_info.BtName);

    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.FactoryReset);
    WG_COM_V2_SET_DATA_UINT(1, wg_com_v2_ctrl.PowerOnOff);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetPowerMode);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetChargMode);
    WG_COM_V2_SET_DATA_UINT(0x0407, wg_com_v2_ctrl.InpBatyType);
    WG_COM_V2_SET_DATA_UINT(0x0400, wg_com_v2_ctrl.OutBatyType);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetBootTimeA);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetBootTimeB);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetOnCurrStartTimeA);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetOnCurrStartTimeB);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ZeroCurrCalibration);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ResetFactoryData);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.BatModeFR);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.MpptSwitch);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SleepModeOnOff);

    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpVoltCalibrK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpVoltCalibrB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpCurrCalibrK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpCurrCalibrB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpShowVoltCalibrK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpShowVoltCalibrB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpShowCurrCalibrK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpShowCurrCalibrB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutVoltCalibrK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutVoltCalibrB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutCurrCalibrK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutCurrCalibrB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutShowVoltCalibrK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutShowVoltCalibrB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutShowCurrCalibrK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutShowCurrCalibrB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.AOutShowCurrCalibrK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.AOutShowCurrCalibrB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.BOutShowCurrCalibrK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.BOutShowCurrCalibrB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.VoltCompensationAK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.VoltCompensationAB);
    WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.VoltCompensationBK);
    WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.VoltCompensationBB);
    for(uint8_t i = 0; i < 2; i++)
    {
        wg_com_v2_param.Retain10[i] = 0xFFFF;
    }
    WG_COM_V2_SET_DATA_UINT(12.0f, wg_com_v2_param.SetInpVolt);
    WG_COM_V2_SET_DATA_UINT(125.0f, wg_com_v2_param.SetInpCurr);
    WG_COM_V2_SET_DATA_UINT(1500.0f, wg_com_v2_param.SetInpCurrPower);
    WG_COM_V2_SET_DATA_UINT(12.0f, wg_com_v2_param.SetOutVolt);
    WG_COM_V2_SET_DATA_UINT(125.0f, wg_com_v2_param.SetOutCurr);
    WG_COM_V2_SET_DATA_UINT(1500.0f, wg_com_v2_param.SetOutCurrPower);
    WG_COM_V2_SET_DATA_UINT(10.0f, wg_com_v2_param.SetInpUvlo);
    WG_COM_V2_SET_DATA_UINT(11.0f, wg_com_v2_param.SetInpUvloRecover);
    WG_COM_V2_SET_DATA_UINT(61.5f, wg_com_v2_param.SetInpOVP);
    WG_COM_V2_SET_DATA_UINT(60.0f, wg_com_v2_param.SetInpOVPRecover);
    WG_COM_V2_SET_DATA_UINT(10.0f, wg_com_v2_param.SetOutUvlo);
    WG_COM_V2_SET_DATA_UINT(11.0f, wg_com_v2_param.SetOutUvloRecover);
    WG_COM_V2_SET_DATA_UINT(61.5f, wg_com_v2_param.SetOutOVP);
    WG_COM_V2_SET_DATA_UINT(60.0f, wg_com_v2_param.SetOutOVPRecover);
    WG_COM_V2_SET_DATA_INT(120.0f, wg_com_v2_param.SetInsideTemp);
    WG_COM_V2_SET_DATA_INT(120.0f, wg_com_v2_param.SetOutsideTemp);
    WG_COM_V2_SET_DATA_UINT(100.0f, wg_com_v2_param.SetInpChargLedCurr);
    WG_COM_V2_SET_DATA_UINT(99.0f, wg_com_v2_param.SetInpFullLedCurr);
    WG_COM_V2_SET_DATA_UINT(100.0f, wg_com_v2_param.SetOutChargLedCurr);
    WG_COM_V2_SET_DATA_UINT(99.0f, wg_com_v2_param.SetOutFullLedCurr);
    WG_COM_V2_SET_DATA_UINT(13.60f, wg_com_v2_param.AuotForwardOpenVoltA);
    WG_COM_V2_SET_DATA_UINT(12.00f, wg_com_v2_param.AuotForwardVeerVoltA);
    WG_COM_V2_SET_DATA_UINT(13.00f, wg_com_v2_param.AuotForwardShutVoltA);
    WG_COM_V2_SET_DATA_UINT(12.50f, wg_com_v2_param.AuotReverseOpenVoltB);
    WG_COM_V2_SET_DATA_UINT(12.00f, wg_com_v2_param.AuotReverseShutVoltB);
    WG_COM_V2_SET_DATA_INT(105.0f, wg_com_v2_param.SetTemp2);

    return eeprom_factory_restore_current_from_runtime_verified();
}

static uint8_t eeprom_profile_slot_from_sys(uint16_t sys)
{
    switch(sys)
    {
        case eSYS_12V:    return 0;
        case eSYS_16V:    return 1;
        case eSYS_24V:    return 2;
        case eSYS_36V:    return 3;
        case eSYS_48V:    return 4;
        case eSYS_10_60V: return 5;
        default:          return 0xFF;
    }
}

static uint8_t eeprom_profile_type_index_from_ctrl(uint16_t bat_type, uint8_t is_a_port)
{
    uint8_t type = (uint8_t)((bat_type & 0xff00) >> 8);

    switch(type)
    {
        case eBAT_LA_AGM:
        case eBAT_LA_GEL:
            return EEPROM_PROFILE_TYPE_AGM;

        case eBAT_LI_LFP:
            return EEPROM_PROFILE_TYPE_LFP;

        case eBAT_LI_NMC:
            return EEPROM_PROFILE_TYPE_NMC;

        case eBAT_DCDC:
            return EEPROM_PROFILE_TYPE_DCDC;

        case eSCAP:
            return EEPROM_PROFILE_TYPE_SCAP;

        case eBAT_AUTOSYS:
            return is_a_port ? EEPROM_PROFILE_TYPE_AUTOSYS : EEPROM_PROFILE_TYPE_LFP;

        default:
            return EEPROM_PROFILE_TYPE_LFP;
    }
}

static uint16_t eeprom_profile_addr_from_ctrl(uint8_t is_a_port, uint16_t bat_type)
{
    uint8_t volt_index = eeprom_profile_slot_from_sys(bat_type & 0x00FF);
    uint8_t raw_type = (uint8_t)((bat_type & 0xff00) >> 8);
    uint8_t type_index;
    uint16_t page;

    if(volt_index == 0xFF)
    {
        return 0xFFFF;
    }

    type_index = eeprom_profile_type_index_from_ctrl(bat_type, is_a_port);
    if((raw_type == eBAT_DCDC) && ((bat_type & 0x00FF) == eSYS_10_60V))
    {
        volt_index = EEPROM_PROFILE_SYS_12V;
    }
    page = eeprom_profile_calc_page((is_a_port != 0) ? EEPROM_PROFILE_PORT_A : EEPROM_PROFILE_PORT_B,
                                    type_index,
                                    volt_index);
    return (page == 0xFFFF) ? 0xFFFF : eeprom_profile_page_to_addr(page);
}

static uint8_t eeprom_profile_type_from_ctrl(uint16_t bat_type)
{
    uint8_t type = (uint8_t)((bat_type & 0xff00) >> 8);
    if(type == eBAT_LA_GEL)
    {
        type = eBAT_TYPE_AGM;
    }
    if(type >= eBAT_TYPE_MAX)
    {
        type = eBAT_TYPE_LFP;
    }
    return type;
}

static uint8_t eeprom_profile_bat_sys_from_ctrl(uint16_t sys)
{
    switch(sys)
    {
        case eSYS_12V: return eBAT_SYS_12V;
        case eSYS_16V: return eBAT_SYS_16V;
        case eSYS_24V: return eBAT_SYS_24V;
        case eSYS_36V: return eBAT_SYS_36V;
        case eSYS_48V: return eBAT_SYS_48V;
        default:       return eBAT_SYS_VOLT_MAX;
    }
}

static uint16_t eeprom_autosys_detect_a(void)
{
    float a_volt = get_wg_com_v2_data.com_realtime_data.InpVolt;
    if((a_volt >= 10.0f) && (a_volt <= 15.0f))
    {
        return eSYS_12V;
    }
    if((a_volt >= 20.0f) && (a_volt <= 30.0f))
    {
        return eSYS_24V;
    }
    if((a_volt >= 40.0f) && (a_volt <= 60.0f))
    {
        return eSYS_48V;
    }
    return eSYS_VOLT_MAX;
}

static uint16_t eeprom_float_to_raw(float value, void *wg_field)
{
    float unit = get_unit_for_addr(wg_field);
    return (uint16_t)(value / unit + ((unit * 5.0f) / 10.0f));
}

static float eeprom_raw_to_float(uint16_t value, void *wg_field)
{
    return ((float)value) * get_unit_for_addr(wg_field);
}

static uint16_t eeprom_get_profile_raw(void *wg_field)
{
    return get_uint16((uint8_t *)wg_field);
}

static void eeprom_set_profile_raw(void *wg_field, uint16_t value)
{
    set_uint16((uint8_t *)wg_field, value);
}

static uint8_t eeprom_profile_in_range(float value, float min, float max)
{
    return ((value >= min) && (value <= max)) ? 1 : 0;
}

#define EEPROM_PROFILE_BAT_VOLT_MIN 8.00f
#define EEPROM_PROFILE_BAT_VOLT_MAX 62.00f

static const BAT_MODE_CONFIG_T eeprom_dcdc_basic_default_cfg = {
    BASIC_BAT_SYS_12V_MAX_OUT_VOLT,
    BASIC_BAT_SYS_12V_MIN_OUT_VOLT,
    BASIC_BAT_SYS_12V_DEFAULT_OUT_VOLT,
    BASIC_BAT_SYS_12V_MAX_CHURR_VOLT,
    BASIC_BAT_SYS_12V_MIN_CHURR_VOLT,
    BASIC_BAT_SYS_12V_DEFAULT_CHURR_VOLT,
    BASIC_BAT_SYS_12V_OPEN_VOLT_A,
    BASIC_BAT_SYS_12V_CLOSE_VOLT_A,
    BASIC_BAT_SYS_12V_VEER_VOLT_A,
    BASIC_BAT_SYS_12V_OPEN_VOLT_B,
    BASIC_BAT_SYS_12V_CLOSE_VOLT_B,
    BASIC_BAT_SYS_12V_SET_LED_CHAR_CURR,
    BASIC_BAT_SYS_12V_SET_LED_FULL_CURR,
    BASIC_BAT_SYS_SET_UVLO,
    BASIC_BAT_SYS_SET_UVLORECOVER,
    BASIC_BAT_SYS_SET_OVP,
    BASIC_BAT_SYS_SET_OVPRECOVER,
    BASIC_BAT_SYS_12V_MAX_POWER_VOLT,
    BASIC_BAT_SYS_12V_MIN_POWER_VOLT,
    BASIC_BAT_SYS_12V_DEFAULT_POWER_VOLT
};

static void eeprom_profile_fill_default(eeprom_system_profile_t *profile, uint8_t is_a_port, uint16_t bat_type)
{
    uint8_t sys = eeprom_profile_bat_sys_from_ctrl(bat_type & 0x00FF);
    uint8_t raw_type = (uint8_t)((bat_type & 0xff00) >> 8);
    uint8_t type = eeprom_profile_type_from_ctrl(bat_type);
    const BAT_MODE_CONFIG_T *cfg = NULL;

    memset((uint8_t *)profile, 0xFF, sizeof(*profile));
    profile->is_writed = EEPROM_SYS_PROFILE_INIT_DATA;

    if(raw_type == eBAT_DCDC)
    {
        cfg = &eeprom_dcdc_basic_default_cfg;
    }
    else if(sys >= eBAT_SYS_VOLT_MAX)
    {
        return;
    }
    else
    {
        cfg = &Bat_Sys_Volt_Config[sys][type];
    }

    profile->SetVolt = eeprom_float_to_raw(cfg->OutVoltDefault, is_a_port ? (void *)&wg_com_v2_param.SetInpVolt : (void *)&wg_com_v2_param.SetOutVolt);
    profile->SetCurr = eeprom_float_to_raw(cfg->OutCurrDefault, is_a_port ? (void *)&wg_com_v2_param.SetInpCurr : (void *)&wg_com_v2_param.SetOutCurr);
    profile->SetCurrPower = eeprom_float_to_raw((float)cfg->OutPowerDefault, is_a_port ? (void *)&wg_com_v2_param.SetInpCurrPower : (void *)&wg_com_v2_param.SetOutCurrPower);
    profile->SetUvlo = eeprom_float_to_raw(cfg->SetUvlo, is_a_port ? (void *)&wg_com_v2_param.SetInpUvlo : (void *)&wg_com_v2_param.SetOutUvlo);
    profile->SetUvloRecover = eeprom_float_to_raw(cfg->SetUvloRecover, is_a_port ? (void *)&wg_com_v2_param.SetInpUvloRecover : (void *)&wg_com_v2_param.SetOutUvloRecover);
    profile->SetOVP = eeprom_float_to_raw(cfg->SetOVP, is_a_port ? (void *)&wg_com_v2_param.SetInpOVP : (void *)&wg_com_v2_param.SetOutOVP);
    profile->SetOVPRecover = eeprom_float_to_raw(cfg->SetOVPRecover, is_a_port ? (void *)&wg_com_v2_param.SetInpOVPRecover : (void *)&wg_com_v2_param.SetOutOVPRecover);
    profile->SetChargLedCurr = eeprom_float_to_raw(cfg->SetChargLedCurr, is_a_port ? (void *)&wg_com_v2_param.SetInpChargLedCurr : (void *)&wg_com_v2_param.SetOutChargLedCurr);
    profile->SetFullLedCurr = eeprom_float_to_raw(cfg->SetFullLedCurr, is_a_port ? (void *)&wg_com_v2_param.SetInpFullLedCurr : (void *)&wg_com_v2_param.SetOutFullLedCurr);
    profile->AutoOpenVolt = eeprom_float_to_raw(is_a_port ? cfg->OpenVoltA : cfg->OpenVoltB, is_a_port ? (void *)&wg_com_v2_param.AuotForwardOpenVoltA : (void *)&wg_com_v2_param.AuotReverseOpenVoltB);
    profile->AutoVeerVolt = eeprom_float_to_raw(is_a_port ? cfg->VeerVoltA : 0.0f, (void *)&wg_com_v2_param.AuotForwardVeerVoltA);
    profile->AutoCloseVolt = eeprom_float_to_raw(is_a_port ? cfg->CloseVoltA : cfg->CloseVoltB, is_a_port ? (void *)&wg_com_v2_param.AuotForwardShutVoltA : (void *)&wg_com_v2_param.AuotReverseShutVoltB);
    profile->SetBootTime = 0;
    profile->SetOnCurrStartTime = 0;
}

static uint8_t eeprom_profile_sanitize(uint8_t is_a_port,
                                       uint16_t bat_type,
                                       eeprom_system_profile_t *profile,
                                       uint8_t *fixed)
{
    uint8_t sys = eeprom_profile_bat_sys_from_ctrl(bat_type & 0x00FF);
    uint8_t raw_type = (uint8_t)((bat_type & 0xff00) >> 8);
    uint8_t type = eeprom_profile_type_from_ctrl(bat_type);
    const BAT_MODE_CONFIG_T *cfg = NULL;
    eeprom_system_profile_t default_profile;
    float volt_min;
    float volt_max;

    if(fixed != NULL)
    {
        *fixed = 0;
    }

    if(profile->is_writed != EEPROM_SYS_PROFILE_INIT_DATA)
    {
        return 0;
    }

    if(raw_type == eBAT_DCDC)
    {
        cfg = &eeprom_dcdc_basic_default_cfg;
    }
    else if(sys >= eBAT_SYS_VOLT_MAX)
    {
        return 0;
    }
    else
    {
        cfg = &Bat_Sys_Volt_Config[sys][type];
    }

    volt_min = EEPROM_PROFILE_BAT_VOLT_MIN;
    volt_max = EEPROM_PROFILE_BAT_VOLT_MAX;
    eeprom_profile_fill_default(&default_profile, is_a_port, bat_type);

    if(!eeprom_profile_in_range(eeprom_raw_to_float(profile->SetVolt, is_a_port ? (void *)&wg_com_v2_param.SetInpVolt : (void *)&wg_com_v2_param.SetOutVolt), volt_min, volt_max))
    {
        profile->SetVolt = default_profile.SetVolt;
        if(fixed != NULL) *fixed = 1;
    }
    if(!eeprom_profile_in_range(eeprom_raw_to_float(profile->SetCurr, is_a_port ? (void *)&wg_com_v2_param.SetInpCurr : (void *)&wg_com_v2_param.SetOutCurr), cfg->OutCurrMin, cfg->OutCurrMax))
    {
        profile->SetCurr = default_profile.SetCurr;
        if(fixed != NULL) *fixed = 1;
    }
    if((profile->SetCurrPower < cfg->OutPowerMin) || (profile->SetCurrPower > cfg->OutPowerMax))
    {
        profile->SetCurrPower = default_profile.SetCurrPower;
        if(fixed != NULL) *fixed = 1;
    }
    if(!eeprom_profile_in_range(eeprom_raw_to_float(profile->SetUvlo, is_a_port ? (void *)&wg_com_v2_param.SetInpUvlo : (void *)&wg_com_v2_param.SetOutUvlo), volt_min, volt_max))
    {
        profile->SetUvlo = default_profile.SetUvlo;
        if(fixed != NULL) *fixed = 1;
    }
    if(!eeprom_profile_in_range(eeprom_raw_to_float(profile->SetUvloRecover, is_a_port ? (void *)&wg_com_v2_param.SetInpUvloRecover : (void *)&wg_com_v2_param.SetOutUvloRecover), volt_min, volt_max))
    {
        profile->SetUvloRecover = default_profile.SetUvloRecover;
        if(fixed != NULL) *fixed = 1;
    }
    if(!is_a_port)
    {
        if(!eeprom_profile_in_range(eeprom_raw_to_float(profile->SetOVP, (void *)&wg_com_v2_param.SetOutOVP), volt_min, volt_max))
        {
            profile->SetOVP = default_profile.SetOVP;
            if(fixed != NULL) *fixed = 1;
        }
        if(!eeprom_profile_in_range(eeprom_raw_to_float(profile->SetOVPRecover, (void *)&wg_com_v2_param.SetOutOVPRecover), volt_min, volt_max))
        {
            profile->SetOVPRecover = default_profile.SetOVPRecover;
            if(fixed != NULL) *fixed = 1;
        }
    }
    if(!eeprom_profile_in_range(eeprom_raw_to_float(profile->SetChargLedCurr, is_a_port ? (void *)&wg_com_v2_param.SetInpChargLedCurr : (void *)&wg_com_v2_param.SetOutChargLedCurr), 0.0f, cfg->OutCurrMax))
    {
        profile->SetChargLedCurr = default_profile.SetChargLedCurr;
        if(fixed != NULL) *fixed = 1;
    }
    if(!eeprom_profile_in_range(eeprom_raw_to_float(profile->SetFullLedCurr, is_a_port ? (void *)&wg_com_v2_param.SetInpFullLedCurr : (void *)&wg_com_v2_param.SetOutFullLedCurr), 0.0f, cfg->OutCurrMax))
    {
        profile->SetFullLedCurr = default_profile.SetFullLedCurr;
        if(fixed != NULL) *fixed = 1;
    }
    if(!eeprom_profile_in_range(eeprom_raw_to_float(profile->AutoOpenVolt, is_a_port ? (void *)&wg_com_v2_param.AuotForwardOpenVoltA : (void *)&wg_com_v2_param.AuotReverseOpenVoltB), volt_min, volt_max))
    {
        profile->AutoOpenVolt = default_profile.AutoOpenVolt;
        if(fixed != NULL) *fixed = 1;
    }
    if(is_a_port &&
       !eeprom_profile_in_range(eeprom_raw_to_float(profile->AutoVeerVolt, (void *)&wg_com_v2_param.AuotForwardVeerVoltA), volt_min, volt_max))
    {
        profile->AutoVeerVolt = default_profile.AutoVeerVolt;
        if(fixed != NULL) *fixed = 1;
    }
    if(!eeprom_profile_in_range(eeprom_raw_to_float(profile->AutoCloseVolt, is_a_port ? (void *)&wg_com_v2_param.AuotForwardShutVoltA : (void *)&wg_com_v2_param.AuotReverseShutVoltB), volt_min, volt_max))
    {
        profile->AutoCloseVolt = default_profile.AutoCloseVolt;
        if(fixed != NULL) *fixed = 1;
    }
    if(profile->SetBootTime > EEPROM_PROFILE_TIME_MAX)
    {
        profile->SetBootTime = default_profile.SetBootTime;
        if(fixed != NULL) *fixed = 1;
    }
    if(profile->SetOnCurrStartTime > EEPROM_PROFILE_TIME_MAX)
    {
        profile->SetOnCurrStartTime = default_profile.SetOnCurrStartTime;
        if(fixed != NULL) *fixed = 1;
    }
    return 1;
}

static void eeprom_profile_read_or_init(uint8_t is_a_port, uint16_t bat_type, eeprom_system_profile_t *profile)
{
    uint16_t addr = eeprom_profile_addr_from_ctrl(is_a_port, bat_type);
    uint8_t port = (is_a_port != 0) ? EEPROM_PROFILE_PORT_A : EEPROM_PROFILE_PORT_B;
    uint8_t type = eeprom_profile_type_index_from_ctrl(bat_type, is_a_port);
    uint8_t sys = eeprom_profile_slot_from_sys(bat_type & 0x00FF);
    uint8_t fixed = 0;

    if((addr == 0xFFFF) || (sys == 0xFF))
    {
        eeprom_profile_fill_default(profile, is_a_port, bat_type);
        app_debug_event_push(APP_DBG_EVT_PROFILE_DEFAULT, APP_DBG_AREA_BAT, 0xFF,
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             type, APP_DBG_RESULT_INVALID, sys);
        return;
    }

    if(!eeprom_profile_read_payload(addr, port, type, sys, (uint8_t *)profile, sizeof(*profile)))
    {
        eeprom_profile_fill_default(profile, is_a_port, bat_type);
        (void)eeprom_profile_write_payload(addr, port, type, sys, (uint8_t *)profile, sizeof(*profile));
        app_debug_event_push(APP_DBG_EVT_PROFILE_DEFAULT, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                             app_debug_page_from_addr(addr),
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             type, APP_DBG_RESULT_INVALID, sys);
        return;
    }


    if((((bat_type & 0xff00) >> 8) != eBAT_DCDC) &&
       (((eeprom_profile_header_t *)eeprom_page_read_data)->reserved != EEPROM_PROFILE_RESERVED_USER))
    {
        eeprom_profile_fill_default(profile, is_a_port, bat_type);
        (void)eeprom_profile_write_payload(addr, port, type, sys, (uint8_t *)profile, sizeof(*profile));
        app_debug_event_push(APP_DBG_EVT_PROFILE_DEFAULT, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                             app_debug_page_from_addr(addr),
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             type, APP_DBG_RESULT_RETRY, sys);
        return;
    }
    if(!eeprom_profile_sanitize(is_a_port, bat_type, profile, &fixed))
    {
        eeprom_profile_fill_default(profile, is_a_port, bat_type);
        (void)eeprom_profile_write_payload(addr, port, type, sys, (uint8_t *)profile, sizeof(*profile));
        app_debug_event_push(APP_DBG_EVT_PROFILE_DEFAULT, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                             app_debug_page_from_addr(addr),
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             type, APP_DBG_RESULT_INVALID, sys);
    }
    else if(fixed != 0)
    {
        (void)eeprom_profile_write_payload(addr, port, type, sys, (uint8_t *)profile, sizeof(*profile));
        app_debug_event_push(APP_DBG_EVT_PROFILE_DEFAULT, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                             app_debug_page_from_addr(addr),
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             type, APP_DBG_RESULT_RETRY, sys);
    }
}

static void eeprom_profile_apply(uint8_t is_a_port, const eeprom_system_profile_t *profile)
{
    if(is_a_port)
    {
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetInpVolt, profile->SetVolt);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetInpCurr, profile->SetCurr);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetInpCurrPower, profile->SetCurrPower);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetInpUvlo, profile->SetUvlo);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetInpUvloRecover, profile->SetUvloRecover);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetInpOVP, profile->SetOVP);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetInpOVPRecover, profile->SetOVPRecover);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetInpChargLedCurr, profile->SetChargLedCurr);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetInpFullLedCurr, profile->SetFullLedCurr);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.AuotForwardOpenVoltA, profile->AutoOpenVolt);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.AuotForwardVeerVoltA, profile->AutoVeerVolt);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.AuotForwardShutVoltA, profile->AutoCloseVolt);
        eeprom_set_profile_raw((void *)&wg_com_v2_ctrl.SetBootTimeA, profile->SetBootTime);
        eeprom_set_profile_raw((void *)&wg_com_v2_ctrl.SetOnCurrStartTimeA, profile->SetOnCurrStartTime);
    }
    else
    {
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetOutVolt, profile->SetVolt);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetOutCurr, profile->SetCurr);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetOutCurrPower, profile->SetCurrPower);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetOutUvlo, profile->SetUvlo);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetOutUvloRecover, profile->SetUvloRecover);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetOutOVP, profile->SetOVP);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetOutOVPRecover, profile->SetOVPRecover);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetOutChargLedCurr, profile->SetChargLedCurr);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.SetOutFullLedCurr, profile->SetFullLedCurr);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.AuotReverseOpenVoltB, profile->AutoOpenVolt);
        eeprom_set_profile_raw((void *)&wg_com_v2_param.AuotReverseShutVoltB, profile->AutoCloseVolt);
        eeprom_set_profile_raw((void *)&wg_com_v2_ctrl.SetBootTimeB, profile->SetBootTime);
        eeprom_set_profile_raw((void *)&wg_com_v2_ctrl.SetOnCurrStartTimeB, profile->SetOnCurrStartTime);
    }
    app_debug_event_push(APP_DBG_EVT_PROFILE_APPLY, APP_DBG_AREA_BAT, 0,
                         (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                         (uint8_t)(is_a_port ? EEPROM_PROFILE_PORT_A : EEPROM_PROFILE_PORT_B),
                         APP_DBG_RESULT_OK, 0);
}

static void eeprom_mppt_profile_apply(const eeprom_system_profile_t *profile)
{
    eeprom_profile_apply(0, profile);
    eeprom_set_profile_raw((void *)&wg_com_v2_ctrl.SetBootTimeA, profile->SetBootTime);
    eeprom_set_profile_raw((void *)&wg_com_v2_ctrl.SetBootTimeB, 0);
}
static void eeprom_profile_capture(uint8_t is_a_port, eeprom_system_profile_t *profile)
{
    memset((uint8_t *)profile, 0xFF, sizeof(*profile));
    profile->is_writed = EEPROM_SYS_PROFILE_INIT_DATA;
    if(is_a_port)
    {
        profile->SetVolt = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetInpVolt);
        profile->SetCurr = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetInpCurr);
        profile->SetCurrPower = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetInpCurrPower);
        profile->SetUvlo = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetInpUvlo);
        profile->SetUvloRecover = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetInpUvloRecover);
        profile->SetOVP = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetInpOVP);
        profile->SetOVPRecover = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetInpOVPRecover);
        profile->SetChargLedCurr = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetInpChargLedCurr);
        profile->SetFullLedCurr = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetInpFullLedCurr);
        profile->AutoOpenVolt = eeprom_get_profile_raw((void *)&wg_com_v2_param.AuotForwardOpenVoltA);
        profile->AutoVeerVolt = eeprom_get_profile_raw((void *)&wg_com_v2_param.AuotForwardVeerVoltA);
        profile->AutoCloseVolt = eeprom_get_profile_raw((void *)&wg_com_v2_param.AuotForwardShutVoltA);
        profile->SetBootTime = eeprom_get_profile_raw((void *)&wg_com_v2_ctrl.SetBootTimeA);
        profile->SetOnCurrStartTime = eeprom_get_profile_raw((void *)&wg_com_v2_ctrl.SetOnCurrStartTimeA);
    }
    else
    {
        profile->SetVolt = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetOutVolt);
        profile->SetCurr = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetOutCurr);
        profile->SetCurrPower = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetOutCurrPower);
        profile->SetUvlo = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetOutUvlo);
        profile->SetUvloRecover = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetOutUvloRecover);
        profile->SetOVP = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetOutOVP);
        profile->SetOVPRecover = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetOutOVPRecover);
        profile->SetChargLedCurr = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetOutChargLedCurr);
        profile->SetFullLedCurr = eeprom_get_profile_raw((void *)&wg_com_v2_param.SetOutFullLedCurr);
        profile->AutoOpenVolt = eeprom_get_profile_raw((void *)&wg_com_v2_param.AuotReverseOpenVoltB);
        profile->AutoCloseVolt = eeprom_get_profile_raw((void *)&wg_com_v2_param.AuotReverseShutVoltB);
        profile->SetBootTime = eeprom_get_profile_raw((void *)&wg_com_v2_ctrl.SetBootTimeB);
        profile->SetOnCurrStartTime = eeprom_get_profile_raw((void *)&wg_com_v2_ctrl.SetOnCurrStartTimeB);
    }
}

static uint8_t eeprom_active_profile_is_valid(uint8_t is_a_port, uint16_t bat_type)
{
    eeprom_system_profile_t profile;
    uint8_t fixed = 0;

    eeprom_profile_capture(is_a_port, &profile);
    return (eeprom_profile_sanitize(is_a_port, bat_type, &profile, &fixed) && (fixed == 0)) ? 1 : 0;
}

static uint8_t eeprom_profile_write(uint8_t is_a_port, uint16_t bat_type)
{
    uint16_t addr = eeprom_profile_addr_from_ctrl(is_a_port, bat_type);
    eeprom_system_profile_t profile;
    uint8_t port = (is_a_port != 0) ? EEPROM_PROFILE_PORT_A : EEPROM_PROFILE_PORT_B;
    uint8_t type = eeprom_profile_type_index_from_ctrl(bat_type, is_a_port);
    uint8_t sys = eeprom_profile_slot_from_sys(bat_type & 0x00FF);

    if((addr == 0xFFFF) || (sys == 0xFF))
    {
        return 0;
    }

    eeprom_profile_capture(is_a_port, &profile);
    if(eeprom_profile_write_user_payload(addr, port, type, sys, (uint8_t *)&profile, sizeof(profile)))
    {
        app_debug_event_push(APP_DBG_EVT_PROFILE_SAVE, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                             app_debug_page_from_addr(addr),
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             type, APP_DBG_RESULT_OK, sys);
        return 1;
    }
    app_debug_event_push(APP_DBG_EVT_PROFILE_SAVE, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                         app_debug_page_from_addr(addr),
                         (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                         type, APP_DBG_RESULT_FAIL, sys);
    return 0;
}

static void eeprom_mppt_profile_capture(eeprom_system_profile_t *profile)
{
    eeprom_profile_capture(0, profile);
    profile->SetBootTime = eeprom_get_profile_raw((void *)&wg_com_v2_ctrl.SetBootTimeA);
}
static uint8_t eeprom_profile_write_snapshot(uint8_t is_a_port,
                                             uint16_t bat_type,
                                             const eeprom_system_profile_t *profile)
{
    uint16_t addr = eeprom_profile_addr_from_ctrl(is_a_port, bat_type);
    uint8_t port = (is_a_port != 0) ? EEPROM_PROFILE_PORT_A : EEPROM_PROFILE_PORT_B;
    uint8_t type = eeprom_profile_type_index_from_ctrl(bat_type, is_a_port);
    uint8_t sys = eeprom_profile_slot_from_sys(bat_type & 0x00FF);

    if((addr == 0xFFFF) || (sys == 0xFF) || (profile == NULL))
    {
        return 0;
    }

    if(eeprom_profile_write_user_payload(addr, port, type, sys, (uint8_t *)profile, sizeof(*profile)))
    {
        app_debug_event_push(APP_DBG_EVT_PROFILE_SAVE, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                             app_debug_page_from_addr(addr),
                             (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                             type, APP_DBG_RESULT_OK, sys);
        return 1;
    }
    app_debug_event_push(APP_DBG_EVT_PROFILE_SAVE, app_debug_area_from_page(app_debug_page_from_addr(addr)),
                         app_debug_page_from_addr(addr),
                         (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                         type, APP_DBG_RESULT_FAIL, sys);
    return 0;
}

static uint8_t eeprom_profile_write_mppt_b_snapshot(uint16_t bat_type,
                                                    const eeprom_system_profile_t *profile)
{
    eeprom_system_profile_t merged;

    if(profile == NULL)
    {
        return 0;
    }

    eeprom_profile_read_or_init(0, bat_type, &merged);
    merged.SetVolt = profile->SetVolt;
    merged.SetCurr = profile->SetCurr;
    merged.SetCurrPower = profile->SetCurrPower;
    merged.SetUvlo = profile->SetUvlo;
    merged.SetUvloRecover = profile->SetUvloRecover;
    merged.SetOVP = profile->SetOVP;
    merged.SetOVPRecover = profile->SetOVPRecover;
    merged.SetChargLedCurr = profile->SetChargLedCurr;
    merged.SetFullLedCurr = profile->SetFullLedCurr;
    merged.AutoOpenVolt = profile->AutoOpenVolt;
    merged.AutoVeerVolt = profile->AutoVeerVolt;
    merged.AutoCloseVolt = profile->AutoCloseVolt;

    if(!eeprom_profile_write_snapshot(0, bat_type, &merged))
    {
        return 0;
    }
    return eeprom_save_mppt_timing(profile->SetBootTime, profile->SetOnCurrStartTime);
}
static void eeprom_refresh_control_cache(void)
{
    uint16_t mode = 0;
    uint16_t bat_type_a = 0;
    uint16_t bat_type_b = 0;
    uint16_t bat_mode_fr = 0;
    uint16_t mppt_switch = 0;
    uint16_t sleep_mode = 0;

    WG_COM_V2_GET_DATA_UINT(mode, wg_com_v2_ctrl.SetPowerMode);
    WG_COM_V2_GET_DATA_UINT(bat_type_a, wg_com_v2_ctrl.InpBatyType);
    WG_COM_V2_GET_DATA_UINT(bat_type_b, wg_com_v2_ctrl.OutBatyType);
    WG_COM_V2_GET_DATA_UINT(bat_mode_fr, wg_com_v2_ctrl.BatModeFR);
    WG_COM_V2_GET_DATA_UINT(mppt_switch, wg_com_v2_ctrl.MpptSwitch);
    WG_COM_V2_GET_DATA_UINT(sleep_mode, wg_com_v2_ctrl.SleepModeOnOff);

    get_wg_com_v2_data.com_ctrl.SetPowerMode = mode;
    get_wg_com_v2_data.com_ctrl.InpBatyType = bat_type_a;
    get_wg_com_v2_data.com_ctrl.OutBatyType = bat_type_b;
    get_wg_com_v2_data.com_ctrl.BatModeFR = bat_mode_fr;
    get_wg_com_v2_data.com_ctrl.MpptSwitch = mppt_switch;
    get_wg_com_v2_data.com_ctrl.SleepModeOnOff = sleep_mode;
}

void eeprom_apply_mppt_fixed_input_params(void)
{
    const float inp_curr = 125.00f;
    const float inp_charge_led = inp_curr * 0.15f;

    WG_COM_V2_SET_DATA_UINT(12.00f, wg_com_v2_param.SetInpVolt);
    WG_COM_V2_SET_DATA_UINT(inp_curr, wg_com_v2_param.SetInpCurr);
    WG_COM_V2_SET_DATA_UINT(1500.00f, wg_com_v2_param.SetInpCurrPower);
    WG_COM_V2_SET_DATA_UINT(10.00f, wg_com_v2_param.SetInpUvlo);
    WG_COM_V2_SET_DATA_UINT(11.00f, wg_com_v2_param.SetInpUvloRecover);
    WG_COM_V2_SET_DATA_UINT(62.00f, wg_com_v2_param.SetInpOVP);
    WG_COM_V2_SET_DATA_UINT(61.00f, wg_com_v2_param.SetInpOVPRecover);
    WG_COM_V2_SET_DATA_UINT(inp_charge_led, wg_com_v2_param.SetInpChargLedCurr);
    WG_COM_V2_SET_DATA_UINT(inp_charge_led - 0.50f, wg_com_v2_param.SetInpFullLedCurr);
    WG_COM_V2_SET_DATA_UINT(13.60f, wg_com_v2_param.AuotForwardOpenVoltA);
    WG_COM_V2_SET_DATA_UINT(13.00f, wg_com_v2_param.AuotForwardVeerVoltA);
    WG_COM_V2_SET_DATA_UINT(12.00f, wg_com_v2_param.AuotForwardShutVoltA);
}

uint8_t eeprom_apply_mppt_mode_profile(void)
{
    uint16_t bat_type;
    uint16_t mppt_boot_time_a = 0;
    uint16_t mppt_soft_start_b = 0;
    eeprom_system_profile_t profile;

    eeprom_refresh_control_cache();
    bat_type = get_wg_com_v2_data.com_ctrl.OutBatyType;

    if(get_wg_com_v2_data.com_ctrl.SetPowerMode != eMPPT_MODE)
    {
        return 0;
    }

    eeprom_profile_read_or_init(0, bat_type, &profile);
    eeprom_mppt_profile_apply(&profile);
    eeprom_apply_mppt_fixed_input_params();
    if(eeprom_load_mppt_timing(&mppt_boot_time_a, &mppt_soft_start_b) != 0U)
    {
        WG_COM_V2_SET_DATA_UINT(mppt_boot_time_a, wg_com_v2_ctrl.SetBootTimeA);
        WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetBootTimeB);
        WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetOnCurrStartTimeA);
        WG_COM_V2_SET_DATA_UINT(mppt_soft_start_b, wg_com_v2_ctrl.SetOnCurrStartTimeB);
    }
    return 1;
}

uint8_t eeprom_apply_standard_mode_profile(void)
{
    eeprom_refresh_control_cache();
    if(get_wg_com_v2_data.com_ctrl.SetPowerMode != eSET_STANDARD_MODE)
    {
        return 0;
    }
    return eeprom_current_param_valid;
}

uint8_t eeprom_apply_basic_mode_profile(void)
{
    eeprom_refresh_control_cache();
    if(get_wg_com_v2_data.com_ctrl.SetPowerMode != eSET_CUSTOM_MODE)
    {
        return 0;
    }
    return eeprom_current_param_valid;
}

static uint8_t eeprom_write_p03_user_current_page(void);

uint8_t eeprom_save_current_mode_profile(void)
{
    eeprom_refresh_control_cache();
    switch(get_wg_com_v2_data.com_ctrl.SetPowerMode)
    {
        case eSET_BAT_MODE:
            if(eeprom_battery_profile_reload_pending != 0)
            {
                return 1;
            }
            return eeprom_save_battery_mode_profiles();

        case eMPPT_MODE:
            {
                eeprom_system_profile_t profile;

                eeprom_mppt_profile_capture(&profile);
                return eeprom_profile_write_mppt_b_snapshot(get_wg_com_v2_data.com_ctrl.OutBatyType, &profile);
            }

        case eSET_STANDARD_MODE:
        case eSET_CUSTOM_MODE:
            return eeprom_write_p03_user_current_page();

        default:
            return 1;
    }
}

uint8_t eeprom_save_current_timing_profile(void)
{
    eeprom_refresh_control_cache();
    switch(get_wg_com_v2_data.com_ctrl.SetPowerMode)
    {
        case eSET_BAT_MODE:
            return eeprom_save_battery_mode_profiles();

        case eMPPT_MODE:
            {
                eeprom_system_profile_t profile;

                eeprom_mppt_profile_capture(&profile);
                return eeprom_profile_write_mppt_b_snapshot(get_wg_com_v2_data.com_ctrl.OutBatyType, &profile);
            }

        default:
            return eeprom_save_current_mode_profile();
    }
}
static uint8_t pending_profile_save = 0;
static uint16_t pending_profile_mode = 0;
static uint16_t pending_profile_a_type = 0;
static uint16_t pending_profile_b_type = 0;
static eeprom_system_profile_t pending_profile_a;
static eeprom_system_profile_t pending_profile_b;
static uint8_t pending_user_profile[EEPROM_PARAM_USER_SIZE];

void eeprom_request_current_profile_save(void)
{
    uint16_t mode;
    uint16_t bat_type_a;
    uint16_t bat_type_b;

    eeprom_refresh_control_cache();
    mode = (uint16_t)get_wg_com_v2_data.com_ctrl.SetPowerMode;
    bat_type_a = (uint16_t)get_wg_com_v2_data.com_ctrl.InpBatyType;
    bat_type_b = (uint16_t)get_wg_com_v2_data.com_ctrl.OutBatyType;

    if((mode == eSET_BAT_MODE) && (eeprom_battery_profile_reload_pending != 0))
    {
        return;
    }

    pending_profile_mode = mode;
    pending_profile_a_type = bat_type_a;
    pending_profile_b_type = bat_type_b;
    if(mode == eSET_BAT_MODE)
    {
        eeprom_profile_capture(1, &pending_profile_a);
        eeprom_profile_capture(0, &pending_profile_b);
    }
    else if(mode == eMPPT_MODE)
    {
        eeprom_mppt_profile_capture(&pending_profile_b);
    }
    else
    {
        eeprom_user_profile_capture(pending_user_profile);
    }
    pending_profile_save = 1;
}

void eeprom_note_battery_profile_reload_pending(void)
{
    eeprom_battery_profile_reload_pending = 1;
    pending_profile_save = 0;
}

static uint8_t eeprom_flush_pending_profile_save(void)
{

    if(pending_profile_save == 0)
    {
        return 1;
    }

    switch(pending_profile_mode)
    {
        case eSET_BAT_MODE:
            if(!eeprom_profile_write_snapshot(1, pending_profile_a_type, &pending_profile_a))
            {
                return 0;
            }

            if(!eeprom_profile_write_snapshot(0, pending_profile_b_type, &pending_profile_b))
            {
                return 0;
            }
            break;

        case eMPPT_MODE:
            if(!eeprom_profile_write_mppt_b_snapshot(pending_profile_b_type, &pending_profile_b))
            {
                return 0;
            }
            break;

        case eSET_STANDARD_MODE:
        case eSET_CUSTOM_MODE:
            break;

        default:
            break;
    }

    pending_profile_save = 0;
    return 1;
}

static uint8_t eeprom_factory_rebuild_current_mode_profile(void)
{
    uint16_t mode;
    eeprom_refresh_control_cache();
    mode = (uint16_t)get_wg_com_v2_data.com_ctrl.SetPowerMode;

    switch(mode)
    {
        case eSET_BAT_MODE:
            if(!eeprom_save_battery_mode_profiles())
            {
                return 0;
            }
            eeprom_battery_profile_reload_pending = 0;
            break;

        case eMPPT_MODE:
            {
                eeprom_system_profile_t profile;

                eeprom_mppt_profile_capture(&profile);
                if(!eeprom_profile_write_mppt_b_snapshot(get_wg_com_v2_data.com_ctrl.OutBatyType, &profile))
                {
                    return 0;
                }
            }
            break;

        default:
            break;
    }

    pending_profile_save = 0;
    return 1;
}

uint8_t eeprom_apply_battery_mode_profiles(void)
{
    uint16_t bat_type_a;
    uint16_t bat_type_b;
    eeprom_system_profile_t profile;

    eeprom_refresh_control_cache();
    bat_type_a = get_wg_com_v2_data.com_ctrl.InpBatyType;
    bat_type_b = get_wg_com_v2_data.com_ctrl.OutBatyType;

    if(get_wg_com_v2_data.com_ctrl.SetPowerMode != eSET_BAT_MODE)
    {
        fault_clear_alarm(ALARM_AUTOSYS_NO_SYSTEM);
        return 0;
    }

    if(((bat_type_a & 0xff00) >> 8) == eBAT_AUTOSYS)
    {
        uint16_t detected_sys = eeprom_autosys_detect_a();
        uint16_t current_sys = bat_type_a & 0x00FF;
        if(detected_sys >= eSYS_VOLT_MAX)
        {
            WG_COM_V2_SET_DATA_UINT((eBAT_AUTOSYS << 8) | eSYS_VOLT_MAX, wg_com_v2_ctrl.InpBatyType);
            WG_COM_V2_SET_DATA_UINT(1, wg_com_v2_ctrl.PowerOnOff);
            WG_COM_V2_SET_DATA_UINT(eIDIE_CHARGE, wg_com_v2_realtime_data.StateCharge);
            fault_set_alarm(ALARM_AUTOSYS_NO_SYSTEM);
            return 1;
        }

        bat_type_a = (eBAT_AUTOSYS << 8) | detected_sys;
        WG_COM_V2_SET_DATA_UINT(bat_type_a, wg_com_v2_ctrl.InpBatyType);
        if(current_sys != detected_sys)
        {
            (void)eeprom_write_p02_current();
        }
        if(current_sys >= eSYS_VOLT_MAX)
        {
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.PowerOnOff);
        }
        fault_clear_alarm(ALARM_AUTOSYS_NO_SYSTEM);
    }
    else
    {
        fault_clear_alarm(ALARM_AUTOSYS_NO_SYSTEM);
    }

    eeprom_profile_read_or_init(1, bat_type_a, &profile);
    eeprom_profile_apply(1, &profile);
    eeprom_profile_read_or_init(0, bat_type_b, &profile);
    eeprom_profile_apply(0, &profile);
    get_wg_com_data_rum();
    if(eeprom_battery_profile_reload_pending != 0)
    {
        (void)eeprom_write_p03_user_current();
        eeprom_battery_profile_reload_pending = 0;
    }
    return 1;
}

uint8_t eeprom_autosys_runtime_update(void)
{
    uint16_t bat_type_a = get_wg_com_v2_data.com_ctrl.InpBatyType;
    uint16_t detected_sys;
    uint16_t current_sys;
    eeprom_system_profile_t profile;

    if(get_wg_com_v2_data.com_ctrl.SetPowerMode != eSET_BAT_MODE)
    {
        fault_clear_alarm(ALARM_AUTOSYS_NO_SYSTEM);
        return 0;
    }

    if(((bat_type_a & 0xff00) >> 8) != eBAT_AUTOSYS)
    {
        fault_clear_alarm(ALARM_AUTOSYS_NO_SYSTEM);
        return 0;
    }

    detected_sys = eeprom_autosys_detect_a();
    current_sys = bat_type_a & 0x00FF;

    if(detected_sys >= eSYS_VOLT_MAX)
    {
        if(current_sys != eSYS_VOLT_MAX)
        {
            WG_COM_V2_SET_DATA_UINT((eBAT_AUTOSYS << 8) | eSYS_VOLT_MAX, wg_com_v2_ctrl.InpBatyType);
            State_Control_Data.InpBatyType = (eBAT_AUTOSYS << 8) | eSYS_VOLT_MAX;
        }
        WG_COM_V2_SET_DATA_UINT(1, wg_com_v2_ctrl.PowerOnOff);
        WG_COM_V2_SET_DATA_UINT(eIDIE_CHARGE, wg_com_v2_realtime_data.StateCharge);
        fault_set_alarm(ALARM_AUTOSYS_NO_SYSTEM);
        return 1;
    }

    fault_clear_alarm(ALARM_AUTOSYS_NO_SYSTEM);

    if(current_sys >= eSYS_VOLT_MAX)
    {
        WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.PowerOnOff);
    }

    if(current_sys == detected_sys)
    {
        if(eeprom_active_profile_is_valid(1, bat_type_a))
        {
            return 0;
        }

        eeprom_profile_read_or_init(1, bat_type_a, &profile);
        eeprom_profile_apply(1, &profile);
        return 0;
    }

    bat_type_a = (eBAT_AUTOSYS << 8) | detected_sys;
    WG_COM_V2_SET_DATA_UINT(bat_type_a, wg_com_v2_ctrl.InpBatyType);
    State_Control_Data.InpBatyType = bat_type_a;
    (void)eeprom_write_p02_current();
    eeprom_profile_read_or_init(1, bat_type_a, &profile);
    eeprom_profile_apply(1, &profile);
    return 1;
}

uint8_t eeprom_save_battery_mode_profiles(void)
{
    eeprom_refresh_control_cache();
    if(get_wg_com_v2_data.com_ctrl.SetPowerMode != eSET_BAT_MODE)
    {
        return 1;
    }

    if(!eeprom_profile_write(1, get_wg_com_v2_data.com_ctrl.InpBatyType))
    {
        return 0;
    }

    if(!eeprom_profile_write(0, get_wg_com_v2_data.com_ctrl.OutBatyType))
    {
        return 0;
    }

    return 1;
}

void eeprom_cfg_init(void)
{
    IICx_Read_Byte(P00_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
    app_debug_event_push(APP_DBG_EVT_BOOT_LOAD, APP_DBG_AREA_P00, EEPROM_PAGE_P00_CURRENT,
                         0, 0, APP_DBG_RESULT_OK, 0);
    memcpy((uint8_t *)&eeprom_wg_com_v2_param, 
           (uint8_t *)&eeprom_page_read_data, 
           (sizeof(eeprom_wg_com_v2_param.is_writed)+sizeof(eeprom_wg_com_v2_param.wg_com_v2_product_info)));

    IICx_Read_Byte(P02_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
    app_debug_event_push(APP_DBG_EVT_BOOT_LOAD, APP_DBG_AREA_P02, EEPROM_PAGE_P02_CURRENT,
                         0, 0, APP_DBG_RESULT_OK, 0);
    memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl, 
           (uint8_t *)&eeprom_page_read_data, 
            sizeof(eeprom_wg_com_v2_param.wg_com_v2_ctrl));
    if(!eeprom_ctrl_is_valid(&eeprom_wg_com_v2_param.wg_com_v2_ctrl))
    {
        app_debug_event_push(APP_DBG_EVT_BOOT_LOAD, APP_DBG_AREA_P02, EEPROM_PAGE_P02_CURRENT,
                             (uint8_t)eeprom_ctrl_get(&eeprom_wg_com_v2_param.wg_com_v2_ctrl.SetPowerMode),
                             (uint8_t)((eeprom_ctrl_get(&eeprom_wg_com_v2_param.wg_com_v2_ctrl.InpBatyType) & 0xFF00U) >> 8),
                             APP_DBG_RESULT_INVALID,
                             (uint8_t)(eeprom_ctrl_get(&eeprom_wg_com_v2_param.wg_com_v2_ctrl.InpBatyType) & 0x00FFU));
        eeprom_ctrl_fill_safe_default();
        eeprom_current_param_valid = 0;
    }
    else
    {
        app_debug_event_push(APP_DBG_EVT_BOOT_LOAD, APP_DBG_AREA_P02, EEPROM_PAGE_P02_CURRENT,
                             (uint8_t)eeprom_ctrl_get(&eeprom_wg_com_v2_param.wg_com_v2_ctrl.SetPowerMode),
                             (uint8_t)((eeprom_ctrl_get(&eeprom_wg_com_v2_param.wg_com_v2_ctrl.InpBatyType) & 0xFF00U) >> 8),
                             APP_DBG_RESULT_OK,
                             (uint8_t)(eeprom_ctrl_get(&eeprom_wg_com_v2_param.wg_com_v2_ctrl.InpBatyType) & 0x00FFU));
    }

    IICx_Read_Byte(P03_1_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
    app_debug_event_push(APP_DBG_EVT_BOOT_LOAD, APP_DBG_AREA_CAL, EEPROM_PAGE_CAL_CURRENT,
                         0, 0, APP_DBG_RESULT_OK, 0);
    eeprom_param_copy_cal_from_page(&eeprom_wg_com_v2_param.wg_com_v2_param);

    IICx_Read_Byte(P03_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
    app_debug_event_push(APP_DBG_EVT_BOOT_LOAD, APP_DBG_AREA_PARAM, EEPROM_PAGE_PARAM_CURRENT,
                         0, 0, APP_DBG_RESULT_OK, 0);
    if(!eeprom_page_is_blank((uint8_t *)&eeprom_page_read_data, EEPROM_PARAM_USER_SIZE))
    {
        eeprom_param_copy_user_from_page(&eeprom_wg_com_v2_param.wg_com_v2_param);
    }
    else
    {
        eeprom_current_param_valid = 0;
    }
    if(!eeprom_param_user_is_valid(&eeprom_wg_com_v2_param.wg_com_v2_param))
    {
        eeprom_current_param_valid = 0;
    }
    if(eeprom_param_fix_safe_temperatures(&eeprom_wg_com_v2_param.wg_com_v2_param))
    {
        eeprom_current_param_valid = 0;
    }
  
    if (eeprom_wg_com_v2_param.is_writed != EEROM_INIT_DATA)
    {
            eeprom_wg_com_v2_param.is_writed = EEROM_INIT_DATA;
            WG_COM_V2_SET_DATA_UINT((('V'<<8)+'1'),wg_com_v2_product_info.ProtocolVersion[0]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'1'),wg_com_v2_product_info.ProtocolVersion[1]);
            WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.ProductType[0]);
            WG_COM_V2_SET_DATA_UINT(5,wg_com_v2_product_info.ProductType[1]);
            WG_COM_V2_SET_DATA_UINT((('V'<<8)+'1'),wg_com_v2_product_info.HardverVerzi[0]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'3'),wg_com_v2_product_info.HardverVerzi[1]);
            WG_COM_V2_SET_DATA_UINT((('V'<<8)+'2'),wg_com_v2_product_info.SoftVersion[0]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'1'),wg_com_v2_product_info.SoftVersion[1]);
            WG_COM_V2_SET_DATA_UINT((('W'<<8)+'G'),wg_com_v2_product_info.SnSerial[0]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'5'),wg_com_v2_product_info.SnSerial[1]);
            WG_COM_V2_SET_DATA_UINT((('-'<<8)+'2'),wg_com_v2_product_info.SnSerial[2]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'2'),wg_com_v2_product_info.SnSerial[3]);
            WG_COM_V2_SET_DATA_UINT((('5'<<8)+'0'),wg_com_v2_product_info.SnSerial[4]);
            WG_COM_V2_SET_DATA_UINT((('7'<<8)+'2'),wg_com_v2_product_info.SnSerial[5]);
            WG_COM_V2_SET_DATA_UINT((('2'<<8)+'-'),wg_com_v2_product_info.SnSerial[6]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'),wg_com_v2_product_info.SnSerial[7]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'),wg_com_v2_product_info.SnSerial[8]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'),wg_com_v2_product_info.SnSerial[9]);
            WG_COM_V2_SET_DATA_UINT((('W'<<8)+'G'),wg_com_v2_product_info.ProductName[0]);
            WG_COM_V2_SET_DATA_UINT((('-'<<8)+'B'),wg_com_v2_product_info.ProductName[1]);
            WG_COM_V2_SET_DATA_UINT((('C'<<8)+'1'),wg_com_v2_product_info.ProductName[2]);
            WG_COM_V2_SET_DATA_UINT((('5'<<8)+'0'),wg_com_v2_product_info.ProductName[3]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'M'),wg_com_v2_product_info.ProductName[4]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[5]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[6]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[7]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[8]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[9]);
            WG_COM_V2_SET_DATA_UINT(1,wg_com_v2_product_info.Address);
            WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.ApplicationScenarios);
            WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.CustomizationVersion);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[0]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[1]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[2]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[3]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[4]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[5]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[6]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[7]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[8]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[9]);
            WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.BtName);
            
            memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_product_info,
                   (uint8_t *)&wg_com_v2_product_info,
                   sizeof(wg_com_v2_product_info));

            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.FactoryReset);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.PowerOnOff);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetPowerMode);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetChargMode);
            WG_COM_V2_SET_DATA_UINT(0x0407, wg_com_v2_ctrl.InpBatyType);
            WG_COM_V2_SET_DATA_UINT(0x0400, wg_com_v2_ctrl.OutBatyType);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetBootTimeA);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetBootTimeB);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetOnCurrStartTimeA);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetOnCurrStartTimeB);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ZeroCurrCalibration);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ResetFactoryData);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.BatModeFR);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.MpptSwitch);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SleepModeOnOff);
            
            memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
                   (uint8_t *)&wg_com_v2_ctrl,
                   sizeof(wg_com_v2_ctrl));
            
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpVoltCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpVoltCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpShowVoltCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpShowVoltCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpShowCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpShowCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutVoltCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutVoltCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutShowVoltCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutShowVoltCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutShowCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutShowCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.AOutShowCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.AOutShowCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.BOutShowCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.BOutShowCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.VoltCompensationAK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.VoltCompensationAB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.VoltCompensationBK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.VoltCompensationBB);
            for (uint8_t i = 0; i < 2; i++)
            {
                wg_com_v2_param.Retain10[i] = 0xFFFF;
            }
            WG_COM_V2_SET_DATA_UINT(12.0f, wg_com_v2_param.SetInpVolt);
            WG_COM_V2_SET_DATA_UINT(125.0f, wg_com_v2_param.SetInpCurr);
            WG_COM_V2_SET_DATA_UINT(1500.0f, wg_com_v2_param.SetInpCurrPower);
            WG_COM_V2_SET_DATA_UINT(12.0f, wg_com_v2_param.SetOutVolt);
            WG_COM_V2_SET_DATA_UINT(125.0f, wg_com_v2_param.SetOutCurr);
            WG_COM_V2_SET_DATA_UINT(1500.0f, wg_com_v2_param.SetOutCurrPower);
            WG_COM_V2_SET_DATA_UINT(10.0f, wg_com_v2_param.SetInpUvlo);
            WG_COM_V2_SET_DATA_UINT(11.0f, wg_com_v2_param.SetInpUvloRecover);
            WG_COM_V2_SET_DATA_UINT(61.5f, wg_com_v2_param.SetInpOVP);
            WG_COM_V2_SET_DATA_UINT(60.0f, wg_com_v2_param.SetInpOVPRecover);
            WG_COM_V2_SET_DATA_UINT(10.0f, wg_com_v2_param.SetOutUvlo);
            WG_COM_V2_SET_DATA_UINT(11.0f, wg_com_v2_param.SetOutUvloRecover);
            WG_COM_V2_SET_DATA_UINT(61.5f, wg_com_v2_param.SetOutOVP);
            WG_COM_V2_SET_DATA_UINT(60.0f, wg_com_v2_param.SetOutOVPRecover);
            WG_COM_V2_SET_DATA_INT(120.0f, wg_com_v2_param.SetInsideTemp);
            WG_COM_V2_SET_DATA_INT(120.0f, wg_com_v2_param.SetOutsideTemp);
            WG_COM_V2_SET_DATA_UINT(100.0f, wg_com_v2_param.SetInpChargLedCurr);
            WG_COM_V2_SET_DATA_UINT(99.0f, wg_com_v2_param.SetInpFullLedCurr);
            WG_COM_V2_SET_DATA_UINT(100.0f, wg_com_v2_param.SetOutChargLedCurr);
            WG_COM_V2_SET_DATA_UINT(99.0f, wg_com_v2_param.SetOutFullLedCurr);
            WG_COM_V2_SET_DATA_UINT(13.60f, wg_com_v2_param.AuotForwardOpenVoltA);
            WG_COM_V2_SET_DATA_UINT(12.00f, wg_com_v2_param.AuotForwardVeerVoltA);
            WG_COM_V2_SET_DATA_UINT(13.00f, wg_com_v2_param.AuotForwardShutVoltA);
            WG_COM_V2_SET_DATA_UINT(12.50f, wg_com_v2_param.AuotReverseOpenVoltB);
            WG_COM_V2_SET_DATA_UINT(12.00f, wg_com_v2_param.AuotReverseShutVoltB);
            WG_COM_V2_SET_DATA_INT(105.0f, wg_com_v2_param.SetTemp2);

            memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param,
                   (uint8_t *)&wg_com_v2_param,
                   sizeof(wg_com_v2_param));

            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_wg_com_v2_param, 
                   (sizeof(eeprom_wg_com_v2_param.is_writed)+sizeof(eeprom_wg_com_v2_param.wg_com_v2_product_info)));
            (void)eeprom_write_page_verify(P00_ADDR);
            
            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl, 
                    sizeof(eeprom_wg_com_v2_param.wg_com_v2_ctrl));
            (void)eeprom_write_page_verify(P02_ADDR);
            
            eeprom_param_copy_cal_to_page(&eeprom_wg_com_v2_param.wg_com_v2_param);
            (void)eeprom_write_page_verify(P03_1_ADDR);

            eeprom_param_copy_user_to_page(&eeprom_wg_com_v2_param.wg_com_v2_param);
            (void)eeprom_write_page_verify(P03_ADDR);

            memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param, 
                   (uint8_t *)&eeprom_wg_com_v2_param, 
                   (sizeof(eeprom_backup_wg_com_v2_param)));

            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_backup_wg_com_v2_param, 
                   (sizeof(eeprom_backup_wg_com_v2_param.is_writed)+sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_product_info)));
            (void)eeprom_write_page_verify(P00_BACKUP_ADDR);

            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl, 
                    sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl));
            (void)eeprom_write_page_verify(P02_BACKUP_ADDR);

            eeprom_param_copy_cal_to_page(&eeprom_backup_wg_com_v2_param.wg_com_v2_param);
            (void)eeprom_write_page_verify(P03_BACKUP_1_ADDR);

            eeprom_param_copy_user_to_page(&eeprom_backup_wg_com_v2_param.wg_com_v2_param);
            (void)eeprom_write_page_verify(P03_BACKUP_ADDR);
    }
    else
    {
        memcpy((uint8_t *)&wg_com_v2_product_info,
               (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_product_info,
               sizeof(wg_com_v2_product_info));
        
        memcpy((uint8_t *)&wg_com_v2_ctrl,
               (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
               sizeof(wg_com_v2_ctrl));

        memcpy((uint8_t *)&wg_com_v2_param,
               (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param,
               sizeof(wg_com_v2_param));

        memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param, 
               (uint8_t *)&eeprom_wg_com_v2_param, 
               (sizeof(eeprom_backup_wg_com_v2_param)));

        IICx_Read_Byte(P00_BACKUP_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param, 
               (uint8_t *)&eeprom_page_read_data, 
               (sizeof(eeprom_backup_wg_com_v2_param.is_writed)+sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_product_info)));

        IICx_Read_Byte(P02_BACKUP_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl, 
               (uint8_t *)&eeprom_page_read_data, 
                sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl));

        IICx_Read_Byte(P03_BACKUP_1_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        eeprom_param_copy_cal_from_page(&eeprom_backup_wg_com_v2_param.wg_com_v2_param);

        IICx_Read_Byte(P03_BACKUP_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        if(!eeprom_page_is_blank((uint8_t *)&eeprom_page_read_data, EEPROM_PARAM_USER_SIZE))
        {
            eeprom_param_copy_user_from_page(&eeprom_backup_wg_com_v2_param.wg_com_v2_param);
        }
    }
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.SetPowerMode, wg_com_v2_ctrl.SetPowerMode);
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.SetChargMode, wg_com_v2_ctrl.SetChargMode);
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.InpBatyType, wg_com_v2_ctrl.InpBatyType);
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.OutBatyType, wg_com_v2_ctrl.OutBatyType);

    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.FactoryReset);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.PowerOnOff);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ZeroCurrCalibration);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ResetFactoryData);
    WG_COM_V2_SET_DATA_UINT(7, wg_com_v2_realtime_data.StateCharge);
    get_wg_com_data_rum();
    if(get_wg_com_v2_data.com_ctrl.SetPowerMode == eMPPT_MODE)
    {
        if(eeprom_apply_mppt_mode_profile() == 0U)
        {
            init_mppt_mode_parameter();
        }
        (void)eeprom_commit_current_pages_for_range((uint16_t)(WG_COM_V2_PARAM_ADDR + (EEPROM_PARAM_CAL_SIZE / 2U)),
                                                    (uint16_t)(EEPROM_PARAM_USER_SIZE / 2U));
        get_wg_com_data_rum();
    }
    if(get_wg_com_v2_data.com_ctrl.SetPowerMode == eSET_BAT_MODE)
    {
        eeprom_note_battery_profile_reload_pending();
        if(eeprom_apply_battery_mode_profiles() == 0U)
        {
            request_update_parameter();
        }
    }
    if(eeprom_current_param_valid == 0)
    {
        request_update_parameter();
    }
}

REG_INIT(eeprom_cfg_init)

static uint8_t update_trig = 0;

static uint8_t eeprom_range_overlap(uint16_t addr,
                                    uint16_t count,
                                    uint16_t start,
                                    uint16_t word_count)
{
    return ((addr < (start + word_count)) && ((addr + count) > start)) ? 1 : 0;
}

static uint8_t eeprom_write_p00_current(void)
{
    memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_product_info,
           (uint8_t *)&wg_com_v2_product_info,
           sizeof(wg_com_v2_product_info));
    memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
    eeprom_wg_com_v2_param.wg_com_v2_product_info.BtName = 0;
    memcpy((uint8_t *)&eeprom_page_read_data,
           (uint8_t *)&eeprom_wg_com_v2_param,
           (sizeof(eeprom_wg_com_v2_param.is_writed) + sizeof(eeprom_wg_com_v2_param.wg_com_v2_product_info)));
    return eeprom_write_page_verify(P00_ADDR);
}

static uint8_t eeprom_write_p02_current(void)
{
    wg_com_v2_ctrl_t store_ctrl;

    eeprom_ctrl_prepare_store_image(&store_ctrl,
                                    &wg_com_v2_ctrl,
                                    &eeprom_wg_com_v2_param.wg_com_v2_ctrl);
    if(!eeprom_ctrl_is_valid(&store_ctrl))
    {
        app_debug_event_push(APP_DBG_EVT_VERIFY_FAIL, APP_DBG_AREA_P02, EEPROM_PAGE_P02_CURRENT,
                             (uint8_t)eeprom_ctrl_get(&store_ctrl.SetPowerMode),
                             (uint8_t)((eeprom_ctrl_get(&store_ctrl.InpBatyType) & 0xFF00U) >> 8),
                             APP_DBG_RESULT_INVALID,
                             (uint8_t)(eeprom_ctrl_get(&store_ctrl.InpBatyType) & 0x00FFU));
        return 0;
    }

    memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
           (uint8_t *)&store_ctrl,
           sizeof(store_ctrl));
    memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&eeprom_page_read_data,
           (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
           sizeof(eeprom_wg_com_v2_param.wg_com_v2_ctrl));
    return eeprom_write_page_verify(P02_ADDR);
}

static uint8_t eeprom_write_p03_cal_current(void)
{
    memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param,
           (uint8_t *)&wg_com_v2_param,
           EEPROM_PARAM_CAL_SIZE);
    eeprom_param_copy_cal_to_page(&eeprom_wg_com_v2_param.wg_com_v2_param);
    return eeprom_write_page_verify(P03_1_ADDR);
}

static uint8_t eeprom_write_p03_user_current_page(void)
{
    memcpy(((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param) + EEPROM_PARAM_CAL_SIZE,
           ((uint8_t *)&wg_com_v2_param) + EEPROM_PARAM_CAL_SIZE,
           EEPROM_PARAM_USER_SIZE);
    eeprom_param_copy_user_to_page(&eeprom_wg_com_v2_param.wg_com_v2_param);
    if(eeprom_write_page_verify(P03_ADDR))
    {
        eeprom_current_param_valid = 1;
        return 1;
    }
    return 0;
}

static uint8_t eeprom_write_p03_user_current(void)
{
    uint16_t mode;

    eeprom_refresh_control_cache();
    mode = (uint16_t)get_wg_com_v2_data.com_ctrl.SetPowerMode;
    if((mode == eSET_STANDARD_MODE) || (mode == eSET_CUSTOM_MODE))
    {
        return eeprom_save_current_mode_profile();
    }

    if(!eeprom_save_current_mode_profile())
    {
        return 0;
    }
    return eeprom_write_p03_user_current_page();
}

static uint8_t eeprom_write_all_current_pages(void)
{
    if(!eeprom_write_p00_current())
    {
        return 0;
    }
    if(!eeprom_write_p02_current())
    {
        return 0;
    }
    if(!eeprom_write_p03_cal_current())
    {
        return 0;
    }
    return eeprom_write_p03_user_current();
}

uint8_t eeprom_commit_current_pages_for_range(uint16_t addr, uint16_t count)
{
    uint8_t ok = 1;

    if(count == 0)
    {
        return 1;
    }

    if(eeprom_range_overlap(addr,
                            count,
                            WG_COM_V2_PRUCUCT_INFO_ADDR,
                            (uint16_t)(sizeof(wg_com_v2_product_info_t) / 2U)))
    {
        if(eeprom_write_p00_current())
        {
            update_trig &= 0xFE;
        }
        else
        {
            update_trig |= 0x01;
            ok = 0;
        }
    }

    if(eeprom_range_overlap(addr,
                            count,
                            WG_COM_V2_CTRL_ADDR,
                            (uint16_t)(sizeof(wg_com_v2_ctrl_t) / 2U)))
    {
        if(eeprom_write_p02_current())
        {
            update_trig &= 0xFD;
        }
        else
        {
            update_trig |= 0x02;
            ok = 0;
        }
    }

    if(eeprom_range_overlap(addr,
                            count,
                            WG_COM_V2_PARAM_ADDR,
                            (uint16_t)(EEPROM_PARAM_CAL_SIZE / 2U)))
    {
        if(eeprom_write_p03_cal_current())
        {
            update_trig &= 0xFB;
        }
        else
        {
            update_trig |= 0x04;
            ok = 0;
        }
    }

    if(eeprom_range_overlap(addr,
                            count,
                            (uint16_t)(WG_COM_V2_PARAM_ADDR + (EEPROM_PARAM_CAL_SIZE / 2U)),
                            (uint16_t)(EEPROM_PARAM_USER_SIZE / 2U)))
    {
        if(eeprom_write_p03_user_current())
        {
            update_trig &= 0xF7;
        }
        else
        {
            update_trig |= 0x08;
            ok = 0;
        }
    }

    return ok;
}

static bool eeprom_memcmp(uint8_t zone,void *ptr1,void *ptr2, size_t size)
{
    uint8_t *p1 = (uint8_t *)ptr1;
    uint8_t *p2 = (uint8_t *)ptr2;
    if (p1 == NULL || p2 == NULL)
    {
        return false;
    }

    for (size_t i = 0; i < size; i++)
    {
        if (p1[i] != p2[i])
        {
            switch(zone)
            {
                case eEEPROM_WRITE_P00_ZONE:
                    if((i >= (0x0008*2))&&(i<(0x29*2)))
                    {
                        return false;
                    }
                    break;
                case eEEPROM_WRITE_P02_ZONE:
                    if(((i >= (2*2)) && (i < (11*2))) ||
                       ((i >= (12*2)) && (i < (15*2))))
                    {
                        return false;
                    }
                    break;
                case eEEPROM_WRITE_P03_1_ZONE:
                        return false;
                default:
                    return true;
            }

        }
    }

    return true;
}

static uint8_t Restore_Factory_Data(void);

void eeprom_cfg_update(void)
{
    wg_com_v2_ctrl_t p02_store_ctrl;

    if(Save_Backups_Data())
    {
        return;
    }
    if(Restore_Factory_Data())
    {
        return;
    }
    (void)eeprom_flush_pending_profile_save();
    if (update_trig == 0)
    {
        if (eeprom_memcmp(eEEPROM_WRITE_P00_ZONE,
                         (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_product_info,
                         (uint8_t *)&wg_com_v2_product_info,
                         sizeof(wg_com_v2_product_info)) == false)
        {
            update_trig |= 1;
            app_debug_event_push(APP_DBG_EVT_DIRTY_RETRY, APP_DBG_AREA_P00, EEPROM_PAGE_P00_CURRENT,
                                 (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                                 0, APP_DBG_RESULT_RETRY, update_trig);
        }
        
        eeprom_ctrl_prepare_store_image(&p02_store_ctrl,
                                        &wg_com_v2_ctrl,
                                        &eeprom_wg_com_v2_param.wg_com_v2_ctrl);
        if (eeprom_memcmp(eEEPROM_WRITE_P02_ZONE,
                         (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
                         (uint8_t *)&p02_store_ctrl,
                         sizeof(p02_store_ctrl)) == false)
        {
            update_trig |= 2;
            app_debug_event_push(APP_DBG_EVT_DIRTY_RETRY, APP_DBG_AREA_P02, EEPROM_PAGE_P02_CURRENT,
                                 (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                                 0, APP_DBG_RESULT_RETRY, update_trig);
        }

        if (eeprom_memcmp(eEEPROM_WRITE_P03_1_ZONE,
                         (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param,
                         (uint8_t *)&wg_com_v2_param,
                         EEPROM_PARAM_CAL_SIZE) == false)
        {
            update_trig |= 4;
            app_debug_event_push(APP_DBG_EVT_DIRTY_RETRY, APP_DBG_AREA_CAL, EEPROM_PAGE_CAL_CURRENT,
                                 (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                                 0, APP_DBG_RESULT_RETRY, update_trig);
        }

        if (eeprom_memcmp(eEEPROM_WRITE_P03_ZONE,
                         ((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param) + EEPROM_PARAM_CAL_SIZE,
                         ((uint8_t *)&wg_com_v2_param) + EEPROM_PARAM_CAL_SIZE,
                         EEPROM_PARAM_USER_SIZE) == false)
        {
            update_trig |= 8;
            app_debug_event_push(APP_DBG_EVT_DIRTY_RETRY, APP_DBG_AREA_PARAM, EEPROM_PAGE_PARAM_CURRENT,
                                 (uint8_t)get_wg_com_v2_data.com_ctrl.SetPowerMode,
                                 0, APP_DBG_RESULT_RETRY, update_trig);
        }
    }

    if (update_trig != 0)
    {
        if((update_trig&0x01) == 0x01)
        {
            if(eeprom_write_p00_current())
            {
                update_trig &= 0xFE;
            }
        }
        else if((update_trig&0x02) == 0x02)
        {
            if(eeprom_write_p02_current())
            {
                update_trig &= 0xFD;
            }
        }
        else if((update_trig&0x04) == 0x04)
        {
            if(eeprom_write_p03_cal_current())
            {
                update_trig &= 0xFB;
            }
        }
        else if((update_trig&0x08) == 0x08)
        {
            if(eeprom_write_p03_user_current())
            {
                update_trig &= 0xF7;
            }
        }
        else
        {
            update_trig = 0;
        }
    }
}


uint8_t Save_Backups_Data(void)
{
    uint16_t GetDataFlag = 0;

    WG_COM_V2_GET_DATA_UINT(GetDataFlag, wg_com_v2_ctrl.ResetFactoryData);

    if((GetDataFlag == 0U) && (eeprom_factory_save_state == EEPROM_FACTORY_SAVE_IDLE))
    {
        return 0U;
    }

    switch(eeprom_factory_save_state)
    {
        case EEPROM_FACTORY_SAVE_IDLE:
            eeprom_factory_step_event(EEPROM_FACTORY_STEP_POWER_OFF, APP_DBG_RESULT_START, 0);
            eeprom_factory_set_power(1U);
            eeprom_factory_save_wait_count = 0;
            eeprom_factory_save_state = EEPROM_FACTORY_SAVE_POWER_OFF_WAIT;
            return 1U;

        case EEPROM_FACTORY_SAVE_POWER_OFF_WAIT:
            if(++eeprom_factory_save_wait_count < EEPROM_FACTORY_SAVE_WAIT_TICKS)
            {
                return 1U;
            }
            eeprom_factory_save_wait_count = 0;
            eeprom_factory_step_event(EEPROM_FACTORY_STEP_POWER_OFF, APP_DBG_RESULT_OK, 1);
            eeprom_factory_save_state = EEPROM_FACTORY_SAVE_RUN_BACKUP;
            return 1U;

        case EEPROM_FACTORY_SAVE_RUN_BACKUP:
            eeprom_factory_step_event(EEPROM_FACTORY_STEP_EXT_BACKUP, APP_DBG_RESULT_START, 0);
            if(!eeprom_save_factory_backup_external_verified())
            {
                eeprom_factory_save_abort(EEPROM_FACTORY_STEP_EXT_BACKUP);
                return 1U;
            }
            eeprom_factory_step_event(EEPROM_FACTORY_STEP_EXT_BACKUP, APP_DBG_RESULT_OK, 4);

            eeprom_factory_step_event(EEPROM_FACTORY_STEP_INTERNAL_BACKUP, APP_DBG_RESULT_START, 0);
            if(!eeprom_save_factory_backup_internal_verified())
            {
                eeprom_factory_save_abort(EEPROM_FACTORY_STEP_INTERNAL_BACKUP);
                return 1U;
            }
            eeprom_factory_step_event(EEPROM_FACTORY_STEP_INTERNAL_BACKUP, APP_DBG_RESULT_OK, (uint8_t)EEPROM_FACTORY_FLASH_PAGE);

            if(!eeprom_reset_battery_profile_pages_to_app_defaults(EEPROM_FACTORY_STEP_PROFILE_RESET))
            {
                eeprom_factory_save_abort(EEPROM_FACTORY_STEP_PROFILE_RESET);
                return 1U;
            }
            if(!eeprom_factory_rebuild_current_mode_profile())
            {
                eeprom_factory_save_abort(EEPROM_FACTORY_STEP_PROFILE_RESET);
                return 1U;
            }

            eeprom_factory_step_event(EEPROM_FACTORY_STEP_FINAL_P02, APP_DBG_RESULT_START, 0);
            GetDataFlag = 0;
            WG_COM_V2_SET_DATA_UINT(GetDataFlag, wg_com_v2_ctrl.ResetFactoryData);
            if(!eeprom_write_p02_current())
            {
                eeprom_factory_save_abort(EEPROM_FACTORY_STEP_FINAL_P02);
                return 1U;
            }
            eeprom_factory_step_event(EEPROM_FACTORY_STEP_FINAL_P02, APP_DBG_RESULT_OK, 0);

            eeprom_factory_step_event(EEPROM_FACTORY_STEP_POWER_ON, APP_DBG_RESULT_START, 0);
            eeprom_factory_save_wait_count = 0;
            eeprom_factory_save_state = EEPROM_FACTORY_SAVE_POWER_ON_WAIT;
            return 1U;

        case EEPROM_FACTORY_SAVE_POWER_ON_WAIT:
            if(++eeprom_factory_save_wait_count < EEPROM_FACTORY_SAVE_WAIT_TICKS)
            {
                return 1U;
            }
            eeprom_factory_save_wait_count = 0;
            eeprom_factory_set_power(0U);
            eeprom_factory_step_event(EEPROM_FACTORY_STEP_POWER_ON, APP_DBG_RESULT_OK, 0);
            eeprom_factory_save_state = EEPROM_FACTORY_SAVE_IDLE;
            return 1U;

        default:
            eeprom_factory_save_state = EEPROM_FACTORY_SAVE_IDLE;
            eeprom_factory_save_wait_count = 0;
            return 1U;
    }

}

static uint8_t Restore_Factory_Data(void)
{
    uint16_t GetDataFlag = 0;
    uint8_t restored = 0;

    WG_COM_V2_GET_DATA_UINT(GetDataFlag, wg_com_v2_ctrl.FactoryReset);
    if((GetDataFlag == 0U) && (eeprom_factory_restore_state == EEPROM_FACTORY_RESTORE_IDLE))
    {
        return 0U;
    }

    switch(eeprom_factory_restore_state)
    {
        case EEPROM_FACTORY_RESTORE_IDLE:
            eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_POWER_OFF, APP_DBG_RESULT_START, 0);
            eeprom_factory_set_power(1U);
            eeprom_factory_restore_wait_count = 0;
            eeprom_factory_restore_state = EEPROM_FACTORY_RESTORE_POWER_OFF_WAIT;
            return 1U;

        case EEPROM_FACTORY_RESTORE_POWER_OFF_WAIT:
            if(++eeprom_factory_restore_wait_count < EEPROM_FACTORY_SAVE_WAIT_TICKS)
            {
                return 1U;
            }
            eeprom_factory_restore_wait_count = 0;
            eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_POWER_OFF, APP_DBG_RESULT_OK, 1);
            eeprom_factory_restore_state = EEPROM_FACTORY_RESTORE_RUN;
            return 1U;

        case EEPROM_FACTORY_RESTORE_RUN:
            eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_EXT_CHECK, APP_DBG_RESULT_START, 0);
            if(eeprom_factory_restore_external_to_runtime())
            {
                eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_EXT_CHECK, APP_DBG_RESULT_OK, 4);
                eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_EXT_APPLY, APP_DBG_RESULT_START, 0);
                if(eeprom_factory_restore_current_from_runtime_verified())
                {
                    eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_EXT_APPLY, APP_DBG_RESULT_OK, 4);
                    restored = 1;
                }
                else
                {
                    eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_EXT_APPLY, APP_DBG_RESULT_FAIL, 4);
                }
            }
            else
            {
                eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_EXT_CHECK, APP_DBG_RESULT_FAIL, 0);
            }

            if(!restored)
            {
                eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_INT_CHECK, APP_DBG_RESULT_START, 0);
                if(eeprom_factory_restore_internal_to_runtime() &&
                   eeprom_factory_restore_current_from_runtime_verified())
                {
                    eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_INT_CHECK, APP_DBG_RESULT_OK, (uint8_t)EEPROM_FACTORY_FLASH_PAGE);
                    restored = 1;
                }
                else
                {
                    eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_INT_CHECK, APP_DBG_RESULT_FAIL, (uint8_t)EEPROM_FACTORY_FLASH_PAGE);
                }
            }

            if(!restored)
            {
                    eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_APP_DEFAULT, APP_DBG_RESULT_START, 0);
                    if(eeprom_factory_restore_app_defaults_verified())
                    {
                        eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_APP_DEFAULT, APP_DBG_RESULT_OK, 0);
                        restored = 1;
                    }
                    else
                    {
                        eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_APP_DEFAULT, APP_DBG_RESULT_FAIL, 0);
                    }
            }

            eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_FINAL_COMMIT, APP_DBG_RESULT_START, 0);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.FactoryReset);
            if(restored && eeprom_write_p02_current())
            {
                eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_FINAL_COMMIT, APP_DBG_RESULT_OK, 0);
            }
            else
            {
                eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_FINAL_COMMIT, APP_DBG_RESULT_FAIL, 0);
                eeprom_factory_set_power(1U);
                eeprom_factory_restore_state = EEPROM_FACTORY_RESTORE_IDLE;
                eeprom_factory_restore_wait_count = 0;
                return 1U;
            }

            if(!eeprom_reset_battery_profile_pages_to_app_defaults(EEPROM_FACTORY_RESTORE_STEP_PROFILE_RESET))
            {
                eeprom_factory_set_power(1U);
                eeprom_factory_restore_state = EEPROM_FACTORY_RESTORE_IDLE;
                eeprom_factory_restore_wait_count = 0;
                return 1U;
            }
            if(!eeprom_factory_rebuild_current_mode_profile())
            {
                eeprom_factory_set_power(1U);
                eeprom_factory_restore_state = EEPROM_FACTORY_RESTORE_IDLE;
                eeprom_factory_restore_wait_count = 0;
                return 1U;
            }

            eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_POWER_ON, APP_DBG_RESULT_START, 0);
            eeprom_factory_restore_wait_count = 0;
            eeprom_factory_restore_state = EEPROM_FACTORY_RESTORE_POWER_ON_WAIT;
            return 1U;

        case EEPROM_FACTORY_RESTORE_POWER_ON_WAIT:
            if(++eeprom_factory_restore_wait_count < EEPROM_FACTORY_SAVE_WAIT_TICKS)
            {
                return 1U;
            }
            eeprom_factory_restore_wait_count = 0;
            eeprom_factory_set_power(0U);
            eeprom_factory_step_event(EEPROM_FACTORY_RESTORE_STEP_POWER_ON, APP_DBG_RESULT_OK, 0);
            eeprom_factory_restore_state = EEPROM_FACTORY_RESTORE_IDLE;
            return 1U;

        default:
            eeprom_factory_restore_state = EEPROM_FACTORY_RESTORE_IDLE;
            eeprom_factory_restore_wait_count = 0;
            return 1U;
    }
}

REG_TASK(100, eeprom_cfg_update)


void erasure_eeprom_data(void)
{
    static uint32_t ErasureDataDelay = 0;
    static uint8_t  ErasureFlag = 0;
    if(bsp_get_addrs() == 0)
    {
        if(ErasureDataDelay > 10)
        {
            ErasureDataDelay = 0;
            if(ErasureFlag == 0)
            {
                ErasureFlag = 1;
                memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
                for(uint32_t i = 0;i < EE_24CXX_CAPACITY;)
                {
                    (void)eeprom_write_page_verify((uint16_t)i);
                    i += EE_24CXX_PAGE_SIZE;
                }
            }
        }
        else
        {
            ++ErasureDataDelay;
        }
    }
    else
    {
        ErasureFlag = 0;
        ErasureDataDelay = 0;
    }
}

REG_TASK(100, erasure_eeprom_data)


