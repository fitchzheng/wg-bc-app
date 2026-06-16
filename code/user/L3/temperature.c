#include "temperature.h"
#include "ntc_table.h"

static float cal_ntc_resistance(float volt, TEMPERATURE_MODE_E mode)
{
    float resistance = 0.0f;
    if (mode == IS_DN_NTC)
    {
        if ((VOLT_MAX - volt) > 0.01f)
        {
            resistance = (volt * FIXED_RES) / (VOLT_MAX - volt);
            //resistance = (VOLT_MAX/volt)*FIXED_RES-FIXED_RES;
        }
        else
        {
            resistance = 0.0f; // 错误处理
        }
    }
    else if (mode == IS_UP_NTC)
    {
        if (volt > 0.005f)
        {
            resistance = FIXED_RES * (VOLT_MAX - volt) / volt;
        }
        else
        {
            resistance = ntc_table[0];
        }
    }
    else
    {
        resistance = 0.0f; // 错误处理
    }
    return resistance;
}

/**
 * 二分查找区间：在降序数组 arr 中查找 target 所在的区间
 * 返回 i，使得 arr[i] ≥ target > arr[i+1]
 * 若 target 大于 arr[0]，返回 -1（超出上界）
 * 若 target 小于 arr[size-1]，返回 size-1（超出下界）
 */
static int binary_search_range(const float arr[], int size, float target)
{
    int left = 0;
    int right = size - 1;
    int mid = left + (right - left) / 2;

    if (target >= arr[0])
    {
        return 0; // 超出上限
    }
    else if (target <= arr[size - 1])
    {
        return size - 1; // 超出下限
    }

    while (left < right - 1)
    {
        mid = (left + right) / 2;
        if (target < arr[mid])
        {
            left = mid;
        }
        else
        {
            right = mid;
        }
    }

    return mid;
}

// 返回插值后温度（单位：℃）
float cal_ntc_temp(float volt, TEMPERATURE_MODE_E mode)
{
    float ntc_res = cal_ntc_resistance(volt, mode);
    int size = sizeof(ntc_table) / sizeof(ntc_table[0]);

    // 边界判断
    if (ntc_res >= ntc_table[0])
    {
        return 0.0f + NTC_OFFSET; // 超出上限（最低温）
    }
    else if (ntc_res <= ntc_table[size - 1])
    {
        return (float)(size - 1) + NTC_OFFSET; // 超出下限（最高温）
    }

    // 二分查找区间
    int mid = binary_search_range(ntc_table, size, ntc_res);

    // 线性插值计算温度
    float r0 = ntc_table[mid];
    float r1 = ntc_table[mid + 1];
    float t0 = (float)mid;
    float t1 = (float)(mid + 1);

    float temp = t1 - (ntc_res - r1) / (r0 - r1);
    return temp + NTC_OFFSET;
}
