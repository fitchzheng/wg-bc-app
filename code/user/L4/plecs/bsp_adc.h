#ifndef __BPS_ADC_H
#define __BPS_ADC_H

#include "plecs.h"

#define BSP_ADC_ILA plecs_get_input(PLECS_INPUT_ILA)
#define BSP_ADC_ILB plecs_get_input(PLECS_INPUT_ILB)
#define BSP_ADC_IHV plecs_get_input(PLECS_INPUT_IHV)
#define BSP_ADC_ILV plecs_get_input(PLECS_INPUT_ILV)
#define BSP_ADC_FVS48 plecs_get_input(PLECS_INPUT_FVS48)
#define BSP_ADC_RVS12 plecs_get_input(PLECS_INPUT_RVS12)
#define BSP_ADC_ACCVS plecs_get_input(PLECS_INPUT_ACCVS)
#define BSP_ADC_ADDRS plecs_get_input(PLECS_INPUT_ADDRS)
#define BSP_ADC_TEMP1 plecs_get_input(PLECS_INPUT_TEMP1)
#define BSP_ADC_TEMP2 plecs_get_input(PLECS_INPUT_TEMP2)
#define BSP_ADC_TEMP3 plecs_get_input(PLECS_INPUT_TEMP3)

float bsp_adc_get_ila(void);

float bsp_adc_get_ilb(void);

float bsp_adc_get_ihv(void);

float bsp_adc_get_ilv(void);

float bsp_adc_get_fvs48(void);

float bsp_adc_get_rvs12(void);

float bsp_adc_get_accvs(void);

float bsp_adc_get_addrs(void);

float bsp_adc_get_temp1(void);

float bsp_adc_get_temp2(void);

float bsp_adc_get_temp3(void);

#endif
