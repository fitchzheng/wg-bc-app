#ifndef __STANDARD_MODE_CHARGE_H
#define __STANDARD_MODE_CHARGE_H

#include "stdint.h"

#define PWC_GPIO_MODE_OUT_PP        0
#define PWC_GPIO_MODE_IPD           1
#define PWC_GPIO_MODE_ANALOG_PU     2
#define PWC_GPIO_MODE_ANALOG_PD     4

/* Sleep GPIO initialization only reads the port and pin fields. */
#define PWC_GPIO_REG_PARM(_unused_name, _gpx, _pin, _unused_mode, _unused_def_lv) { \
    .gpio_periph = (uint8_t)(_gpx),                                                 \
    .pin = (uint16_t)GPIO_PIN_##_pin,                                               \
}

typedef enum
{
    PWC_PC13,
    PWC_PC14,
    PWC_PC15,
    PWC_PD0,
    PWC_PD1,
    PWC_PC0,
    PWC_PC1,
    PWC_PC2,
    PWC_PC3,
    PWC_PA0,
    PWC_PA1,
    PWC_PA2,
    PWC_PA3,
    PWC_PA4,
    PWC_PA5,
    PWC_PA6,
    PWC_PA7,
    PWC_PC4,
    PWC_PC5,
    PWC_PB0,
    PWC_PB1,
    PWC_PB2,
    PWC_PB10,
    PWC_PB11,
    PWC_PB12,
    PWC_PB13,
    PWC_PB14,
    PWC_PB15,
    PWC_PC6,
    PWC_PC7,
    PWC_PC8,
    PWC_PC9,
    PWC_PA8,
    PWC_PA9,
    PWC_PA10,
    PWC_PA11,
    PWC_PA12,
    PWC_PA13,
    PWC_PA14,
    PWC_PA15,
    PWC_PC10,
    PWC_PC11,
    PWC_PD2,
    PWC_PB3,
    PWC_PB4,
    PWC_PB5,
    PWC_PB6,
    PWC_PB7,
    PWC_PB8,
    PWC_PB9,
    PWC_PF2,
    PWC_GPIO_TABLE_MAX
} bsp_gpio_table_pwc_e;

typedef struct
{
  uint8_t gpio_periph;
  uint16_t pin;
} bsp_gpio_parm_pwc_t;


uint8_t bsp_pwc_stop_rum(void);
void sleep_low_power_commit(void);
#endif
