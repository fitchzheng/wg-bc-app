#include "gd32f30x.h"
#include "section.h"
#include "systick.h"
#include "bsp_pwm.h"

int main(void)
{
    systick_config();
    section_init();
    while (1)
    {
        run_task();
    }
}

void TIMER1_IRQHandler(void)
{
    if (RESET != timer_interrupt_flag_get(TIMER1, TIMER_INT_UP))
    {
        bsp_pwm_update_chclt2();
        volatile uint32_t err = 100;
        while (dma_flag_get(DMA0, DMA_CH0, DMA_FLAG_FTF) == 0)
        {
            err--;
            if (err == 0)
            {
                break;
            }
        }
        section_interrupt();
        dma_flag_clear(DMA0, DMA_CH0, DMA_FLAG_G);
        timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
    }
}
