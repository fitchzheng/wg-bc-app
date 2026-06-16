#ifndef __BUCK_BOOST_MODE_H
#define __BUCK_BOOST_MODE_H

#include "stdint.h"

#define BUCK_BOOST_MODE_SSW_TO_BB_MODE_THR 0.95f
#define BUCK_BOOST_MODE_BB_MODE_TO_SSW_THR 0.85f
#define BUCK_BOOST_MODE_DUTY_SUM 1.9f
#define BUCK_BOOST_MODE_DUTY_MAX 0.97f

typedef enum
{
    BUCK_BOOST_MODE_BUCK,
    BUCK_BOOST_MODE_BOOST,
    BUCK_BOOST_MODE_BUCK_BOOST,
} BUCK_BOOST_MODE_DUTY_E;

typedef struct
{
    float v_l;
    float v_in;
    float v_out;
} buck_boost_mode_input_t;

typedef struct
{
    BUCK_BOOST_MODE_DUTY_E mode;
} buck_boost_mode_inter_t;

typedef struct
{
    float buck_duty;
    float boost_duty;
    uint8_t buck_dn_en;
    uint8_t boost_up_en;
    uint8_t is_half_freq;
} buck_boost_mode_output_t;

typedef struct
{
    buck_boost_mode_input_t input;
    buck_boost_mode_inter_t inter;
    buck_boost_mode_output_t output;
} buck_boost_mode_t;

void buck_boost_mode_init(buck_boost_mode_t *str);

void buck_boost_mode_func(buck_boost_mode_t *str);

#endif
