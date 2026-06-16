#include "wg_com_v2.h"
#include "bsp_usart.h"
#include "section.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include <stdarg.h>
#include "bsp_dma.h"

extern uint32_t systemtime;

static uint8_t host_addr = WG_COM_V2_HOST_ADDR;

static uint8_t wg_com_tx_buffer[WG_COM_V2_BUFFER_SIZE];
static uint32_t wg_com_tx_buffer_cnt = 0;

static uint8_t wg_com_rx_buffer[WG_COM_V2_BUFFER_SIZE];
static uint32_t wg_com_rx_buffer_cnt = 0;

wg_com_v2_product_info_t wg_com_v2_product_info;
wg_com_v2_realtime_data_t wg_com_v2_realtime_data;
wg_com_v2_ctrl_t wg_com_v2_ctrl;
wg_com_v2_param_t wg_com_v2_param;
usart_dma_bt_buf_t usart_dma_bt_buf;

static const realtime_data_unit_map_t unit_map[] = {
    {&wg_com_v2_realtime_data.InpVolt, 0.01f},
    {&wg_com_v2_realtime_data.InpCurr, 0.01f},
    {&wg_com_v2_realtime_data.OutVolt, 0.01f},
    {&wg_com_v2_realtime_data.OutCurr, 0.01f},
    {&wg_com_v2_realtime_data.ADDVolt, 0.01f},
    {&wg_com_v2_realtime_data.CompensationVoltA, 0.01f},
    {&wg_com_v2_realtime_data.CompensationVoltB, 0.01f},
    {&wg_com_v2_param.SetInpVolt, 0.01f},
    {&wg_com_v2_param.SetInpCurr, 0.01f},
    {&wg_com_v2_param.SetOutVolt, 0.01f},
    {&wg_com_v2_param.SetOutCurr, 0.01f},
    {&wg_com_v2_param.SetInpUvlo, 0.01f},
    {&wg_com_v2_param.SetInpUvloRecover, 0.01f},
    {&wg_com_v2_param.SetInpOVP, 0.01f},
    {&wg_com_v2_param.SetInpOVPRecover, 0.01f},
    {&wg_com_v2_param.SetOutUvlo, 0.01f},
    {&wg_com_v2_param.SetOutUvloRecover, 0.01f},
    {&wg_com_v2_param.SetOutOVP, 0.01f},
    {&wg_com_v2_param.SetOutOVPRecover, 0.01f},
    {&wg_com_v2_param.SetInpChargLedCurr, 0.01f},
    {&wg_com_v2_param.SetInpFullLedCurr, 0.01f},
    {&wg_com_v2_param.SetOutChargLedCurr, 0.01f},
    {&wg_com_v2_param.SetOutFullLedCurr, 0.01f},
    {&wg_com_v2_param.InpVoltCalibrK, 0.001f},
    {&wg_com_v2_param.InpCurrCalibrK, 0.001f},
    {&wg_com_v2_param.InpShowVoltCalibrK, 0.001f},
    {&wg_com_v2_param.InpShowCurrCalibrK, 0.001f},
    {&wg_com_v2_param.OutVoltCalibrK, 0.001f},
    {&wg_com_v2_param.OutCurrCalibrK, 0.001f},
    {&wg_com_v2_param.OutShowVoltCalibrK, 0.001f},
    {&wg_com_v2_param.OutShowCurrCalibrK, 0.001f},
    {&wg_com_v2_param.AOutShowCurrCalibrK, 0.001f},
    {&wg_com_v2_param.BOutShowCurrCalibrK, 0.001f},
    {&wg_com_v2_param.VoltCompensationAK, 0.001f},   
    {&wg_com_v2_param.VoltCompensationBK, 0.001f},

    {&wg_com_v2_param.AuotForwardOpenVoltA, 0.01f},
    {&wg_com_v2_param.AuotForwardVeerVoltA, 0.01f},
    {&wg_com_v2_param.AuotForwardShutVoltA, 0.01f},
    {&wg_com_v2_param.AuotReverseOpenVoltB, 0.01f},
    {&wg_com_v2_param.AuotReverseShutVoltB, 0.01f},
};

