#ifndef __BPS_ADC_H
#define __BPS_ADC_H

#include "gd32f30x.h"
#include "stdint.h"

#define SAMPLE_TIME ADC_SAMPLETIME_7POINT5

#define BSP_ADC_ILA_GAIN (132.0f * 2.0f / 4095.0f)
#define BSP_ADC_ILA_BIAS (-132.0f)

#define BSP_ADC_ILB_GAIN (132.0f * 2.0f / 4095.0f)
#define BSP_ADC_ILB_BIAS (-132.0f)

#define BSP_ADC_ILV_GAIN ((132.0f * 2.0f * (60.0f / 64.3f) / 4095.0f))
#define BSP_ADC_ILV_BIAS (-132.0f * (60.0f / 64.3f))

#define BSP_ADC_IHV_GAIN ((132.0f * 2.0f * (37.8f / 40.6f) / 4095.0f))
#define BSP_ADC_IHV_BIAS (-132.0f * (37.8f / 40.6f))

#define BSP_ADC_FVS48_GAIN (69.3f / 4095.0f)
#define BSP_ADC_FVS48_BIAS (0.0f)

#define BSP_ADC_RVS12_GAIN (69.3f / 4095.0f)
#define BSP_ADC_RVS12_BIAS (0.0f)

#define BSP_ADC_MCUTEMP_GAIN (69.3f / 4095.0f)
#define BSP_ADC_MCUTEMP_BIAS (0.0f)

#define BSP_ADC_VDD_8V_GAIN (13.2f / 4095.0f)
#define BSP_ADC_VDD_8V_BIAS (0.0f)

#define BSP_ADC_RMTVS_GAIN (69.3f / 4095.0f)
#define BSP_ADC_RMTVS_BIAS (0.0f)

#define BSP_ADC_TEMP1_GAIN (3.3f / 4095.0f)
#define BSP_ADC_TEMP1_BIAS (0.0f)

#define BSP_ADC_TEMP2_GAIN (3.3f / 4095.0f)
#define BSP_ADC_TEMP2_BIAS (0.0f)

#define BSP_ADC_TEMP3_GAIN (3.3f / 4095.0f)
#define BSP_ADC_TEMP3_BIAS (0.0f)

#define BSP_ADC_ACCVS_GAIN (69.3f / 4095.0f)
#define BSP_ADC_ACCVS_BIAS (0.0f)

#define BSP_ADC_ADDRS_GAIN (3.3f / 4095.0f)
#define BSP_ADC_ADDRS_BIAS (0.0f)


typedef struct
{
    float gain;
    float bias;
} bsp_adc_linear_calib_t;

#define BSP_ADC0_1_REG_PARM(_adc_name,                      \
                            _gpio_periph,                   \
                            _pin,                           \
                            ch)                             \
    .adc_name = _adc_name,                                  \
    .gpio_periph = _gpio_periph,                            \
    .pin = GPIO_PIN_##_pin,                                 \
    .adc_periph = ADC0 + ((ADC1 - ADC0) * (_adc_name % 2)), \
    .adc_ch = ADC_CHANNEL_##ch,                             \
    .rank = _adc_name / 2,                                  \
    .sample_time = SAMPLE_TIME,                             \
    .p_adc_value = (uint16_t *)&adc0_1_value[_adc_name]

#define BSP_ADC2_REG_PARM(_adc_name,    \
                          _gpio_periph, \
                          _pin,         \
                          ch)           \
    .adc_name = _adc_name,              \
    .gpio_periph = _gpio_periph,        \
    .pin = GPIO_PIN_##_pin,             \
    .adc_periph = ADC2,                 \
    .adc_ch = ADC_CHANNEL_##ch,         \
    .rank = _adc_name,                  \
    .sample_time = SAMPLE_TIME,         \
    .p_adc_value = (uint16_t *)&adc2_value[_adc_name]

typedef enum
{
    ILA,
    ILB,
    ILV,
    IHV,
    FVS48,
    RVS12,
    VDD8V,
    RMTVS,
    ADC0_1_TABLE_MAX,
} ADC0_1_TABLE_E;

typedef enum
{
    TEMP1,
    TEMP2,
    TEMP3,
    ACCVS,
    ADDRS,
    ADC2_TABLE_MAX,
} ADC2_TABLE_E;

typedef struct
{
    uint32_t adc_name;
    uint32_t gpio_periph;
    uint32_t pin;
    uint32_t adc_periph;
    uint8_t adc_ch;
    uint8_t rank;
    uint32_t sample_time;
    uint16_t *p_adc_value;
} bsp_adc_param_t;

void bsp_adc_init(void);
float bsp_adc0_1_get_value(ADC0_1_TABLE_E name);
float bsp_adc2_get_value(ADC2_TABLE_E name);

float bsp_adc_get_ila(void);

float bsp_adc_get_ilb(void);

float bsp_adc_get_ihv(void);

float bsp_adc_get_ilv(void);

float bsp_adc_get_fvs48(void);

float bsp_adc_get_rvs12(void);

float bsp_adc_get_rmtvs(void);
    
float bsp_adc_get_accvs(void);

float bsp_adc_get_addrs(void);

float bsp_adc_get_temp1(void);

float bsp_adc_get_temp2(void);

float bsp_adc_get_temp3(void);

float bsp_adc_get_vdd_8V(void);
#endif
