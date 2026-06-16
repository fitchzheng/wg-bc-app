#ifndef __BSP_INTERRUPT_H__
#define __BSP_INTERRUPT_H__

#include "hc32_ll.h"

#define TIMER_INT_PRIORITY    DDL_IRQ_PRIO_00
#define TIMER_INT_SRC         INT_SRC_HRPWM_1_SCMP_A
#define TIMER_INT_IRQn        INT001_IRQn

#define USART1_INT_PRIORITY       DDL_IRQ_PRIO_00
#define USART1_INT_SRC            INT_SRC_USART1_RTO
#define USART1_RX_TIMEOUT_IRQn    INT002_IRQn

#define USART2_INT_PRIORITY       DDL_IRQ_PRIO_00
#define USART2_INT_SRC            INT_SRC_USART2_RTO
#define USART2_RX_TIMEOUT_IRQn    INT003_IRQn

void bsp_interrupt_init(void);
void bsp_timer_irq_register(void(*func)(void));
void bsp_interrupt_deinit(void);
#endif

