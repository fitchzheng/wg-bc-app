#ifndef __ADC_CHECK_H
#define __ADC_CHECK_H

#include "stdint.h"

#define ADC_CHECK_FVS48_NORMAL_UP 64.0f
#define ADC_CHECK_FVS48_NORMAL_DN 7.0f
#define ADC_CHECK_FVS48_ABNORMAL_UP 65.0f
#define ADC_CHECK_FVS48_ABNORMAL_DN 8.0f
#define ADC_CHECK_FVS48_OVP 65.0f
#define ADC_CHECK_FVS48_OVP_HYS 64.0f
#define ADC_CHECK_FVS48_UVP 7.0f
#define ADC_CHECK_FVS48_UVP_HYS 8.0f
#define ADC_CHECK_FVS48_OVP_TIME TIME_CNT_20MS_IN_1MS
#define ADC_CHECK_FVS48_UVP_TIME TIME_CNT_20MS_IN_1MS
#define ADC_CHECK_FVS48_NORMAL_TIME TIME_CNT_1S_IN_1MS
#define ADC_CHECK_FVS48_ABNORMAL_TIME TIME_CNT_20MS_IN_1MS

#define ADC_CHECK_RVS12_NORMAL_UP 64.0f
#define ADC_CHECK_RVS12_NORMAL_DN 7.0f
#define ADC_CHECK_RVS12_ABNORMAL_UP 65.0f
#define ADC_CHECK_RVS12_ABNORMAL_DN 8.0f
#define ADC_CHECK_RVS12_OVP 65.0f
#define ADC_CHECK_RVS12_OVP_HYS 64.0f
#define ADC_CHECK_RVS12_UVP 7.0f
#define ADC_CHECK_RVS12_UVP_HYS 8.0f
#define ADC_CHECK_RVS12_OVP_TIME TIME_CNT_20MS_IN_1MS
#define ADC_CHECK_RVS12_UVP_TIME TIME_CNT_20MS_IN_1MS
#define ADC_CHECK_RVS12_NORMAL_TIME TIME_CNT_200MS_IN_1MS
#define ADC_CHECK_RVS12_ABNORMAL_TIME TIME_CNT_20MS_IN_1MS

#define ADC_CHECK_AUX_NORMAL_UP 9.0f
#define ADC_CHECK_AUX_NORMAL_DN 7.0f
#define ADC_CHECK_AUX_ABNORMAL_UP 10.0f
#define ADC_CHECK_AUX_ABNORMAL_DN 6.0f
#define ADC_CHECK_AUX_NORMAL_TIME TIME_CNT_200MS_IN_1MS

#define ADC_CHECK_ADDRS_IDLE_UP 3.3f
#define ADC_CHECK_ADDRS_IDLE_DN 3.1f
#define ADC_CHECK_ADDRS_FORWARD_UP 3.0f
#define ADC_CHECK_ADDRS_FORWARD_DN 2.4f
#define ADC_CHECK_ADDRS_BACKWARD_UP 2.2f
#define ADC_CHECK_ADDRS_BACKWARD_DN 1.9f
#define ADC_CHECK_ADDRS_BIDIRECTIONAL_UP 1.85f
#define ADC_CHECK_ADDRS_BIDIRECTIONAL_DN 1.5f
#define ADDRS_STATE_CONFIRM_TIME TIME_CNT_1S_IN_1MS

// 状态枚举
typedef enum
{
    ADC_CHECK_VOLT_NULL,
    ADC_CHECK_VOLT_NORMAL,
    ADC_CHECK_VOLT_OVP,
    ADC_CHECK_VOLT_UVP
} ADC_CHECK_VOLT_E;

typedef enum
{
    ADDRS_IDLE,
    ADDRS_FORWARD,
    ADDRS_BACKWARD,
    ADDRS_BIDIRECTIONAL,
    ADDRS_MAX,
} ADC_CHECK_ADDRS_E;

// 电压检测配置结构体
typedef struct
{
    float ovp_threshold;    // 过压保护阈值
    float uvp_threshold;    // 欠压保护阈值
    float normal_up;        // 正常范围上限
    float normal_dn;        // 正常范围下限
    float abnormal_up;      // 异常范围上限
    float abnormal_dn;      // 异常范围下限
    float ovp_hysteresis;   // OVP恢复迟滞
    float uvp_hysteresis;   // UVP恢复迟滞
    uint32_t ovp_time;      // OVP判定时间
    uint32_t uvp_time;      // UVP判定时间
    uint32_t normal_time;   // 正常判定时间
    uint32_t abnormal_time; // 异常判定时间
} voltage_check_config_t;

// 电压检测状态结构体
typedef struct
{
    uint8_t is_ok;                        // 状态是否正常
    uint32_t is_ok_cnt;                   // 正常计数器
    uint32_t ovp_cnt;                     // OVP计数器
    uint32_t uvp_cnt;                     // UVP计数器
    ADC_CHECK_VOLT_E status;              // 当前状态
    const voltage_check_config_t *config; // 配置指针
} voltage_check_state_t;

// 使用结构体封装状态变量
typedef struct
{
    uint8_t is_ok;
    uint32_t is_ok_cnt;
} aux_check_state_t;

uint8_t adc_check_get_fvs48_is_ok(void);

uint8_t adc_check_get_rvs12_is_ok(void);

uint8_t adc_check_get_aux_is_ok(void);

ADC_CHECK_ADDRS_E adc_check_get_addrs_state(void);

float adc_check_get_ntc1_temp(void);

float adc_check_get_ntc2_temp(void);

float adc_check_get_ntc3_temp(void);

#endif
