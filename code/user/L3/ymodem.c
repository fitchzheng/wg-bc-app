// ========================= ymodem.c =========================
#include "ymodem.h"
#include <string.h>
#include <stdio.h>
#include "flash.h"
#include "section.h"
#include "bsp_usart.h"
#include "gpio.h"
#include <stdarg.h>
#include "bsp_dma.h"
#include "wg_com_v2.h"

extern void usart2_printf(const char *fmt, ...);
extern void usart0_printf(const char *fmt, ...);

enum
{
    YMODEM_STA_INIT,
    YMODEM_STA_IDLE,
    YMODEM_STA_CHK_FIRST,
    YMODEM_STA_WAIT_DATA,
    YMODEM_STA_PROCESS_DATA,
    YMODEM_STA_RECV_END,
    YMODEM_STA_RECV_CP_DATA,
    YMODEM_STA_RECV_CP_END,
    YMODEM_STA_RECV_WRITE_FLASH,
    YMODEM_STA_WAIT_REPLY,
    YMODEM_STA_END_DATA,
    YMODEM_STA_RESET,
};

uint32_t ymodem_state = YMODEM_STA_INIT;
uint32_t ymodem_state_next = YMODEM_STA_INIT;

uint8_t usart_link = 0;

// 声明 DMA 环形缓冲（使用 volatile 防止优化丢读）
extern volatile uint8_t usart2_rx_buffer[];
extern volatile uint8_t usart0_rx_buffer[];
extern uint32_t dma_transfer_number_get(uint32_t periph, uint8_t channel);
ymodem_soh_first_parsed_t ymodem_soh_first_parsed;
ymodem_frame_soh_data_t ymodem_frame_soh_data;
ymodem_frame_stx_t ymodem_frame_stx;
//static uint16_t flash_page_2k = 0;
//static uint32_t code_size_byte = 0;

// 空闲判定阈值：连续空闲 50ms 视为当前帧接收完成
#undef YMODEM_IDLE_GAP_MS
#define YMODEM_IDLE_GAP_MS 50U

// 回复发送队列（按 200ms 间隔发送）
static uint8_t reply_fifo[4];
static uint8_t reply_head[2], reply_tail[2], reply_len[2];
static uint16_t reply_wait_ms[2];

static uint8_t is_recv_cplt[2];

//static uint32_t swap_endian_u32(uint32_t in)
//{
//    return ((in & 0xFF000000) >> 24) | // Move byte 3 to byte 0
//           ((in & 0x00FF0000) >> 8) |  // Move byte 2 to byte 1
//           ((in & 0x0000FF00) << 8) |  // Move byte 1 to byte 2
//           ((in & 0x000000FF) << 24);  // Move byte 0 to byte 3
//}

// 安全增加计数器（防止溢出）
static inline void safe_increment(uint32_t *counter, uint32_t max)
{
    if (*counter < max)
        (*counter)++;
}

// 入队一个待回复字节
static inline void ymodem_queue_reply(uint8_t c,uint8_t port_com)
{
    if (reply_len[port_com] < sizeof(reply_fifo))
    {
        reply_fifo[reply_tail[port_com]] = c;
        reply_tail[port_com] = (uint8_t)((reply_tail[port_com] + 1) % sizeof(reply_fifo));
        reply_len[port_com]++;
    }
}
static uint32_t ymodem_usart0_delay = 0;
// 实际发送一个待回复字节（printf），并启动下一次 50ms 间隔计时
static inline void ymodem_tx_reply_tick(uint8_t port_com)
{
    if (reply_len[port_com] == 0)
        return;

    if (reply_wait_ms[port_com] > 0)
    {
        reply_wait_ms[port_com]--;
        return;
    }

    // 发送队首
    uint8_t c = reply_fifo[reply_head[port_com]];
    reply_head[port_com] = (uint8_t)((reply_head[port_com] + 1) % sizeof(reply_fifo));
    reply_len[port_com]--;

    // 发送
    if (port_com == OUTPUT_USART0)
    {
        gpio_set_re(1);
        usart0_printf("%c", c);
        ymodem_usart0_delay = 0;
        while(USART_GetStatus(CM_USART1, USART_FLAG_TX_CPLT) == 0)
        {
            safe_increment(&ymodem_usart0_delay,USART0_DELAY_CONT);
            if(ymodem_usart0_delay >= USART0_DELAY_CONT)
            {
                break;
            }
        }
        gpio_set_re(0);
    }
    else if (port_com == OUTPUT_USART2)
    {
        usart2_printf("%c", c);
    }

    // 设置下一条回复的最小间隔 50ms
    reply_wait_ms[port_com] = 200;
}

