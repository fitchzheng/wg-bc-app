#include "bsp_pwm.h"
#include "my_math.h"
#include "gd32f30x.h"
#include "bsp_gpio.h"

uint32_t timer_pwma_chctl2;
uint32_t timer_pwmb_chctl2;

void bsp_pwm_init(void)
{
    rcu_periph_clock_enable(RCU_TIMER0);
    rcu_periph_clock_enable(RCU_TIMER1);
    rcu_periph_clock_enable(RCU_TIMER7);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_7);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_8);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_9);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_0);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_13);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_14);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_6);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_7);
    timer_deinit(TIMER_PWMA);
    timer_deinit(TIMER_PWMB);

    TIMER_CTL0(TIMER_PWMA) = 0x000000C0;
    TIMER_CTL1(TIMER_PWMA) = 0x00000020;
    TIMER_CHCTL0(TIMER_PWMA) = 0x00007868;
    TIMER_CHCTL2(TIMER_PWMA) = 0x00000000;
    TIMER_CAR(TIMER_PWMA) = CTRL_PERIOD;
    TIMER_CREP(TIMER_PWMA) = 7;
    TIMER_CH0CV(TIMER_PWMA) = 0;
    TIMER_CH1CV(TIMER_PWMA) = CTRL_PERIOD + 1;
    TIMER_CCHP(TIMER_PWMA) = 0x0000880C;
    TIMER_CTL0(TIMER_PWMA) |= 1;

    TIMER_CTL0(TIMER_PWMB) = 0x000000C0;
    TIMER_SMCFG(TIMER_PWMB) = 0x00000084;
    TIMER_CHCTL0(TIMER_PWMB) = 0x00006878;
    TIMER_CHCTL2(TIMER_PWMB) = 0x00000000;
    TIMER_CAR(TIMER_PWMB) = CTRL_PERIOD;
    TIMER_CREP(TIMER_PWMB) = 7;
    TIMER_CH0CV(TIMER_PWMB) = CTRL_PERIOD + 1;
    TIMER_CH1CV(TIMER_PWMB) = 0;
    TIMER_CCHP(TIMER_PWMB) = 0x0000880C;
    TIMER_CTL0(TIMER_PWMB) |= 1;

    TIMER_CTL0(TIMER1) = 0x00000080;
    TIMER_SMCFG(TIMER1) = 0x00000084;
    TIMER_CHCTL0(TIMER1) = 0x00006800;
    TIMER_CHCTL2(TIMER1) = 0x00000010;
    TIMER_CAR(TIMER1) = (CTRL_PERIOD + 1) * 8 - 1;
    TIMER_CH1CV(TIMER1) = 10;
    TIMER_CTL0(TIMER1) |= 1;
    TIMER_DMAINTEN(TIMER1) |= 1;
    
    timer_pwma_chctl2 = TIMER_CHCTL2(TIMER_PWMA);
    timer_pwmb_chctl2 = TIMER_CHCTL2(TIMER_PWMB);
    nvic_irq_enable(TIMER1_IRQn, 1, 0);
}

void bsp_pwm_set_a_duty(bsp_pwm_t *str)
{
    UP_DN_LMT(str->left_duty, 1.0f, 0.0f);
    UP_DN_LMT(str->right_duty, 1.0f, 0.0f);
    timer_pwma_chctl2 = TIMER_CHCTL2(TIMER_PWMA);
    uint32_t timer_pwma_ch0cv = 0;
    uint32_t timer_pwma_ch1cv = 0;
    uint32_t period = TIMER_CAR(TIMER_PWMA) + 1;

    if ((str->left_up_en == 1) &&
        (str->left_dn_en == 1))
    {
        timer_pwma_chctl2 |= TIMER_CHCTL2_CH0EN;
        timer_pwma_chctl2 |= TIMER_CHCTL2_CH0NEN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0P;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0NP;
        timer_pwma_ch0cv = (uint32_t)(period * str->left_duty);
    }
    else if (str->left_up_en == 1)
    {
        timer_pwma_chctl2 |= TIMER_CHCTL2_CH0EN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0NEN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0P;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0NP;
        timer_pwma_ch0cv = (uint32_t)(period * str->left_duty);
    }
    else if (str->left_dn_en == 1)
    {
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0EN;
        timer_pwma_chctl2 |= TIMER_CHCTL2_CH0NEN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0P;
        timer_pwma_chctl2 |= TIMER_CHCTL2_CH0NP;
        timer_pwma_ch0cv = (uint32_t)(period * str->left_duty);
    }
    else
    {
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0EN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0NEN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0P;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH0NP;
        timer_pwma_ch0cv = 0;
    }

    if ((str->right_up_en == 1) &&
        (str->right_dn_en == 1))
    {
        timer_pwma_chctl2 |= TIMER_CHCTL2_CH1EN;
        timer_pwma_chctl2 |= TIMER_CHCTL2_CH1NEN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1P;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1NP;
        timer_pwma_ch1cv = (uint32_t)(period * (1.0f - str->right_duty));
    }
    else if (str->right_up_en == 1)
    {
        timer_pwma_chctl2 |= TIMER_CHCTL2_CH1EN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1NEN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1P;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1NP;
        timer_pwma_ch1cv = (uint32_t)(period * (1.0f - str->right_duty));
    }
    else if (str->right_dn_en == 1)
    {
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1EN;
        timer_pwma_chctl2 |= TIMER_CHCTL2_CH1NEN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1P;
        timer_pwma_chctl2 |= TIMER_CHCTL2_CH1NP;
        timer_pwma_ch1cv = (uint32_t)(period * (1.0f - str->right_duty));
    }
    else
    {
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1EN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1NEN;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1P;
        timer_pwma_chctl2 &= ~TIMER_CHCTL2_CH1NP;
        timer_pwma_ch1cv = period;
    }
    TIMER_CH0CV(TIMER_PWMA) = timer_pwma_ch0cv;
    TIMER_CH1CV(TIMER_PWMA) = timer_pwma_ch1cv;
}

