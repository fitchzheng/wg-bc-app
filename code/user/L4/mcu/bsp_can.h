#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "gd32f30x.h"

void bsp_can_init(void);

void bsp_can_tx(uint32_t id, uint8_t *p_data);

int bsp_can_rx(uint32_t *p_raw, uint8_t *p_data);

#endif
