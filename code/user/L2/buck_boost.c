#include "buck_boost.h"
#include "stdint.h"
#include "math.h"
#include "l_loop_open.h"
#include "my_math.h"
#include "shell.h"
#include "buck_boost_mode.h"
#include "section.h"
#include "can_wg.h"
#include "pid.h"

extern float Get_Set_Out_Curr_Value_Lmt(void);
uint8_t dcm_obs_trig = 0;
float dcm_i_ref_obs = 0.0f;
float dcm_i_l_obs = 0.0f;
float dcm_i_ripple_obs = 0.0f;
float I_ref_Show = 0.0f;
uint8_t trig_obs = 0;
float i_ref_obs[10]={0};
float vout_obs[10]={0};
float i_out_obs[10] = {0};
uint8_t i_ref_obs_cnt = 0;

#if (SHELL_ON_OFF == 1)
REG_SHELL_VAR(dcm_obs_trig, dcm_obs_trig, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(dcm_i_ref_obs, dcm_i_ref_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(dcm_i_l_obs, dcm_i_l_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(dcm_i_ripple_obs, dcm_i_ripple_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(I_ref_Show, I_ref_Show, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(trig_obs, trig_obs, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs_cnt, i_ref_obs_cnt, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs0, i_ref_obs[0], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs1, i_ref_obs[1], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs2, i_ref_obs[2], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs3, i_ref_obs[3], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs4, i_ref_obs[4], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs5, i_ref_obs[5], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs6, i_ref_obs[6], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs7, i_ref_obs[7], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs8, i_ref_obs[8], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_ref_obs9, i_ref_obs[9], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)

REG_SHELL_VAR(vout_obs0, vout_obs[0], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(vout_obs1, vout_obs[1], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(vout_obs2, vout_obs[2], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(vout_obs3, vout_obs[3], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(vout_obs4, vout_obs[4], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(vout_obs5, vout_obs[5], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(vout_obs6, vout_obs[6], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(vout_obs7, vout_obs[7], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(vout_obs8, vout_obs[8], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(vout_obs9, vout_obs[9], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)

REG_SHELL_VAR(i_out_obs0, i_out_obs[0], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_out_obs1, i_out_obs[1], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_out_obs2, i_out_obs[2], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_out_obs3, i_out_obs[3], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_out_obs4, i_out_obs[4], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_out_obs5, i_out_obs[5], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_out_obs6, i_out_obs[6], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_out_obs7, i_out_obs[7], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_out_obs8, i_out_obs[8], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_out_obs9, i_out_obs[9], SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
#endif

void buck_boost_init(buck_boost_t *str)
{
    for (uint32_t i = 0; i < BUCK_BOOST_L_LOOP_NUM; i++)
    {
        str->inter.is_ccm[i] = 1;
        str->inter.l_loop[i].cfg.l_val = str->cfg.l_val[i];
        str->inter.l_loop[i].cfg.freq_cut = 2000.0f;
        str->inter.l_loop[i].cfg.lmt = 100.0f;
        str->inter.l_loop[i].cfg.freq_ctrl = str->cfg.ctrl_freq;
        l_loop_init(&str->inter.l_loop[i]);
    }
    str->inter.volt_loop.cfg.c_val = str->cfg.c_val;
    str->inter.volt_loop.cfg.freq_ctrl = str->cfg.ctrl_freq;
    str->inter.volt_loop.cfg.freq_cut = 350.0f;
    str->inter.volt_loop.cfg.lmt = 100.0f;
    volt_loop_init(&str->inter.volt_loop);

    str->inter.vin_lmt.cfg.obj_val = str->cfg.c_val;
    str->inter.vin_lmt.cfg.freq_ctrl = str->cfg.ctrl_freq;
    str->inter.vin_lmt.cfg.freq_cut = str->cfg.ctrl_freq * 0.1f * 0.12f;
    str->inter.vin_lmt.cfg.lmt = 140.0f;
    curr_lmt_init(&str->inter.vin_lmt);

    str->inter.iin_lmt.cfg.obj_val = str->cfg.l_val[0];
    str->inter.iin_lmt.cfg.freq_ctrl = str->cfg.ctrl_freq;
    str->inter.iin_lmt.cfg.freq_cut = 2000.0f;
    str->inter.iin_lmt.cfg.lmt = 140.0f;
    curr_lmt_init(&str->inter.iin_lmt);

    str->inter.iout_lmt.cfg.obj_val = str->cfg.l_val[0];
    str->inter.iout_lmt.cfg.freq_ctrl = str->cfg.ctrl_freq;
    str->inter.iout_lmt.cfg.freq_cut = 2000.0f;
    str->inter.iout_lmt.cfg.lmt = 140.0f;
    curr_lmt_init(&str->inter.iout_lmt);

//    str->inter.buck_boost_mode = BB_MODE_BUCK;
//    buck_boost_mode_init(&str->inter.bb_mode_duty[0]);
//    buck_boost_mode_init(&str->inter.bb_mode_duty[1]);

    str->inter.volt_comp_loop.cfg.kp = sinf(60.0f / 180.0f * M_PI) * 2.0f * M_PI * 2.0f * str->cfg.c_val;
    str->inter.volt_comp_loop.cfg.ki = 2.0f * M_PI * 2.0f * str->inter.volt_comp_loop.cfg.kp / tanf(60.0f / 180.0f * M_PI) / str->cfg.ctrl_freq;    
    str->inter.volt_comp_loop.cfg.kd = 0.0f;
    str->inter.volt_comp_loop.cfg.ki_inv = 1.0f / str->inter.volt_comp_loop.cfg.ki;
    str->inter.volt_comp_loop.cfg.i_err_lmt_max = 2.0f;
    str->inter.volt_comp_loop.cfg.i_err_lmt_min = -2.0f;
    str->inter.volt_comp_loop.cfg.output_lmt_max = 2.0f;
    str->inter.volt_comp_loop.cfg.output_lmt_min = -2.0f;
    str->inter.volt_comp_loop.input.ref = 0.0f;
}

static void RAMFUNC buck_boost_mode_cal(buck_boost_t *str)
{
    float gain = str->input.v_out / str->input.v_in;
    switch (str->inter.buck_boost_mode)
    {
    case BB_MODE_BOOST:
        if (gain < BOOST_TO_BUCK_BOOST_THR)
        {
            str->inter.buck_boost_mode = BB_MODE_BUCK_BOOST;
        }
        break;
    case BB_MODE_BUCK:
        if (gain > BUCK_TO_BUCK_BOOST_THR)
        {
            str->inter.buck_boost_mode = BB_MODE_BUCK_BOOST;
        }
        break;
    case BB_MODE_BUCK_BOOST:
        if (gain > BUCK_BOOST_TO_BOOST_THR)
        {
            str->inter.buck_boost_mode = BB_MODE_BOOST;
        }
        else if (gain < BUCK_BOOST_TO_BUCK_THR)
        {
            str->inter.buck_boost_mode = BB_MODE_BUCK;
        }
        break;
    default:
        str->inter.buck_boost_mode = BB_MODE_BUCK;
        break;
    }
}

extern void gpio_set_db2(uint8_t val);
extern void gpio_set_db1(uint8_t val);
static void RAMFUNC buck_boost_l_open_loop(buck_boost_t *str, float i_ref, uint32_t i)
{
    float buck_duty = 0.0f;
    float boost_duty = 0.0f;
    switch (str->inter.buck_boost_mode)
    {
    case BB_MODE_BOOST:
							// gpio_set_db2(0);
					// gpio_set_db2(1);
        DN_LMT(i_ref, 0.0f);
        boost_duty = cal_boost_dcm_duty(i_ref, str->cfg.pwm_ts,
                                        str->input.v_in,
                                        str->input.v_out,
                                        5.8e-6f);
        UP_DN_LMT(boost_duty, 1.0f, 0.0f);
        buck_duty = 1.0f;
        str->output.duty[i].buck_duty = buck_duty;
        str->output.duty[i].boost_duty = 1.0f - boost_duty;
        str->output.duty[i].buck_dn_en = 1;
        str->output.duty[i].buck_up_en = 1;
        str->output.duty[i].boost_dn_en = 1;
        str->output.duty[i].boost_up_en = 0;
							// gpio_set_db2(0);
					// gpio_set_db2(1);
        break;
    case BB_MODE_BUCK:
									// gpio_set_db2(0);
					// gpio_set_db2(1);
        DN_LMT(i_ref, 0.0f);
        buck_duty = cal_buck_dcm_duty(i_ref,
                                      str->cfg.pwm_ts,
                                      str->input.v_in,
                                      str->input.v_out,
																			     5.8e-6f);             //str->cfg.l_val[i]);
        UP_DN_LMT(buck_duty, 1.0f, 0.0f);
        boost_duty = 1.0f;
        str->output.duty[i].buck_duty = buck_duty;
        str->output.duty[i].boost_duty = boost_duty;
        str->output.duty[i].buck_dn_en = 0;
        str->output.duty[i].buck_up_en = 1;
        str->output.duty[i].boost_dn_en = 1;
        str->output.duty[i].boost_up_en = 1;
							// gpio_set_db2(0);
					// gpio_set_db2(1);
        break;
    case BB_MODE_BUCK_BOOST:
							// gpio_set_db1(0);
					// gpio_set_db1(1);
        DN_LMT(i_ref, 0.0f);
        cal_buck_boost_dcm_duty(str->input.v_in,
                                str->input.v_out,
                                i_ref,
                                str->cfg.pwm_ts,
                                5.8e-6f,
                                &buck_duty,
                                &boost_duty);
        str->output.duty[i].buck_duty = buck_duty;
        str->output.duty[i].boost_duty = 1.0f - boost_duty;
        str->output.duty[i].buck_dn_en = 0;
        str->output.duty[i].buck_up_en = 1;
        str->output.duty[i].boost_dn_en = 1;
        str->output.duty[i].boost_up_en = 0;
							// gpio_set_db1(0);
					// gpio_set_db1(1);
        break;
    }
}

static void RAMFUNC buck_boost_l_close_loop(buck_boost_t *str, float i_ref, uint32_t i)
{
    str->inter.l_loop[i].input.i_act = str->input.i_l[i];
    str->inter.l_loop[i].input.i_ref = i_ref;
    str->inter.l_loop[i].input.i_load = 0.0f;
    l_loop_run(&str->inter.l_loop[i]);
}

static void RAMFUNC buck_boost_set_close_loop_duty(buck_boost_t *str, uint32_t i)
{
    str->inter.bb_mode_duty[i].input.v_in = str->input.v_in;
    str->inter.bb_mode_duty[i].input.v_out = str->input.v_out;
    str->inter.bb_mode_duty[i].input.v_l = str->inter.l_loop[i].output.l_volt;
    buck_boost_mode_func(&str->inter.bb_mode_duty[i]);

    str->output.duty[i].buck_duty = str->inter.bb_mode_duty[i].output.buck_duty;
    str->output.duty[i].boost_duty = str->inter.bb_mode_duty[i].output.boost_duty;

    str->output.duty[i].buck_dn_en = str->inter.bb_mode_duty[i].output.buck_dn_en;
    str->output.duty[i].buck_up_en = 1;
    str->output.duty[i].boost_dn_en = 1;
    str->output.duty[i].boost_up_en = str->inter.bb_mode_duty[i].output.boost_up_en;
}

static float RAMFUNC cal_boost_l_curr_ripple(float t_s, float v_in, float v_out, float l)
{
    // 计算公式
    float result = (t_s * v_in * (1.0f - v_in / v_out)) / l;
    return result;
}

static float RAMFUNC cal_buck_l_curr_ripple(float t_s, float v_in, float v_out, float l)
{
    // 计算公式
    float result = (t_s * (v_in - v_out) * v_out) / (l * v_in);
    return result;
}

static float RAMFUNC cal_buck_boost_l_curr_ripple(float t_s, float v_in, float v_out, float l)
{

    if (v_in > v_out)
    {
        return (0.2f * t_s * v_out) / l;
    }
    else
    {
        return (t_s * v_in * (1.0f - (BUCK_BOOST_BUCK_BOOST_FIX_DUTY * v_in) / v_out)) / l;
    }
}

static float RAMFUNC buck_boost_cal_l_curr_ripple(buck_boost_t *str, uint32_t index)
{
    switch (str->inter.buck_boost_mode)
    {
    case BB_MODE_BUCK:
        return cal_buck_l_curr_ripple(str->cfg.pwm_ts,
                                      str->input.v_in,
                                      str->input.v_out,
                                      5.8E-6F);
        //break;
    case BB_MODE_BOOST:
        return cal_boost_l_curr_ripple(str->cfg.pwm_ts,
                                       str->input.v_in,
                                       str->input.v_out,
                                       5.8E-6F);
        //break;
    case BB_MODE_BUCK_BOOST:
        return cal_buck_boost_l_curr_ripple(str->cfg.pwm_ts,
                                            str->input.v_in,
                                            str->input.v_out,
                                            5.8E-6F);
        //break;
    default:
        return 0.0f;
        //break;
    }
}

float iout_lmt_output = 0;
float iin_lmt_output = 0;
float vin_lmt_output = 0;

float i_in_lmt = 0.0f;
float i_out_lmt = 0.0f;

float l_curr_ripple_min = 0.0f;
float l_curr_ripple_min_lmt = 0.0f;

float i_l_cal = 0.0f;
float i_l_cal_in = 0.0f;
float i_l_cal_out = 0.0f;

extern void gpio_set_db2(uint8_t val);
extern void gpio_set_db1(uint8_t val);

static uint8_t lock = 0;
static uint8_t is_ccm_last = 0;

#if (SHELL_ON_OFF == 1)
static uint8_t b_mode_obs = 0;
static uint8_t b_buck_up_en_obs = 0;
static uint8_t b_buck_dn_en_obs = 0;
static uint8_t b_boost_up_en_obs = 0;
static uint8_t b_boost_dn_en_obs = 0;
static float b_vpwm_obs = 0.0f;
static float b_i_act_obs = 0.0f;
static float b_i_ref_obs = 0.0f;
static float b_l_volt_obs = 0.0f;
static float b_v_in_obs = 0.0f;
static float b_v_out_obs = 0.0f;
static float b_buck_duty_obs = 0.0f;
static float b_boost_duty_obs = 0.0f;
REG_SHELL_VAR(i_in_lmt, i_in_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_out_lmt, i_out_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)

REG_SHELL_VAR(l_curr_ripple_min, l_curr_ripple_min, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(l_curr_ripple_min_lmt, l_curr_ripple_min_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)

REG_SHELL_VAR(i_l_cal, i_l_cal, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_l_cal_in, i_l_cal_in, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(i_l_cal_out, i_l_cal_out, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)

REG_SHELL_VAR(lock, lock, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_mode_obs, b_mode_obs, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_buck_up_en_obs, b_buck_up_en_obs, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_buck_dn_en_obs, b_buck_dn_en_obs, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_boost_up_en_obs, b_boost_up_en_obs, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_boost_dn_en_obs, b_boost_dn_en_obs, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)

REG_SHELL_VAR(b_vpwm_obs, b_vpwm_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_i_act_obs, b_i_act_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_i_ref_obs, b_i_ref_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_l_volt_obs, b_l_volt_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_v_in_obs, b_v_in_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_v_out_obs, b_v_out_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_buck_duty_obs, b_buck_duty_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(b_boost_duty_obs, b_boost_duty_obs, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
#endif

extern uint8_t force_ccm;
void RAMFUNC buck_boost_run(buck_boost_t *str)
{
	// // gpio_set_db2(0);
	// // gpio_set_db2(1);
    str->inter.volt_loop.input.v_act = str->input.v_out;// * 0.3f + str->inter.volt_loop.input.v_act * 0.7f;
    str->inter.volt_loop.input.v_ref = str->input.v_out_ref;//-0.01f * str->input.i_out;// + str->input.volt_comp;

    //pid_cal(&str->inter.volt_comp_loop,str->input.v_out_ref,str->input.v_out);

    str->inter.vin_lmt.input.pos = str->input.v_in;
    str->inter.vin_lmt.input.neg = str->input.vin_lmt;

    UP_LMT(str->input.iin_lmt, str->input.pwr_in_lmt / DN_LMT(str->input.v_in, 1.0f));
    UP_LMT(str->input.iout_lmt, str->input.pwr_out_lmt / DN_LMT(str->input.v_out, 1.0f));

    str->inter.iin_lmt.input.pos = str->input.iin_lmt;
    str->inter.iin_lmt.input.neg = str->input.i_in;

    str->inter.iout_lmt.input.pos = str->input.iout_lmt;
    str->inter.iout_lmt.input.neg = str->input.i_out;
// // gpio_set_db2(0);
	// // gpio_set_db2(1);
    buck_boost_mode_cal(str);
	// // gpio_set_db2(0);
	// // gpio_set_db2(1);
		float i_lmt = 0.0f;

	float temp = str->input.v_out_ref / str->input.v_in + 0.1f;
    float i_out_lmt_gain = MAX(1.2f, temp);
	temp = str->input.v_in / str->input.v_out_ref + 0.1f;
    float i_in_lmt_gain = MAX(1.2f, temp);

	float iin_iout_lmt = MAX(str->input.iin_lmt * i_in_lmt_gain, str->input.iout_lmt * i_out_lmt_gain);

    str->inter.iin_lmt.inter.pid.cfg.i_err_lmt_max = iin_iout_lmt;
    str->inter.iin_lmt.inter.pid.cfg.output_lmt_max = iin_iout_lmt;
    str->inter.iout_lmt.inter.pid.cfg.i_err_lmt_max = iin_iout_lmt;
    str->inter.iout_lmt.inter.pid.cfg.output_lmt_max = iin_iout_lmt;

	// // gpio_set_db2(0);
	// // gpio_set_db2(1);
    curr_lmt_cal(&str->inter.iin_lmt);
    curr_lmt_cal(&str->inter.iout_lmt);
	// // gpio_set_db2(0);
	// // gpio_set_db2(1);
    iout_lmt_output = str->inter.iout_lmt.output.lmt;
    iin_lmt_output = str->inter.iin_lmt.output.lmt;
		
		i_lmt = MIN(iout_lmt_output,iin_lmt_output);
		
		str->inter.vin_lmt.inter.pid.cfg.i_err_lmt_max = i_lmt;
    str->inter.vin_lmt.inter.pid.cfg.output_lmt_max = i_lmt;
		str->inter.vin_lmt.inter.pid.cfg.i_err_lmt_min = Get_Set_Out_Curr_Value_Lmt();
    str->inter.vin_lmt.inter.pid.cfg.output_lmt_min = Get_Set_Out_Curr_Value_Lmt();

    curr_lmt_cal(&str->inter.vin_lmt);
    vin_lmt_output = str->inter.vin_lmt.output.lmt;
    i_lmt = MIN(i_lmt, vin_lmt_output);
		
    str->inter.volt_loop.inter.pid.cfg.i_err_lmt_max = i_lmt;
		str->inter.volt_loop.inter.pid.cfg.output_lmt_max = i_lmt;
		
    volt_loop_cal(&str->inter.volt_loop);
		// // gpio_set_db2(0);
	// // gpio_set_db2(1);
	float i_out_temp = str->input.i_out;
	if(i_out_temp < 0.0f)
	{
		i_out_temp = 0.0f;
	}
    float i_ref = str->inter.volt_loop.output.out + i_out_temp * 0.9f;

		UP_LMT(i_ref,i_lmt);
		
    DN_LMT(i_ref, -10.0f);
    i_ref *= 0.5f;
    // // gpio_set_db2(0);
	// // gpio_set_db2(1);
    if((i_ref > 0.0f)&&
        str->input.v_out > str->input.v_out_ref+1.0f)
    {
        i_ref *= 0.5f;
		str->inter.volt_loop.inter.pid.inter.i_err *= 0.5f;
        
    }
    I_ref_Show=i_ref;
		if(trig_obs == 0){
		i_ref_obs[i_ref_obs_cnt] = i_ref;
			vout_obs[i_ref_obs_cnt] = str->input.v_out;
			i_out_obs[i_ref_obs_cnt] = str->input.i_out;
		i_ref_obs_cnt++;
		i_ref_obs_cnt%=10;}
	
		if(str->input.v_out < 11.0f)
		{
			trig_obs = 1;
		}

    l_curr_ripple_min = buck_boost_cal_l_curr_ripple(str, 0) * 0.5f;

    l_curr_ripple_min_lmt = l_curr_ripple_min;
    UP_LMT(l_curr_ripple_min_lmt, 5.0f);
		// // gpio_set_db2(0);
	// gpio_set_db2(1);
				// gpio_set_db2(0);
	// gpio_set_db2(1);
        str->inter.bb_mode_duty[0].output.is_half_freq = 0;
		str->inter.bb_mode_duty[1].output.is_half_freq = 0;
        if (str->input.i_out < -5.0f)
				{
            str->inter.dcm_cnt[0] = 0;
            str->inter.is_ccm[0] = 0;
					  i_ref = 0.0f;
            pid_clr_i_err(&str->inter.volt_loop.inter.pid);
				}
				else if ((str->input.i_out < 0.0f)&&
            (str->inter.is_ccm[0] == 1))
				{
					str->inter.iout_dcm_cnt++;
					if(str->inter.iout_dcm_cnt > (uint32_t)str->cfg.ctrl_freq * 0.002f)
					{
						str->inter.iout_dcm_cnt = (uint32_t)str->cfg.ctrl_freq * 0.002f;
						    str->inter.dcm_cnt[0] = 0;
							str->inter.is_ccm[0] = 0;
						i_ref = 0.0f;
						pid_clr_i_err(&str->inter.volt_loop.inter.pid);
					}
				}
				else if ((i_ref < l_curr_ripple_min_lmt * 0.4f) &&
            (str->inter.is_ccm[0] == 1))
        {
			str->inter.iout_dcm_cnt = 0;
            if (str->inter.dcm_cnt[0] < (uint32_t)str->cfg.ctrl_freq * 0.1f)
            {
                str->inter.dcm_cnt[0]++;
            }
            else
            {
                str->inter.dcm_cnt[0] = 0;
                str->inter.is_ccm[0] = 0;
            }
        }
        else if ((i_ref > l_curr_ripple_min_lmt * 0.9f) ||
        (str->input.i_l[0] > l_curr_ripple_min * 1.2f)||
		(str->input.i_l[1] > l_curr_ripple_min * 1.2f))		
        
				{
                    str->inter.iout_dcm_cnt = 0;
//						if(dcm_obs_trig == 0)
//						{
//							dcm_i_ref_obs = i_ref;
//							dcm_i_l_obs = str->input.i_l[i];
//							dcm_i_ripple_obs = l_curr_ripple_min;
//						}
            str->inter.is_ccm[0] = 1;
            str->inter.dcm_cnt[0] = 0;
        }
		else
		{
			str->inter.iout_dcm_cnt = 0;
		}

        str->inter.is_ccm[0] = (force_ccm == 1) ? (1) : (str->inter.is_ccm[0]);

				// gpio_set_db2(0);
	// gpio_set_db2(1);
//          str->inter.is_ccm[0] =1;  //关闭DCM
        if (str->inter.is_ccm[0])
        {
			    for (uint32_t i = 0; i < BUCK_BOOST_L_LOOP_NUM; i++)
    {
			// gpio_set_db2(0);
	// gpio_set_db2(1);
            buck_boost_l_close_loop(str, i_ref, i);
            buck_boost_set_close_loop_duty(str, i);
		    }
        }
        else
        {
			// gpio_set_db1(1);
            pid_clr_i_err(&str->inter.l_loop[0].inter.pid); // DCM下清除积分误差
			pid_clr_i_err(&str->inter.l_loop[1].inter.pid); // DCM下清除积分误差
					if(str->inter.volt_loop.inter.pid.output.output < 0.0f)
					{
						pid_clr_i_err(&str->inter.volt_loop.inter.pid); // DCM下清除积分误差
					}
            str->inter.bb_mode_duty[0].inter.mode = (str->inter.buck_boost_mode == BB_MODE_BOOST)
                                                        ? BUCK_BOOST_MODE_BOOST
                                                        : ((str->inter.buck_boost_mode == BB_MODE_BUCK)
                                                               ? BUCK_BOOST_MODE_BUCK
                                                               : BUCK_BOOST_MODE_BUCK_BOOST);
            str->inter.bb_mode_duty[1].inter.mode = str->inter.bb_mode_duty[0].inter.mode;
            buck_boost_l_open_loop(str, i_ref, 0);
					str->output.duty[1] = str->output.duty[0];
					// gpio_set_db1(0);
        }
    if ((is_ccm_last == 0) &&
        (str->inter.is_ccm[0] == 1) &&
        (str->output.duty[0].boost_duty < 0.8f) &&
        (str->output.duty[1].boost_duty > 0.8f) &&
        (lock == 0))
    {
        lock = 1;
//        b_i_act_obs = str->inter.l_loop[1].input.i_act;
//        b_i_ref_obs = str->inter.l_loop[1].input.i_ref;
//        b_l_volt_obs = str->inter.l_loop[1].output.l_volt;
//        b_v_in_obs = str->inter.bb_mode_duty[1].input.v_in;
//        b_v_out_obs = str->inter.bb_mode_duty[1].input.v_out;
//        b_mode_obs = str->inter.bb_mode_duty[1].inter.mode;
//        b_buck_duty_obs = str->inter.bb_mode_duty[1].output.buck_duty;
//        b_buck_up_en_obs = str->output.duty[1].buck_up_en;
//        b_buck_dn_en_obs = str->output.duty[1].buck_dn_en;
//        b_boost_duty_obs = str->inter.bb_mode_duty[1].output.boost_duty;
//        b_boost_up_en_obs = str->output.duty[1].boost_up_en;
//        b_boost_dn_en_obs = str->output.duty[1].boost_dn_en;
    }
    is_ccm_last = str->inter.is_ccm[0];

	// gpio_set_db2(0);
}

void RAMFUNC buck_boost_bidirectional(buck_boost_t *str)
{
    str->inter.volt_loop.input.v_act = str->input.v_out;
    str->inter.volt_loop.input.v_ref = str->input.v_out_ref;

    str->inter.vin_lmt.input.pos = str->input.vin_lmt;
    str->inter.vin_lmt.input.neg = str->input.v_in;

    UP_LMT(str->input.iin_lmt, str->input.pwr_in_lmt / DN_LMT(str->input.v_in, 1.0f));
    UP_LMT(str->input.iout_lmt, str->input.pwr_out_lmt / DN_LMT(str->input.v_out, 1.0f));

    str->inter.iin_lmt.input.pos = str->input.iin_lmt;
    str->inter.iin_lmt.input.neg = fabsf(str->input.i_in);

    str->inter.iout_lmt.input.pos = str->input.iout_lmt;
    str->inter.iout_lmt.input.neg = fabsf(str->input.i_out);

    buck_boost_mode_cal(str);

    volt_loop_cal(&str->inter.volt_loop);
    curr_lmt_cal(&str->inter.vin_lmt);

    float i_ref = str->inter.volt_loop.output.out - str->inter.vin_lmt.output.lmt;

    curr_lmt_cal(&str->inter.iin_lmt);
    curr_lmt_cal(&str->inter.iout_lmt);

    UP_DN_LMT(i_ref, str->inter.iin_lmt.output.lmt, -str->inter.iin_lmt.output.lmt);
    UP_DN_LMT(i_ref, str->inter.iout_lmt.output.lmt, -str->inter.iout_lmt.output.lmt);

    i_ref /= BUCK_BOOST_L_LOOP_NUM;

    for (uint32_t i = 0; i < BUCK_BOOST_L_LOOP_NUM; i++)
    {
        buck_boost_l_close_loop(str, i_ref, i);
        buck_boost_set_close_loop_duty(str, i);
    }
}

// open_loop

#include "section.h"
#include "bsp_pwm.h"

buck_boost_mode_t open_loop_bb;
bsp_pwm_t open_loop_pwm;

float open_loop_vin = 1.0f;
float open_loop_vout = 0.5f;
float open_loop_vout_now = 0.0f;

uint8_t open_loop_mode = 3;

extern uint32_t RAMFUNC HRPWM_GetPeriodValue_Buff(const CM_HRPWM_TypeDef *HRPWMx);




void PWM_para_config(void)
{
    bsp_pwm_change_freq_pwma(); 
    bsp_pwm_change_full_freq_pwma();//设置PWMB为全频率pwm
    bsp_pwm_change_full_freq_pwmb();
    open_loop_pwm.left_duty   = 0.0f;
    open_loop_pwm.left_dn_en  = 0;
    open_loop_pwm.left_up_en  = 0;
    open_loop_pwm.right_dn_en = 0;
    open_loop_pwm.right_up_en = 0;
    open_loop_pwm.right_duty  = 0.0f;
    bsp_pwm_set_a_duty(&open_loop_pwm);
    bsp_pwm_set_b_duty(&open_loop_pwm);
    bsp_pwm_update_chclt2();  // 更新通道配置
    bsp_hrpwm1_update_trig(); // 触发hrpwm1单次缓存
    HRPWM_IDLE_Exit(HRPWM_PWM_SYNC_UNIT, HRPWM_SW_SYNC_CH_ALL); //所有通道退出空闲
    HRPWM_CountStart(PWM_UNIT); // 启动定时器必须从HRPWM1开始，1是主通道
}

//p烓粉翹羹m cfg channel .left upen=1;
//pwm cfg channel .left dn en=1;
//pwm cfg channel .right up en=1;
//Pwm cfg channel right dn en=1
//pwm cfg channel .right duty=0.2f;
//pwm cfg channel B.left duty=0.2f:
//pwm cfg channel B.left up en=1;
//pwm cfg channel B.left dn en=;
//pwm cfg channel B.right up en =l;
//pwm cfg channel B.right dnen1
//pwm cfg channel B.right duty=0.2f;
//bsp pwm set a duty(spwm cfg channel )bsp pwm set b duty(spw cfg channel B);
//bsp pwm update chclt2();
//bsp hrpwml update trig();//触发hrpwm1单次缓存

#include "adc.h"
#include "data_com.h"
extern float fvs48;
extern float ihv;
extern float ilv;
extern float rvs12;
extern float ila;
extern float ilb;
extern float fvs48_pwr_lmt;
extern float rvs12_pwr_lmt;


void open_loop_test(void)
{
    fvs48 = adc_get_fvs48();
    ihv = adc_get_ihv();
    ilv = adc_get_ilv();
    rvs12 = adc_get_rvs12();
    ila = adc_get_ila();
    ilb = adc_get_ilb();
    fvs48_pwr_lmt = data_com_get_fvs48_pwr_lmt();
    rvs12_pwr_lmt = data_com_get_rvs12_pwr_lmt();
	
    bsp_pwm_change_freq_pwma(); 
    
    open_loop_bb.input.v_in = open_loop_vin;
    open_loop_bb.input.v_out = open_loop_vout_now;
    open_loop_bb.input.v_l = 0.0f;
    buck_boost_mode_func(&open_loop_bb);

    open_loop_pwm.left_duty = open_loop_bb.output.buck_duty;
    open_loop_pwm.right_duty = open_loop_bb.output.boost_duty;

    open_loop_pwm.left_dn_en = 1;
    open_loop_pwm.left_up_en = 1;
    open_loop_pwm.right_dn_en = 1;
    open_loop_pwm.right_up_en = 1;

    if (open_loop_bb.output.is_half_freq == 0)
    {
        bsp_pwm_change_full_freq_pwma(); // 设置PWMA为全频率
        bsp_pwm_change_full_freq_pwmb(); // 设置PWMB为全频率
//        HRPWM_SetPeriodValue_Buf(PWM_UNIT,CTRL_PERIOD);
//        HRPWM_SetPeriodValue_Buf(PWM_UNIT,CTRL_PERIOD);
    }
    else
    {
        bsp_pwm_change_half_freq_pwma(); // 设置PWMA为半频率
        bsp_pwm_change_half_freq_pwmb(); // 设置PWMB为半频率
//        HRPWM_SetPeriodValue_Buf(PWM_UNIT,((CTRL_PERIOD + 1) << 1)-64);
//        HRPWM_SetPeriodValue_Buf(PWM_UNIT,((CTRL_PERIOD + 1) << 1)-64);
    }
    
    if (open_loop_mode == 0)
    {

        open_loop_pwm.left_duty = 0.0f;
        open_loop_pwm.right_duty = 0.0f;

        open_loop_pwm.left_dn_en = 0;
        open_loop_pwm.left_up_en = 0;
        open_loop_pwm.right_dn_en = 0;
        open_loop_pwm.right_up_en = 0;

        open_loop_vout = 0;
        bsp_pwm_set_a_duty(&open_loop_pwm);
        bsp_pwm_set_b_duty(&open_loop_pwm);
    }
    else if (open_loop_mode == 1)
    {
        RAMP(open_loop_vout_now, open_loop_vout, 0.001f);

        open_loop_pwm.left_duty = open_loop_bb.output.buck_duty;
        open_loop_pwm.right_duty = open_loop_bb.output.boost_duty;

        open_loop_pwm.left_dn_en = 1;
        open_loop_pwm.left_up_en = 1;
        open_loop_pwm.right_dn_en = 1;
        open_loop_pwm.right_up_en = 1;
        bsp_pwm_set_a_duty(&open_loop_pwm);

        open_loop_pwm.left_duty = 0.0f;
        open_loop_pwm.right_duty = 0.0f;

        open_loop_pwm.left_dn_en = 0;
        open_loop_pwm.left_up_en = 0;
        open_loop_pwm.right_dn_en = 0;
        open_loop_pwm.right_up_en = 0;
        bsp_pwm_set_b_duty(&open_loop_pwm);
    }
    else if (open_loop_mode == 2)
    {
        RAMP(open_loop_vout_now, open_loop_vout, 0.001f);

        open_loop_pwm.left_duty = open_loop_bb.output.buck_duty;
        open_loop_pwm.right_duty = open_loop_bb.output.boost_duty;

        open_loop_pwm.left_dn_en = 1;
        open_loop_pwm.left_up_en = 1;
        open_loop_pwm.right_dn_en = 1;
        open_loop_pwm.right_up_en = 1;
        bsp_pwm_set_b_duty(&open_loop_pwm);

        open_loop_pwm.left_duty = 0.0f;
        open_loop_pwm.right_duty = 0.0f;

        open_loop_pwm.left_dn_en = 0;
        open_loop_pwm.left_up_en = 0;
        open_loop_pwm.right_dn_en = 0;
        open_loop_pwm.right_up_en = 0;
        bsp_pwm_set_a_duty(&open_loop_pwm);
    }
    else if (open_loop_mode == 3)
    {
        RAMP(open_loop_vout_now, open_loop_vout, CTRL_TS);

        open_loop_pwm.left_duty = open_loop_bb.output.buck_duty;
        open_loop_pwm.right_duty = open_loop_bb.output.boost_duty;

        open_loop_pwm.left_dn_en = 1;
        open_loop_pwm.left_up_en = 1;
        open_loop_pwm.right_dn_en = 1;
        open_loop_pwm.right_up_en = 1;
        bsp_pwm_set_a_duty(&open_loop_pwm);
        bsp_pwm_set_b_duty(&open_loop_pwm);
    }
    else if (open_loop_mode == 4)
    {
        bsp_pwm_set_a_duty(&open_loop_pwm);
        bsp_pwm_set_b_duty(&open_loop_pwm);
    }
//    bsp_hrpwm1_update_trig(); // 触发hrpwm1单次缓存
//    bsp_pwm_update_chclt2();  // 更新通道配置
    bsp_hrpwm1_update_trig(); // 触发hrpwm1单次缓存
}
