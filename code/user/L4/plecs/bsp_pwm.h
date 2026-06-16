#ifndef __BSP_PWM_H
#define __BSP_PWM_H

#include "stdint.h"

typedef struct
{
    float left_duty;
    uint8_t left_up_en;
    uint8_t left_dn_en;
    float right_duty;
    uint8_t right_up_en;
    uint8_t right_dn_en;
} bsp_pwm_t;

void bsp_pwm_set_a_duty(bsp_pwm_t *str);

void bsp_pwm_set_b_duty(bsp_pwm_t *str);

#endif