static const wg_com_v2_data_lmt_map_t lmt_map[] = {
    {&wg_com_v2_product_info.Address, 147, 1},
    {&wg_com_v2_product_info.ApplicationScenarios,    4, 0},
    {&wg_com_v2_product_info.BtName,    1, 0},
    {&wg_com_v2_realtime_data.StateCharge,    8, 0},
    {&wg_com_v2_realtime_data.InsideTemp, 150, (uint16_t)(-55)},
    {&wg_com_v2_realtime_data.OutsideTemp,150, (uint16_t)(-55)},
    {&wg_com_v2_realtime_data.PowerMode, 3, 0},
    {&wg_com_v2_realtime_data.ChargMode, 9, 0},
    {&wg_com_v2_realtime_data.Temp2, 150, (uint16_t)(-55)},
    
    {&wg_com_v2_ctrl.FactoryReset, 1, 0},
    {&wg_com_v2_ctrl.PowerOnOff, 1, 0},
    {&wg_com_v2_ctrl.SetPowerMode, 3, 0},
    {&wg_com_v2_ctrl.SetChargMode, 3, 0},
    {&wg_com_v2_ctrl.SetBootTimeA, 180, 0},
    {&wg_com_v2_ctrl.SetBootTimeB, 180, 0},
    {&wg_com_v2_ctrl.SetOnCurrStartTimeA, 180, 0},
    {&wg_com_v2_ctrl.SetOnCurrStartTimeB, 180, 0},
    {&wg_com_v2_ctrl.ZeroCurrCalibration, 1, 0},
    {&wg_com_v2_ctrl.ResetFactoryData, 1, 0},
    {&wg_com_v2_ctrl.BatModeFR, 2, 0},
    {&wg_com_v2_ctrl.MpptSwitch, 1, 0},

    {&wg_com_v2_param.InpVoltCalibrK, 1100, 900},
    {&wg_com_v2_param.InpVoltCalibrB, 1100, 900},
    {&wg_com_v2_param.InpCurrCalibrK, 1300, 700},
    {&wg_com_v2_param.InpCurrCalibrB, 1300, 700},
    {&wg_com_v2_param.InpShowVoltCalibrK, 1100, 900},
    {&wg_com_v2_param.InpShowVoltCalibrB, 1100, 900},
    {&wg_com_v2_param.InpShowCurrCalibrK, 1300, 700},
    {&wg_com_v2_param.InpShowCurrCalibrB, 1300, 700},
    {&wg_com_v2_param.OutVoltCalibrK, 1100, 900},
    {&wg_com_v2_param.OutVoltCalibrB, 1100, 900},
    {&wg_com_v2_param.OutCurrCalibrK, 1300, 700},
    {&wg_com_v2_param.OutCurrCalibrB, 1300, 700},
    {&wg_com_v2_param.OutShowVoltCalibrK, 1100, 900},
    {&wg_com_v2_param.OutShowVoltCalibrB, 1100, 900},
    {&wg_com_v2_param.OutShowCurrCalibrK, 1300, 700},
    {&wg_com_v2_param.OutShowCurrCalibrB, 1300, 700},
    {&wg_com_v2_param.AOutShowCurrCalibrK, 1300, 700},
    {&wg_com_v2_param.AOutShowCurrCalibrB, 1300, 700},
    {&wg_com_v2_param.BOutShowCurrCalibrK, 1300, 700},
    {&wg_com_v2_param.BOutShowCurrCalibrB, 1300, 700},
    {&wg_com_v2_param.VoltCompensationAK, 1100, 900},
    {&wg_com_v2_param.VoltCompensationAB, 1100, 900},
    {&wg_com_v2_param.VoltCompensationBK, 1100, 900},
    {&wg_com_v2_param.VoltCompensationBB, 1300, 700},
    {&wg_com_v2_param.SetInpVolt, 6000, 1000},
    {&wg_com_v2_param.SetInpCurr, 12500, 100},
    {&wg_com_v2_param.SetInpCurrPower, 6000, 50},
    {&wg_com_v2_param.SetOutVolt, 6000, 1000},
    {&wg_com_v2_param.SetOutCurr, 12500, 100},
    {&wg_com_v2_param.SetOutCurrPower, 6000, 50},
    {&wg_com_v2_param.SetInpUvlo, 6100, 1000},
    {&wg_com_v2_param.SetInpUvloRecover, 6200, 1100},
    {&wg_com_v2_param.SetInpOVP, 6200, 1100},
    {&wg_com_v2_param.SetInpOVPRecover, 6100, 1000},
    {&wg_com_v2_param.SetOutUvlo, 6100, 1000},
    {&wg_com_v2_param.SetOutUvloRecover, 6200, 1100},
    {&wg_com_v2_param.SetOutOVP, 6200, 1100},
    {&wg_com_v2_param.SetOutOVPRecover, 6100, 1000},
    {&wg_com_v2_param.SetInsideTemp, 130, 50},
    {&wg_com_v2_param.SetOutsideTemp,130, 50},
    {&wg_com_v2_param.SetInpChargLedCurr, 12500, 100},
    {&wg_com_v2_param.SetInpFullLedCurr,  12500, 100},
    {&wg_com_v2_param.SetOutChargLedCurr, 12500, 100},
    {&wg_com_v2_param.SetOutFullLedCurr,  12500, 100},
    {&wg_com_v2_param.AuotForwardOpenVoltA, 6000, 100},
    {&wg_com_v2_param.AuotForwardVeerVoltA, 6000, 100},
    {&wg_com_v2_param.AuotForwardShutVoltA, 6000, 100},
    {&wg_com_v2_param.AuotReverseOpenVoltB, 6000, 100},
    {&wg_com_v2_param.AuotReverseShutVoltB, 6000, 100},
    {&wg_com_v2_param.SetTemp2, 115, 50},
};

