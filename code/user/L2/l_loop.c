#include "l_loop.h"
#include "math.h"
#include "pid.h"
#include "my_math.h"
#include "section.h"

#define L_LOOP_SIN_70       (0.939692621f)
#define L_LOOP_TAN_70       (2.747477419f)

void l_loop_init(l_loop_t *str)
{
    pid_reset(&str->inter.pid);
    str->cfg.w_cut = 2.0f * M_PI * str->cfg.freq_cut;
    str->inter.pid.cfg.kp = L_LOOP_SIN_70 * str->cfg.w_cut * str->cfg.l_val;
    str->inter.pid.cfg.ki = str->cfg.w_cut * str->inter.pid.cfg.kp / L_LOOP_TAN_70 / str->cfg.freq_ctrl;
    str->inter.pid.cfg.ki_inv = 1.0f / str->inter.pid.cfg.ki;
    str->inter.pid.cfg.i_err_lmt_max = str->cfg.lmt * str->inter.pid.cfg.ki_inv;
    str->inter.pid.cfg.i_err_lmt_min = -str->cfg.lmt * str->inter.pid.cfg.ki_inv;
    str->inter.pid.cfg.output_lmt_max = str->cfg.lmt;
    str->inter.pid.cfg.output_lmt_min = -str->cfg.lmt;
}

void RAMFUNC l_loop_run(l_loop_t *str)
{
    str->output.l_volt = pid_cal(&str->inter.pid, str->input.i_ref, str->input.i_act);
    float l_volt_fw = 0.0f;
    float delta_i_load = 0.0f;
    delta_i_load = (str->input.i_load - str->inter.i_load_last);
    l_volt_fw = delta_i_load * str->cfg.l_val * str->cfg.freq_ctrl;
    str->output.l_volt += l_volt_fw;
    str->inter.i_load_last = str->input.i_load;
}
