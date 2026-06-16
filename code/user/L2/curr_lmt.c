#include "curr_lmt.h"
#include "pid.h"
#include "my_math.h"
#include "section.h"
void curr_lmt_init(curr_lmt_t *str)
{
    pid_reset(&str->inter.pid);
    str->cfg.w_cut = 2.0f * M_PI * str->cfg.freq_cut;
    str->inter.pid.cfg.kp = 0.35f;
    str->inter.pid.cfg.ki = 91.0f / str->cfg.freq_ctrl;
    str->inter.pid.cfg.ki_inv = 1.0f / str->inter.pid.cfg.ki;
    str->inter.pid.cfg.i_err_lmt_max = str->cfg.lmt;
    str->inter.pid.cfg.i_err_lmt_min = -3.0f;
    str->inter.pid.cfg.output_lmt_max = str->cfg.lmt;
    str->inter.pid.cfg.output_lmt_min = -3.0f;
}

float RAMFUNC curr_lmt_cal(curr_lmt_t *str)
{
    str->output.lmt = pid_cal(&str->inter.pid, str->input.pos, str->input.neg);
    return str->output.lmt;
}
