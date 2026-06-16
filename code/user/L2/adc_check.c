#include "adc_check.h"
#include "adc.h"
#include "my_math.h"
#include "stdint.h"
#include "section.h"
#include "shell.h"
#include "temperature.h"
#include "wg_com_v2.h"
#include "gpio.h"
#include "fault.h"
#include "data_com.h"
#include "bat_charge_pattern.h"
void voltage_check_init(voltage_check_state_t *state, const voltage_check_config_t *config)
{
    state->is_ok = 0;
    state->is_ok_cnt = 0;
    state->ovp_cnt = 0;
    state->uvp_cnt = 0;
    state->status = ADC_CHECK_VOLT_NULL;
    state->config = config;
}

// 安全增加计数器（防止溢出）
static inline void safe_increment(uint32_t *counter, uint32_t max)
{
    if (*counter < max)
        (*counter)++;
}

// 安全减少计数器（防止下溢）
static inline void safe_decrement(uint32_t *counter)
{
    if (*counter > 0)
        (*counter)--;
}

// 重置所有计数器
static inline void reset_all_counters(voltage_check_state_t *state)
{
    state->is_ok_cnt = 0;
    state->ovp_cnt = 0;
    state->uvp_cnt = 0;
}
// 状态转移函数 - 提取公共操作
static void transition_state(voltage_check_state_t *state, ADC_CHECK_VOLT_E new_status, uint8_t is_ok)
{
    state->status = new_status;
    state->is_ok = is_ok;
    reset_all_counters(state);
}

// NULL状态处理
static void handle_null_state(voltage_check_state_t *state, float voltage)
{
    const voltage_check_config_t *cfg = state->config;

    // 处理OVP计数
    voltage > cfg->ovp_threshold ? safe_increment(&state->ovp_cnt, cfg->ovp_time) : safe_decrement(&state->ovp_cnt);

    // 处理UVP计数
    voltage < cfg->uvp_threshold ? safe_increment(&state->uvp_cnt, cfg->uvp_time) : safe_decrement(&state->uvp_cnt);

    // 处理正常范围计数
    if ((voltage < cfg->normal_up) &&
        (voltage > cfg->normal_dn))
    {
        safe_increment(&state->is_ok_cnt, cfg->normal_time);
    }
    else
    {
        safe_decrement(&state->is_ok_cnt);
    }

    // 状态转移判断
    if (state->ovp_cnt >= cfg->ovp_time)
    {
        transition_state(state, ADC_CHECK_VOLT_OVP, 0);
    }
    else if (state->uvp_cnt >= cfg->uvp_time)
    {
        transition_state(state, ADC_CHECK_VOLT_UVP, 0);
    }
    else if (state->is_ok_cnt >= cfg->normal_time)
    {
        transition_state(state, ADC_CHECK_VOLT_NORMAL, 1);
    }
}

// NORMAL状态处理
static void handle_normal_state(voltage_check_state_t *state, float voltage)
{
    const voltage_check_config_t *cfg = state->config;

    // 处理OVP计数
    voltage > cfg->ovp_threshold ? safe_increment(&state->ovp_cnt, cfg->ovp_time) : safe_decrement(&state->ovp_cnt);

    // 处理UVP计数
    voltage < cfg->uvp_threshold ? safe_increment(&state->uvp_cnt, cfg->uvp_time) : safe_decrement(&state->uvp_cnt);

    // 处理异常范围计数
    if ((voltage > cfg->abnormal_up) ||
        (voltage < cfg->abnormal_dn))
    {
        safe_increment(&state->is_ok_cnt, cfg->abnormal_time);
    }
    else
    {
        safe_decrement(&state->is_ok_cnt);
    }

    // 状态转移判断
    if (state->is_ok_cnt >= cfg->abnormal_time)
    {
        transition_state(state, ADC_CHECK_VOLT_NULL, 0);
    }
    else if (state->ovp_cnt >= cfg->ovp_time)
    {
        transition_state(state, ADC_CHECK_VOLT_OVP, 0);
    }
    else if (state->uvp_cnt >= cfg->uvp_time)
    {
        transition_state(state, ADC_CHECK_VOLT_UVP, 0);
    }
}

// OVP状态处理
static void handle_ovp_state(voltage_check_state_t *state, float voltage)
{
    const voltage_check_config_t *cfg = state->config;

    // 处理恢复计数
    voltage < cfg->ovp_hysteresis ? safe_increment(&state->is_ok_cnt, cfg->normal_time) : safe_decrement(&state->is_ok_cnt);

    // 状态转移判断
    if (state->is_ok_cnt >= cfg->normal_time)
    {
        transition_state(state, ADC_CHECK_VOLT_NULL, 0);
    }
}

