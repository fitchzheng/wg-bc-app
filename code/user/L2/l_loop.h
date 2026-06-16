#ifndef __L_LOOP_H
#define __L_LOOP_H

#include "pid.h"

typedef struct
{
    float i_ref;
    float i_act;
    float i_load;
} l_loop_input_t;

typedef struct
{
    float l_val;
    float freq_ctrl;
    float freq_cut;
    float w_cut;
    float lmt;
    float ts_ctrl;
} l_loop_cfg_t;

typedef struct
{
    pid_param_t pid;
    float i_load_last;
} l_loop_inter_t;

typedef struct
{
    float l_volt;
} l_loop_output_t;

typedef struct
{
    l_loop_input_t input;
    l_loop_cfg_t cfg;
    l_loop_inter_t inter;
    l_loop_output_t output;
} l_loop_t;

void l_loop_init(l_loop_t *str);

void l_loop_run(l_loop_t *str);

#endif

