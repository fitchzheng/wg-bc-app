#ifndef __BSP_STOP_H__
#define __BSP_STOP_H__

#include "hc32_ll_pwc.h"
#include "hc32_ll_tmr0.h"
#include "ev_hc32f334_lqfp64.h"

/* TMR0 unit and channel definition */
#define TMR0_UNIT                       (CM_TMR0_1)
#define TMR0_CLK                        (FCG2_PERIPH_TMR0_1)
#define TMR0_CH                         (TMR0_CH_A)
#define TMR0_TRIG_CH                    (AOS_TMR0)
#define TMR0_CH_INT                     (TMR0_INT_CMP_A)
#define TMR0_CH_FLAG                    (TMR0_FLAG_CMP_A)
#define TMR0_INT_SRC                    (INT_SRC_TMR0_1_CMP_A)
#define TMR0_IRQn                       (INT015_IRQn)
#define TMR0_CMP_VALUE                  640//320//(32000 / 4U / 2U - 1U)

int32_t STOP_IsReady(void);
void TMR0_Config(void);
void STOP_Config(void);

#endif

