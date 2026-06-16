#include "bsp_timer.h"
#include "hc32_ll_tmr0.h"
#include "hc32_ll_clk.h"
#include "bsp_pwm.h"

//void (*timer_irq)(void) = NULL;

//extern void timer_irq_callback(void);

//#define Timer_Freq_Hz                   (25000U)  //中断频率25K

////#define TMR0_CMP_VALUE                  (XTAL32_VALUE / 4U / 2U - 1U)

//uint32_t speed_test = 0;

//void TMR0_CompareIrqCallback(void)
//{
//   speed_test++;
//   TMR0_ClearStatus(TMR0_UNIT, TMR0_CH_FLAG);
//}


//uint32_t get_timer_compare_value(void)
//{
//    stc_clock_freq_t pstcClockFreq;
//    uint32_t u32Tmr0CmpValue;
//    CLK_GetClockFreq(&pstcClockFreq);
//    u32Tmr0CmpValue = pstcClockFreq.u32Pclk1Freq / 4 / Timer_Freq_Hz - 1U;  //
//    return  u32Tmr0CmpValue;

//}

//void TMR0_Config(void)
//{
//    stc_tmr0_init_t stcTmr0Init;
////    stc_irq_signin_config_t stcIrqSignConfig;
//    uint32_t TMR0_CMP_VALUE = get_timer_compare_value();

//    /* Enable timer0 and AOS clock */
//    FCG_Fcg2PeriphClockCmd(TMR0_CLK, ENABLE);
//    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);

//    /* TIMER0 configuration */
//    (void)TMR0_StructInit(&stcTmr0Init);
//    stcTmr0Init.u32ClockSrc     = TMR0_CLK_SRC_INTERN_CLK;
//    stcTmr0Init.u32ClockDiv     = TMR0_CLK_DIV4;
//    stcTmr0Init.u32Func         = TMR0_FUNC_CMP;
//    stcTmr0Init.u16CompareValue = (uint16_t)TMR0_CMP_VALUE;
//    (void)TMR0_Init(TMR0_UNIT, TMR0_CH, &stcTmr0Init);
//    TMR0_IntCmd(TMR0_UNIT, TMR0_CH_INT, ENABLE);

//    TMR0_Start(TMR0_UNIT, TMR0_CH);
//}
// 

//extern void gpio_set_re(uint8_t val);

////中断处理函数
//void timer_irq_handle(void)
//{
//    gpio_set_re(1);
//    //在此处添加中断处理代码，或者在中断回调中处理相关任务
//    if( timer_irq != NULL)
//    {
//        timer_irq();
//    }
//    TMR0_ClearStatus(TMR0_UNIT, TMR0_CH_FLAG);
//    gpio_set_re(0);
//}

////注册中断回调函数
//void bsp_timer_irq_register(void(*func)(void))
//{
//    timer_irq = func;
//}





//void bsp_interrupt_init(void)
//{
//    stc_irq_signin_config_t stcIrqSignConfig;

//        /* Interrupt configuration */
//    stcIrqSignConfig.enIntSrc    = TMR0_INT_SRC;
//    stcIrqSignConfig.enIRQn      = TMR0_IRQn;
//    stcIrqSignConfig.pfnCallback = &timer_irq_handle;////如果不需要用回调形式，可以直接修改为   stcIrq.pfnCallback = &timer_irq_callback;
//    (void)INTC_IrqSignIn(&stcIrqSignConfig);
//    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
//    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
//    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);
//    
////    stcIrq.enIntSrc    = TIMER_INT_SRC;
////    stcIrq.enIRQn      = TIMER_INT_IRQn;
////    stcIrq.pfnCallback = &timer_irq_handle;  //如果不需要用回调形式，可以直接修改为   stcIrq.pfnCallback = &timer_irq_callback;
////    (void)INTC_IrqSignIn(&stcIrq);
////    NVIC_ClearPendingIRQ(stcIrq.enIRQn);
////    NVIC_SetPriority(stcIrq.enIRQn, TIMER_INT_PRIORITY);
////    NVIC_EnableIRQ(stcIrq.enIRQn);
//}
