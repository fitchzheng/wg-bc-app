#ifndef __WG_COM_V2_H
#define __WG_COM_V2_H

#include "stdint.h"
#include "my_math.h"
#include"bsp_usart.h"
#define LMT_MAP_SIZE (sizeof(lmt_map) / sizeof(lmt_map[0]))

#define INT_TYPE     1
#define UINT_TYPE    0

#define WG_COM_V2_GET_DATA(user_data, wg_com_v2_data,type)                                     \
    do                                                                                         \
    {                                                                                          \
        if ((type) == UINT_TYPE)                                                               \
        {                                                                                      \
            (user_data) = wg_com_v2_get_data_uint((float)(user_data), (void *)&(wg_com_v2_data)); \
        }                                                                                      \
        else                                                                                   \
        {                                                                                      \
            (user_data) = wg_com_v2_get_data_int((float)(user_data), (void *)&(wg_com_v2_data));  \
        }                                                                                      \
    } while (0)

#define WG_COM_V2_GET_DATA_UINT(user_data, wg_com_v2_data) \
    (user_data) = wg_com_v2_get_data_uint((float)(user_data), (void *)&(wg_com_v2_data))

#define WG_COM_V2_GET_DATA_INT(user_data, wg_com_v2_data) \
    (user_data) = wg_com_v2_get_data_int((float)(user_data), (void *)&(wg_com_v2_data))


#define WG_COM_V2_SET_DATA(user_data, wg_com_v2_data,type)                                     \
    do                                                                                         \
    {                                                                                          \
        if ((type) == UINT_TYPE)                                                               \
        {                                                                                      \
            wg_com_v2_set_data_uint((float)(user_data), (void *)&(wg_com_v2_data));            \
        }                                                                                      \
        else                                                                                   \
        {                                                                                      \
            wg_com_v2_set_data_int((float)(user_data), (void *)&(wg_com_v2_data));             \
        }                                                                                      \
    } while (0)

#define WG_COM_V2_SET_DATA_UINT(user_data, wg_com_v2_data) \
    wg_com_v2_set_data_uint((float)(user_data), (void *)&(wg_com_v2_data))

#define WG_COM_V2_SET_DATA_INT(user_data, wg_com_v2_data) \
    wg_com_v2_set_data_int((float)(user_data), (void *)&(wg_com_v2_data))

#define WG_COM_V2_BUFFER_SIZE 256

#define WG_COM_V2_BROADCAST_ADDR 0xFF
#define WG_COM_V2_HOST_ADDR 0x01

#define WG_COM_V2_CMD_READ 0x03
#define WG_COM_V2_CMD_WRITE_DATA 0x06
#define WG_COM_V2_CMD_WRITE_STR 0x10

#define MODBUS_MIN_FRAME_LEN 5 // 地址(1) + 功能码(1) + 起始寄存器(2) + CRC(2)

#define USART0_DELAY_CONT   0xffff

#define DEFINE_ADDR_REGION(offset, var)             \
    {                                               \
        .start_addr = (offset),                     \
        .end_addr = (offset) + sizeof(var) / 2 - 1, \
        .data_ptr = &(var)}

typedef struct
{
    void *addr;
    float unit;
} realtime_data_unit_map_t;

typedef struct
{
    void *addr;
    uint16_t up_lmt;
    uint16_t dn_lmt;
} wg_com_v2_data_lmt_map_t;

// 地址区域描述结构体
typedef struct
{
    uint16_t start_addr;
    uint16_t end_addr;
    void *data_ptr; // 数据结构起始地址
} addr_region_t;

#pragma pack(1)

#define WG_COM_V2_PRUCUCT_INFO_ADDR 0x0
typedef struct
{
    uint16_t ProtocolVersion[2];   // 协议版本
    uint16_t ProductType[2];       // 产品类型
    uint16_t HardverVerzi[2];      // 硬件版本
    uint16_t SoftVersion[2];       // 软件版本
    uint16_t SnSerial[10];         // SN序列号
    uint16_t ProductName[10];      // 产品名称
    uint16_t Address;              // 地址
    uint16_t ApplicationScenarios; // 应用场景
    uint16_t CustomizationVersion; // 协议定制
    uint16_t MacAddress[10];       // mac地址
    uint16_t BtName;               // 设置蓝牙名称
} wg_com_v2_product_info_t;

