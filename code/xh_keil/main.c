#include "hc32_ll.h"
#include "section.h"
#include "ev_hc32f334_lqfp64.h"
#include "bsp_pwm.h"
#include "bsp_interrupt.h"

#define LL_PERIPH_SEL (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | \
                       LL_PERIPH_EFM  | LL_PERIPH_SRAM)

extern void open_loop_test(void);
extern uint8_t bsp_pwc_stop_rum(void);
extern uint8_t pwc_stop_flag;
int main(void)
{
    LL_PERIPH_WE(LL_PERIPH_SEL);
    BSP_CLK_Init();
    bsp_timer_irq_register(section_interrupt); // 注册中断回调
    SysTick_Init(1000);
    section_init();
    while (1)
    {
        if(pwc_stop_flag == 0)
        {
            run_task();
        }
        else
        {
            if(bsp_pwc_stop_rum() == 0)
            {
                pwc_stop_flag = 0;
            }
        }
        
    }
}




