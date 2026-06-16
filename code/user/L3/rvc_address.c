/**
 * @file rvc_address.c
 * @brief RV-C 地址分配核心实现
 * @note 基于 J1939 地址声明机制
 */

#include "rvc_address.h"
#include "rvc_address_config.h"
#include "stdbool.h"
#include "section.h"
#include "bsp_can.h"

extern uint32_t systemtime;
/* ========== 内部数据结构 ========== */

/**
 * @brief 地址管理器
 */
typedef struct {
    rvc_address_mode_t mode;            /**< 地址模式 */
    rvc_address_state_t state;          /**< 当前状态 */
    uint8_t current_address;            /**< 当前地址 */
    uint8_t claimed_address;            /**< 已声明的地址 */
    rvc_name_t name;                    /**< NAME 字段 */
    uint32_t claim_timestamp;           /**< 声明时间戳 */
    bool pending_claim;                 /**< 待发送声明 */
} address_manager_t;

/* ========== 全局变量 ========== */

static address_manager_t g_addr_mgr;

/**
 * @brief 初始化 NAME 字段
 */
static void init_name(rvc_name_t *name, rvc_address_mode_t mode)
{
    memset(name, 0, sizeof(rvc_name_t));

    name->serial_number = RVC_DEVICE_SERIAL_NUMBER & 0x1FFFFF;
    name->manufacturer_code = RVC_MANUFACTURER_CODE & 0x7FF;
    name->node_instance = 0xFF;  // 已废弃
    name->compatibility[0] = 0x00;
    name->compatibility[1] = 0x00;
    name->arbitrary_capable = (mode == RVC_ADDRESS_MODE_DYNAMIC) ? 1 : 0;
    name->reserved = 0;

}

/**
 * @brief 比较两个 NAME 字段
 * @return -1: name1 < name2 (name1 优先级高)
 *          0: name1 == name2
 *          1: name1 > name2 (name2 优先级高)
 * @note 按大端序比较（从高字节到低字节）
 */
static int compare_name(const rvc_name_t *name1, const rvc_name_t *name2)
{
    // 按字节从高到低比较（大端序）
    const uint8_t *p1 = (const uint8_t *)name1 + 7;
    const uint8_t *p2 = (const uint8_t *)name2 + 7;

    for (int i = 0; i < 8; i++) {
        if (*p1 < *p2) return -1;  // name1 优先级高
        if (*p1 > *p2) return 1;   // name2 优先级高
        p1--;
        p2--;
    }
    return 0;  // 相等
}

/**
 * @brief 构造 CAN ID
 * @param priority 优先级（0-7）
 * @param dgn DGN（17-bit）
 * @param sa 源地址（8-bit）
 * @return CAN ID（29-bit）
 *
 * CAN ID 结构（29-bit）：
 *   Bit 28-26: Priority (3-bit)
 *   Bit 25:    Reserved (1-bit, always 0)
 *   Bit 24-16: DGN-High (9-bit)
 *   Bit 15-8:  DGN-Low (8-bit)
 *   Bit 7-0:   SA (8-bit)
 */
static uint32_t build_can_id(uint8_t priority, uint32_t dgn, uint8_t sa)
{
    uint32_t dgn_high = (dgn >> 8) & 0x1FF;  // DGN 高 9 位
    uint32_t dgn_low = dgn & 0xFF;           // DGN 低 8 位

    return ((uint32_t)priority << 26) |
           (dgn_high << 16)           |
           (dgn_low << 8)             |
           sa;
}

/**
 * @brief 发送 ADDRESS_CLAIM
 */
static uint8_t send_address_claim(uint8_t sa)
{
    uint32_t can_id = build_can_id(RVC_ADDRESS_CLAIM_PRIORITY,
                                    RVC_ADDRESS_CLAIM_DGN,
                                    RVC_GLOBAL_ADDRESS);

    uint8_t data[8];
    memcpy(data, &g_addr_mgr.name, 8);

    return bsp_rvc_can_tx(can_id, data, 8);
}

/**
 * @brief 尝试声明地址
 */
