#ifndef __BUCK_BOOST_H
#define __BUCK_BOOST_H

#include "l_loop.h"
#include "volt_loop.h"
#include "l_loop_open.h"
#include "curr_lmt.h"
#include "buck_boost_mode.h"

#define BUCK_BOOST_L_LOOP_NUM 2

#define BUCK_TO_BUCK_BOOST_THR 0.9f
#define BUCK_BOOST_TO_BUCK_THR 0.85f
#define BUCK_BOOST_TO_BOOST_THR 1.17f
#define BOOST_TO_BUCK_BOOST_THR 1.11f

#define BUCK_BOOST_BUCK_BOOST_FIX_DUTY 0.8f
extern buck_boost_mode_t open_loop_bb;
typedef enum
{
    BB_MODE_BUCK,
    BB_MODE_BOOST,
    BB_MODE_BUCK_BOOST,
} BUCK_BOOST_MODE_E;

typedef struct
{
    float v_in;
    float i_in;
    float i_l[BUCK_BOOST_L_LOOP_NUM];
    float v_out;
    float i_out;
    float v_out_ref;
    float vin_lmt;
    float iin_lmt;
    float iout_lmt;
    float pwr_in_lmt;
    float pwr_out_lmt;
    float volt_comp;
} buck_boost_input_t;

typedef struct
{
    float ctrl_freq;
    float ts;
    float pwm_freq;
    float pwm_ts;
    float l_val[BUCK_BOOST_L_LOOP_NUM];
    float c_val;
} buck_boost_cfg_t;

typedef struct
{
    volt_loop_t volt_loop;
    l_loop_t l_loop[BUCK_BOOST_L_LOOP_NUM];
    BUCK_BOOST_MODE_E buck_boost_mode;
    uint8_t is_ccm[1];
    uint32_t dcm_cnt[1];
    uint32_t iout_dcm_cnt;
    curr_lmt_t vin_lmt;
    curr_lmt_t iin_lmt;
    curr_lmt_t iout_lmt;
    buck_boost_mode_t bb_mode_duty[BUCK_BOOST_L_LOOP_NUM];
    pid_param_t volt_comp_loop;
} buck_boost_inter_t;

typedef struct
{
    float buck_duty;
    uint8_t buck_up_en;
    uint8_t buck_dn_en;
    float boost_duty;
    uint8_t boost_up_en;
    uint8_t boost_dn_en;
} buck_boost_duty_t;

typedef struct
{
    buck_boost_duty_t duty[BUCK_BOOST_L_LOOP_NUM];
} buck_boost_output_t;

typedef struct
{
    buck_boost_input_t input;
    buck_boost_cfg_t cfg;
    buck_boost_inter_t inter;
    buck_boost_output_t output;
} buck_boost_t;
extern buck_boost_t buck_boost;
void buck_boost_init(buck_boost_t *str);

void buck_boost_run(buck_boost_t *str);

void buck_boost_bidirectional(buck_boost_t *str);

#endif
