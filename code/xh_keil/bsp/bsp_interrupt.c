#include "bsp_interrupt.h"
#include "bsp_pwm.h"
#include "bsp_gpio.h"
#include "buck_boost.h"
#include "buck_boost.h"
void (*timer_irq)(void) = NULL;

#if (APP_ADC_SAMPLE_GPIO_PROBE_FEATURES == 1)
static void adc_sample_debug_pulse(bsp_gpio_table_e pin)
{
    bsp_gpio_set_bit(pin, 1U);
    bsp_gpio_set_bit(pin, 0U);
}

void RAMFUNC HRPWM_1_SCmp_Handler(void)
{
    if ((CM_HRPWM1->STFLR1 & HRPWM_STFLR1_CMSAUF) != 0U)
    {
        CM_HRPWM1->STFLR1 &= ~HRPWM_STFLR1_CMSAUF;
        adc_sample_debug_pulse(PIN_ADC_SAMPLE_A);
    }
    if ((CM_HRPWM1->STFLR1 & HRPWM_STFLR1_CMSBUF) != 0U)
    {
        CM_HRPWM1->STFLR1 &= ~HRPWM_STFLR1_CMSBUF;
        adc_sample_debug_pulse(PIN_ADC_SAMPLE_B);
    }
}
#endif
// 中断处理函数
void RAMFUNC HRPWM_1_Ovf_Udf_Handler(void)
{
    bsp_gpio_set_bit(PIN_INTC25K, 1);
    // 在此处添加中断处理代码，或者在中断回调中处理相关任�?
    if (CM_HRPWM1->STFLR1 & (1 << 7))
    {
        if (timer_irq != NULL)
        {
            timer_irq();
        }
        CM_HRPWM1->STFLR1 &= ~(1 << 7);
    }
    bsp_gpio_set_bit(PIN_INTC25K, 0);

    bCM_HRPWM_COMMON->GBCONR_b.OSTBTRU1 = 1; /* 1：单�?单次缓存触发 */
}

// 注册中断回调函数
void bsp_timer_irq_register(void (*func)(void))
{
    timer_irq = func;
}

void bsp_interrupt_init(void)
{
}

void timer_irq_dehandle(void)
{
}

void bsp_interrupt_deinit(void)
{
}
