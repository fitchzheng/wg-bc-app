/**
 * @file rvc_message_handler.h
 * @brief RV-C 消息处理接口
 * @note 处理除 ADDRESS_CLAIM 外的其他 DGN
 */

#ifndef __RVC_MESSAGE_HANDLER_H__
#define __RVC_MESSAGE_HANDLER_H__

#include "my_math.h"


#define CAN_ON_OFF                                         2
/* ========== DGN 定义 ========== */
#define RVC_DGN_DM_RV                                      0x1FECA

// 充电器相关 DGN
#define RVC_DGN_CHARGER_COMMAND                            0x1FFC5
#define RVC_DGN_CHARGER_CONFIGURATION_COMMAND              0x1FFC4
#define RVC_DGN_CHARGER_CONFIGURATION_COMMAND_3            0x1FECB
#define RVC_DGN_REQUEST_FOR_DGN                            0xEA00
#define RVC_DGN_ACKNOWLEDGEMENT                            0xE800

// 充电器状态 DGN（用于发送）
#define RVC_DGN_CHARGER_STATUS                             0x1FFC7
#define RVC_DGN_CHARGER_STATUS_2                           0x1FEA3
#define RVC_DGN_CHARGER_CONFIGURATION_STATUS               0x1FFC6
#define RVC_DGN_CHARGER_CONFIGURATION_STATUS_3             0x1FECC
#define RVC_DGN_CHARGER_PROPERTIES                         0x1FDAA

/* ========== 私有 DGN ========== */

