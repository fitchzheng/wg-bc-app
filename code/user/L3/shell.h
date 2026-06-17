#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdint.h>
#include <stddef.h>

#include "app_features.h"
#include "section.h"

#ifndef SHELL_ON_OFF
#define SHELL_ON_OFF 0
#endif

/* Parser state owned by one link handler instance. */
typedef struct
{
    uint8_t shell_buffer[128];
    uint8_t shell_index;
} shell_ctx_t;

#define DECLARE_SHELL_CTX(name) \
    static shell_ctx_t name = {0}

typedef enum
{
    SHELL_INT8 = 0,
    SHELL_UINT8,
    SHELL_INT16,
    SHELL_UINT16,
    SHELL_INT32,
    SHELL_UINT32,
    SHELL_FP32,
    SHELL_CMD,
} SHELL_TYPE_E;

#define SHELL_STR_SIZE_MAX 40

#define SHELL_STA_NULL (0u)
#define SHELL_STA_AUTO (1u << 2)

/* Shell registry entry collected from SECTION_SHELL at startup. */
typedef struct section_shell_t
{
    const char *p_name;
    uint32_t p_name_size;

    void *p_var;
    uint32_t type;
    void *p_max;
    void *p_min;
    void (*func)(DEC_MY_PRINTF);
    uint32_t status;

    DEC_MY_PRINTF;
    struct section_shell_t *p_next;
} section_shell_t;

#define SHELL_LIMIT_TYPE_SHELL_INT8 int8_t
#define SHELL_LIMIT_TYPE_SHELL_UINT8 uint8_t
#define SHELL_LIMIT_TYPE_SHELL_INT16 int16_t
#define SHELL_LIMIT_TYPE_SHELL_UINT16 uint16_t
#define SHELL_LIMIT_TYPE_SHELL_INT32 int32_t
#define SHELL_LIMIT_TYPE_SHELL_UINT32 uint32_t
#define SHELL_LIMIT_TYPE_SHELL_FP32 float
#define SHELL_LIMIT_TYPE_SELECT(type) SHELL_LIMIT_TYPE_SELECT_(type)
#define SHELL_LIMIT_TYPE_SELECT_(type) SHELL_LIMIT_TYPE_##type
#define SHELL_STATIC_ASSERT_NAME(name) SHELL_STATIC_ASSERT_NAME_(name)
#define SHELL_STATIC_ASSERT_NAME_(name) shell_name_len_check_##name

#if (APP_DEBUG_FEATURES == 1)

