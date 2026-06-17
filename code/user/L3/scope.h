#ifndef __SCOPE_H__
#define __SCOPE_H__

#include "app_features.h"
#include "my_math.h"
#include "section.h"
#include <stddef.h>
#include "stdint.h"

#ifndef SCOPE_ENABLE_PRINTF
#define SCOPE_ENABLE_PRINTF 0
#endif

typedef enum
{
    SCOPE_STATE_IDLE,
    SCOPE_STATE_RUNNING,
    SCOPE_STATE_TRIGGERED,
} scope_state_e;

typedef struct
{
    uint32_t write_index;
    uint32_t trigger_index;
    uint8_t is_triggered;
    uint8_t is_running;
    uint32_t buffer_size;
    uint32_t trigger_post_cnt;
    uint8_t var_count;
    uint32_t trigger_counter;
    uint8_t in_trigger;
    float *buffer;
    float **var_ptrs;
    const char **var_names;
    scope_state_e state;
} scope_t;

typedef struct scope_service_obj_t
{
    uint8_t scope_id;
    const char *p_name;
    scope_t *p_scope;
    uint32_t sample_period_us;
    uint32_t capture_tag;
    uint8_t data_ready;
    scope_state_e last_state;
    struct scope_service_obj_t *p_next;
} scope_service_obj_t;

#define CMD_SET_SCOPE 0x01u

#define CMD_WORD_SCOPE_LIST_QUERY 0x18u
#define CMD_WORD_SCOPE_INFO_QUERY 0x19u
#define CMD_WORD_SCOPE_VAR_QUERY 0x1Au
#define CMD_WORD_SCOPE_START 0x1Bu
#define CMD_WORD_SCOPE_TRIGGER 0x1Cu
#define CMD_WORD_SCOPE_STOP 0x1Du
#define CMD_WORD_SCOPE_RESET 0x1Eu
#define CMD_WORD_SCOPE_SAMPLE_QUERY 0x1Fu

typedef enum
{
    SCOPE_READ_MODE_NORMAL = 0,
    SCOPE_READ_MODE_FORCE = 1,
} scope_read_mode_e;

typedef enum
{
    SCOPE_TOOL_STATUS_OK = 0,
    SCOPE_TOOL_STATUS_SCOPE_ID_INVALID = 1,
    SCOPE_TOOL_STATUS_VAR_INDEX_INVALID = 2,
    SCOPE_TOOL_STATUS_SAMPLE_INDEX_INVALID = 3,
    SCOPE_TOOL_STATUS_RUNNING_DENIED = 4,
    SCOPE_TOOL_STATUS_DATA_NOT_READY = 5,
    SCOPE_TOOL_STATUS_BUSY = 6,
    SCOPE_TOOL_STATUS_CAPTURE_CHANGED = 7,
} scope_tool_status_e;

#pragma pack(push, 1)
typedef struct
{
    uint8_t reserved;
} scope_list_query_t;

typedef struct
{
    uint8_t scope_id;
    uint8_t is_last;
    uint8_t name_len;
    uint8_t reserved;
} scope_list_item_t;

typedef struct
{
    uint8_t scope_id;
    uint8_t reserved[3];
} scope_info_query_t;

typedef struct
{
    uint8_t scope_id;
    uint8_t status;
    uint8_t state;
    uint8_t data_ready;
    uint8_t var_count;
    uint8_t reserved[3];
    uint32_t sample_count;
    uint32_t write_index;
    uint32_t trigger_index;
    uint32_t trigger_post_cnt;
    uint32_t trigger_display_index;
    uint32_t sample_period_us;
    uint32_t capture_tag;
} scope_info_ack_t;

typedef struct
{
    uint8_t scope_id;
    uint8_t var_index;
    uint8_t reserved[2];
} scope_var_query_t;

