#include "buck_boost_mode.h"
#include "stddef.h"
#include "my_math.h"
#include "section.h"

void buck_boost_mode_init(buck_boost_mode_t *str)
{
    str->inter.mode = BUCK_BOOST_MODE_BUCK;
}

void RAMFUNC buck_boost_mode_func(buck_boost_mode_t *str)
{
    if (str == NULL)
        return;

    DN_LMT(str->input.v_in, 0.001f);
    DN_LMT(str->input.v_out, 0.001f);

    str->output.buck_dn_en = 1;
    str->output.boost_up_en = 1;

    switch (str->inter.mode)
    {
    case BUCK_BOOST_MODE_BUCK:
        str->output.buck_duty = (str->input.v_l + str->input.v_out) / str->input.v_in;
        if (str->output.buck_duty > BUCK_BOOST_MODE_SSW_TO_BB_MODE_THR)
        {
            str->output.boost_duty = BUCK_BOOST_MODE_DUTY_MAX;
            str->output.buck_duty = (str->input.v_l + str->input.v_out * BUCK_BOOST_MODE_DUTY_MAX) / str->input.v_in;
            str->inter.mode = BUCK_BOOST_MODE_BUCK_BOOST;
        }
        else
        {
            str->output.boost_duty = 1.0f;
        }
        break;

    case BUCK_BOOST_MODE_BOOST:
        str->output.boost_duty = (str->input.v_in - str->input.v_l) / str->input.v_out;
        if (str->output.boost_duty > BUCK_BOOST_MODE_SSW_TO_BB_MODE_THR)
        {
            str->output.buck_duty = BUCK_BOOST_MODE_DUTY_MAX;
            str->output.boost_duty = (str->input.v_in * BUCK_BOOST_MODE_DUTY_MAX - str->input.v_l) / str->input.v_out;
            str->inter.mode = BUCK_BOOST_MODE_BUCK_BOOST;
        }
        else
        {
            str->output.buck_duty = 1.0f;
        }
        break;

    case BUCK_BOOST_MODE_BUCK_BOOST:
        str->output.buck_duty = (str->input.v_l + str->input.v_out) / str->input.v_in;
        if (str->output.buck_duty > BUCK_BOOST_MODE_DUTY_MAX)
        {
            str->output.buck_duty = BUCK_BOOST_MODE_DUTY_MAX;
            str->output.boost_duty = (str->input.v_in * BUCK_BOOST_MODE_DUTY_MAX - str->input.v_l) / str->input.v_out;
            if (str->output.boost_duty + str->output.buck_duty > BUCK_BOOST_MODE_DUTY_SUM)
            {
                str->output.buck_duty = BUCK_BOOST_MODE_DUTY_SUM - str->output.boost_duty;
                str->output.boost_duty = (str->input.v_in * str->output.buck_duty - str->input.v_l) / str->input.v_out;
            }
            if (str->output.boost_duty < BUCK_BOOST_MODE_BB_MODE_TO_SSW_THR)
            {
                str->inter.mode = BUCK_BOOST_MODE_BOOST;
                str->output.buck_duty = 1.0f;
                str->output.boost_duty = (str->input.v_in - str->input.v_l) / str->input.v_out;
            }
        }
        else
        {
            str->output.boost_duty = BUCK_BOOST_MODE_DUTY_MAX;
            str->output.buck_duty = (str->input.v_l + str->input.v_out * BUCK_BOOST_MODE_DUTY_MAX) / str->input.v_in;
            if (str->output.boost_duty + str->output.buck_duty > BUCK_BOOST_MODE_DUTY_SUM)
            {
                str->output.boost_duty = BUCK_BOOST_MODE_DUTY_SUM - str->output.buck_duty;
                str->output.buck_duty = (str->input.v_out * str->output.boost_duty + str->input.v_l) / str->input.v_in;
            }
            if (str->output.buck_duty < BUCK_BOOST_MODE_BB_MODE_TO_SSW_THR)
            {
                str->inter.mode = BUCK_BOOST_MODE_BUCK;
                str->output.boost_duty = 1.0f;
                str->output.buck_duty = (str->input.v_l + str->input.v_out) / str->input.v_in;
            }
        }
        break;

    default:
        str->inter.mode = BUCK_BOOST_MODE_BUCK;
        str->output.buck_duty = 0.0f;
        str->output.boost_duty = 0.0f;
        break;
    }

    (str->inter.mode == BUCK_BOOST_MODE_BUCK_BOOST)
        ? (str->output.is_half_freq = 1)
        : (str->output.is_half_freq = 0);

    UP_DN_LMT(str->output.buck_duty, 1.0f, 0.0f);
    UP_DN_LMT(str->output.boost_duty, 1.0f, 0.0f);

    if (str->inter.mode == BUCK_BOOST_MODE_BUCK)
    {
        if ((str->output.buck_duty < 0.05f)||(str->input.v_out < 2.5f))
        {
            str->output.buck_dn_en = 0;
        }
    }
    else if (str->inter.mode == BUCK_BOOST_MODE_BOOST)
    {
        if ((str->output.boost_duty > 0.95f)||(str->input.v_out < 2.5f))
        {
            str->output.boost_up_en = 0;
        }
    }
}
