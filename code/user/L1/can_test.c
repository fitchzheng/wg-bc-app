#include "section.h"
#include "can_packet.h"
#include "can_rmp.h"
#include "can_comm.h"
#include "can_wg.h"
#include "rvc_address.h"
#include "rvc_message_handler.h"
#include "client_can.h"
#include "ymodem.h"
#include "hc32_ll.h"

static can_packet_t can_rx_packet;
static uint32_t can_ota_pending_size = 0U;
static uint8_t can_ota_start_seen = 0U;
static uint16_t can_ota_reset_delay_ms = 0U;

#define CAN_OTA_CTRL_RX_ID              0x1C510000UL
#define CAN_OTA_CTRL_TX_ID              0x1C520000UL
#define CAN_OTA_CMD_START               0xA0U
#define CAN_OTA_CMD_START_CRC           0xA1U
#define CAN_OTA_FRAMES_PER_BLOCK        28U
#define CAN_OTA_ACK_OK                  0xC0U
#define CAN_OTA_ACK_NAK                 0x15U
#define CAN_OTA_ERR_NONE                0U
#define CAN_OTA_ERR_PARAM               2U

extern uint8_t bsp_rvc_can_tx(uint32_t id, const uint8_t *p_data, uint8_t len);

static uint32_t can_ota_read_le32(const uint8_t *data)
{
    return ((uint32_t)data[0]) |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

static void can_ota_app_ack(uint8_t code, uint8_t err)
{
    uint8_t tx[8] = {0};

    tx[0] = code;
    tx[1] = err;
    (void)bsp_rvc_can_tx(CAN_OTA_CTRL_TX_ID, tx, 8U);
}

static uint8_t can_ota_app_trigger_process(uint32_t can_id, uint8_t *data, uint8_t len)
{
    if ((can_id != CAN_OTA_CTRL_RX_ID) || (data == 0) || (len < 8U))
    {
        return 0U;
    }

    if (data[0] == CAN_OTA_CMD_START)
    {
        uint32_t image_size = can_ota_read_le32(&data[2]);
        if ((data[1] != 1U) ||
            (data[6] != CAN_OTA_FRAMES_PER_BLOCK) ||
            (data[7] != 0U) ||
            (image_size == 0U) ||
            (image_size > APP_MAX_SIZE))
        {
            can_ota_start_seen = 0U;
            can_ota_app_ack(CAN_OTA_ACK_NAK, CAN_OTA_ERR_PARAM);
            return 1U;
        }

        can_ota_pending_size = image_size;
        can_ota_start_seen = 1U;
        can_ota_app_ack(CAN_OTA_ACK_OK, CAN_OTA_ERR_NONE);
        return 1U;
    }

    if (data[0] == CAN_OTA_CMD_START_CRC)
    {
        uint32_t image_crc32;

        if (can_ota_start_seen == 0U)
        {
            can_ota_app_ack(CAN_OTA_ACK_NAK, CAN_OTA_ERR_PARAM);
            return 1U;
        }

        image_crc32 = can_ota_read_le32(&data[1]);
        if (ymodem_request_can_ota(can_ota_pending_size, image_crc32) != 0U)
        {
            can_ota_app_ack(CAN_OTA_ACK_OK, CAN_OTA_ERR_NONE);
            can_ota_reset_delay_ms = 100U;
        }
        else
        {
            can_ota_app_ack(CAN_OTA_ACK_NAK, CAN_OTA_ERR_PARAM);
        }

        can_ota_start_seen = 0U;
        return 1U;
    }

    return 1U;
}

void can_test_1ms_task(void)
{
    // 轮询接收CAN0
    /*if (can_packet_receive(&can_rx_packet) == 0)
    {
        // 判断协议号
        if (can_rx_packet.id.id.protno == PROTNO_RMP)
        {
            can_comm_rmp_dispatch(&can_rx_packet);
        }
        else if (can_rx_packet.id.id.protno == PROTNO_WG)
        {
            can_comm_wg_dispatch(&can_rx_packet);
        }
    }*/
    
    uint8_t rx_count = 0;
    while ((rx_count < 10U) && (can_packet_receive(&can_rx_packet) == 0))
    {
        rx_count++;
        #if(CAN_ON_OFF == 1)
        if (can_ota_app_trigger_process(can_rx_packet.id.raw, can_rx_packet.data.raw, 8U) != 0U)
        {
            continue;
        }

        // 1. 先交给地址管理器处理（处理 ADDRESS_CLAIM）
        rvc_address_can_rx_callback(can_rx_packet.id.raw, can_rx_packet.data.raw, 8);
        
        // 2. 再交给消息处理器处理（处理其他 DGN）
        rvc_message_handler_process(can_rx_packet.id.raw, can_rx_packet.data.raw, 8);

        // 3. Modbus 桥接处理（处理私有 DGN 0xEF00）
        //rvc_modbus_bridge_process(can_rx_packet.id.raw, can_rx_packet.data.raw, 8);
        #elif(CAN_ON_OFF == 2) 
        Get_CAN_Communications_Content (can_rx_packet.id.raw, can_rx_packet.data.raw, 8);
        #endif
    }

    if (can_ota_reset_delay_ms > 0U)
    {
        can_ota_reset_delay_ms--;
        if (can_ota_reset_delay_ms == 0U)
        {
            NVIC_SystemReset();
        }
    }

}

#if(CAN_ON_OFF != 0)
REG_TASK(1, can_test_1ms_task)
#endif