// UVP状态处理
static void handle_uvp_state(voltage_check_state_t *state, float voltage)
{
    const voltage_check_config_t *cfg = state->config;

    // 处理恢复计数
    voltage > cfg->uvp_hysteresis ? safe_increment(&state->is_ok_cnt, cfg->normal_time) : safe_decrement(&state->is_ok_cnt);

    // 状态转移判断
    if (state->is_ok_cnt >= cfg->normal_time)
    {
        transition_state(state, ADC_CHECK_VOLT_NULL, 0);
    }
}
// 电压检测主函数
void voltage_check_func(voltage_check_state_t *state, float voltage)
{
    typedef void (*state_handler_t)(voltage_check_state_t *, float);

    static const state_handler_t state_handlers[] = {
        handle_null_state,
        handle_normal_state,
        handle_ovp_state,
        handle_uvp_state};

    if (state->status < (sizeof(state_handlers) / sizeof(state_handlers[0])))
    {
        state_handlers[state->status](state, voltage);
    }
}

// 具体电压检测实例
// 48V系统配置
static const voltage_check_config_t fvs48_config = {
    .ovp_threshold = ADC_CHECK_FVS48_OVP,
    .uvp_threshold = ADC_CHECK_FVS48_UVP,
    .normal_up = ADC_CHECK_FVS48_NORMAL_UP,
    .normal_dn = ADC_CHECK_FVS48_NORMAL_DN,
    .abnormal_up = ADC_CHECK_FVS48_ABNORMAL_UP,
    .abnormal_dn = ADC_CHECK_FVS48_ABNORMAL_DN,
    .ovp_hysteresis = ADC_CHECK_FVS48_OVP_HYS,
    .uvp_hysteresis = ADC_CHECK_FVS48_UVP_HYS,
    .ovp_time = TIME_CNT_20MS_IN_1MS,
    .uvp_time = TIME_CNT_20MS_IN_1MS,
    .normal_time = TIME_CNT_200MS_IN_1MS,
    .abnormal_time = TIME_CNT_20MS_IN_1MS,
};

// 12V系统配置
static const voltage_check_config_t rvs12_config = {
    .ovp_threshold = ADC_CHECK_RVS12_OVP,
    .uvp_threshold = ADC_CHECK_RVS12_UVP,
    .normal_up = ADC_CHECK_RVS12_NORMAL_UP,
    .normal_dn = ADC_CHECK_RVS12_NORMAL_DN,
    .abnormal_up = ADC_CHECK_RVS12_ABNORMAL_UP,
    .abnormal_dn = ADC_CHECK_RVS12_ABNORMAL_DN,
    .ovp_hysteresis = ADC_CHECK_RVS12_OVP_HYS,
    .uvp_hysteresis = ADC_CHECK_RVS12_UVP_HYS,
    .ovp_time = ADC_CHECK_RVS12_OVP_TIME,
    .uvp_time = ADC_CHECK_RVS12_UVP_TIME,
    .normal_time = ADC_CHECK_RVS12_NORMAL_TIME,
    .abnormal_time = ADC_CHECK_RVS12_ABNORMAL_TIME,
};

// 48V系统状态
voltage_check_state_t fvs48_state;

// 12V系统状态
voltage_check_state_t rvs12_state;

// 48V检测函数
void adc_check_fvs_48_is_ok_func(void)
{
    voltage_check_func(&fvs48_state, get_show_fvs48_show());
}

// 12V检测函数
void adc_check_rvs_12_is_ok_func(void)
{
    voltage_check_func(&rvs12_state, get_show_rvs12_show());
}

