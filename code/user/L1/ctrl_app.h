#ifndef __CTRL_APP_H
#define __CTRL_APP_H

#include "stdint.h"

#include "my_math.h"

#define L_VAL 3.3e-6f//4.7e-6f
#define V_CAP 12 * 470e-6f

typedef enum
{
    CTRL_IDLE,
    CTRL_FORWARD,
    CTRL_BACKWARD,
    CTRL_BIDIRECTIONAL,
} CTRL_MODE_E;

void ctrl_app_run(void);

void ctrl_app_buck_boost_dc_init(CTRL_MODE_E mode);

void ctrl_app_disable(void);

void ctrl_app_set_rvs12_lmt(float val);

void ctrl_app_set_fvs48_lmt(float val);

float ctrl_app_get_rvs12_lmt(void);

float ctrl_app_get_fvs48_lmt(void);

CTRL_MODE_E ctrl_app_get_ctrl_mode(void);

uint8_t ctrl_app_get_is_run(void);

float get_ihv_lmt_soft_curr(void);

float get_ilv_lmt_soft_curr(void);

void set_ihv_lmt_soft_curr(float val);

void set_ilv_lmt_soft_curr(float val);
#endif
