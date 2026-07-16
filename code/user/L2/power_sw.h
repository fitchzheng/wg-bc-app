#ifndef __POWER_SW_H
#define __POWER_SW_H

#include "stdint.h"

uint8_t power_sw_get_power_is_on(void);
void power_sw_force_power_on(void);

#endif
