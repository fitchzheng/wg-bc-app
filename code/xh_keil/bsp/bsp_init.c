#include "section.h"
#include "bsp_init.h"
#include "bsp_adc.h"
#include "bsp_pwm.h"
#include "bsp_dma.h"
#include "bsp_gpio.h"
#include "bsp_usart.h"
#include "bsp_can.h"
#include "bsp_interrupt.h"
#include "bsp_iic.h"
#include "bsp_timer.h"
#include "bsp_stop.h"

void bsp_init(void)
{
    bsp_gpio_init();
    bsp_adc_init();
    bsp_pwm_init();
    bsp_usart_init();
    bsp_dma_init();
    bsp_can_init();
    bsp_interrupt_init();
    bsp_iic_init();
    //bsp_pwc_init();
    //TMR0_Config();
    TMR0_Stop(TMR0_UNIT, TMR0_CH);
} 

REG_INIT_TP(bsp_init);