#define RVC_DGN_PROPRIETARY_PROTOCOL_VERSION_R             0xEF00  // 私有 DGN 协议版本
#define RVC_DGN_PROPRIETARY_PRODUCT_TYPE_R                 0xEF02  // 私有 DGN 产品类型
#define RVC_DGN_PROPRIETARY_HARDVER_VERSION_R              0xEF04  // 私有 DGN 硬件版本
#define RVC_DGN_PROPRIETARY_SOFT_VERSION_R                 0xEF06  // 私有 DGN 软件版本
#define RVC_DGN_PROPRIETARY_SN_SERIAL_R                    0xEF08  // 私有 DGN SN序列号
#define RVC_DGN_PROPRIETARY_SN_SERIAL_W                    0xEF09  // 私有 DGN SN序列号
#define RVC_DGN_PROPRIETARY_PRODUCT_NAME_R                 0xEF0A  // 私有 DGN 产品名称
#define RVC_DGN_PROPRIETARY_PRODUCT_NAME_W                 0xEF0B  // 私有 DGN 产品名称
#define RVC_DGN_PROPRIETARY_ADDRE_APPLI_R                  0xEF0C  // 私有 DGN 地址/应用场景
#define RVC_DGN_PROPRIETARY_ADDRE_APPLI_W                  0xEF0D  // 私有 DGN 地址/应用场景
#define RVC_DGN_PROPRIETARY_CUST_BT_NAME_R                 0xEF0E  // 私有 DGN 协议定制/设置蓝牙名称
#define RVC_DGN_PROPRIETARY_CUST_BT_NAME_W                 0xEF0F  // 私有 DGN 协议定制/设置蓝牙名称
#define RVC_DGN_PROPRIETARY_MAC_ADDRESS_R                  0xEF10  // 私有 DGN mac地址
#define RVC_DGN_PROPRIETARY_MAC_ADDRESS_W                  0xEF11  // 私有 DGN mac地址
#define RVC_DGN_PROPRIETARY_AVOLT_ACURR_APOWER_R           0xEF12  // 私有 DGN A端电压/A端电流/A端功率
#define RVC_DGN_PROPRIETARY_BVOLT_BCURR_BPOWER_R           0xEF14  // 私有 DGN B端电压/B端电流/B端功率
#define RVC_DGN_PROPRIETARY_T1_T2_TA_R                     0xEF16  // 私有 DGN T1温度/T2温度/Ta温度
#define RVC_DGN_PROPRIETARY_POWER_CHARG_STATE_CHARGEE_R    0xEF18  // 私有 DGN 电源模式/充电模式/充电状态
#define RVC_DGN_PROPRIETARY_FAULT_SIGN_ALARM_SIGN_R        0xEF1A  // 私有 DGN 故障信号/告警信号
#define RVC_DGN_PROPRIETARY_VOLTA_VOLTB_ADDVOLT_R          0xEF1C  // 私有 DGN A端补偿/B端补偿/ADD辅源电压
#define RVC_DGN_PROPRIETARY_ON_Off_SETPOWER_SET_CHARG_R    0xEF1E  // 私有 DGN 开关机状态/电源模式/充电模式
#define RVC_DGN_PROPRIETARY_ON_Off_SETPOWER_SET_CHARG_W    0xEF1F  // 私有 DGN 开关机状态/电源模式/充电模式
#define RVC_DGN_PROPRIETARY_FACTORY_RESET_RESET_FACTORY_R  0xEF20  // 私有 DGN 恢复出厂设置/恢复厂家数据
#define RVC_DGN_PROPRIETARY_FACTORY_RESET_RESET_FACTORY_W  0xEF21  // 私有 DGN 恢复出厂设置/恢复厂家数据
#define RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTA_R   0xEF22  // 私有 DGN A端电池类型/A端开机时间/A端开机电流软起动时间
#define RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTA_W   0xEF23  // 私有 DGN A端电池类型/A端开机时间/A端开机电流软起动时间
#define RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTB_R   0xEF24  // 私有 DGN B端电池类型/B端开机时间/B端开机电流软起动时间
#define RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTB_W   0xEF25  // 私有 DGN B端电池类型/B端开机时间/B端开机电流软起动时间
#define RVC_DGN_PROPRIETARY_ZERO_CURR_BAT_MODE_FR_R        0xEF26  // 私有 DGN 零电流校准/电池模式正反向切换
#define RVC_DGN_PROPRIETARY_ZERO_CURR_BAT_MODE_FR_W        0xEF27  // 私有 DGN 零电流校准/电池模式正反向切换
#define RVC_DGN_PROPRIETARY_SET_AVOLT_ACURR_APOWER_R       0xEF28  // 私有 DGN A端电压/A端电流/A端功率
#define RVC_DGN_PROPRIETARY_SET_AVOLT_ACURR_APOWER_W       0xEF29  // 私有 DGN A端电压/A端电流/A端功率
#define RVC_DGN_PROPRIETARY_SET_BVOLT_BCURR_BPOWER_R       0xEF2A  // 私有 DGN B端电压/B端电流/B端功率
#define RVC_DGN_PROPRIETARY_SET_BVOLT_BCURR_BPOWER_W       0xEF2B  // 私有 DGN B端电压/B端电流/B端功率
#define RVC_DGN_PROPRIETARY_SET_AUVLO_AUVLO_RECOVER_R      0xEF2C  // 私有 DGN A端欠压保护/A端欠压保护恢复
#define RVC_DGN_PROPRIETARY_SET_AUVLO_AUVLO_RECOVER_W      0xEF2D  // 私有 DGN A端欠压保护/A端欠压保护恢复
#define RVC_DGN_PROPRIETARY_SET_AOVP_AUVLO_OVPRECOVER_R    0xEF2E  // 私有 DGN A端过压保护/A端过压保护恢复
#define RVC_DGN_PROPRIETARY_SET_AOVP_AUVLO_OVPRECOVER_W    0xEF2F  // 私有 DGN A端过压保护/A端过压保护恢复
#define RVC_DGN_PROPRIETARY_SET_BUVLO_BUVLO_RECOVER_R      0xEF30  // 私有 DGN B端欠压保护/B端欠压保护恢复
#define RVC_DGN_PROPRIETARY_SET_BUVLO_BUVLO_RECOVER_W      0xEF31  // 私有 DGN B端欠压保护/B端欠压保护恢复
#define RVC_DGN_PROPRIETARY_SET_BOVP_BUVLO_OVPRECOVER_R    0xEF32  // 私有 DGN B端过压保护/B端过压保护恢复
#define RVC_DGN_PROPRIETARY_SET_BOVP_BUVLO_OVPRECOVER_W    0xEF33  // 私有 DGN B端过压保护/B端过压保护恢复
#define RVC_DGN_PROPRIETARY_SET_T1_T2_TA_R                 0xEF34  // 私有 DGN T1温度/T2温度/Ta温度
#define RVC_DGN_PROPRIETARY_SET_T1_T2_TA_W                 0xEF35  // 私有 DGN T1温度/T2温度/Ta温度
#define RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_R    0xEF36  // 私有 DGN A端充电指示灯电流/A端充满指示灯电流
#define RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_W    0xEF37  // 私有 DGN A端充电指示灯电流/A端充满指示灯电流
#define RVC_DGN_PROPRIETARY_SET_BCHARG_BFULL_LED_CURR_R    0xEF38  // 私有 DGN B端充电指示灯电流/B端充满指示灯电流
#define RVC_DGN_PROPRIETARY_SET_BCHARG_BFULL_LED_CURR_W    0xEF39  // 私有 DGN B端充电指示灯电流/B端充满指示灯电流
#define RVC_DGN_PROPRIETARY_AUOT_OPEN_VEER_SHUT_VOLT_A_R   0xEF3A  // 私有 DGN 自动模式正向A端开启电压/自动模式正向转向A电压/自动模式正向A端关闭电压
#define RVC_DGN_PROPRIETARY_AUOT_OPEN_VEER_SHUT_VOLT_A_W   0xEF3B  // 私有 DGN 自动模式正向A端开启电压/自动模式正向转向A电压/自动模式正向A端关闭电压
#define RVC_DGN_PROPRIETARY_AUOT_OPEN_SHUT_VOLT_B_R        0xEF3C  // 私有 DGN 自动模式反向B端开启电压/自动模式反向B端关闭电压
#define RVC_DGN_PROPRIETARY_AUOT_OPEN_SHUT_VOLT_B_W        0xEF3D  // 私有 DGN 自动模式反向B端开启电压/自动模式反向B端关闭电压

