#ifndef __EEPROM_H
#define __EEPROM_H

#include "stdint.h"

#define EEPROM_SIZE  65535
#define PAGE_SIZE    128

#define EEPROM_WRITE_ABNORMAL   2
#define EEPROM_TIMEOUT_FAULT    1
#define EEPROM_WRITE_SUCCEED    0


uint8_t Eeprom_Write(uint32_t Address,uint8_t* ndata,uint32_t size);
void Eeprom_Read(uint32_t Address,uint8_t* ndata,uint32_t size);
#endif