float get_unit_for_addr(void *p)
{
    for (size_t i = 0; i < sizeof(unit_map) / sizeof(unit_map[0]); ++i)
    {
        if (unit_map[i].addr == p)
            return unit_map[i].unit;
    }
    return 1.0f; // default unit 1.0f
}

const wg_com_v2_data_lmt_map_t *get_lmt_for_addr(void *p)
{
    for (size_t i = 0; i < sizeof(lmt_map) / sizeof(lmt_map[0]); ++i)
    {
        if (lmt_map[i].addr == p)
            return &lmt_map[i];
    }
    return NULL;
}

float wg_com_v2_get_data_uint(float user_data, void *wg_com_v2_data)
{
    uint16_t litend_uint16 = get_uint16((uint8_t *)wg_com_v2_data);
    float unit = get_unit_for_addr(wg_com_v2_data);
    const wg_com_v2_data_lmt_map_t *lmt_map = get_lmt_for_addr(wg_com_v2_data);

    if ((uint16_t)(user_data / unit) != litend_uint16)
    {
        if (lmt_map != NULL)
        {
            UP_DN_LMT(litend_uint16, lmt_map->up_lmt, lmt_map->dn_lmt);
            set_uint16((uint8_t *)wg_com_v2_data, litend_uint16);
        }
        user_data = litend_uint16 * unit;
    }

    return user_data;
}

float wg_com_v2_get_data_int(float user_data, void *wg_com_v2_data)
{
    int16_t litend_int16 = get_int16((uint8_t *)wg_com_v2_data);
    float unit = get_unit_for_addr(wg_com_v2_data);
    const wg_com_v2_data_lmt_map_t *lmt_map = get_lmt_for_addr(wg_com_v2_data);

    if ((int16_t)(user_data / unit) != litend_int16)
    {
        if (lmt_map != NULL)
        {
            UP_DN_LMT(litend_int16, (int16_t)lmt_map->up_lmt, (int16_t)lmt_map->dn_lmt);
            set_int16((uint8_t *)wg_com_v2_data, litend_int16);
        }
        user_data = litend_int16 * unit;
    }

    return user_data;
}

void wg_com_v2_set_data_uint(float user_data, void *wg_com_v2_data)
{
    float unit = get_unit_for_addr(wg_com_v2_data);
    const wg_com_v2_data_lmt_map_t *lmt_map = get_lmt_for_addr(wg_com_v2_data);
    uint16_t act_data_temp = (uint16_t)(user_data / unit + ((unit * 5) / 10.0f));

    if (lmt_map != NULL)
    {
        UP_DN_LMT(act_data_temp, lmt_map->up_lmt, lmt_map->dn_lmt);
    }

    set_uint16((uint8_t *)wg_com_v2_data, act_data_temp);
}