/* ========== 数据结构 ========== */

/**
 * @brief DM_RV 数据结构
 */
typedef struct {
    uint8_t instance;           /**< 实例号 */
    uint8_t dsn;                /**< 诊断序列号（递增） */
    uint16_t spn;               /**< 故障参数号（0=无故障） */
    uint8_t fmi;                /**< 故障模式标识 */
    uint8_t occurrence_count;   /**< 故障发生次数 */
    uint8_t dsa;                /**< 诊断源地址（自己的 SA） */
} rvc_dm_rv_t;

/* ========== 公共 API ========== */

/**
 * @brief CHARGER_COMMAND 数据结构
 */
typedef struct {
    uint8_t instance;           /**< 实例号 */
    uint8_t status;             /**< 0=禁用, 1=使能 */
    uint16_t voltage;           /**< 目标电压（分辨率 0.05V） */
    uint16_t current;           /**< 目标电流（分辨率 0.05A） */
} rvc_charger_command_t;

/**
 * @brief REQUEST_FOR_DGN 数据结构
 */
typedef struct {
    uint32_t requested_dgn;     /**< 请求的 DGN */
    uint8_t instance;           /**< 实例号 */
    uint8_t sender_sa;          /**< 请求者地址 */
} rvc_request_for_dgn_t;

/* ========== 回调函数类型 ========== */

/**
 * @brief CHARGER_COMMAND 回调函数
 * @param cmd 命令数据
 */
typedef void (*rvc_charger_command_callback_t)(const rvc_charger_command_t *cmd);

/**
 * @brief REQUEST_FOR_DGN 回调函数
 * @param req 请求数据
 */
typedef void (*rvc_request_for_dgn_callback_t)(const rvc_request_for_dgn_t *req);



/**
 * @brief 发送 DM_RV
 * @param instance 实例号
 * @param spn 故障参数号（0=无故障）
 * @param fmi 故障模式标识
 * @return true: 发送成功, false: 发送失败
 */
uint8_t rvc_send_dm_rv(uint8_t instance, uint16_t spn, uint8_t fmi);
uint8_t rvc_send_ack(uint8_t ack_code, uint8_t instance, uint32_t dgn_acked, uint8_t target_sa);
void rvc_message_handler_process(uint32_t can_id, uint8_t *data, uint8_t len);
#endif