static uint8_t try_claim_address(uint8_t address)
{
    g_addr_mgr.current_address = address;
    g_addr_mgr.state = RVC_ADDR_STATE_WAIT_CLAIM;
    g_addr_mgr.pending_claim = true;

    return send_address_claim(address);
}

/**
 * @brief 动态地址申请（143 → 128）
 */
static bool claim_dynamic_address(void)
{
    for (uint8_t addr = RVC_DYNAMIC_START_ADDRESS;
         addr >= RVC_DYNAMIC_END_ADDRESS;
         addr--) {
        if (try_claim_address(addr)) {
            return true;
        }
    }

    // 无可用地址
    g_addr_mgr.state = RVC_ADDR_STATE_CANNOT_CLAIM;
    g_addr_mgr.current_address = RVC_NULL_ADDRESS;
    return false;
}

/**
 * @brief 处理地址冲突
 */
static void handle_address_conflict(uint8_t sa, const rvc_name_t *received_name)
{
    // 比较 NAME 优先级
    int cmp = compare_name(&g_addr_mgr.name, received_name);

    if (cmp < 0) {
        // 我的 NAME 更小（优先级更高），重新声明地址
        send_address_claim(g_addr_mgr.current_address);
    } else {
        // 对方优先级更高，我必须让步

        if (g_addr_mgr.mode == RVC_ADDRESS_MODE_STATIC) {
            // 静态地址冲突：无法解决
            g_addr_mgr.state = RVC_ADDR_STATE_CANNOT_CLAIM;
            g_addr_mgr.current_address = RVC_NULL_ADDRESS;
        } else {
            // 动态地址：尝试下一个地址
            uint8_t next_addr = g_addr_mgr.current_address - 1;
            if (next_addr >= RVC_DYNAMIC_END_ADDRESS) {
                try_claim_address(next_addr);
            } else {
                g_addr_mgr.state = RVC_ADDR_STATE_CANNOT_CLAIM;
                g_addr_mgr.current_address = RVC_NULL_ADDRESS;
            }
        }
    }
}

/* ========== 公共 API 实现 ========== */

/**
 * @brief 初始化地址管理器
 */
void rvc_address_init(rvc_address_mode_t mode)
{
    memset(&g_addr_mgr, 0, sizeof(g_addr_mgr));

    g_addr_mgr.mode = mode;
    g_addr_mgr.state = RVC_ADDR_STATE_INIT;
    g_addr_mgr.current_address = RVC_NULL_ADDRESS;
    g_addr_mgr.claimed_address = RVC_NULL_ADDRESS;

    init_name(&g_addr_mgr.name, mode);

}

/**
 * @brief 启动地址声明流程
 */
uint8_t rvc_address_claim_start(void)
{
    if (g_addr_mgr.mode == RVC_ADDRESS_MODE_STATIC) {
        // 静态地址：直接声明
        return try_claim_address(RVC_STATIC_ADDRESS);
    } else {
        // 动态地址：从 143 开始申请
        return claim_dynamic_address();
    }
}

/**
 * @brief 周期调用处理函数
 */
void rvc_address_process(void)
{
    uint32_t now = systemtime;

    switch (g_addr_mgr.state) {
    case RVC_ADDR_STATE_WAIT_CLAIM:
        // 发送 ADDRESS_CLAIM 后，进入验证等待
        g_addr_mgr.state = RVC_ADDR_STATE_WAIT_VERIFY;
        g_addr_mgr.claim_timestamp = now;
        break;
    case RVC_ADDR_STATE_WAIT_VERIFY:
        // 等待 250ms，无冲突则声明成功
        if (now - g_addr_mgr.claim_timestamp >= RVC_ADDRESS_VERIFY_TIMEOUT) {
            g_addr_mgr.state = RVC_ADDR_STATE_CLAIMED;
            g_addr_mgr.claimed_address = g_addr_mgr.current_address;

            // 配置 CAN 过滤器
            //rvc_can_filter_config(g_addr_mgr.claimed_address);
        }
        break;

    case RVC_ADDR_STATE_CLAIMED:
        // 地址已声明，正常运行
        break;

    case RVC_ADDR_STATE_CANNOT_CLAIM:
        // 无法声明地址，停止工作
        break;

    default:
        break;
    }
}

