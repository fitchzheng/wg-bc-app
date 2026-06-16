#include "bsp_pwm.h"
#include "plecs.h"
#include "my_math.h"

void bsp_pwm_set_a_duty(bsp_pwm_t *str)
{
    UP_DN_LMT(str->left_duty, 1.0f, 0.0f);
    UP_DN_LMT(str->right_duty, 1.0f, 0.0f);
    plecs_set_output(PLECS_OUTPUT_PWMA1_DUTY, str->left_duty);
    plecs_set_output(PLECS_OUTPUT_PWMA2_DUTY, str->right_duty);
    plecs_set_output(PLECS_OUTPUT_PWMHA1_EN, str->left_up_en);
    plecs_set_output(PLECS_OUTPUT_PWMHA2_EN, str->right_up_en);
    plecs_set_output(PLECS_OUTPUT_PWMLA1_EN, str->left_dn_en);
    plecs_set_output(PLECS_OUTPUT_PWMLA2_EN, str->right_dn_en);
}

void bsp_pwm_set_b_duty(bsp_pwm_t *str)
{
    UP_DN_LMT(str->left_duty, 1.0f, 0.0f);
    UP_DN_LMT(str->right_duty, 1.0f, 0.0f);
    plecs_set_output(PLECS_OUTPUT_PWMB1_DUTY, str->left_duty);
    plecs_set_output(PLECS_OUTPUT_PWMB2_DUTY, str->right_duty);
    plecs_set_output(PLECS_OUTPUT_PWMHB1_EN, str->left_up_en);
    plecs_set_output(PLECS_OUTPUT_PWMHB2_EN, str->right_up_en);
    plecs_set_output(PLECS_OUTPUT_PWMLB1_EN, str->left_dn_en);
    plecs_set_output(PLECS_OUTPUT_PWMLB2_EN, str->right_dn_en);
}
