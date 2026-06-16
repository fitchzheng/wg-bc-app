#ifndef __VOLT_LOOP_H
#define __VOLT_LOOP_H

#include "pid.h"

typedef struct
{
    float v_ref;
    float v_act;
}volt_loop_input_t;

typedef struct
{
    float c_val;
    float lmt;
    float freq_ctrl;
    float freq_cut;
    float w_cut;
}volt_loop_cfg_t;

typedef struct
{
    pid_param_t pid;
}volt_loop_inter_t;

typedef struct
{
    float out;
}volt_loop_output_t;

typedef struct
{
    volt_loop_input_t input;
    volt_loop_cfg_t cfg;
    volt_loop_inter_t inter;
    volt_loop_output_t output;
}volt_loop_t;

void volt_loop_init(volt_loop_t *volt_loop);

float volt_loop_cal(volt_loop_t *volt_loop);

#endif

