#ifndef __BSP_DMA_H
#define __BSP_DMA_H

#include "gd32f30x.h"

#define USART2_RX_DMA_CNT dma_transfer_number_get(DMA0, DMA_CH2)
#define USART0_RX_DMA_CNT dma_transfer_number_get(DMA0, DMA_CH4)


#define USARTX_RX_DMA_CNT(ch) dma_transfer_number_get(DMA0, ch)

void bsp_dma_init(void);

#endif
