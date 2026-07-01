#ifndef __EEPROM_CFG_H
#define __EEPROM_CFG_H

#include "wg_com_v2.h"
#include "app_features.h"

#define P00_ADDR             (0)
#define P02_ADDR             (EE_24CXX_PAGE_SIZE)
#define P03_1_ADDR           (EE_24CXX_PAGE_SIZE*2)
#define P03_ADDR             (EE_24CXX_PAGE_SIZE*3)

#define P00_BACKUP_ADDR      (EE_24CXX_PAGE_SIZE*4)
#define P02_BACKUP_ADDR      (EE_24CXX_PAGE_SIZE*5)
#define P03_BACKUP_1_ADDR    (EE_24CXX_PAGE_SIZE*6)
#define P03_BACKUP_ADDR      (EE_24CXX_PAGE_SIZE*7)

/* Final EEPROM page layout. Keep the legacy SYS_PROFILE_* macros below
 * untouched until the business paths are switched to the new formula. */
#define EEPROM_PAGE_P00_CURRENT             (0U)
#define EEPROM_PAGE_P02_CURRENT             (1U)
#define EEPROM_PAGE_CAL_CURRENT             (2U)
#define EEPROM_PAGE_PARAM_CURRENT           (3U)
#define EEPROM_PAGE_P00_FACTORY             (4U)
#define EEPROM_PAGE_P02_FACTORY             (5U)
#define EEPROM_PAGE_CAL_FACTORY             (6U)
#define EEPROM_PAGE_PARAM_FACTORY           (7U)

#define EEPROM_PAGE_BAT_PROFILE_A_BASE      (8U)
#define EEPROM_PAGE_BAT_PROFILE_B_BASE      (38U)
#define EEPROM_BAT_PROFILE_VOLT_COUNT       (5U)
#define EEPROM_BAT_PROFILE_A_TYPE_COUNT     (6U)
#define EEPROM_BAT_PROFILE_B_TYPE_COUNT     (5U)
#define EEPROM_PAGE_MPPT_PROFILE_BASE       (63U)
#define EEPROM_MPPT_PROFILE_COUNT           (5U)
#define EEPROM_PAGE_EXT_PROFILE_BASE        (68U)
#define EEPROM_PAGE_GLOBAL_RESERVED         (255U)

#define EEPROM_PAGE_ADDR(page)              ((uint16_t)(EE_24CXX_PAGE_SIZE * (page)))
#define EEPROM_PROFILE_MAGIC_VALID          (0x5350U)
#define EEPROM_PROFILE_MAGIC_INVALID        (0x0000U)
#define EEPROM_PROFILE_VERSION              (2U)

#define EEPROM_PROFILE_PORT_A               (0U)
#define EEPROM_PROFILE_PORT_B               (1U)
#define EEPROM_PROFILE_PORT_MPPT            (2U)

#define EEPROM_PROFILE_TYPE_AGM             (0U)
#define EEPROM_PROFILE_TYPE_LFP             (1U)
#define EEPROM_PROFILE_TYPE_NMC             (2U)
#define EEPROM_PROFILE_TYPE_DCDC            (3U)
#define EEPROM_PROFILE_TYPE_SCAP            (4U)
#define EEPROM_PROFILE_TYPE_AUTOSYS         (5U)

#define EEPROM_PROFILE_SYS_12V              (0U)
#define EEPROM_PROFILE_SYS_16V              (1U)
#define EEPROM_PROFILE_SYS_24V              (2U)
#define EEPROM_PROFILE_SYS_36V              (3U)
#define EEPROM_PROFILE_SYS_48V              (4U)

#define EEPROM_PARAM_CAL_SIZE               ((uint16_t)(26U * 2U))
#define EEPROM_PARAM_USER_SIZE              ((uint16_t)(sizeof(wg_com_v2_param_t) - EEPROM_PARAM_CAL_SIZE))

#define APP_DBG_EVT_BOOT_LOAD       1U
#define APP_DBG_EVT_READ            2U
#define APP_DBG_EVT_WRITE_BEGIN     3U
#define APP_DBG_EVT_VERIFY_OK       4U
#define APP_DBG_EVT_VERIFY_FAIL     5U
#define APP_DBG_EVT_PROFILE_DEFAULT 6U
#define APP_DBG_EVT_PROFILE_APPLY   7U
#define APP_DBG_EVT_PROFILE_SAVE    8U
#define APP_DBG_EVT_DIRTY_RETRY     9U
#define APP_DBG_EVT_FACTORY_STEP    10U
#define APP_DBG_EVT_BT_NAME         11U

#define APP_DBG_AREA_P00       1U
#define APP_DBG_AREA_P02       2U
#define APP_DBG_AREA_CAL       3U
#define APP_DBG_AREA_PARAM     4U
#define APP_DBG_AREA_FACTORY   5U
#define APP_DBG_AREA_BAT       6U
#define APP_DBG_AREA_MPPT      7U
#define APP_DBG_AREA_STANDARD  8U
#define APP_DBG_AREA_BASIC     9U
#define APP_DBG_AREA_EXT       10U
#define APP_DBG_AREA_BT        11U

#define APP_DBG_RESULT_START   0U
#define APP_DBG_RESULT_OK      1U
#define APP_DBG_RESULT_FAIL    2U
#define APP_DBG_RESULT_INVALID 3U
#define APP_DBG_RESULT_RETRY   4U

