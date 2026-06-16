#ifndef __RVC_ADDRESS_CONFIG_H__
#define __RVC_ADDRESS_CONFIG_H__

#include "my_math.h"

/* ========== 地址模式配置 ========== */

/**
 * 地址模式选择
 * - RVC_ADDRESS_MODE_STATIC: 静态地址（单台设备）
 * - RVC_ADDRESS_MODE_DYNAMIC: 动态地址（多台并联）
 */
#define RVC_ADDRESS_MODE_DEFAULT    RVC_ADDRESS_MODE_STATIC

/* ========== 设备信息配置 ========== */

/**
 * 制造商码（11-bit）
 * 开发阶段：使用测试值 0x001
 * 量产前：向 RVIA 或 SAE 申请正式码
 * 申请地址：https://www.rv-c.com/
 */
#define RVC_MANUFACTURER_CODE       0x001

/**
 * 设备序列号（21-bit）
 * 开发阶段：使用固定值
 * 量产：从 MCU 唯一 ID 或 EEPROM 读取
 */
#define RVC_DEVICE_SERIAL_NUMBER    0x12345

/* ========== 静态地址配置 ========== */

/**
 * 静态地址定义
 * RV-C 规范为 Charger 分配了 3 个静态地址：
 * - 74 (0x4A): 第一个充电器
 * - 75 (0x4B): 第二个充电器
 * - 76 (0x4C): 第三个充电器
 */
#define RVC_STATIC_ADDRESS          74

/* ========== 动态地址配置 ========== */

/**
 * 动态地址范围：143 → 128
 * 按照 J1939 规范，从高地址向低地址申请
 */
#define RVC_DYNAMIC_START_ADDRESS   143
#define RVC_DYNAMIC_END_ADDRESS     128

/* ========== 协议参数 ========== */

/**
 * ADDRESS_CLAIM DGN
 */
#define RVC_ADDRESS_CLAIM_DGN       0xEE00

/**
 * ADDRESS_CLAIM 优先级
 */
#define RVC_ADDRESS_CLAIM_PRIORITY  6

/**
 * 全局广播地址
 */
#define RVC_GLOBAL_ADDRESS          0xFF

/**
 * 无效地址
 */
#define RVC_NULL_ADDRESS            0xFE

/**
 * 地址验证等待时间（ms）
 * 规范要求：发送 ADDRESS_CLAIM 后等待 250ms
 */
#define RVC_ADDRESS_VERIFY_TIMEOUT  250

/* ========== 调试配置 ========== */

/**
 * 使能调试日志输出
 * 0: 禁用
 * 1: 使能
 */
#define RVC_ADDRESS_DEBUG_ENABLE    1

#endif



