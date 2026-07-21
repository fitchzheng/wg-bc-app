#include "bsp_interrupt.h"
#include "bsp_pwm.h"
#include "bsp_gpio.h"
#include "app_features.h"
#include "buck_boost.h"
#include "buck_boost.h"
void (*timer_irq)(void) = NULL;
static volatile uint8_t adc_sample_a_seen = 0U;

#if ((APP_ADC_SAMPLE_GPIO_PROBE_FEATURES == 1) && (APP_ADC_SAMPLE_EVENT_PORT_PROBE_FEATURES != 1))
static void adc_sample_debug_pulse(bsp_gpio_table_e pin)
{
    bsp_gpio_set_bit(pin, 1U);
    bsp_gpio_set_bit(pin, 0U);
}
#endif

// 中断处理函数
void RAMFUNC HRPWM_1_SCmp_Handler(void)
{
    bsp_gpio_set_bit(PIN_INTC25K, 1);
    // 在此处添加中断处理代码，或者在中断回调中处理相关任�?
    if ((CM_HRPWM1->STFLR1 & HRPWM_STFLR1_CMSAUF) != 0U)
    {
        CM_HRPWM1->STFLR1 &= ~HRPWM_STFLR1_CMSAUF;
#if ((APP_ADC_SAMPLE_GPIO_PROBE_FEATURES == 1) && (APP_ADC_SAMPLE_EVENT_PORT_PROBE_FEATURES != 1))
        adc_sample_debug_pulse(PIN_ADC_SAMPLE_A);
#endif
        adc_sample_a_seen = 1U;
    }
    if ((CM_HRPWM1->STFLR1 & HRPWM_STFLR1_CMSBUF) != 0U)
    {
        CM_HRPWM1->STFLR1 &= ~HRPWM_STFLR1_CMSBUF;
#if ((APP_ADC_SAMPLE_GPIO_PROBE_FEATURES == 1) && (APP_ADC_SAMPLE_EVENT_PORT_PROBE_FEATURES != 1))
        adc_sample_debug_pulse(PIN_ADC_SAMPLE_B);
#endif
    }
    bsp_gpio_set_bit(PIN_INTC25K, 0);

}

void RAMFUNC HRPWM_1_Ovf_Udf_Handler(void)
{
    if ((CM_HRPWM1->STFLR1 & HRPWM_STFLR1_OVFF) != 0U)
    {
        CM_HRPWM1->STFLR1 &= ~HRPWM_STFLR1_OVFF;
    }
    if ((adc_sample_a_seen != 0U) && (timer_irq != NULL))
    {
        adc_sample_a_seen = 0U;
        timer_irq();
    }
    if ((CM_HRPWM1->STFLR1 & HRPWM_STFLR1_UDFF) != 0U)
    {
        CM_HRPWM1->STFLR1 &= ~HRPWM_STFLR1_UDFF;
    }
    bCM_HRPWM_COMMON->GBCONR_b.OSTBTRU1 = 1;
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
