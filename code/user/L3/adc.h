#ifndef __ADC_H
#define __ADC_H

#include "bsp_adc.h"

#define ADC_ILA bsp_adc_get_ila()
#define ADC_ILB bsp_adc_get_ilb()
#define ADC_IHV (bsp_adc_get_ihv() * ihv_k)
#define ADC_ILV (bsp_adc_get_ilv() * ilv_k)
#define ADC_FVS48 (bsp_adc_get_fvs48() * fvs48_k)
#define ADC_RVS12 (bsp_adc_get_rvs12() * rvs12_k)

#define SHOW_IHV   adc_get_ihv_show()//(ADC_IHV   * ihv_curr_show_k)
#define SHOW_ILV   adc_get_ilv_show()//(ADC_ILV   * ilv_curr_show_k)
#define SHOW_FVS48 adc_get_fvs48_show()//(ADC_FVS48 * fvs48_show_k)
#define SHOW_RVS12 adc_get_rvs12_show()//(ADC_RVS12 * rvs12_show_k)

#define ADC_RMTVS (bsp_adc_get_rmtvs() * Rmt_k)
#define ADC_ACCVS (bsp_adc_get_accvs() * Acc_k)
#define ADC_ADDRS bsp_adc_get_addrs()
#define ADC_TEMP1 bsp_adc_get_temp1()
#define ADC_TEMP2 bsp_adc_get_temp2()
#define ADC_TEMP3 bsp_adc_get_temp3()
#define ADC_VDD_8V bsp_adc_get_vdd8v()

#define ADC_CALI_K_UP_LMT 1.1f
#define ADC_CALI_K_DN_LMT 0.9f

#define ADC_CALI_K_LMT(val) val =                             \
                                ((val > ADC_CALI_K_UP_LMT) || \
                                 (val < ADC_CALI_K_DN_LMT))   \
                                    ? 1.0f                    \
                                    : val

#define ADC_CALI_CURR_K_UP_LMT 1.3f
#define ADC_CALI_CURR_K_DN_LMT 0.7f

#define ADC_CALI_CURR_K_LMT(val) val =                             \
                                ((val > ADC_CALI_CURR_K_UP_LMT) || \
                                 (val < ADC_CALI_CURR_K_DN_LMT))   \
                                    ? 1.0f                    \
                                    : val

float get_show_ihv_show(void);

float get_show_ilv_show(void);

float get_show_fvs48_show(void);

float get_show_rvs12_show(void);

float adc_get_ila(void);

float adc_get_ilb(void);

float adc_get_ihv(void);

float adc_get_ilv(void);

float adc_get_ila_no_bias(void);

float adc_get_ilb_no_bias(void);

float adc_get_ihv_no_bias(void);

float adc_get_ilv_no_bias(void);

float adc_get_fvs48(void);

float adc_get_rvs12(void);

float adc_get_rmtvs(void);
    
float adc_get_accvs(void);

float adc_get_addrs(void);

float adc_get_temp1(void);

float adc_get_temp2(void);

float adc_get_temp3(void);

void adc_set_ihv_bias(float bias);

void adc_set_ilv_bias(float bias);

void adc_set_ila_bias(float bias);

void adc_set_ilb_bias(float bias);

float adc_get_vdd_8v(void);

float get_fvs48_show_k(void);

float get_acc_show_k(void);
#endif