void bsp_pwm_set_b_duty(bsp_pwm_t *str)
{
    UP_DN_LMT(str->left_duty, 1.0f, 0.0f);
    UP_DN_LMT(str->right_duty, 1.0f, 0.0f);
    timer_pwmb_chctl2 = TIMER_CHCTL2(TIMER_PWMB);
    uint32_t timer_pwmb_ch0cv = 0;
    uint32_t timer_pwmb_ch1cv = 0;
    uint32_t period = TIMER_CAR(TIMER_PWMB) + 1;
    
    if ((str->left_up_en == 1) &&
        (str->left_dn_en == 1))
    {
        timer_pwmb_chctl2 |= TIMER_CHCTL2_CH0EN;
        timer_pwmb_chctl2 |= TIMER_CHCTL2_CH0NEN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0P;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0NP;
        timer_pwmb_ch0cv = (uint32_t)(period * (1.0f - str->left_duty));
    }
    else if (str->left_up_en == 1)
    {
        timer_pwmb_chctl2 |= TIMER_CHCTL2_CH0EN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0NEN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0P;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0NP;
        timer_pwmb_ch0cv = (uint32_t)(period * (1.0f - str->left_duty));
    }
    else if (str->left_dn_en == 1)
    {
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0EN;
        timer_pwmb_chctl2 |= TIMER_CHCTL2_CH0NEN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0P;
        timer_pwmb_chctl2 |= TIMER_CHCTL2_CH0NP;
        timer_pwmb_ch0cv = (uint32_t)(period * (1.0f - str->left_duty));
    }
    else
    {
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0EN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0NEN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0P;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH0NP;
        timer_pwmb_ch0cv = period;
    }

    if ((str->right_up_en == 1) &&
        (str->right_dn_en == 1))
    {
        timer_pwmb_chctl2 |= TIMER_CHCTL2_CH1EN;
        timer_pwmb_chctl2 |= TIMER_CHCTL2_CH1NEN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1P;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1NP;
        timer_pwmb_ch1cv = (uint32_t)(period * (1.0f - (1.0f - str->right_duty)));
    }
    else if (str->right_up_en == 1)
    {
        timer_pwmb_chctl2 |= TIMER_CHCTL2_CH1EN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1NEN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1P;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1NP;
        timer_pwmb_ch1cv = (uint32_t)(period * (1.0f - (1.0f - str->right_duty)));
    }
    else if (str->right_dn_en == 1)
    {
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1EN;
        timer_pwmb_chctl2 |= TIMER_CHCTL2_CH1NEN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1P;
        timer_pwmb_chctl2 |= TIMER_CHCTL2_CH1NP;
        timer_pwmb_ch1cv = (uint32_t)(period * (1.0f - (1.0f - str->right_duty)));
    }
    else
    {
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1EN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1NEN;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1P;
        timer_pwmb_chctl2 &= ~TIMER_CHCTL2_CH1NP;
        timer_pwmb_ch1cv = 0;
    }
    TIMER_CH0CV(TIMER_PWMB) = timer_pwmb_ch0cv;
    TIMER_CH1CV(TIMER_PWMB) = timer_pwmb_ch1cv;
}

void bsp_pwm_update_chclt2(void)
{
    TIMER_CHCTL2(TIMER_PWMA) = timer_pwma_chctl2;
    TIMER_CHCTL2(TIMER_PWMB) = timer_pwmb_chctl2;
}

void bsp_pwm_change_full_freq_pwma(void)
{
    TIMER_CREP(TIMER_PWMA) = 7;
    TIMER_CAR(TIMER_PWMA) = CTRL_PERIOD;
}

void bsp_pwm_change_full_freq_pwmb(void)
{
    TIMER_CREP(TIMER_PWMB) = 7;
    TIMER_CAR(TIMER_PWMB) = CTRL_PERIOD;
}

void bsp_pwm_change_half_freq_pwma(void)
{
    TIMER_CREP(TIMER_PWMA) = 3;
    TIMER_CAR(TIMER_PWMA) = ((CTRL_PERIOD + 1) << 1) - 1;
}

void bsp_pwm_change_half_freq_pwmb(void)
{
    TIMER_CREP(TIMER_PWMB) = 3;
    TIMER_CAR(TIMER_PWMB) = ((CTRL_PERIOD + 1) << 1) - 1;
}