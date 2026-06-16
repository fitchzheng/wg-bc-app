#ifndef __CURR_LMT_H
#define __CURR_LMT_H

#include "stdint.h"
#include "pid.h"

typedef struct
{
    float pos;
    float neg;
} curr_lmt_input_t;

typedef struct
{
    float obj_val;
    float lmt;
    float freq_ctrl;
    float freq_cut;
    float w_cut;
} curr_lmt_cfg_t;

typedef struct
{
    pid_param_t pid;
} curr_lmt_inter_t;

typedef struct
{
    float lmt;
} curr_lmt_output_t;

typedef struct
{
    curr_lmt_input_t input;
    curr_lmt_cfg_t cfg;
    curr_lmt_inter_t inter;
    curr_lmt_output_t output;
} curr_lmt_t;

void curr_lmt_init(curr_lmt_t *str);

float curr_lmt_cal(curr_lmt_t *str);

#endif
