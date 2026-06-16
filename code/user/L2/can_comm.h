#ifndef __CAN_COMM_H
#define __CAN_COMM_H

#include "can_packet.h"

#define PROTNO_LAP 0x010   // Local Alarm Protocol EES协议内容
#define PROTNO_NCP 0x020   // Network Configuration Protocol EES协议内容
#define PROTNO_FSP 0x030   // Function State Protocol EES协议内容
#define PROTNO_MCP 0x040   // Measurement And Control Protocol EES协议内容
#define PROTNO_SMCP 0x041  // Simple Measurement and Control Protocol EES协议内容
#define PROTNO_PSP 0x050   // Power Statue Protocol EES协议内容
#define PROTNO_SCP 0x100   // Simple Configuration Protocol EES协议内容
#define PROTNO_PIP 0x110   // Product Information Protocol EES协议内容
#define PROTNO_TDP 0x1FE   // Test And Debuge Protocol EES协议内容
#define PROTNO_RFDBP 0x1C8 // Bootloader request first data block protocal Bootloader 第一包数据块请求协议
#define PROTNO_RNDBP 0x1C9 // Bootloader request next data block protocal Bootloader 下一包数据块请求协议
#define PROTNO_BDTP 0x1CA  // Bootloader download trigger protocol Bootloader 下载触发协议
#define PROTNO_RSNP 0x1CB  // Bootloader request serial number protocal Bootloader 请求序列号协议
#define PROTNO_RBVP 0x1CC  // Bootloader request boot version protocal Bootloader 请求引导区版本协议
#define PROTNO_HMP 0x1FD   // Host Monitor Protocol 监控模块与后台调试协议
#define PROTNO_RMP 0x060   // Rectifier Monitor Protocol 监控模块与整流模块协议
#define PROTNO_IMP 0x061   // Invert Monitor Protocol 监控模块与逆变模块协议
#define PROTNO_LDMP 0x062  // LC Distribution Monitor Protocol 监控模块与LC配电模块协议
#define PROTNO_LCMP 0x063  // LC for DC/DC Converter Monitor Protocol 监控模块与山特LC变换器协议
#define PROTNO_NACMP 0x064 // NA Converter Monitor Protocol 监控模块与北美变换器模块协议
#define PROTNO_RRP 0x070   // Rectifier Rectifier Protocol 整流模块之间协议
#define PROTNO_NACCP 0x071 // NA Converter Converter Protocol 北美变换器之间协议
#define PROTNO_WG 0x188    // WG Communication Protocol WG通信协议

#define ERRTYPE_OK 0xF0               // 无错误，正常响应
#define ERRTYPE_INVALID_ADDR 0xF1     // 节点地址无效
#define ERRTYPE_INVALID_CMD 0xF2      // 命令无效
#define ERRTYPE_CHECKSUM_FAIL 0xF3    // 数据校验错误
#define ERRTYPE_ADDR_RECOGNIZING 0xF4 // 地址识别过程中

#define MSGTYPE_CMD0_SET_REQ 0x00 // 设置请求数据 - 综合命令0
#define MSGTYPE_CMD1_SET_REQ 0x10 // 设置请求数据 - 综合命令1
#define MSGTYPE_CMD2_SET_REQ 0x20 // 设置请求数据 - 综合命令2

#define MSGTYPE_BYTE_READ_REQ 0x01  // 请求字节数据 - 按字节读取数据
#define MSGTYPE_BYTE_READ_RESP 0x41 // 应答请求 - 按字节读取数据

#define MSGTYPE_BIT_READ_REQ 0x02  // 请求位数据 - 按位读取数据
#define MSGTYPE_BIT_READ_RESP 0x42 // 应答请求 - 按位读取数据

#define MSGTYPE_BYTE_WRITE_REQ 0x03  // 设置数据 - 按字节设置数据
#define MSGTYPE_BYTE_WRITE_RESP 0x43 // 应答设置 - 按字节设置数据

#endif
