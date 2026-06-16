#ifndef __CAN_WG_H
#define __CAN_WG_H

#include "stdint.h"
#include "wg_com_v2.h"
#include "can_packet.h"

#define HOST_ADDR 0x01

#define CAN_WG_MSGTYPE_SET_MFG 0x00    // 设置厂家数据
#define CAN_WG_MSGTYPE_GET_MFG 0x01    // 获取厂家数据
#define CAN_WG_MSGTYPE_GET_RTDATA 0x02 // 获取实时数据
#define CAN_WG_MSGTYPE_SET_CTRL 0x03   // 设置控制数据
#define CAN_WG_MSGTYPE_GET_CTRL 0x04   // 获取控制数据
#define CAN_WG_MSGTYPE_SET_PARAM 0x05  // 设置参数数据
#define CAN_WG_MSGTYPE_GET_PARAM 0x06  // 获取参数数据

#define CAN_WG_MSGTYPE_VOLTCOMP 0x07 // 主机主动设置从机输出电流

#define CAN_WG_MFG_VALUETYPE_PROTOCOL 0x00             // 协议版本
#define CAN_WG_MFG_VALUETYPE_PRODUCT_TYPE 0x01         // 产品类型
#define CAN_WG_MFG_VALUETYPE_HARDWARE_VERSION 0x02     // 硬件版本
#define CAN_WG_MFG_VALUETYPE_SOFTWARE_VERSION 0x03     // 软件版本
#define CAN_WG_MFG_VALUETYPE_SN 0x04                   // SN序列号
#define CAN_WG_MFG_VALUETYPE_PRODUCT_NAME 0x05         // 产品名称
#define CAN_WG_MFG_VALUETYPE_ADDRESS 0x06              // 地址
#define CAN_WG_MFG_VALUETYPE_APPLICATION_SCENARIO 0x07 // 应用场景
#define CAN_WG_MFG_VALUETYPE_PROTOCOL_CUSTOM 0x08      // 协议定制

#define CAN_WG_CTRL_VALUETYPE_COMMON 0x00       // 通用控制
#define CAN_WG_CTRL_VALUETYPE_BATTERY_TYPE 0x01 // 电池类型
#define CAN_WG_CTRL_VALUETYPE_BOOT_TIME 0x02    // 开机时间
#define CAN_WG_CTRL_VALUETYPE_SOFT_START 0x03   // 软起时间

#define CAN_WG_CTRL_COMMON_CHARG_MODE_AUTO 0x02     // 自动正反向模式
#define CAN_WG_CTRL_COMMON_CHARG_MODE_BACKWARD 0x01 // 反向模式
#define CAN_WG_CTRL_COMMON_CHARG_MODE_FORWARD 0x00  // 正向模式

#define CAN_WG_CTRL_COMMON_POWER_MODE_BATTERY 0x01 // 电池模式
#define CAN_WG_CTRL_COMMON_POWER_MODE_NORMAL 0x00  // 正常模式

#define CAN_WG_CTRL_COMMON_POWER_ONOFF_OFF 0x01 // 关机状态
#define CAN_WG_CTRL_COMMON_POWER_ONOFF_ON 0x00  // 开机状态

#define CAN_WG_CTRL_COMMON_FACTORY_RESET_RESET 0x01  // 恢复出厂设置
#define CAN_WG_CTRL_COMMON_FACTORY_RESET_NORMAL 0x00 // 正常

typedef struct
{
    uint8_t valuetype;
    uint8_t offset;
} can_wg_mfg_byte2_3_t;

typedef struct
{
    uint8_t data[4];
} can_wg_mfg_byte4_7_t;

typedef struct
{
    can_wg_mfg_byte2_3_t byte2_3; // valuetype + offset
    can_wg_mfg_byte4_7_t byte4_7; // 数据内容，4字节
} can_wg_mfg_t;

typedef struct
{
    uint8_t data_offset;
    uint8_t data_num; // 数据个数
} can_wg_get_rtdata_byte2_3_t;

typedef struct
{
    uint8_t data[4]; // 实时数据内容，8字节
} can_wg_get_rtdata_byte4_7_t;

typedef struct
{
    can_wg_get_rtdata_byte2_3_t byte2_3; // 数据偏移 + 数据个数
    can_wg_get_rtdata_byte4_7_t byte4_7; // 实时数据内容
} can_wg_get_rtdata_t;

typedef struct
{
    uint8_t data_offset; // 数据偏移
    uint8_t data_num;    // 数据个数
    uint8_t data[4];
} can_wg_param_t;

typedef struct
{
    uint8_t factory_reset : 1; // bit0: 恢复出厂设置(1:复位, 0:正常)
    uint8_t power_onoff : 1;   // bit1: 开关机状态(1:关机, 0:开机)
    uint8_t power_mode : 1;    // bit2: 电源模式(1:电池, 0:正常)
    uint8_t charg_mode : 2;    // bit3~bit4: 充电模式(0b10:自动, 0b01:反向, 0b00:正向)
    uint8_t reserved : 3;      // bit5~bit7: 保留
} can_wg_ctrl_common_byte4_t;

typedef struct
{
    can_wg_ctrl_common_byte4_t byte4; // 控制字节
    uint8_t rsvd[3];                  // 保留字节
} can_wg_ctrl_common_t;

typedef struct
{
    uint8_t a_battery_type; // Byte 4: A端电池类型
    uint8_t a_battery_volt; // Byte 5: A端电池电压
    uint8_t b_battery_type; // Byte 6: B端电池类型
    uint8_t b_battery_volt; // Byte 7: B端电池电压
} can_wg_ctrl_battery_t;

typedef struct
{
    uint16_t a_boot_time; // Byte 4: A端开机时间
    uint16_t b_boot_time; // Byte 5: B端开机时间
} can_wg_ctrl_boot_time_t;

typedef struct
{
    uint16_t a_soft_start_time; // Byte 4~5: A端开机电流软起动时间
    uint16_t b_soft_start_time; // Byte 6~7: B端开机电流软起动时间
} can_wg_ctrl_soft_start_t;

void can_comm_wg_dispatch(const can_packet_t *packet);

void can_wg_set_volt_comp(float val);

void can_wg_get_volt_comp(float *val);
#endif
