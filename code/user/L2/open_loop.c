#include "open_loop.h"

// 初始化函数
void open_loop_init(open_loop_t *str)
{
    if (!str)
        return;

    // 初始化输入增益为 0
    str->input.gain = 0.0f;

    // 初始化模式为降压模式
    str->inter.mode = MODE_BUCK;

    // 初始化输出占空比为 0
    str->output.buck_duty = 0.0f;
    str->output.boost_duty = 0.0f;
}

// 开环控制函数
void open_loop_control(open_loop_t *str)
{
    if (!str)
        return;

    // 根据当前模式和增益判断下一个模式
    switch (str->inter.mode)
    {
    case MODE_BUCK:
        if (str->input.gain > BUCK_TO_BUCK_BOOST_THRESHOLD)
        {
            str->inter.mode = MODE_BUCK_BOOST;
        }
        break;

    case MODE_BUCK_BOOST:
        if (str->input.gain > BUCK_BOOST_TO_BOOST_THRESHOLD)
        {
            str->inter.mode = MODE_BOOST;
        }
        else if (str->input.gain < BUCK_BOOST_TO_BUCK_THRESHOLD)
        {
            str->inter.mode = MODE_BUCK;
        }
        break;

    case MODE_BOOST:
        if (str->input.gain < BOOST_TO_BUCK_BOOST_THRESHOLD)
        {
            str->inter.mode = MODE_BUCK_BOOST;
        }
        break;

    default:
        // 不可能到达这里
        str->inter.mode = MODE_BUCK;
        break;
    }

    // 根据模式确定占空比
    switch (str->inter.mode)
    {
    case MODE_BUCK:
        str->output.buck_duty = str->input.gain;
        str->output.boost_duty = 1.0f;
        break;

    case MODE_BUCK_BOOST:
        str->output.buck_duty = 0.8f;
        str->output.boost_duty = 0.8f / str->input.gain;
        break;

    case MODE_BOOST:
        str->output.buck_duty = 1.0f;
        str->output.boost_duty = 1.0f / str->input.gain;
        break;

    default:
        // 不可能到达这里
        str->output.buck_duty = 0.0f;
        str->output.boost_duty = 0.0f;
        break;
    }
}