void wg_com_v2_set_data_int(float user_data, void *wg_com_v2_data)
{
    float unit = get_unit_for_addr(wg_com_v2_data);
    const wg_com_v2_data_lmt_map_t *lmt_map = get_lmt_for_addr(wg_com_v2_data);
    int16_t act_data_temp = (int16_t)user_data / unit;

    if ((int16_t)user_data < 0)
    {
        act_data_temp -= (int16_t)((unit * 5) / 10.0f);
    }
    else
    {
        act_data_temp += (int16_t)((unit * 5) / 10.0f);
    }

    if (lmt_map != NULL)
    {
        UP_DN_LMT(act_data_temp, (int16_t)lmt_map->up_lmt, (int16_t)lmt_map->dn_lmt);
    }

    set_uint16((uint8_t *)wg_com_v2_data, (uint16_t)act_data_temp);
}

// CRC check calculation
static uint16_t ModBusCRC16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF; // Initial value of CRC register

    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001; // XOR polynomial
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return (crc >> 8) | (crc << 8); // Swap bytes for little-endian
}

uint16_t get_uint16(uint8_t *p_data)
{
    return (uint16_t)(p_data[1] | (p_data[0] << 8));
}

int16_t get_int16(uint8_t *p_data)
{
    return (int16_t)(p_data[1] | (p_data[0] << 8));
}

void set_uint16(uint8_t *p_data, uint16_t data)
{
    p_data[0] = (uint8_t)((data >> 8) & 0x00FF);
    p_data[1] = (uint8_t)(data & 0x00FF);
}

void set_int16(uint8_t *p_data, uint16_t data)
{
    p_data[0] = (uint8_t)((data >> 8) & 0x00FF);
    p_data[1] = (uint8_t)(data & 0x00FF);
}

// 地址区域注册表
static const addr_region_t addr_regions[] = {
    DEFINE_ADDR_REGION(WG_COM_V2_PRUCUCT_INFO_ADDR, wg_com_v2_product_info),
    DEFINE_ADDR_REGION(WG_COM_V2_REALTIME_DATA_ADDR, wg_com_v2_realtime_data),
    DEFINE_ADDR_REGION(WG_COM_V2_CTRL_ADDR, wg_com_v2_ctrl),
    DEFINE_ADDR_REGION(WG_COM_V2_PARAM_ADDR, wg_com_v2_param)};

// 查找匹配的地址区域
static const addr_region_t *find_addr_region(uint16_t addr, uint16_t count)
{
    for (size_t i = 0; i < sizeof(addr_regions) / sizeof(addr_regions[0]); i++)
    {
        if (addr >= addr_regions[i].start_addr &&
            (addr + count - 1) <= addr_regions[i].end_addr)
        {
            return &addr_regions[i];
        }
    }
    return NULL;
}

// 各区域的具体读写实现
static uint8_t unified_read(uint16_t addr, uint16_t count, uint8_t *data)
{
    const addr_region_t *region = find_addr_region(addr, count);
    if (region == NULL)
        return 0;

    uint16_t offset = addr - region->start_addr;
    memcpy(data, (uint8_t *)region->data_ptr + offset * 2, count * 2);
    return 1;
}

static uint8_t unified_write(uint16_t addr, uint16_t count, const uint8_t *data)
{
    const addr_region_t *region = find_addr_region(addr, count);
    if (region == NULL)
        return 0;

    uint16_t offset = addr - region->start_addr;
    memcpy((uint8_t *)region->data_ptr + offset * 2, data, count * 2);
    return 1;
}

