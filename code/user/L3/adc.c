#include "adc.h"
#include "stdint.h"
#include "shell.h"
#include "wg_com_v2.h"
#include "section.h"
#include "adc_check.h"

float ihv_bias = 0.0f;
float ilv_bias = 0.0f;
float ila_bias = 0.0f;
float ilb_bias = 0.0f;

float fvs48_k = 0.0f;
float rvs12_k = 0.0f;
static float fvs48_show_k = 0.0f;
static float rvs12_show_k = 0.0f;
float ihv_k = 0.0f;
float ilv_k = 0.0f;

static float Acc_k = 0.0f;
static float Rmt_k = 0.0f;
static float ihv_curr_show_k = 0.0f;
static float ilv_curr_show_k = 0.0f;

extern void adc_run(void);
void adc_init(void)
{
    adc_run();
}

REG_INIT(adc_init)
void adc_run(void)
{
    float rvs12_k_Value = 0.0f;
    float fvs48_k_Value = 0.0f;
    float rvs12_show_k_Value = 0.0f;
    float fvs48_show_k_Value = 0.0f;
    float ihv_k_Value = 0.0f;
    float ilv_k_Value = 0.0f;
    float Acc_k_Value = 0.0f;
    float Rmt_k_Value = 0.0f;

    float ihv_set_curr = 0;
    float ilv_set_curr = 0;
    float ihv_show_curr = 0;
    float ilv_show_curr = 0;
    
    
    WG_COM_V2_GET_DATA_UINT(rvs12_k_Value, wg_com_v2_param.OutVoltCalibrK);
    WG_COM_V2_GET_DATA_UINT(fvs48_k_Value, wg_com_v2_param.InpVoltCalibrK);
    WG_COM_V2_GET_DATA_UINT(rvs12_show_k_Value, wg_com_v2_param.OutShowVoltCalibrK);
    WG_COM_V2_GET_DATA_UINT(fvs48_show_k_Value, wg_com_v2_param.InpShowVoltCalibrK);
    WG_COM_V2_GET_DATA_UINT(Acc_k_Value, wg_com_v2_param.VoltCompensationAK);
    WG_COM_V2_GET_DATA_UINT(Rmt_k_Value, wg_com_v2_param.VoltCompensationBK); 
    WG_COM_V2_GET_DATA_UINT(ilv_k_Value, wg_com_v2_param.OutCurrCalibrK);
    WG_COM_V2_GET_DATA_UINT(ihv_k_Value, wg_com_v2_param.InpCurrCalibrK);

    WG_COM_V2_GET_DATA_UINT(ihv_set_curr, wg_com_v2_param.InpShowCurrCalibrK);
    WG_COM_V2_GET_DATA_UINT(ilv_show_curr, wg_com_v2_param.BOutShowCurrCalibrK);
    WG_COM_V2_GET_DATA_UINT(ilv_set_curr, wg_com_v2_param.OutShowCurrCalibrK);
    WG_COM_V2_GET_DATA_UINT(ihv_show_curr, wg_com_v2_param.AOutShowCurrCalibrK);

    rvs12_show_k = rvs12_show_k_Value;
    fvs48_show_k = fvs48_show_k_Value;
    Acc_k = Acc_k_Value;
    Rmt_k = Rmt_k_Value;

    if(ADDRS_FORWARD == adc_check_get_addrs_state()){
        rvs12_k = 1.0f/rvs12_k_Value;
        fvs48_k = fvs48_show_k_Value;
        ilv_curr_show_k = ilv_show_curr;
        ihv_curr_show_k = ihv_set_curr;
        ilv_k = 1.0f/ilv_k_Value;
        ihv_k = ihv_set_curr;
    }else{
        rvs12_k = rvs12_show_k_Value;
        fvs48_k = 1.0f/fvs48_k_Value;
        ihv_curr_show_k = ihv_show_curr;
        ilv_curr_show_k = ilv_set_curr;
        ihv_k = 1.0f/ihv_k_Value;
        ilv_k = ilv_set_curr;
    }

    ADC_CALI_K_LMT(rvs12_k);
    ADC_CALI_K_LMT(fvs48_k);
    ADC_CALI_K_LMT(rvs12_show_k);
    ADC_CALI_K_LMT(fvs48_show_k);
    ADC_CALI_K_LMT(Acc_k);
    ADC_CALI_K_LMT(Rmt_k);
    ADC_CALI_CURR_K_LMT(ihv_k);
    ADC_CALI_CURR_K_LMT(ilv_k);
    ADC_CALI_CURR_K_LMT(ilv_curr_show_k);
    ADC_CALI_CURR_K_LMT(ihv_curr_show_k);
}
REG_TASK(500, adc_run);

float get_fvs48_show_k(void)
{
    return (fvs48_k * fvs48_show_k);
}

float get_acc_show_k(void)
{
    return Acc_k;
}

float RAMFUNC get_show_ihv_show(void)
{
    return ((bsp_adc_get_ihv() * ihv_curr_show_k) - ihv_bias);
}

float RAMFUNC get_show_ilv_show(void)
{
    return ((bsp_adc_get_ilv() * ilv_curr_show_k) - ilv_bias);
}

float RAMFUNC get_show_fvs48_show(void)
{
    return bsp_adc_get_fvs48() * fvs48_show_k;
}

float RAMFUNC get_show_rvs12_show(void)
{
    return bsp_adc_get_rvs12() * rvs12_show_k;
}

float RAMFUNC adc_get_ila(void)
{
    return ADC_ILA - ila_bias;
}

float RAMFUNC adc_get_ilb(void)
{
    return ADC_ILB - ilb_bias;
}

float RAMFUNC adc_get_ihv(void)
{
    return ADC_IHV - ihv_bias;
}

float RAMFUNC adc_get_ilv(void)
{
    return ADC_ILV - ilv_bias;
}

float RAMFUNC adc_get_ila_no_bias(void)
{
    return ADC_ILA;
}

float RAMFUNC adc_get_ilb_no_bias(void)
{
    return ADC_ILB;
}

float RAMFUNC adc_get_ihv_no_bias(void)
{
    return ADC_IHV;
}

float RAMFUNC adc_get_ilv_no_bias(void)
{
    return ADC_ILV;
}

float RAMFUNC adc_get_fvs48(void)
{
    return ADC_FVS48;
}

float RAMFUNC adc_get_rvs12(void)
{
    return ADC_RVS12;
}

float RAMFUNC get_rvs12(void)
{
    return rvs12_k;
}

float adc_get_vdd_8v(void)
{
    return ADC_VDD_8V;
}

float adc_get_rmtvs(void)
{
    return ADC_RMTVS;
}

float RAMFUNC adc_get_accvs(void)
{
    return ADC_ACCVS;
}

float adc_get_temp1(void)
{
    return ADC_TEMP1;
}

float adc_get_temp2(void)
{
    return ADC_TEMP2;
}

float adc_get_temp3(void)
{
    return ADC_TEMP3;
}

void RAMFUNC adc_set_ihv_bias(float bias)
{
    ihv_bias = bias;
}

void RAMFUNC adc_set_ilv_bias(float bias)
{
    ilv_bias = bias;
}

void RAMFUNC adc_set_ila_bias(float bias)
{
    ila_bias = bias;
}

void RAMFUNC adc_set_ilb_bias(float bias)
{
    ilb_bias = bias;
}