#define WG_COM_V2_REALTIME_DATA_ADDR 0x200
typedef struct
{
    uint16_t InpVolt;      // A端电压
    uint16_t InpCurr;      // A端电流
    uint16_t InpCurrPower; // A端功率
    uint16_t OutVolt;      // B端电压
    uint16_t OutCurr;      // B端电流
    uint16_t OutCurrPower; // B端功率
    int16_t InsideTemp;   // 内部温度
    int16_t OutsideTemp;  // 外部温度
    uint16_t PowerMode;    // 电源模式
    uint16_t ChargMode;    // 充电模式
    uint16_t FaultSign;    // 故障信号
    uint16_t AlarmSign;    // 告警信号
    uint16_t CompensationVoltA; // A端补偿
    uint16_t CompensationVoltB; // B端补偿
    int16_t Temp2;        // 器件温度2
    uint16_t StateCharge;  // 充电状态  
    uint16_t ADDVolt;      // ADD辅源电压
} wg_com_v2_realtime_data_t;

#define WG_COM_V2_CTRL_ADDR 0x400
typedef struct
{
    uint16_t FactoryReset;        // 400: 恢复出厂设置
    uint16_t PowerOnOff;          // 401: 开关机状态
    uint16_t SetPowerMode;        // 402: 电源模式
    uint16_t SetChargMode;        // 403: 充电模式
    uint16_t InpBatyType;         // 404: A端电池类型（高8位类型，低8位电压）
    uint16_t OutBatyType;         // 405: B端电池类型（高8位类型，低8位电压）
    uint16_t SetBootTimeA;        // 406: A端开机时间
    uint16_t SetBootTimeB;        // 407: B端开机时间
    uint16_t SetOnCurrStartTimeA; // 408: A端开机电流软起动时间
    uint16_t SetOnCurrStartTimeB; // 409: B端开机电流软起动时间
    uint16_t ZeroCurrCalibration; // 40A: 端零电流校准
    uint16_t ResetFactoryData;    // 40B: 恢复厂家数据
    uint16_t BatModeFR;           // 40C: 电池模式正反向切换
    uint16_t MpptSwitch;          // MPPT开关状态
    uint16_t SleepModeOnOff;      // 休眠功能
} wg_com_v2_ctrl_t;