/**
 * @brief 获取当前地址
 */
uint8_t rvc_address_get_current(void)
{
    return g_addr_mgr.current_address;
}

/**
 * @brief 检查地址是否已声明
 */
uint8_t rvc_address_is_claimed(void)
{
    return (g_addr_mgr.state == RVC_ADDR_STATE_CLAIMED);
}

/**
 * @brief 获取当前状态
 */
rvc_address_state_t rvc_address_get_state(void)
{
    return g_addr_mgr.state;
}

/**
 * @brief CAN 接收回调函数
 */
void rvc_address_can_rx_callback(uint32_t can_id, uint8_t *data, uint8_t len)
{
    // 提取 DGN 和 SA
    uint16_t dgn = (can_id >> 8) & 0x1FFFF;
    uint8_t sa = can_id & 0xFF;

    // 只处理 ADDRESS_CLAIM
    if (dgn != RVC_ADDRESS_CLAIM_DGN) {
        return;
    }

    // 解析 NAME 字段
    rvc_name_t received_name;
    memcpy(&received_name, data, 8);

    // 检查是否是自己发的
    if (compare_name(&received_name, &g_addr_mgr.name) == 0) {
        // 是自己的 ADDRESS_CLAIM，忽略
        return;
    }


    // 检查地址冲突
    if (sa == g_addr_mgr.current_address) {
        handle_address_conflict(sa, &received_name);
    }
}

/**
 * @brief 获取 NAME 字段
 */
void rvc_address_get_name(rvc_name_t *name)
{
    if (name != NULL) {
        memcpy(name, &g_addr_mgr.name, sizeof(rvc_name_t));
    }
}



void address_init(void)
{
    rvc_address_init(RVC_ADDRESS_MODE_STATIC);

    /* 启动地址声明流程 */
    if (!rvc_address_claim_start()) {
        // 声明异常
    }
}

//REG_INIT_TP(address_init)

/**
 * @brief 发送 CHARGER_STATUS 示例
 */
void send_charger_status(uint8_t my_addr)
{
    static uint32_t last_send_time = 0;
    uint32_t now = systemtime;

    // 每 5000ms 发送一次（空闲状态）
    if (now - last_send_time < 5000) {
        return;
    }
    last_send_time = now;

    // 构造 CAN ID
    // Priority=6, DGN=0x1FFC7, DGN-Low=0xFF（广播）
    uint32_t can_id = (6 << 26) | (0x1FF << 17) | (0xC7 << 8) | 0xFF;

    // 构造数据
    uint8_t data[8];
    data[0] = 1;                    // Instance
    data[1] = 0xE0;                 // Charge Voltage Low (24.0V)
    data[2] = 0x01;                 // Charge Voltage High
    data[3] = 0x64;                 // Charge Current Low (10.0A)
    data[4] = 0x00;                 // Charge Current High
    data[5] = 0x50;                 // Charge Current % (80%)
    data[6] = 0x02;                 // Operating State (Bulk)
    data[7] = 0x01;                 // Default state on power-up (Enabled)

    // 发送
    bsp_rvc_can_tx(can_id, data, 8);
}

//void rvc_address_run(void)
//{
//    /* 周期调用地址管理器（10ms） */
//    rvc_address_process();
//    
//    /* 检查地址状态 */
//    if (rvc_address_is_claimed()) {
//        // 地址已声明，可以发送其他 RV-C 消息
//        uint8_t my_addr = rvc_address_get_current();

//        // 示例：发送 CHARGER_STATUS
//        send_charger_status(my_addr);

//        // 正常运行

//    } else if (rvc_address_get_state() == RVC_ADDR_STATE_CANNOT_CLAIM) {
//        // 地址冲突，无法声明地址

//        // 错误状态

//        // 停止工作，等待人工处理
//        
//    } else {
//        // 正在声明地址
//    }

//}

//REG_TASK(10, rvc_address_run)







