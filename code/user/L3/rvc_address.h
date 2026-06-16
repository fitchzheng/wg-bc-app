#ifndef __RVC_ADDRESS_H__
#define __RVC_ADDRESS_H__

#include "my_math.h"

/* ========== 类型定义 ========== */

/**
 * @brief 地址模式
 */
typedef enum {
    RVC_ADDRESS_MODE_STATIC = 0,    /**< 静态地址（Arbitrary Address Capable = 0） */
    RVC_ADDRESS_MODE_DYNAMIC = 1    /**< 动态地址（Arbitrary Address Capable = 1） */
} rvc_address_mode_t;

/**
 * @brief 地址声明状态
 */
typedef enum {
    RVC_ADDR_STATE_INIT,            /**< 初始化 */
    RVC_ADDR_STATE_WAIT_CLAIM,      /**< 等待发送 ADDRESS_CLAIM */
    RVC_ADDR_STATE_WAIT_VERIFY,     /**< 等待验证（250ms） */
    RVC_ADDR_STATE_CLAIMED,         /**< 地址已声明 */
    RVC_ADDR_STATE_CONFLICT,        /**< 地址冲突 */
    RVC_ADDR_STATE_CANNOT_CLAIM     /**< 无法声明地址 */
} rvc_address_state_t;

/**
 * @brief NAME 字段结构（64-bit）
 * @note 按照 J1939 规范定义
 */
typedef struct __attribute__((packed)) {
    uint32_t serial_number : 21;        /**< 序列号（bit 0-20） */
    uint32_t manufacturer_code : 11;    /**< 制造商码（bit 21-31） */
    uint8_t  node_instance;             /**< 节点实例（已废弃，填 0xFF） */
    uint8_t  compatibility[2];          /**< 兼容性字段（填 0x00） */
    uint8_t  arbitrary_capable : 1;     /**< 动态地址能力（bit 63） */
    uint8_t  reserved : 7;              /**< 保留（bit 56-62） */
} rvc_name_t;


void rvc_address_init(rvc_address_mode_t mode);
void rvc_address_can_rx_callback(uint32_t can_id, uint8_t *data, uint8_t len);
void rvc_address_get_name(rvc_name_t *name);
void rvc_address_process(void);
uint8_t rvc_address_is_claimed(void);
uint8_t rvc_address_get_current(void);
uint8_t rvc_address_claim_start(void);
rvc_address_state_t rvc_address_get_state(void);
#endif

