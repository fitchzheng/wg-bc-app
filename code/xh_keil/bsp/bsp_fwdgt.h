#ifndef __BSP_SWDT_H__
#define __BSP_SWDT_H__

#include "hc32_ll.h"
#include "hc32_ll_swdt.h"

void SWDT_Config(void);

void bsp_fwdgt_init(void);

void bsp_fwdgt_feed(void);

#endif
