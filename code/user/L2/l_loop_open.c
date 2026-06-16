#include "l_loop_open.h"
#include "math.h"
#include "stdlib.h"
#include "section.h"
// 安全开方
static inline float RAMFUNC safe_sqrtf(float x)
{
    return (x > 0.0f) ? sqrtf(x) : 0.0f;
}

// 安全除法
static inline float RAMFUNC safe_divf(float numerator, float denominator)
{
    return (fabsf(denominator) > 1e-6f) ? (numerator / denominator) : (numerator / 1e-6f);
}

// 限定 duty 范围
static inline float RAMFUNC limit_duty(float duty)
{
    if (duty < 0.0f)
        return 0.0f;
    if (duty > 1.0f)
        return 1.0f;
    return duty;
}

float RAMFUNC cal_boost_dcm_duty(float i_ref, float t_s, float v_in, float v_out, float l)
{
    float factor = (2.0f * i_ref * t_s * l * (v_out - v_in)) / (v_in * v_out);
    float result = safe_sqrtf(factor);
    return limit_duty(safe_divf(result, t_s));
}

float RAMFUNC cal_buck_dcm_duty(float i_ref, float t_s, float v_in, float v_out, float l)
{
    float denominator = (v_in * (v_in - v_out)) / l;
    float factor = (2.0f * i_ref * t_s * v_out) / denominator;
    float result = safe_sqrtf(factor);
    return limit_duty(safe_divf(result, t_s));
}

static float RAMFUNC buck_boost_dcm_calculate(float d, float t, float w, float v_in, float v_out, float l, float i_ref)
{
	float w_v_in = w - v_in;
	float w_v_in_3_v_out = w_v_in - 3.0f * v_out;
    float term1 = 2.0f * l * i_ref * (w_v_in) * (w_v_in_3_v_out);
    float temp = w * (w_v_in) * v_in + (w * w - w * v_in + v_in * v_in) * v_out;
    float term2 = d * d * t * temp;
    float A = term1 + term2;

    float sqrt_term = safe_sqrtf(t * v_out * A);
    float numerator = d * t * (w + v_in) * (w_v_in - v_out) + 2.0f * sqrt_term;
    float denominator = (w_v_in) * (w_v_in_3_v_out);

    return safe_divf(numerator, denominator);
}

static float RAMFUNC check_buck_duty(float w, float t, float l, float v_in, float v_out, float i_ref)
{
    float numerator = 2.0f * l * i_ref * v_out;
    float denominator = t * ((w + v_in) * (w + v_in) + (w - v_in) * v_out);
    return limit_duty(2.0f * safe_sqrtf(safe_divf(numerator, denominator)));
}


static float RAMFUNC check_buck_duty2(float w,
                                             float t,
                                             float l,
                                             float v_in,
                                             float v_out,
                                             float i_ref)
{
    float numerator;
    float denominator;

    (void)v_out;

    numerator = 2.0f * l * i_ref * v_out;
    denominator = t * v_in * w;

    return limit_duty(safe_sqrtf(safe_divf(numerator, denominator)));
}

void RAMFUNC cal_buck_boost_dcm_duty(float v_in,
                             float v_out,
                             float i_ref,
                             float t,
                             float l,
                             float *p_buck_duty,
                             float *p_boost_duty)
{
    if (i_ref < 0.0f)
    {
        *p_buck_duty = 0.0f;
        *p_boost_duty = 0.0f;
        return;
    }
    float w = v_in - v_out;

    if (w > 0.0f)
    {
        *p_buck_duty = check_buck_duty(w, t, l, v_in, v_out, i_ref);
        if (*p_buck_duty > OPEN_LOOP_FIX_DUTY)
        {
            *p_buck_duty = OPEN_LOOP_FIX_DUTY; // 限制最大占空比
        }
    }
    else
    {
        w=-w;
           *p_buck_duty = check_buck_duty2(w, t, l, v_in, v_out, i_ref);
        if (*p_buck_duty > OPEN_LOOP_FIX_DUTY)
        {
            *p_buck_duty = OPEN_LOOP_FIX_DUTY; // 限制最大占空比
        }
    }

    float ton = buck_boost_dcm_calculate(*p_buck_duty, t, w, v_in, v_out, l, i_ref);
    *p_boost_duty = limit_duty(safe_divf(ton, t));
}
