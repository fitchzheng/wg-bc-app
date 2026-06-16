#ifndef __BB_PWM_H
#define __BB_PWM_H

#include "stdint.h"
#include "my_math.h"
#include "gd32f30x.h"

#define TIMER_PWMA TIMER0
#define TIMER_PWMB TIMER7

#define TIMER0_FREQ (120000000 >> 1)
#define CTRL_PERIOD ((uint32_t)(((float)TIMER0_FREQ / PWM_FREQ) - 1.0f))

typedef struct
{
    float left_duty;
    uint8_t left_up_en;
    uint8_t left_dn_en;
    float right_duty;
    uint8_t right_up_en;
    uint8_t right_dn_en;
} bsp_pwm_t;

void bsp_pwm_init(void);

void bsp_pwm_update_chclt2(void);

void bsp_pwm_change_full_freq_pwma(void);

void bsp_pwm_change_full_freq_pwmb(void);

void bsp_pwm_change_half_freq_pwma(void);

void bsp_pwm_change_half_freq_pwmb(void);

void bsp_pwm_set_a_duty(bsp_pwm_t *str);

void bsp_pwm_set_b_duty(bsp_pwm_t *str);

#endif