static inline void ymodem_send_nak(uint8_t port_com)
{
    ymodem_queue_reply(YMODEM_NAK,port_com);
    // if (ymodem_retry_cnt < 0xFF) ymodem_retry_cnt++;
    // ymodem_update_time_dn_cnt = 0;
}

// ========== 补回缺失的静态全局变量与前置声明 ==========
static uint8_t recv_buffer[2][YMODEM_BUFFER_SIZE];
static uint16_t recv_index[2];

// === CRC16 Modbus 实现（高位在前） ===
static uint16_t ymodem_crc16(const uint8_t *data, uint32_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint32_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return (uint16_t)((crc >> 8) | (crc << 8));
}

static inline void ymodem_reset_rx(uint8_t port_com)
{
    recv_index[port_com] = 0;
    memset(recv_buffer[port_com], 0, sizeof(recv_buffer[port_com]));
}

int ymodem_recv_byte(uint8_t byte,uint8_t port_com)
{
    if (recv_index[port_com] >= sizeof(recv_buffer[port_com]))
    {
        recv_index[port_com] = 0;
        return -1;
    }
    recv_buffer[port_com][recv_index[port_com]++] = byte;
    return (int)recv_index[port_com];
}

void ymodem_init(void)
{
    ymodem_reset_rx(OUTPUT_USART0);
    ymodem_reset_rx(OUTPUT_USART2);
    ymodem_state = YMODEM_STA_WAIT_DATA;
}

void ymodem_process_usart_dma_input(const ymodem_usart_dma_port_t *port)
{
    uint32_t cndtr = USARTX_RX_DMA_CNT(port->dma_channel);
    uint16_t wr_idx = (uint16_t)(port->buffer_size - cndtr);

    if(wr_idx >= port->buffer_size)
    {
        wr_idx -= port->buffer_size;
    }

    while ((*(port->rd_idx)) != wr_idx)
    {
        uint8_t byte = port->dma_buffer[(*(port->rd_idx))];
        (*(port->rd_idx))++;
        if ((*(port->rd_idx)) >= port->buffer_size)
            (*(port->rd_idx)) = 0;

        (*(port->overflow)) = ymodem_recv_byte(byte,port->USARTx);
        (*(port->idle_ms))  = YMODEM_IDLE_GAP_MS;
    }

    if ((*(port->overflow)) == -1)
    {
        ymodem_send_nak(port->USARTx);
        ymodem_reset_rx(port->USARTx);
        (*(port->overflow)) = 0;
        (*(port->idle_ms)) = 0;
    }

    if (*(port->idle_ms))
    {
        (*(port->idle_ms))--;
    }
    
    if (recv_index[port->USARTx] != 0 && (*(port->idle_ms)) == 0)
    {
        is_recv_cplt[port->USARTx] = 1;
    }
    
    // 统一在这里按节流间隔发送待回复字节
    ymodem_tx_reply_tick(port->USARTx);
}