#define SYS_PROFILE_A_12V_ADDR   (EE_24CXX_PAGE_SIZE*8)
#define SYS_PROFILE_A_16V_ADDR   (EE_24CXX_PAGE_SIZE*9)
#define SYS_PROFILE_A_24V_ADDR   (EE_24CXX_PAGE_SIZE*10)
#define SYS_PROFILE_A_36V_ADDR   (EE_24CXX_PAGE_SIZE*11)
#define SYS_PROFILE_A_48V_ADDR   (EE_24CXX_PAGE_SIZE*12)
#define SYS_PROFILE_A_DCDC_ADDR  (EE_24CXX_PAGE_SIZE*13)
#define SYS_PROFILE_B_12V_ADDR   (EE_24CXX_PAGE_SIZE*14)
#define SYS_PROFILE_B_16V_ADDR   (EE_24CXX_PAGE_SIZE*15)
#define SYS_PROFILE_B_24V_ADDR   (EE_24CXX_PAGE_SIZE*16)
#define SYS_PROFILE_B_36V_ADDR   (EE_24CXX_PAGE_SIZE*17)
#define SYS_PROFILE_B_48V_ADDR   (EE_24CXX_PAGE_SIZE*18)
#define SYS_PROFILE_B_DCDC_ADDR  (EE_24CXX_PAGE_SIZE*19)

#define EEROM_INIT_DATA      0x5A50
#define EEPROM_SYS_PROFILE_INIT_DATA 0x5350

typedef struct
{
    uint16_t is_writed;
    wg_com_v2_product_info_t wg_com_v2_product_info;
    wg_com_v2_ctrl_t wg_com_v2_ctrl;
    wg_com_v2_param_t wg_com_v2_param;
} eeprom_wg_com_v2_param_t;

typedef struct
{
    uint16_t is_writed;
    uint16_t SetVolt;
    uint16_t SetCurr;
    uint16_t SetCurrPower;
    uint16_t SetUvlo;
    uint16_t SetUvloRecover;
    uint16_t SetOVP;
    uint16_t SetOVPRecover;
    uint16_t SetChargLedCurr;
    uint16_t SetFullLedCurr;
    uint16_t AutoOpenVolt;
    uint16_t AutoVeerVolt;
    uint16_t AutoCloseVolt;
    uint16_t SetBootTime;
    uint16_t SetOnCurrStartTime;
} eeprom_system_profile_t;

typedef struct
{
    uint16_t magic;
    uint16_t version;
    uint8_t port;
    uint8_t type;
    uint8_t sys;
    uint8_t reserved;
    uint16_t checksum;
} eeprom_profile_header_t;

typedef struct
{
    eeprom_profile_header_t header;
    eeprom_system_profile_t payload;
} eeprom_profile_page_t;

typedef struct
{
    eeprom_profile_header_t header;
    uint8_t payload[EEPROM_PARAM_USER_SIZE];
} eeprom_user_profile_page_t;


#define EEPROM_P03_ZONE_SIZE   100
enum {
    eEEPROM_WRITE_P00_ZONE,
    eEEPROM_WRITE_P02_ZONE,
    eEEPROM_WRITE_P03_1_ZONE,
    eEEPROM_WRITE_P03_ZONE,
    EEPROM_ZONE_MAX
};

extern uint8_t Save_Backups_Data(void);
extern uint8_t eeprom_apply_battery_mode_profiles(void);
extern uint8_t eeprom_autosys_runtime_update(void);
extern uint8_t eeprom_save_battery_mode_profiles(void);
extern uint8_t eeprom_apply_mppt_mode_profile(void);
extern void eeprom_apply_mppt_fixed_input_params(void);
extern uint8_t eeprom_apply_standard_mode_profile(void);
extern uint8_t eeprom_apply_basic_mode_profile(void);
extern uint8_t eeprom_save_current_mode_profile(void);
extern uint8_t eeprom_save_current_timing_profile(void);
extern void eeprom_save_mppt_return_battery_types(uint16_t bat_type_a, uint16_t bat_type_b);
extern uint8_t eeprom_load_mppt_return_battery_types(uint16_t *bat_type_a, uint16_t *bat_type_b);
extern void eeprom_request_current_profile_save(void);
extern void eeprom_note_battery_profile_reload_pending(void);
extern uint8_t eeprom_commit_current_pages_for_range(uint16_t addr, uint16_t count);
extern uint16_t eeprom_profile_calc_page(uint8_t port, uint8_t type_index, uint8_t volt_index);
extern uint16_t eeprom_profile_page_to_addr(uint16_t page);
extern uint16_t eeprom_profile_checksum16(const uint8_t *data, uint16_t len);
extern uint8_t eeprom_profile_header_is_valid(const eeprom_profile_header_t *header,
                                              const uint8_t *payload,
                                              uint16_t payload_len);
#if (APP_DEBUG_EVENT_FEATURES == 1)
extern void app_debug_event_push(uint8_t event,
                                 uint8_t area,
                                 uint8_t page,
                                 uint8_t mode,
                                 uint8_t profile,
                                 uint8_t result,
                                 uint8_t value);
#else
#define app_debug_event_push(event, area, page, mode, profile, result, value) ((void)0)
#endif
extern void app_debug_event_read_regs(uint16_t reg_offset, uint16_t reg_count, uint8_t *data);
#endif
