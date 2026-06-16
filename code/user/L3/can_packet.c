#include "can_packet.h"
#include "bsp_can.h"
#include "string.h"

can_packet_id_t can_build_ext_id(
    uint16_t protno,
    uint8_t ptp,
    uint8_t dstaddr,
    uint8_t srcaddr,
    uint8_t cnt,
    uint8_t res1,
    uint8_t res2)
{
    can_packet_id_t packet_id;
    packet_id.protno = protno;
    packet_id.ptp = ptp;
    packet_id.dstaddr = dstaddr;
    packet_id.srcaddr = srcaddr;
    packet_id.cnt = cnt;
    packet_id.res1 = res1;
    packet_id.res2 = res2;
    packet_id.rsvd = 0; // 保留位
    return packet_id;
}

void can_build_payload(uint8_t *data, uint8_t group, uint8_t msg_type, uint8_t cmd_type, uint32_t cmd_data)
{
    data[0] = ((group & 0x0F) << 4) | (msg_type & 0x0F);
    data[1] = cmd_type;
    data[2] = 0x00; // 保留
    data[3] = 0x00; // 保留
    data[4] = (cmd_data >> 24) & 0xFF;
    data[5] = (cmd_data >> 16) & 0xFF;
    data[6] = (cmd_data >> 8) & 0xFF;
    data[7] = (cmd_data >> 0) & 0xFF;
}

void can_packet_transmit_data(uint16_t protno,
                              uint8_t ptp,
                              uint8_t dstaddr,
                              uint8_t srcaddr,
                              uint8_t cnt,
                              uint8_t res1,
                              uint8_t res2,
                              uint8_t *data)
{
    can_packet_id_u id;
    id.id = can_build_ext_id(protno, ptp, dstaddr, srcaddr, cnt, res1, res2);

    bsp_can_tx(id.raw, data);
}

/**
 * @brief 接收一个CAN扩展帧（29bit ID + 8字节数据），并解析到can_packet_t结构体
 * @param can_periph CAN外设（如CAN0）
 * @param packet     输出，接收到的can_packet_t结构体
 * @return 0=成功，非0=失败
 */
int can_packet_receive(can_packet_t *packet)
{
    return bsp_can_rx(&packet->id.raw, &packet->data.raw[0]);
}
