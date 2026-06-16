#ifndef __PERF_H__
#define __PERF_H__

#include <stdint.h>

typedef enum
{
    SECTION_PERF_RECORD = 0,
    SECTION_PERF_BASE,
} SECTION_PERF_E;

typedef struct
{
    uint32_t *p_cnt;
} section_perf_base_t;

typedef struct
{
    uint32_t perf_type;
    void *p_perf;
} section_perf_t;

typedef struct
{
    const char *p_name;
    uint16_t start;
    uint16_t time;
    uint32_t max_time;
    void *p_next;
} section_perf_record_t;

extern uint32_t *g_section_perf_cnt;

#define REG_PERF_BASE_CNT(timer_cnt)                \
    const section_perf_base_t section_perf_base_timer = { \
        .p_cnt = (timer_cnt),                       \
    };                                              \
    const section_perf_t section_timer_cnt_perf = { \
        .perf_type = SECTION_PERF_BASE,             \
        .p_perf = (void *)&section_perf_base_timer, \
    };                                              \
    REG_SECTION_FUNC(SECTION_PERF, section_timer_cnt_perf)

#define PERF_RECORD_ENABLE 1

#if (PERF_RECORD_ENABLE == 1)

#define PERF_START(name)                                                           \
    do                                                                             \
    {                                                                              \
        if (g_section_perf_cnt != NULL)                                             \
        {                                                                          \
            section_perf_record_##name.start = (uint16_t)(*g_section_perf_cnt);     \
        }                                                                          \
    } while (0)

#define PERF_END(name)                                                                                                       \
    do                                                                                                                       \
    {                                                                                                                        \
        if (g_section_perf_cnt != NULL)                                                                                      \
        {                                                                                                                    \
            section_perf_record_##name.time = (uint16_t)((uint16_t)(*g_section_perf_cnt) - section_perf_record_##name.start); \
            if (section_perf_record_##name.time > section_perf_record_##name.max_time)                                       \
            {                                                                                                                \
                section_perf_record_##name.max_time = section_perf_record_##name.time;                                       \
            }                                                                                                                \
        }                                                                                                                    \
    } while (0)

#define P_RECORD_PERF(name) ((section_perf_record_t *)&section_perf_record_##name)

#define REG_PERF_RECORD(name)                            \
    section_perf_record_t section_perf_record_##name = { \
        .p_name = #name,                                 \
        .start = 0,                                      \
        .time = 0,                                       \
        .max_time = 0,                                   \
        .p_next = NULL,                                  \
    };                                                   \
    const section_perf_t section_perf_record_##name##_perf = { \
        .perf_type = SECTION_PERF_RECORD,                \
        .p_perf = (void *)&section_perf_record_##name,   \
    };                                                   \
    REG_SECTION_FUNC(SECTION_PERF, section_perf_record_##name##_perf)

#else

#define PERF_START(name)
#define PERF_END(name)
#define P_RECORD_PERF(name) NULL
#define REG_PERF_RECORD(name)

#endif

#endif
