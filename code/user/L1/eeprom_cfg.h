#ifndef __EEPROM_CFG_H
#define __EEPROM_CFG_H

#include "wg_com_v2.h"

#define P00_ADDR             (0)
#define P02_ADDR             (EE_24CXX_PAGE_SIZE)
#define P03_1_ADDR           (EE_24CXX_PAGE_SIZE*2)
#define P03_ADDR             (EE_24CXX_PAGE_SIZE*3)

#define P00_BACKUP_ADDR      (EE_24CXX_PAGE_SIZE*4)
#define P02_BACKUP_ADDR      (EE_24CXX_PAGE_SIZE*5)
#define P03_BACKUP_1_ADDR    (EE_24CXX_PAGE_SIZE*6)
#define P03_BACKUP_ADDR      (EE_24CXX_PAGE_SIZE*7)

#define EEROM_INIT_DATA      0x5A50

typedef struct
{
    uint16_t is_writed;
    wg_com_v2_product_info_t wg_com_v2_product_info;
    wg_com_v2_ctrl_t wg_com_v2_ctrl;
    wg_com_v2_param_t wg_com_v2_param;
} eeprom_wg_com_v2_param_t;

#define EEPROM_P03_ZONE_SIZE   100
enum {
    eEEPROM_WRITE_P00_ZONE,
    eEEPROM_WRITE_P02_ZONE,
    eEEPROM_WRITE_P03_1_ZONE,
    eEEPROM_WRITE_P03_ZONE,
    EEPROM_ZONE_MAX
};

extern void Save_Backups_Data(void);
#endif
