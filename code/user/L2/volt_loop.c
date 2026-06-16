#include "volt_loop.h"
#include "pid.h"
#include "my_math.h"
#include "section.h"
void volt_loop_init(volt_loop_t *volt_loop)
{
    pid_reset(&volt_loop->inter.pid);
    volt_loop->cfg.w_cut = 2.0f * M_PI * volt_loop->cfg.freq_cut;
    volt_loop->inter.pid.cfg.kp = sinf(70.0f/180.0f*M_PI) * volt_loop->cfg.w_cut * volt_loop->cfg.c_val;
    volt_loop->inter.pid.cfg.ki = volt_loop->cfg.w_cut * volt_loop->inter.pid.cfg.kp/tanf(70.0f/180.0f*M_PI) / volt_loop->cfg.freq_ctrl;
    volt_loop->inter.pid.cfg.ki_inv = 1.0f / volt_loop->inter.pid.cfg.ki;
    volt_loop->inter.pid.cfg.i_err_lmt_max = volt_loop->cfg.lmt;
    volt_loop->inter.pid.cfg.i_err_lmt_min = -10.0f;
    volt_loop->inter.pid.cfg.output_lmt_max = volt_loop->cfg.lmt;
    volt_loop->inter.pid.cfg.output_lmt_min = -10.0f;
}

float RAMFUNC volt_loop_cal(volt_loop_t *volt_loop)
{
    volt_loop->output.out = pid_cal(&volt_loop->inter.pid, volt_loop->input.v_ref, volt_loop->input.v_act);
    return volt_loop->output.out;
}
