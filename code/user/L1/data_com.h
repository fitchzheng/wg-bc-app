#ifndef __DATA_COM_H
#define __DATA_COM_H

#include "stdint.h"

// 下发命令
#define WG_COM_CMD_GET_OUTPUT_INFO 0x03       // 获取模块输出电压、电流、告警量
#define WG_COM_CMD_CONTROL_SWITCH 0x04        // 控制开关机
#define WG_COM_CMD_SET_OUTPUT_PARAMS 0x06     // 设置模块输出电压、电流、功率
#define WG_COM_CMD_GET_SET_PARAMS 0x07        // 获取模块电压电流功率设置参数
#define WG_COM_CMD_SET_OVERTEMP_PROTECT 0x32  // 设置模块过温保护点
#define WG_COM_CMD_GET_OVERTEMP_PROTECT 0x33  // 读取模块过温保护点
#define WG_COM_CMD_SET_UNDERVOLT_PROTECT 0x36 // 设置模块过欠压点
#define WG_COM_CMD_GET_UNDERVOLT_PROTECT 0x37 // 读取模块过欠压点
#define WG_COM_CMD_SET_LED_CURRENT 0x38       // 设置LED电流点
#define WG_COM_CMD_GET_LED_CURRENT 0x39       // 读取LED电流点

// 上传命令
#define WG_COM_CMD_RET_OUTPUT_INFO 0x83       // 返回模块输出电压、电流、告警量
#define WG_COM_CMD_RET_CONTROL_SWITCH 0x84    // 控制开关机（返回）
#define WG_COM_CMD_RET_ACK_OUTPUT_PARAMS 0x86 // 返回模块输出电压、电流、功率
#define WG_COM_CMD_RET_GET_SET_PARAMS 0x87    // 返回模块电压电流功率设置参数
#define WG_COM_CMD_RET_OVERTEMP_PROTECT 0x72  // 返回模块过温保护点
#define WG_COM_CMD_RET_GET_OVERTEMP 0x73      // 返回模块过温保护点（读取）
#define WG_COM_CMD_RET_SET_UNDERVOLT 0x76     // 返回设置模块过欠压点
#define WG_COM_CMD_RET_GET_UNDERVOLT 0x77     // 返回读取模块过欠压点
#define WG_COM_CMD_RET_SET_LED_CURRENT 0x78   // 返回设置LED电流点
#define WG_COM_CMD_RET_GET_LED_CURRENT 0x79   // 返回读取LED电流点
#define RATIO                          0.99f  // 滤波系数Ratio(0-1)
#pragma pack(1)
typedef struct
{
    float uvp;
    float uvp_hys;
    float ovp;
    float ovp_hys;
} volt_protect_t;

typedef struct
{
    uint8_t ack;                  // 应答结果(1 个字节，0 表示正常）
    uint16_t output_voltage;      // 模块输出电压 10mV
    uint16_t output_current;      // 模块输出电流 10mA
    uint16_t temperature1;        // 模块温度1
    uint16_t temperature2;        // 模块温度2
    uint16_t temperature3;        // 模块温度3
    uint8_t warning_flags;        // 模块告警量
    uint8_t protection_type;      // 模块保护类型
    uint16_t input_voltage;       // 模块输入电压 10mV
    uint16_t input_current;       // 模块输入电流 10mA
    uint16_t primary_temperature; // 原边温度
} output_info_t;

typedef struct
{
    uint8_t ack;            // 应答结果(1 个字节，0 表示正常）
    uint16_t rvs12_lmt;     // 模块输出电压限制 10mV
    uint16_t ilv_lmt;       // 模块输出电流限制 10mA
    uint16_t rvs12_pwr_lmt; // 模块输出功率限制 10mW
} output_params_ack_t;

typedef struct
{
    uint16_t uvp;     // 设置输入欠压点（2 个字节）
    uint16_t uvp_hys; // 设置输入欠压恢复点（2 个字节）
    uint16_t ovp;     // 设置输入过压恢复点（2 个字节）
    uint16_t ovp_hys; // 设置输入过压点（2 个字节）
} volt_protect_ret_t;


typedef struct
{
    uint16_t SetPowerMode;
    uint16_t SetChargMode;
    uint16_t InpBatyType;
    uint16_t OutBatyType;
    
} STATE_CONTROL_PARAMETER_T;

typedef struct
{
    float fvs48_lmt;
    float ihv_lmt;
    float rvs12_lmt;
    float ilv_lmt;
} filter_show_data_t;

#pragma pack()

float data_com_get_rvs12_lmt(void);

float data_com_get_ihv_lmt(void);

float data_com_get_ilv_lmt(void);

float data_com_get_fvs48_lmt(void);

float data_com_get_fvs48_pwr_lmt(void);

float data_com_get_rvs12_pwr_lmt(void);

float data_com_get_open_loop_gain(void);

uint8_t data_com_get_open_loop_mode(void);

float Get_Set_Out_Curr_Value_Lmt(void);

#endif