void ymodem_recv_task(void)
{
    // USART2部分
    static uint16_t rd_idx2 = 0;
    static int overflow2 = 0;
    static uint16_t idle2_ms = 0;
    
    // USART0部分
    static uint16_t rd_idx0 = 0;
    static int overflow0 = 0;
    static uint16_t idle0_ms = 0;

    static const ymodem_usart_dma_port_t usart2_port = {
        .dma_buffer = usart2_rx_buffer,
        .buffer_size = USART2_RX_BUFFER_SIZE,
        .dma_channel = DMA_CH3,
        .rd_idx = &rd_idx2,
        .overflow = &overflow2,
        .idle_ms = &idle2_ms,
        .tag = "usart2",
        .USARTx = OUTPUT_USART2};
    
    static const ymodem_usart_dma_port_t usart0_port = {
        .dma_buffer = usart0_rx_buffer,
        .buffer_size = USART0_RX_BUFFER_SIZE,
        .dma_channel = DMA_CH1,
        .rd_idx = &rd_idx0,
        .overflow = &overflow0,
        .idle_ms = &idle0_ms,
        .tag = "usart0",
        .USARTx = OUTPUT_USART0};

    ymodem_process_usart_dma_input(&usart2_port);
    ymodem_process_usart_dma_input(&usart0_port);
}

REG_TASK(1, ymodem_recv_task)

static uint32_t Delay_Cot = 0;
static uint8_t  up_data = 0;
static void ymodem_chk_first(void)
{
    uint16_t crc_data = ymodem_crc16((const uint8_t *)&ymodem_soh_first_parsed, sizeof(ymodem_soh_first_parsed) - 2);
    if (crc_data == ((uint16_t)ymodem_soh_first_parsed.crc_high << 8 | ymodem_soh_first_parsed.crc_low))
    {
        ymodem_soh_first_parsed.link_id = usart_link;
        while(Delay_Cot < 0xFFFF)
        {
            up_data = 1;
            safe_increment(&Delay_Cot, 0xFFFF);
        }
        flash_write(FLASH_UPDATE, 0, (uint16_t *)&ymodem_soh_first_parsed);
        ymodem_state = YMODEM_STA_RESET;
    }
}

uint8_t get_up_data_flag(void)
{
    return up_data;
}

//static void ymodem_wait_reply(void)
//{
//    // 等待回复状态
//    if (reply_len == 0)
//    {
//        ymodem_state = ymodem_state_next;
//    }
//}

static void ymodem_wait_data(void)
{
    uint8_t updating_flag = 0;
    // 等待数据状态
    for(uint16_t i = 0;i < 2;i++)
    {
        if(updating_flag == 1)
        {
            return;
        }

        if (is_recv_cplt[i])
        {
            is_recv_cplt[i] = 0;
            if (recv_buffer[i][0] == YMODEM_SOH && recv_index[i] == YMODEM_START_TOTAL_LEN)
            {
                memcpy(&ymodem_soh_first_parsed, &recv_buffer[i][0], sizeof(ymodem_soh_first_parsed));
                ymodem_state = YMODEM_STA_CHK_FIRST;
                updating_flag = 1;
                usart_link = i;
            }
            recv_index[i] = 0;
        }
    }
}

static void ymodem_process_current_frame(void)
{
    switch (ymodem_state)
    {
    case YMODEM_STA_INIT:
        ymodem_init();
        break;
//    case YMODEM_STA_IDLE:
//        break;
    case YMODEM_STA_CHK_FIRST:
        ymodem_chk_first();
        break;
    case YMODEM_STA_PROCESS_DATA:
//        //ymodem_process_data();
        break;
    case YMODEM_STA_RECV_END:
//        ymodem_recv_end();
        break;
    case YMODEM_STA_RECV_CP_DATA:
        break;
    case YMODEM_STA_RECV_CP_END:
        break;
//    case YMODEM_STA_WAIT_REPLY:
//        ymodem_wait_reply();
//        break;
    case YMODEM_STA_WAIT_DATA:
        ymodem_wait_data();
        break;
    case YMODEM_STA_RECV_WRITE_FLASH:
        //ymodem_write_flash();
        break;
    case YMODEM_STA_END_DATA:
        //ymodem_end_data();
        break;
    case YMODEM_STA_RESET:
        NVIC_SystemReset();
        break;
    }
}

REG_TASK(1, ymodem_process_current_frame)