/* Register a writable shell variable with typed limits. */
#define REG_SHELL_VAR(_name, _var, _type, _max, _min, _func, _status)                                      \
    typedef char SHELL_STATIC_ASSERT_NAME(_name)[(sizeof(#_name) <= (SHELL_STR_SIZE_MAX + 1)) ? 1 : -1];   \
    static SHELL_LIMIT_TYPE_SELECT(_type) _name##_##max = (SHELL_LIMIT_TYPE_SELECT(_type))(_max);          \
    static SHELL_LIMIT_TYPE_SELECT(_type) _name##_##min = (SHELL_LIMIT_TYPE_SELECT(_type))(_min);          \
    section_shell_t section_shell_##_name = {                                              \
        .p_name = #_name,                                                                  \
        .p_name_size = (uint32_t)(sizeof(#_name) - 1),                                     \
        .p_var = (void *)&(_var),                                                          \
        .type = (uint32_t)(_type),                                                         \
        .p_max = (void *)&_name##_##max,                                                   \
        .p_min = (void *)&_name##_##min,                                                   \
        .func = (_func),                                                                   \
        .status = (uint32_t)(_status),                                                     \
        .p_next = NULL,                                                                    \
    };                                                                                     \
    REG_SECTION_FUNC(SECTION_SHELL, section_shell_##_name)

/* Register an executable shell command. */
#define REG_SHELL_CMD(_name, _func)                                                                        \
    typedef char SHELL_STATIC_ASSERT_NAME(_name)[(sizeof(#_name) <= (SHELL_STR_SIZE_MAX + 1)) ? 1 : -1];   \
    section_shell_t section_shell_##_name = {                                              \
        .p_name = #_name,                                                                  \
        .p_name_size = (uint32_t)(sizeof(#_name) - 1),                                     \
        .p_var = NULL,                                                                     \
        .type = (uint32_t)SHELL_CMD,                                                       \
        .func = (_func),                                                                   \
        .status = 0,                                                                       \
        .p_next = NULL,                                                                    \
    };                                                                                     \
    REG_SECTION_FUNC(SECTION_SHELL, section_shell_##_name)

#else

#define REG_SHELL_VAR(_name, _var, _type, _max, _min, _func, _status)
#define REG_SHELL_CMD(_name, _func)

#endif

void shell_run(uint8_t data, DEC_MY_PRINTF, void *ctx);

#define CMD_SET_SHELL_DATA_NUM 0x01
#define CMD_WORD_SHELL_DATA_NUM 0x01

#define CMD_SET_SHELL_REPORT_LIST 0x01
#define CMD_WORD_SHELL_REPORT_LIST 0x04

#define CMD_SET_SHELL_READ_DATA 0x01
#define CMD_WORD_SHELL_READ_DATA 0x02

#define CMD_SET_SHELL_WRITE_DATA 0x01
#define CMD_WORD_SHELL_WRITE_DATA 0x03

#define CMD_SET_SHELL_WAVE_ENABLE_PARAM 0x01
#define CMD_WORD_SHELL_WAVE_ENABLE_PARAM 0x05

#define CMD_SET_SHELL_WAVE_START 0x01
#define CMD_WORD_SHELL_WAVE_START 0x0C

#define CMD_SET_SHELL_WAVE_PERIOD 0x01
#define CMD_WORD_SHELL_WAVE_PERIOD 0x06

#define CMD_SET_SHELL_WAVE_PARAM 0x01
#define CMD_WORD_SHELL_WAVE_PARAM 0x07

/* Context used while reporting shell entries through the binary protocol. */
typedef struct
{
    uint8_t active;
    DEC_MY_PRINTF;
    uint8_t src;
    uint8_t d_src;
    uint8_t dst;
    uint8_t d_dst;
    section_shell_t *p_shell;
} shell_report_ctx_t;

#pragma pack(1)

/* Packed payloads exchanged with the host shell tool. */
typedef struct
{
    uint8_t name_len;
    uint8_t type;
    uint32_t data;
    uint32_t data_max;
    uint32_t data_min;
    uint8_t auto_report;
    char name[SHELL_STR_SIZE_MAX];
} shell_report_list_t;

typedef struct
{
    uint8_t name_len;
    char name[SHELL_STR_SIZE_MAX];
} shell_read_data_t;

typedef struct
{
    uint8_t name_len;
    uint8_t type;
    uint32_t data;
    char name[SHELL_STR_SIZE_MAX];
} shell_read_data_ret_t;

typedef struct
{
    uint8_t name_len;
    uint32_t data;
    uint32_t data_max;
    uint32_t data_min;
    char name[SHELL_STR_SIZE_MAX];
} shell_write_data_t;

typedef struct
{
    uint8_t name_len;
    uint8_t type;
    uint32_t data;
    uint32_t data_max;
    uint32_t data_min;
    char name[SHELL_STR_SIZE_MAX];
} shell_write_data_ret_t;

typedef struct
{
    uint8_t name_len;
    uint8_t auto_report;
    char name[SHELL_STR_SIZE_MAX];
} shell_wave_enable_param_t;

typedef struct
{
    uint8_t ok;
} shell_wave_enable_param_ack_t;

typedef struct
{
    uint8_t name_len;
    uint8_t type;
    uint32_t data;
    char name[SHELL_STR_SIZE_MAX];
} shell_wave_param_t;

typedef struct
{
    uint8_t start_report;
} shell_wave_start_t;

typedef struct
{
    uint32_t reprot_period;
} shell_wave_period_t;

typedef struct
{
    uint32_t reprot_period;
} shell_wave_period_ack_t;

#pragma pack()

#endif