// 命令处理函数
static void handle_read_command(void)
{
    uint16_t start_addr = get_uint16(&wg_com_rx_buffer[2]);
    uint16_t reg_count = get_uint16(&wg_com_rx_buffer[4]);

    wg_com_tx_buffer[0] = wg_com_rx_buffer[0];          // 从机地址
    wg_com_tx_buffer[1] = WG_COM_V2_CMD_READ; // 功能码

    if (unified_read(start_addr, reg_count, &wg_com_tx_buffer[3]))
    {
        wg_com_tx_buffer[2] = reg_count * 2; // 字节数
        uint16_t crc = ModBusCRC16(wg_com_tx_buffer, 3 + reg_count * 2);
        set_uint16(&wg_com_tx_buffer[3 + reg_count * 2], crc);
        wg_com_tx_buffer_cnt = 5 + reg_count * 2;
        return;
    }

    // 错误处理
    wg_com_tx_buffer[1] |= 0x80; // 设置错误标志
    wg_com_tx_buffer[2] = 0x02;  // 非法数据地址
    uint16_t crc = ModBusCRC16(wg_com_tx_buffer, 3);
    set_uint16(&wg_com_tx_buffer[3], crc);
    wg_com_tx_buffer_cnt = 5;
}

static void handle_write_data_command(void)
{
    uint16_t reg_addr = get_uint16(&wg_com_rx_buffer[2]);
    uint16_t reg_value = get_uint16(&wg_com_rx_buffer[4]);

    memcpy(wg_com_tx_buffer, wg_com_rx_buffer, 6); // 回显

    uint8_t data[2];
    set_uint16(data, reg_value);

    if (unified_write(reg_addr, 1, data))
    {
        uint16_t crc = ModBusCRC16(wg_com_tx_buffer, 6);
        set_uint16(&wg_com_tx_buffer[6], crc);
        wg_com_tx_buffer_cnt = 8;
        return;
    }

    // 错误处理
    wg_com_tx_buffer[1] |= 0x80;
    wg_com_tx_buffer[2] = 0x02;
    uint16_t crc = ModBusCRC16(wg_com_tx_buffer, 3);
    set_uint16(&wg_com_tx_buffer[3], crc);
    wg_com_tx_buffer_cnt = 5;
}

static void handle_write_str_command(void)
{
    uint16_t start_addr = get_uint16(&wg_com_rx_buffer[2]);
    uint16_t reg_count = get_uint16(&wg_com_rx_buffer[4]);
    uint8_t byte_count = wg_com_rx_buffer[6];

    if (byte_count != reg_count * 2)
    {
        wg_com_tx_buffer[0] = host_addr;
        wg_com_tx_buffer[1] = WG_COM_V2_CMD_WRITE_STR | 0x80;
        wg_com_tx_buffer[2] = 0x03; // 非法数据值
        uint16_t crc = ModBusCRC16(wg_com_tx_buffer, 3);
        set_uint16(&wg_com_tx_buffer[3], crc);
        wg_com_tx_buffer_cnt = 5;
        return;
    }

    memcpy(wg_com_tx_buffer, wg_com_rx_buffer, 6); // 回显

    if (unified_write(start_addr, reg_count, &wg_com_rx_buffer[7]))
    {
        uint16_t crc = ModBusCRC16(wg_com_tx_buffer, 6);
        set_uint16(&wg_com_tx_buffer[6], crc);
        wg_com_tx_buffer_cnt = 8;
        return;
    }

    // 错误处理
    wg_com_tx_buffer[1] |= 0x80;
    wg_com_tx_buffer[2] = 0x02;
    uint16_t crc = ModBusCRC16(wg_com_tx_buffer, 3);
    set_uint16(&wg_com_tx_buffer[3], crc);
    wg_com_tx_buffer_cnt = 5;
}

static void process_command(void)
{
    memset(wg_com_tx_buffer, 0, sizeof(wg_com_tx_buffer));
    wg_com_tx_buffer_cnt = 0;

    uint8_t cmd = wg_com_rx_buffer[1];
    switch (cmd)
    {
    case WG_COM_V2_CMD_READ:
        handle_read_command();
        break;
    case WG_COM_V2_CMD_WRITE_DATA:
        handle_write_data_command();
        break;
    case WG_COM_V2_CMD_WRITE_STR:
        handle_write_str_command();
        break;
    default:
        break;
    }
}

#ifdef IS_PLECS

