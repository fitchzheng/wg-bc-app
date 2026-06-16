#include "bsp_adc.h"
#include "my_math.h"

float bsp_adc_get_ila(void)
{
    float temp = BSP_ADC_ILA;
    UP_DN_LMT(temp, 132.0f, -132.0f);
    return temp;
}

float bsp_adc_get_ilb(void)
{
    float temp = BSP_ADC_ILB;
    UP_DN_LMT(temp, 132.0f, -132.0f);
    return temp;
}

float bsp_adc_get_ihv(void)
{
    float temp = BSP_ADC_IHV;
    UP_DN_LMT(temp, 132.0f, -132.0f);
    return temp;
}

float bsp_adc_get_ilv(void)
{
    float temp = BSP_ADC_ILV;
    UP_DN_LMT(temp, 132.0f, -132.0f);
    return temp;
}

float bsp_adc_get_fvs48(void)
{
    float temp = BSP_ADC_FVS48;
    UP_DN_LMT(temp, 66.0f, 0.0f);
    return temp;
}

float bsp_adc_get_rvs12(void)
{
    float temp = BSP_ADC_RVS12;
    UP_DN_LMT(temp, 66.0f, 0.0f);
    return temp;
}

float bsp_adc_get_accvs(void)
{
    float temp = BSP_ADC_ACCVS;
    UP_DN_LMT(temp, 66.0f, 0.0f);
    return temp;
}

float bsp_adc_get_addrs(void)
{
    float temp = BSP_ADC_ADDRS;
    UP_DN_LMT(temp, 3.3f, 0.0f);
    return temp;
}

float bsp_adc_get_temp1(void)
{
    float temp = BSP_ADC_TEMP1;
    return temp;
}

float bsp_adc_get_temp2(void)
{
    float temp = BSP_ADC_TEMP2;
    return temp;
}

float bsp_adc_get_temp3(void)
{
    float temp = BSP_ADC_TEMP3;
    return temp;
}