typedef struct
{
    uint8_t scope_id;
    uint8_t status;
    uint8_t var_index;
    uint8_t is_last;
    uint8_t name_len;
    uint8_t reserved[3];
} scope_var_ack_t;

typedef struct
{
    uint8_t scope_id;
    uint8_t status;
    uint8_t state;
    uint8_t data_ready;
    uint32_t capture_tag;
} scope_ctrl_ack_t;

typedef struct
{
    uint8_t scope_id;
    uint8_t read_mode;
    uint8_t reserved[2];
    uint32_t sample_index;
    uint32_t expected_capture_tag;
} scope_sample_query_t;

typedef struct
{
    uint8_t scope_id;
    uint8_t status;
    uint8_t read_mode;
    uint8_t var_count;
    uint32_t sample_index;
    uint32_t capture_tag;
    uint8_t is_last_sample;
    uint8_t reserved[3];
} scope_sample_ack_t;
#pragma pack(pop)

#define SCOPE_ADDR(x) (&x)
#define SCOPE_EXPAND(...) __VA_ARGS__

#define SCOPE_FOR_EACH_1(m, a) m(a)
#define SCOPE_FOR_EACH_2(m, a, ...) m(a), SCOPE_FOR_EACH_1(m, __VA_ARGS__)
#define SCOPE_FOR_EACH_3(m, a, ...) m(a), SCOPE_FOR_EACH_2(m, __VA_ARGS__)
#define SCOPE_FOR_EACH_4(m, a, ...) m(a), SCOPE_FOR_EACH_3(m, __VA_ARGS__)
#define SCOPE_FOR_EACH_5(m, a, ...) m(a), SCOPE_FOR_EACH_4(m, __VA_ARGS__)
#define SCOPE_FOR_EACH_6(m, a, ...) m(a), SCOPE_FOR_EACH_5(m, __VA_ARGS__)
#define SCOPE_FOR_EACH_7(m, a, ...) m(a), SCOPE_FOR_EACH_6(m, __VA_ARGS__)
#define SCOPE_FOR_EACH_8(m, a, ...) m(a), SCOPE_FOR_EACH_7(m, __VA_ARGS__)
#define SCOPE_FOR_EACH_9(m, a, ...) m(a), SCOPE_FOR_EACH_8(m, __VA_ARGS__)
#define SCOPE_FOR_EACH_10(m, a, ...) m(a), SCOPE_FOR_EACH_9(m, __VA_ARGS__)
#define SCOPE_FOR_EACH_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) SCOPE_FOR_EACH_##N
#define SCOPE_FOR_EACH(m, ...) \
    SCOPE_EXPAND(SCOPE_FOR_EACH_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)(m, __VA_ARGS__))