void wg_com_v2_init(void)
{
    WG_COM_V2_SET_DATA(host_addr, wg_com_v2_product_info.Address);
    WG_COM_V2_SET_DATA(1, wg_com_v2_ctrl.PowerOnOff); // 默认关机
    WG_COM_V2_SET_DATA(100, wg_com_v2_param.SetInsideTemp);
    WG_COM_V2_SET_DATA(100, wg_com_v2_param.SetOutsideTemp);
    WG_COM_V2_SET_DATA(20.0f, wg_com_v2_param.SetOutOVP);
    WG_COM_V2_SET_DATA(9.0f, wg_com_v2_param.SetInpUvlo);
    WG_COM_V2_SET_DATA(100, wg_com_v2_param.SetTemp2);
}

REG_INIT(wg_com_v2_init)

void wg_com_v2_run(void)
{
    static uint32_t time = 0;
    time++;
    if (time < 2000)
    {
        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.SetChargMode);
    }
    else
    {
        WG_COM_V2_SET_DATA(1, wg_com_v2_ctrl.SetChargMode); // 模拟充电模式
    }
}
#else

#include "gpio.h"

void usart0_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    g_output_port = OUTPUT_USART0;
    vprintf(fmt, args);
    va_end(args);
}

void usart2_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    g_output_port = OUTPUT_USART2;
    vprintf(fmt, args);
    va_end(args);
}

