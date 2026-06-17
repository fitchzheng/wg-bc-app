// ========================= ymodem.h =========================
#ifndef __YMODEM_H
#define __YMODEM_H

#include <stdint.h>
#include <stddef.h>
#include "bsp_flash.h"
#include "bsp_usart.h"

#define APP_START_ADDR 0x00005000U
#define APP_END_ADDR   0x0001EFFFU
#define APP_MAX_SIZE   (APP_END_ADDR - APP_START_ADDR + 1U)
#define IAP_META_ADDR  0x0001F000U

#define YMODEM_SOH 0x01
#define YMODEM_STX 0x02
#define YMODEM_EOT 0x04
#define YMODEM_ACK 0x06
#define YMODEM_NAK 0x15
#define YMODEM_CAN 0x18
#define YMODEM_C 0x43
#define RYM_CODE_O 0x4F

#define YMODEM_START_DATA_LEN 128U
#define YMODEM_START_TOTAL_LEN (1U + 1U + 1U + YMODEM_START_DATA_LEN + 2U)
#define YMODEM_SOH_DATA_LEN 128U
#define YMODEM_SOH_TOTAL_LEN (1U + 1U + 1U + YMODEM_SOH_DATA_LEN + 2U)
#define YMODEM_BT_DATA_LEN 128U
#define YMODEM_STX_BT_TOTAL_LEN (1U + 1U + 1U + 1U + YMODEM_BT_DATA_LEN + 2U)

#define YMODEM_BT_BTPN_MIN 0
#define YMODEM_BT_BTPN_MAX 7

#define YMODEM_BUFFER_SIZE 256
#define CODE_BUFFER_PAGE_COUNT 32
#define CODE_BUFFER_PAGE_SIZE 128

#define YMODEM_RX_TIMEOUT_MS 500
#define YMODEM_MAX_RETRY 30

#define YMODEM_INFO_NAME_LEN 20
#define YMODEM_INFO_SIZE_OFFSET 20
#define YMODEM_INFO_SIZE_LEN 4
#define YMODEM_INFO_BAUD_OFFSET 24
#define YMODEM_INFO_BAUD_LEN 2
#define YMODEM_INFO_PASSTHROUGH_IDX 26
#define YMODEM_INFO_UPGRADE_MODE_IDX 27
#define YMODEM_INFO_HW_VER_OFFSET 28
#define YMODEM_INFO_HW_VER_LEN 4
#define YMODEM_INFO_IMAGE_CRC32_OFFSET 0
#define YMODEM_INFO_IMAGE_CRC32_LEN 4
#define YMODEM_LINK_CAN_OTA 0xCAU

#pragma pack(push, 1)

typedef struct
{
    uint8_t header;
    uint8_t pn;
    uint8_t xpn;
    uint8_t data[YMODEM_START_DATA_LEN];
    uint8_t crc_high;
    uint8_t crc_low;
} ymodem_frame_soh_first_t;

typedef struct
{
    uint8_t header;
    uint8_t pn;
    uint8_t xpn;
    uint8_t data[YMODEM_SOH_DATA_LEN];
    uint8_t crc_high;
    uint8_t crc_low;
} ymodem_frame_soh_data_t;

typedef struct
{
    uint8_t header;
    uint8_t pn;
    uint8_t xpn;
    uint8_t bt_pn;
    uint8_t data[YMODEM_BT_DATA_LEN];
    uint8_t crc_high;
    uint8_t crc_low;
} ymodem_frame_stx_t;

typedef struct
{
    uint8_t header;
    uint8_t pn;
    uint8_t xpn;
    char product_name[20]; // 产品名称 ASCII
    uint32_t file_size;    // 文件大小（Byte，LE）
    uint16_t baud;         // 升级波特率（默认96表示9600dps）
    uint8_t passthrough;   // 透传（默认0xFF）
    uint8_t upgrade_mode;  // 功能升级选择（默认整片升级）
    uint32_t hw_ver;       // 协议硬件版本号
    uint8_t link_id;       // 链路标记
    uint8_t padding[95];   // 填充区（0x00）
    uint8_t crc_high;      // CRC-H
    uint8_t crc_low;       // CRC-L
} ymodem_soh_first_parsed_t;

// 将原始首帧与解析视图做联合体，便于原样/字段化两种访问
typedef union
{
    ymodem_frame_soh_first_t raw;     // 原始缓冲视图（与传输格式一致）
    ymodem_soh_first_parsed_t parsed; // 字段化视图（便于直接读取字段）
} ymodem_soh_first_u;


typedef struct
{
    volatile uint8_t *dma_buffer;
    uint32_t buffer_size;
    uint32_t dma_channel;
    uint16_t *rd_idx;
    uint16_t *idle_ms;
    int *overflow;
    const char *tag;
    usart_output_port_t USARTx;
} ymodem_usart_dma_port_t;

#pragma pack(pop)

#ifdef __cplusplus
extern "C"
{
#endif

    int ymodem_recv_byte(uint8_t byte,uint8_t port_com);
    void ymodem_task(void);
    void ymodem_recv_task(void);
    void ymodem_init(void);
    uint8_t ymodem_request_can_ota(uint32_t image_size, uint32_t image_crc32);

#ifdef __cplusplus
}
#endif

#endif /* __YMODEM_H */