#define SCOPE_COUNT_ARGS_N(_, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define SCOPE_COUNT_ARGS(...) \
    SCOPE_EXPAND(SCOPE_COUNT_ARGS_N(_, __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define SCOPE_STR(x) #x

#if SCOPE_ENABLE_PRINTF
#define REG_SCOPE_STATUS_CMD(name)
#define REG_SCOPE_START_CMD(name)
#define REG_SCOPE_DATA_STEP_CMD(name)
#else
#define REG_SCOPE_STATUS_CMD(name)
#define REG_SCOPE_START_CMD(name)
#define REG_SCOPE_DATA_STEP_CMD(name)
#endif

#if (APP_DEBUG_FEATURES == 1)

#define REG_SCOPE_EX(name, buf_size, trig_post_cnt, _sample_period_us, ...)                                         \
    float scope_##name##_buffer[SCOPE_COUNT_ARGS(__VA_ARGS__)][buf_size];                                           \
    float __VA_ARGS__;                                                                                              \
    float *scope_##name##_var_ptrs[SCOPE_COUNT_ARGS(__VA_ARGS__)] = {SCOPE_FOR_EACH(SCOPE_ADDR, __VA_ARGS__)};      \
    const char *scope_##name##_var_names[SCOPE_COUNT_ARGS(__VA_ARGS__)] = {SCOPE_FOR_EACH(SCOPE_STR, __VA_ARGS__)}; \
    scope_t scope_##name = {                                                                                        \
        .write_index = 0,                                                                                           \
        .trigger_index = 0,                                                                                         \
        .is_triggered = 0,                                                                                          \
        .is_running = 0,                                                                                            \
        .buffer_size = buf_size,                                                                                    \
        .var_count = SCOPE_COUNT_ARGS(__VA_ARGS__),                                                                 \
        .trigger_post_cnt = trig_post_cnt,                                                                          \
        .var_ptrs = &scope_##name##_var_ptrs[0],                                                                    \
        .buffer = &scope_##name##_buffer[0][0],                                                                     \
        .var_names = &scope_##name##_var_names[0],                                                                  \
        .trigger_counter = 0,                                                                                       \
        .in_trigger = 0,                                                                                            \
        .state = SCOPE_STATE_IDLE,                                                                                  \
    };                                                                                                              \
    scope_service_obj_t scope_service_obj_##name = {                                                                \
        .scope_id = 0u,                                                                                             \
        .p_name = #name,                                                                                            \
        .p_scope = &scope_##name,                                                                                   \
        .sample_period_us = (_sample_period_us),                                                                    \
        .capture_tag = 0u,                                                                                          \
        .data_ready = 0u,                                                                                           \
        .last_state = SCOPE_STATE_IDLE,                                                                             \
        .p_next = NULL,                                                                                             \
    };                                                                                                              \
    static void scope_service_auto_reg_##name(void)                                                                 \
    {                                                                                                               \
        scope_service_register(&scope_service_obj_##name);                                                          \
    }                                                                                                               \
    REG_INIT(1, scope_service_auto_reg_##name)                                                                      \
    REG_SCOPE_STATUS_CMD(name)                                                                                      \
    REG_SCOPE_START_CMD(name)                                                                                       \
    REG_SCOPE_DATA_STEP_CMD(name)

#define REG_SCOPE(name, buf_size, trig_post_cnt, ...) \
    REG_SCOPE_EX(name, buf_size, trig_post_cnt, (uint32_t)(CTRL_TS * 1000000.0f + 0.5f), __VA_ARGS__)

#else

#define REG_SCOPE_EX(name, buf_size, trig_post_cnt, _sample_period_us, ...)
#define REG_SCOPE(name, buf_size, trig_post_cnt, ...)

#endif

void scope_run(scope_t *scope);
void scope_start(scope_t *scope);
void scope_stop(scope_t *scope);
void scope_trigger(scope_t *scope);
void scope_reset(scope_t *scope);
void scope_service_register(scope_service_obj_t *p_obj);
scope_service_obj_t *scope_service_first(void);
uint8_t scope_service_count(void);

#if (APP_DEBUG_FEATURES == 1)
#define SCOPE_RUN(name) scope_run(&scope_##name)
#define SCOPE(name) scope_run(&scope_##name)
#define SCOPE_TRIGGER(name) scope_trigger(&scope_##name)
#define SCOPE_GET_BUFFER(name) (scope_##name.buffer)
#define SCOPE_GET_BUFFER_SIZE(name) (scope_##name.buffer_size)
#define SCOPE_GET_VAR_NUM(name) (scope_##name.var_count)
#define SCOPE_GET_VAR_PTRS(name) (scope_##name.var_ptrs)
#else
#define SCOPE_RUN(name) ((void)0)
#define SCOPE(name) ((void)0)
#define SCOPE_TRIGGER(name) ((void)0)
#define SCOPE_GET_BUFFER(name) NULL
#define SCOPE_GET_BUFFER_SIZE(name) 0U
#define SCOPE_GET_VAR_NUM(name) 0U
#define SCOPE_GET_VAR_PTRS(name) NULL
#endif

#endif