// 安全增加计数器（防止溢出）
static inline void safe_increment(uint32_t *counter, uint32_t max)
{
    if (*counter < max)
        (*counter)++;
}
static uint32_t usart0_delay = 0;
void process_usart_dma_input(const usart_dma_port_t *port)
{
    if((USART_GetStatus(CM_USART1, USART_FLAG_FRAME_ERR) == 1) && (port->USARTx == OUTPUT_USART0))
    {
        USART_SetFirstBit(CM_USART1, USART_CR1_CFE);
        CM_USART1->CR1 |= USART_CR1_CFE;
    }
    while (*(port->rx_cnt) != USARTX_RX_DMA_CNT(port->dma_channel))
    {
        uint8_t rx_data = port->dma_buffer[port->buffer_size - *(port->rx_cnt)];
        (*(port->rx_cnt))--;
        (*(port->rx_cnt)) = (*(port->rx_cnt) == 0) ? port->buffer_size : (*(port->rx_cnt));
        wg_com_rx_buffer[wg_com_rx_buffer_cnt++] = rx_data;
        if (wg_com_rx_buffer_cnt >= WG_COM_V2_BUFFER_SIZE)
        {
            wg_com_rx_buffer_cnt = 0;
        }
        if((port->USARTx == OUTPUT_USART2) && 
          ((usart_dma_bt_buf.rx_step == 1) || 
           (usart_dma_bt_buf.rx_step == 3)))
        {
            usart_dma_bt_buf.usart_buf[usart_dma_bt_buf.buffer_size++] = rx_data;
            if (usart_dma_bt_buf.buffer_size >= WG_COM_V2_BUFFER_SIZE)
            {
                usart_dma_bt_buf.buffer_size = 0;
            }
        }
        *(port->is_rx_flag) = 1; 
        *(port->timeout) = systemtime;
    }

    if (((*(port->timeout)+20) < systemtime) && (*(port->is_rx_flag)))
    {
        *(port->is_rx_flag) = 0;

        if (wg_com_rx_buffer_cnt < MODBUS_MIN_FRAME_LEN)
        {
            memset(wg_com_rx_buffer, 0, sizeof(wg_com_rx_buffer));
            wg_com_rx_buffer_cnt = 0;
            return;
        }

        uint16_t *rx_crc = (uint16_t *)&wg_com_rx_buffer[wg_com_rx_buffer_cnt - 2];
        uint16_t cal_crc = ModBusCRC16(wg_com_rx_buffer, wg_com_rx_buffer_cnt - 2);
        set_uint16((uint8_t *)&cal_crc, cal_crc);
        WG_COM_V2_GET_DATA_UINT(host_addr, wg_com_v2_product_info.Address);  
        if ((*rx_crc == cal_crc) &&
            ((wg_com_rx_buffer[0] == host_addr) ||
             (wg_com_rx_buffer[0] == WG_COM_V2_BROADCAST_ADDR)))
        {
            process_command();
                  
            if (port->my_printf)
            {
                if(port->USARTx == OUTPUT_USART0)
                {
                    gpio_set_re(1);
                    for (uint32_t i = 0; i < wg_com_tx_buffer_cnt; i++)
                    {
                        port->my_printf("%c", wg_com_tx_buffer[i]); // 按字节输出
                    }
                    usart0_delay = 0;
                    while(USART_GetStatus(CM_USART1, USART_FLAG_TX_CPLT) == 0)
                    {
                        safe_increment(&usart0_delay,USART0_DELAY_CONT);
                        if(usart0_delay >= USART0_DELAY_CONT)
                        {
                            break;
                        }
                    }
                    gpio_set_re(0);
                }
                else
                {
                    for (uint32_t i = 0; i < wg_com_tx_buffer_cnt; i++)
                    {
                        port->my_printf("%c", wg_com_tx_buffer[i]); // 按字节输出
                    }
                }
            }
        }

        memset(wg_com_rx_buffer, 0, sizeof(wg_com_rx_buffer));
        wg_com_rx_buffer_cnt = 0;
    }
    uint16_t BtNameValue = 0;
    WG_COM_V2_GET_DATA_UINT(BtNameValue, wg_com_v2_product_info.BtName); 
    if(port->USARTx == OUTPUT_USART2)
    {
        if(BtNameValue == 1)
        {
            BtNameValue = 0;
            WG_COM_V2_SET_DATA_UINT(BtNameValue, wg_com_v2_product_info.BtName); 
            usart_dma_bt_buf.rx_step = 1;
            usart_dma_bt_buf.rx_data_step = 1;
            usart_dma_bt_buf.buffer_size = 0;
            memset((uint8_t*)usart_dma_bt_buf.usart_buf, 0, sizeof(usart_dma_bt_buf.usart_buf));
            memset(&usart_dma_bt_buf.bt_name, 0, sizeof(usart_dma_bt_buf.bt_name));
            port->my_printf("%s", "AT+NAME?"); // 按字节输出
        }
        if(usart_dma_bt_buf.rx_step == 2)
        {
            char buf[22];
            usart_dma_bt_buf.rx_step = 3;
            usart_dma_bt_buf.buffer_size = 0;
            memset((uint8_t*)usart_dma_bt_buf.usart_buf, 0, sizeof(usart_dma_bt_buf.usart_buf));
            memset(&buf, 0, sizeof(buf));
            memcpy((char *)&buf, (char *)&usart_dma_bt_buf.bt_name[6], sizeof(buf));
            usart_dma_bt_buf.bt_name[0] = 'A';
            usart_dma_bt_buf.bt_name[1] = 'T';
            usart_dma_bt_buf.bt_name[2] = '+';
            usart_dma_bt_buf.bt_name[3] = 'N';
            usart_dma_bt_buf.bt_name[4] = 'A';
            usart_dma_bt_buf.bt_name[5] = 'M';
            usart_dma_bt_buf.bt_name[6] = 'E';
            usart_dma_bt_buf.bt_name[7] = '=';
            for(uint16_t i = 0;i < sizeof(wg_com_v2_product_info.SnSerial);i=i+2)
            {
                usart_dma_bt_buf.bt_name[i+8] = (wg_com_v2_product_info.SnSerial[i/2]&0x00ff);
                usart_dma_bt_buf.bt_name[i+9] = ((wg_com_v2_product_info.SnSerial[i/2]&0xff00)>>8);
            }
            usart_dma_bt_buf.bt_name[10] = 'B';
            usart_dma_bt_buf.bt_name[11] = 'T';
            int result = strcmp(buf, (usart_dma_bt_buf.bt_name+8));
            if(result != 0)
            {
                usart_dma_bt_buf.rx_step = 0;
                usart_dma_bt_buf.rx_data_step = 0;
                for (uint32_t i = 0; i < 28; i++)
                {
                    port->my_printf("%c", usart_dma_bt_buf.bt_name[i]); // 按字节输出
                }
            }
        }
    }
}