#define WG_COM_V2_PARAM_ADDR 0x800
typedef struct
{
    uint16_t InpVoltCalibrK;     // A端电压校准K值
    uint16_t InpVoltCalibrB;     // A端电压校准B值
    uint16_t InpCurrCalibrK;     // A端电流校准K值
    uint16_t InpCurrCalibrB;     // A端电流校准B值
    uint16_t InpShowVoltCalibrK; // A端显示电压校准K值
    uint16_t InpShowVoltCalibrB; // A端显示电压校准B值
    uint16_t InpShowCurrCalibrK; // A端显示电流校准K值
    uint16_t InpShowCurrCalibrB; // A端显示电流校准B值
    uint16_t OutVoltCalibrK;     // B端电压校准K值
    uint16_t OutVoltCalibrB;     // B端电压校准B值
    uint16_t OutCurrCalibrK;     // B端电流校准K值
    uint16_t OutCurrCalibrB;     // B端电流校准B值
    uint16_t OutShowVoltCalibrK; // B端显示电压校准K值
    uint16_t OutShowVoltCalibrB; // B端显示电压校准B值
    uint16_t OutShowCurrCalibrK; // B端显示电流校准K值
    uint16_t OutShowCurrCalibrB; // B端显示电流校准B值
    uint16_t BOutShowCurrCalibrK;// B端显示电流校准K值	
    uint16_t BOutShowCurrCalibrB;// B端显示电流校准B值	
    uint16_t AOutShowCurrCalibrK;// A端显示电流校准K值	
    uint16_t AOutShowCurrCalibrB;// A端显示电流校准B值	
    uint16_t VoltCompensationAK; // ACC/A端电压补偿校准K值	
    uint16_t VoltCompensationAB; // ACC/A端电压补偿校准B值	
    uint16_t VoltCompensationBK; // RTM/B端电压补偿校准K值	
    uint16_t VoltCompensationBB; // RTM/B端电压补偿校准B值	
    uint16_t Retain10[2];        // 保留
    uint16_t SetInpVolt;         // A端电压
    uint16_t SetInpCurr;         // A端电流
    uint16_t SetInpCurrPower;    // A端功率
    uint16_t SetOutVolt;         // B端电压
    uint16_t SetOutCurr;         // B端电流
    uint16_t SetOutCurrPower;    // B端功率
    uint16_t SetInpUvlo;         // A端欠压保护
    uint16_t SetInpUvloRecover;  // A端欠压保护恢复
    uint16_t SetInpOVP;          // A端过压保护
    uint16_t SetInpOVPRecover;   // A端过压保护恢复
    uint16_t SetOutUvlo;         // B端欠压保护
    uint16_t SetOutUvloRecover;  // B端欠压保护恢复
    uint16_t SetOutOVP;          // B端过压保护
    uint16_t SetOutOVPRecover;   // B端过压保护恢复
    int16_t SetInsideTemp;      // 内部温度
    int16_t SetOutsideTemp;     // 外部温度
    uint16_t SetInpChargLedCurr; // A端充电指示灯电流
    uint16_t SetInpFullLedCurr;  // A端充满指示灯电流
    uint16_t SetOutChargLedCurr; // B端充电指示灯电流
    uint16_t SetOutFullLedCurr;  // B端充满指示灯电流
    uint16_t AuotForwardOpenVoltA; // 自动模式正向A端开启电压
    uint16_t AuotForwardVeerVoltA; // 自动模式正向转向A电压
    uint16_t AuotForwardShutVoltA; // 自动模式正向A端关闭电压
    uint16_t AuotReverseOpenVoltB; // 自动模式反向B端开启电压
    uint16_t AuotReverseShutVoltB; // 自动模式反向B端关闭电压
    int16_t SetTemp2;             // 内部温度
} wg_com_v2_param_t;

#pragma pack()

float get_unit_for_addr(void *p);

const wg_com_v2_data_lmt_map_t *get_lmt_for_addr(void *p);
float wg_com_v2_get_data_uint(float user_data, void *wg_com_v2_data);
float wg_com_v2_get_data_int(float user_data, void *wg_com_v2_data);
void wg_com_v2_set_data_uint(float user_data, void *wg_com_v2_data);
void wg_com_v2_set_data_int(float user_data, void *wg_com_v2_data);

uint16_t get_uint16(uint8_t *p_data);
int16_t get_int16(uint8_t *p_data);
void set_uint16(uint8_t *p_data, uint16_t data);
void set_int16(uint8_t *p_data, uint16_t data);

extern wg_com_v2_product_info_t wg_com_v2_product_info;
extern wg_com_v2_realtime_data_t wg_com_v2_realtime_data;
extern wg_com_v2_ctrl_t wg_com_v2_ctrl;
extern wg_com_v2_param_t wg_com_v2_param;

typedef struct
{
    volatile uint8_t *dma_buffer;
    uint32_t buffer_size;
    uint32_t dma_channel;
    uint32_t *rx_cnt;
    uint8_t *is_rx_flag;
    uint32_t *timeout;
    const char *tag;
    void (*my_printf)(const char *fmt, ...); // 串口专用 printf 接口
    usart_output_port_t USARTx;
} usart_dma_port_t;

typedef struct
{
    uint32_t buffer_size;
    uint8_t  usart_buf[WG_COM_V2_BUFFER_SIZE];
    uint8_t  rx_step;
    uint8_t  rx_data_step;
    char     bt_name[28];
} usart_dma_bt_buf_t;

#endif
