#ifndef __GPIO_H
#define __GPIO_H

#include "stdint.h"

#define LED_ON_OFF      0

extern uint8_t mos_g150_flag;
extern uint8_t mos_g300_flag;

void gpio_set_auxoff(uint8_t val);

void gpio_set_lg(uint8_t val);

void gpio_set_lr(uint8_t val);

void gpio_set_led1_green(uint8_t val);

void gpio_set_led2_red(uint8_t val);

void gpio_set_led3_green(uint8_t val);

uint8_t gpio_get_pg(void);

void gpio_set_db2(uint8_t val);

void gpio_set_db1(uint8_t val);

void gpio_set_da2(uint8_t val);

void gpio_set_da1(uint8_t val);

void gpio_set_re(uint8_t val);

uint8_t bsp_get_addrs(void);

void mos_on_off_G150(uint8_t model);

void mos_on_off_G300(uint8_t model);

uint8_t get_key_pg_val(void);
#endif
