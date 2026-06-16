#ifndef __BPS_ADC_H
#define __BPS_ADC_H

#include "hc32_ll.h"
#include "hc32_ll_adc.h"

#define SAMPLE_TIME   16 //采样周期
#define ADC_TRIG_EVT  EVT_SRC_HRPWM_1_SCMP_A  //ADC触发事件

#define BSP_ADC_ILA_GAIN (66.0f * 2.0f / 4095.0f)
#define BSP_ADC_ILA_BIAS (-66.0f)

#define BSP_ADC_ILB_GAIN (66.0f * 2.0f / 4095.0f)
#define BSP_ADC_ILB_BIAS (-66.0f)

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

#define BSP_ADC_TEMP_INT_GAIN (1.0f)
#define BSP_ADC_TEMP_INT_BIAS (0.0f)

#define BSP_ADC_RMTVS_GAIN (71.5f / 4095.0f)
#define BSP_ADC_RMTVS_BIAS (0.0f)

#define BSP_ADC_TEMP1_GAIN (3.3f / 4095.0f)
#define BSP_ADC_TEMP1_BIAS (0.0f)

#define BSP_ADC_TEMP2_GAIN (3.3f / 4095.0f)
#define BSP_ADC_TEMP2_BIAS (0.0f)

#define BSP_ADC_TEMP3_GAIN (3.3f / 4095.0f)
#define BSP_ADC_TEMP3_BIAS (0.0f)

#define BSP_ADC_ACCVS_GAIN (71.5f / 4095.0f)
#define BSP_ADC_ACCVS_BIAS (0.0f)

//#define BSP_ADC_ADDRS_GAIN (3.3f / 4095.0f)
//#define BSP_ADC_ADDRS_BIAS (0.0f)

#define BSP_ADC_VDD_8V_GAIN (13.2f / 4095.0f) // 
#define BSP_ADC_VDD_8V_BIAS (0.0f)

typedef struct
{
    float gain;
    float bias;
} bsp_adc_linear_calib_t;


#define BSP_ADC0_REG_PARM(_adc_name,                      \
                            _gpio_periph,                   \
                            _pin,                           \
                            ch)                             \
    .adc_name = _adc_name,                                  \
    .gpio_periph = _gpio_periph,                            \
    .pin = GPIO_PIN_##_pin,                                 \
    .adc_periph = (uint32_t)CM_ADC1,                        \
    .adc_ch = ADC_CH##ch,                                   \
    .rank = _adc_name / 2,                                  \
    .sample_time = SAMPLE_TIME,                             \
    .p_adc_value = (uint16_t *)&adc0_value[_adc_name]

#define BSP_ADC1_REG_PARM(_adc_name,                      \
                            _gpio_periph,                   \
                            _pin,                           \
                            ch)                             \
    .adc_name = _adc_name,                                  \
    .gpio_periph = _gpio_periph,                            \
    .pin = GPIO_PIN_##_pin,                                 \
    .adc_periph = (uint32_t)CM_ADC2,                        \
    .adc_ch = ADC_CH##ch,                                   \
    .rank = _adc_name / 2,                                  \
    .sample_time = SAMPLE_TIME,                             \
    .p_adc_value = (uint16_t *)&adc1_value[_adc_name]

#define BSP_ADC2_REG_PARM(_adc_name,    \
                          _gpio_periph, \
                          _pin,         \
                          ch)           \
    .adc_name = _adc_name,              \
    .gpio_periph = _gpio_periph,        \
    .pin = GPIO_PIN_##_pin,             \
    .adc_periph = (uint32_t)CM_ADC3,    \
    .adc_ch = ADC_CH##ch,               \
    .rank = _adc_name,                  \
    .sample_time = SAMPLE_TIME,         \
    .p_adc_value = (uint16_t *)&adc2_value[_adc_name]


typedef enum
{
    ILA,
    ILV,
    FVS48,
    RVS12,
    TEMP1,
    TEMP2,
    ADC0_TABLE_MAX,
} ADC0_TABLE_E;

typedef enum
{
    ILB,
    IHV,
    VDD8V,
    ACCVS,
//    ADDRS,
    TEMP3,
    RMTVS,
    ADC1_TABLE_MAX,
} ADC1_TABLE_E;

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
float bsp_adc0_get_value(ADC0_TABLE_E name);
float bsp_adc1_get_value(ADC1_TABLE_E name);

float bsp_adc_get_ila(void);

float bsp_adc_get_ilb(void);

float bsp_adc_get_ihv(void);

float bsp_adc_get_ilv(void);

float bsp_adc_get_fvs48(void);

float bsp_adc_get_rvs12(void);

float bsp_adc_get_rmtvs(void);
    
float bsp_adc_get_accvs(void);

//float bsp_adc_get_addrs(void);

float bsp_adc_get_temp1(void);

float bsp_adc_get_temp2(void);

float bsp_adc_get_temp3(void);

float bsp_adc_get_vdd8v(void);

void bsp_adc_deinit(void);
#endif
