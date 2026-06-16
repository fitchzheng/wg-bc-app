#ifndef __OPEN_LOOP_H
#define __OPEN_LOOP_H

// 定义模式切换的阈值
#define BUCK_TO_BUCK_BOOST_THRESHOLD 0.93f
#define BUCK_BOOST_TO_BOOST_THRESHOLD 1.17f
#define BOOST_TO_BUCK_BOOST_THRESHOLD 1.11f
#define BUCK_BOOST_TO_BUCK_THRESHOLD 0.85f

// 定义模式枚举
typedef enum
{
    MODE_BUCK,
    MODE_BUCK_BOOST,
    MODE_BOOST
} operation_mode_e;

// 定义输入结构体
typedef struct
{
    float gain; // 输入增益
} input_t;

// 定义中间状态结构体
typedef struct
{
    operation_mode_e mode; // 当前模式
} inter_t;

// 定义输出结构体
typedef struct
{
    float buck_duty;  // Buck 占空比
    float boost_duty; // Boost 占空比
} output_t;

// 定义主结构体
typedef struct
{
    input_t input;   // 输入结构体
    inter_t inter;   // 中间状态结构体
    output_t output; // 输出结构体
} open_loop_t;

void open_loop_init(open_loop_t *control);

void open_loop_control(open_loop_t *control);

#endif