void wg_com_v2_run(void)
{
    static uint32_t rx2_data_cnt = USART2_RX_BUFFER_SIZE;
    static uint8_t is_rx2 = 0;
    static uint32_t timeout2 = 0;

    static uint32_t rx0_data_cnt = USART0_RX_BUFFER_SIZE;
    static uint8_t is_rx0 = 0;
    static uint32_t timeout0 = 0;

    static const usart_dma_port_t usart2_port = {
        .dma_buffer = usart2_rx_buffer,
        .buffer_size = USART2_RX_BUFFER_SIZE,
#ifdef HC32F334
        .dma_channel = DMA_CH3,
#else
        .dma_channel = DMA_CH2,
#endif
        .rx_cnt = &rx2_data_cnt,
        .is_rx_flag = &is_rx2,
        .timeout = &timeout2,
        .tag = "usart2",
        .my_printf = usart2_printf,
        .USARTx = OUTPUT_USART2};

    
    static const usart_dma_port_t usart0_port = {
        .dma_buffer = usart0_rx_buffer,
        .buffer_size = USART0_RX_BUFFER_SIZE,
#ifdef HC32F334
        .dma_channel = DMA_CH1,
#else
        .dma_channel = DMA_CH4,
#endif
        .rx_cnt = &rx0_data_cnt,
        .is_rx_flag = &is_rx0,
        .timeout = &timeout0,
        .tag = "usart0",
        .my_printf = usart0_printf,
        .USARTx = OUTPUT_USART0};
 
    process_usart_dma_input(&usart2_port);
    process_usart_dma_input(&usart0_port);
}

#endif

REG_TASK(1, wg_com_v2_run)

void get_bt_data_run(void)
{
    static uint16_t get_data_delay =0;
    int result;
    char *token;
    switch(usart_dma_bt_buf.rx_step)
    {
        case 0:
            break;
        case 1:
            if(++get_data_delay >= 100)
            {
                get_data_delay = 0;
                token = strtok((char*)usart_dma_bt_buf.usart_buf, "\r\n");
                usart_dma_bt_buf.rx_data_step = 0;
                while(token != NULL) {
                    switch(usart_dma_bt_buf.rx_data_step)
                    {
                        case 0:
                            result = strcmp(token, "AT+NAME?");
                            if(result != 0)
                            {
                                usart_dma_bt_buf.rx_step = 0xff;
                                return;
                            }
                            usart_dma_bt_buf.rx_data_step = 1;
                            break;
                        case 1:
                            if(strlen(token) > 26)
                            {
                                usart_dma_bt_buf.rx_step = 0xff;
                                return;
                            }
                            for(uint16_t i = 0;i < strlen(token);i++)
                            {
                                usart_dma_bt_buf.bt_name[i] = usart_dma_bt_buf.usart_buf[strlen("AT+NAME?\r\n")+i];
                            }
                            usart_dma_bt_buf.rx_data_step = 2;
                            break;
                        case 2:
                            result = strcmp(token, "OK");
                            if(result != 0)
                            {
                                usart_dma_bt_buf.rx_step = 0xff;
                                return;
                            }
                            usart_dma_bt_buf.rx_data_step = 3;
                            break;
                        case 3:
                            usart_dma_bt_buf.rx_data_step = 4;
                            return;
                    }
                    token = strtok(NULL, "\r\n");
                }
                if(usart_dma_bt_buf.rx_data_step == 3)
                {
                    usart_dma_bt_buf.rx_step = 2;
                }
                else
                {
                    usart_dma_bt_buf.rx_step = 0;
                }
            }
            break;
        case 3:
            if(++get_data_delay >= 100)
            {
                get_data_delay = 0;
                result = strcmp(token, "OK");
                if(result != 0)
                {
                    usart_dma_bt_buf.rx_step = 0xff;
                    return;
                }
                usart_dma_bt_buf.rx_data_step = 4;
            }
            break;
        default:
            get_data_delay = 0;
            usart_dma_bt_buf.buffer_size = 0;
            memset((uint8_t*)usart_dma_bt_buf.usart_buf, 0, sizeof(usart_dma_bt_buf.usart_buf));
            usart_dma_bt_buf.rx_data_step = 0;
            usart_dma_bt_buf.rx_step = 0;
            break;
    }
}

REG_TASK(10, get_bt_data_run)




