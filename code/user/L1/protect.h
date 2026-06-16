#ifndef __PROTECT_H
#define __PROTECT_H

#include "stdint.h"
#include "my_math.h"
#include "stdbool.h"
#include "fault.h"

#define IL_OCP_THRESHOLD 64.0f
#define IL_OCP_TIME 1

#define FAST_OVP_MULTIPLIER 1.3f
#define SLOW_OVP_MULTIPLIER_2 1.2f
#define SLOW_OVP_MULTIPLIER_1 1.1f

#define SLOW_OVP_TIME_2 TIME_CNT_5MS_IN_1MS
#define SLOW_OVP_TIME_1 TIME_CNT_10MS_IN_1MS

#define OVP_DN_THR 9.0f

#define PROTECT_TRIGGER_DELAY_COUNT TIME_CNT_500MS_IN_1MS
#define PROTECT_SCP_TRIGGER_DELAY_COUNT TIME_CNT_20MS_IN_1MS
#define PROTECT_TRIGGER_R_DELAY_COUNT TIME_CNT_2S_IN_1MS
#define PROTECT_SCP_TRIGGER_R_DELAY_COUNT TIME_CNT_2S_IN_1MS

#define FAST_UVP_THRESHOLD 9.0f
#define FAST_UVP_TIME TIME_CNT_40US_IN_CTRL_TS


typedef struct
{
    float val;                         // 当前值
    float limit;                       // 触发阈值
    float recover;                     // 恢复阈值
    int counter;                       // 计数器
    int counter_r;                     // 恢复计数器
    bool flag;                         // 当前是否处于保护中
    FAULT_E fault;                     // 故障类型
    bool (*cmp_trigger)(float, float); // 触发比较函数
    bool (*cmp_recover)(float, float); // 恢复比较函数
    uint8_t enable;
    uint32_t TriggerTime;                // 触发时间
    uint32_t RecoverTime;                // 恢复时间
} protect_item_t;

typedef struct
{
    float val;                         // 当前值
    float limit;                       // 触发阈值
    float recover;                     // 恢复阈值
    int counter;                       // 计数器
    int counter_r;                     // 恢复计数器
    bool flag;                         // 当前是否处于保护中
    ALARM_E alarm;                     // 故障类型
    bool (*cmp_trigger)(float, float); // 触发比较函数
    bool (*cmp_recover)(float, float); // 恢复比较函数
    uint8_t enable;
    uint32_t TriggerTime;                // 触发时间
    uint32_t RecoverTime;                // 恢复时间
} warning_item_t;

void protect_fast(void);
uint8_t get_volt_low_fault(void);
#endif
