#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "stdint.h"

typedef enum
{
    AUXOFF,
    RED,
    Green,
    BLED1,
    BLED2,
    LEDS,
    PG_EN,
    DB2,
    DB1,
    DA2,
    DA1,
    RE,
    GPIO_TABLE_MAX
} bsp_gpio_table_e;

void bsp_gpio_set_bit(bsp_gpio_table_e num, uint8_t val);
void bsp_gpio_get_bit(bsp_gpio_table_e num, uint8_t *val);

#endif