#if (SHELL_ON_OFF == 1)
REG_SHELL_VAR(fvs48_is_ok, fvs48_state.is_ok, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(rvs12_is_ok, rvs12_state.is_ok, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
#endif

// 获取状态函数
uint8_t adc_check_get_fvs48_is_ok(void)
{
    return fvs48_state.is_ok;
}

uint8_t adc_check_get_rvs12_is_ok(void)
{
    return rvs12_state.is_ok;
}

static aux_check_state_t aux_state = {0};

uint8_t adc_check_get_aux_is_ok(void)
{
    return aux_state.is_ok;
}

static float aux_volt = 0.0f;
#if (SHELL_ON_OFF == 1)
REG_SHELL_VAR(aux_volt, aux_volt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
#endif
uint32_t auxoffDelay = 0,auxonDelay = 0;
static void adc_check_aux_is_ok_func(void)
{
    aux_volt = adc_get_vdd_8v();
    // 定义清晰的电压范围检查
    const uint8_t is_normal = (aux_volt < ADC_CHECK_AUX_NORMAL_UP) &&
                              (aux_volt > ADC_CHECK_AUX_NORMAL_DN);
    const uint8_t is_abnormal = (aux_volt > ADC_CHECK_AUX_ABNORMAL_UP) ||
                                (aux_volt < ADC_CHECK_AUX_ABNORMAL_DN);

    // 状态转换逻辑
    if (is_normal)
    {
        safe_increment(&aux_state.is_ok_cnt, ADC_CHECK_AUX_NORMAL_TIME);
    }
    else if (is_abnormal)
    {
        aux_state.is_ok_cnt = 0; // 立即重置计数器
    }
    else
    {
        // 中间状态，可选的递减逻辑
        safe_decrement(&aux_state.is_ok_cnt);
    }

    // 状态更新（带迟滞）
    aux_state.is_ok = (aux_state.is_ok_cnt >= ADC_CHECK_AUX_NORMAL_TIME) ? 1 : 0;
}

// 状态计数器
//static uint32_t adds_state_cnt = 0;
static ADC_CHECK_ADDRS_E adds_state = ADDRS_IDLE;

#if (SHELL_ON_OFF == 1)
REG_SHELL_VAR(adds_state, adds_state, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
#endif

ADC_CHECK_ADDRS_E RAMFUNC adc_check_get_addrs_state(void)
{
    return adds_state;
}

//static void adc_check_addrs_state_func(void)
//{
//    const float voltage = adc_get_addrs();
//    static ADC_CHECK_ADDRS_E candidate_state = ADDRS_IDLE;

//    // 确定当前电压对应的候选状态
//    ADC_CHECK_ADDRS_E new_candidate = ADDRS_IDLE;

//    if (voltage > ADC_CHECK_ADDRS_IDLE_DN &&
//        voltage < ADC_CHECK_ADDRS_IDLE_UP)
//    {
//        new_candidate = ADDRS_IDLE;
//    }
//    else if (voltage > ADC_CHECK_ADDRS_FORWARD_DN &&
//             voltage < ADC_CHECK_ADDRS_FORWARD_UP)
//    {
//        new_candidate = ADDRS_FORWARD;
//    }
//    else if (voltage > ADC_CHECK_ADDRS_BACKWARD_DN &&
//             voltage < ADC_CHECK_ADDRS_BACKWARD_UP)
//    {
//        new_candidate = ADDRS_BACKWARD;
//    }
//    else if (voltage > ADC_CHECK_ADDRS_BIDIRECTIONAL_DN &&
//             voltage < ADC_CHECK_ADDRS_BIDIRECTIONAL_UP)
//    {
//        new_candidate = ADDRS_BIDIRECTIONAL;
//    }
//    else
//    {
//        // 电压不在任何定义范围内，保持IDLE状态
//        new_candidate = ADDRS_IDLE;
//    }

//    // 检查状态是否变化
//    if (new_candidate != candidate_state)
//    {
//        candidate_state = new_candidate;
//        adds_state_cnt = 0; // 状态变化，重置计数器
//    }
//    else
//    {
//        // 状态持续，增加计数器（不超过最大值）
//        if (adds_state_cnt < ADDRS_STATE_CONFIRM_TIME)
//        {
//            adds_state_cnt++;
//        }
//    }

//    // 确认状态转换（持续达到1秒）
//    if (adds_state_cnt >= ADDRS_STATE_CONFIRM_TIME)
//    {
//        adds_state = candidate_state;
//    }
//    else
//    {
//        // 未达到确认时间，保持IDLE状态
//        adds_state = ADDRS_IDLE;
//    }
//}

static void adc_check_soft_set_adds_func(void)
{
    uint8_t chg_mode = 0;

	chg_mode = get_check_state_data();
    if (chg_mode == ADDRS_FORWARD)
    {
        adds_state = ADDRS_FORWARD;
    }
    else if (chg_mode == ADDRS_BACKWARD)
    {
        adds_state = ADDRS_BACKWARD;
    }
    else if (chg_mode == ADDRS_BIDIRECTIONAL)
    {
        adds_state = ADDRS_BIDIRECTIONAL;
    }
    else
    {
        adds_state = ADDRS_IDLE;
    }
}

void adc_check_init(void)
{
    voltage_check_init(&fvs48_state, &fvs48_config);
    voltage_check_init(&rvs12_state, &rvs12_config);
}

REG_INIT(adc_check_init)

float ntc1_temp = 0.0f;
float ntc2_temp = 0.0f;
float ntc3_temp = 0.0f;

float adc_check_get_ntc1_temp(void)
{
    return ntc1_temp;
}

float adc_check_get_ntc2_temp(void)
{
    return ntc2_temp;
}

float adc_check_get_ntc3_temp(void)
{
    return ntc3_temp;
}

void adc_check_run(void)
{
    adc_check_fvs_48_is_ok_func();
    adc_check_rvs_12_is_ok_func();
    adc_check_aux_is_ok_func();
    adc_check_soft_set_adds_func();

    ntc1_temp = cal_ntc_temp(adc_get_temp1(), IS_UP_NTC);
    ntc2_temp = cal_ntc_temp(adc_get_temp2(), IS_UP_NTC);
    ntc3_temp = cal_ntc_temp(adc_get_temp3(), IS_UP_NTC);
}

REG_TASK(1, adc_check_run)
