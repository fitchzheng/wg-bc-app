#include "can_rmp.h"
#include "can_packet.h"
#include "can_comm.h"

can_packet_data_value_u valuetype_rmp_buffer[256] = {0};

// RMP相关变量
can_rmp_msgtype_cmd0_set_req_byte2_t can_rmp_msgtype_cmd0_set_req_byte2 = {0};
can_rmp_msgtype_cmd0_set_req_byte3_t can_rmp_msgtype_cmd0_set_req_byte3 = {0};
can_rmp_msgtype_cmd0_set_req_byte4_7_t can_rmp_msgtype_cmd0_set_req_byte4_7 = {0};
can_rmp_msgtype_cmd1_set_req_byte2_t can_rmp_msgtype_cmd1_set_req_byte2 = {0};
can_rmp_msgtype_cmd1_set_req_byte3_7_t can_rmp_msgtype_cmd1_set_req_byte3_7 = {0};
can_rmp_msgtype_cmd2_set_req_byte2_7_t can_rmp_msgtype_cmd2_set_req_byte2_7 = {0};

uint8_t can_rmp_msgtype_cmd0_set_req_order[] = CAN_RMP_MSGTYPE_CMD0_SET_REQ_REDATA;

// 函数声明
static void can_rmp_handle_cmd0_set_req(const can_packet_t *packet);
static void can_rmp_handle_cmd1_set_req(const can_packet_t *packet);
static void can_rmp_handle_cmd2_set_req(const can_packet_t *packet);
static void can_rmp_handle_byte_read_req(const can_packet_t *packet);
static void can_rmp_handle_bit_read_req(const can_packet_t *packet);
static void can_rmp_handle_byte_write_req(const can_packet_t *packet);

// 上层分发接口
void can_comm_rmp_dispatch(const can_packet_t *packet)
{
    uint8_t msgtype = packet->data.data.msgtype;
    switch (msgtype)
    {
    case MSGTYPE_CMD0_SET_REQ:
        can_rmp_handle_cmd0_set_req(packet);
        break;
    case MSGTYPE_CMD1_SET_REQ:
        can_rmp_handle_cmd1_set_req(packet);
        break;
    case MSGTYPE_CMD2_SET_REQ:
        can_rmp_handle_cmd2_set_req(packet);
        break;
    case MSGTYPE_BYTE_READ_REQ:
        can_rmp_handle_byte_read_req(packet);
        break;
    case MSGTYPE_BIT_READ_REQ:
        can_rmp_handle_bit_read_req(packet);
        break;
    case MSGTYPE_BYTE_WRITE_REQ:
        can_rmp_handle_byte_write_req(packet);
        break;
    default:
        // 未知类型
        break;
    }
}

static void can_rmp_handle_cmd0_set_req(const can_packet_t *packet)
{
    can_rmp_msgtype_cmd0_set_req_byte2 = *(can_rmp_msgtype_cmd0_set_req_byte2_t *)&packet->data.raw[2];
    can_rmp_msgtype_cmd0_set_req_byte3 = *(can_rmp_msgtype_cmd0_set_req_byte3_t *)&packet->data.raw[3];
    can_rmp_msgtype_cmd0_set_req_byte4_7 = *(can_rmp_msgtype_cmd0_set_req_byte4_7_t *)&packet->data.raw[4];
    can_packet_data_u can_packet_data = {0};
    static const uint8_t cmd0_order[] = CAN_RMP_MSGTYPE_CMD0_SET_REQ_REDATA;
    for (uint8_t i = 0; i < sizeof(cmd0_order); i++)
    {
        can_packet_data.data.err = 0;
        can_packet_data.data.msgtype = packet->data.data.msgtype;
        can_packet_data.data.errtype = 0; // 无错误
        can_packet_data.data.data_info.valueinfo.valuetype.byte2 = 0x00;
        can_packet_data.data.data_info.valueinfo.valuetype.byte3 = cmd0_order[i];
        can_packet_data.data.data_info.valueinfo.value.u32 = valuetype_rmp_buffer[cmd0_order[i]].u32;
        can_packet_transmit_data(
            PROTNO_RMP,
            packet->id.id.ptp,     // ptp
            packet->id.id.srcaddr, // dstaddr
            packet->id.id.dstaddr, // srcaddr
            1,                     // cnt
            0,                     // res1
            0,                     // res2
            can_packet_data.raw);
    }
}

static void can_rmp_handle_cmd1_set_req(const can_packet_t *packet)
{
    can_rmp_msgtype_cmd1_set_req_byte2 = *(can_rmp_msgtype_cmd1_set_req_byte2_t *)&packet->data.raw[2];
    can_rmp_msgtype_cmd1_set_req_byte3_7 = *(can_rmp_msgtype_cmd1_set_req_byte3_7_t *)&packet->data.raw[3];
    can_packet_data_u can_packet_data = {0};
    static const uint8_t cmd1_order[] = CAN_PACK_MSGTYPE_CMD1_SET_REQ_REDATA;
    for (uint8_t i = 0; i < sizeof(cmd1_order); i++)
    {
        can_packet_data.data.err = 0;
        can_packet_data.data.msgtype = packet->data.data.msgtype;
        can_packet_data.data.errtype = 0;
        can_packet_data.data.data_info.valueinfo.valuetype.byte2 = 0x00;
        can_packet_data.data.data_info.valueinfo.valuetype.byte3 = cmd1_order[i];
        can_packet_data.data.data_info.valueinfo.value.u32 = valuetype_rmp_buffer[cmd1_order[i]].u32;
        can_packet_transmit_data(
            PROTNO_RMP,
            packet->id.id.ptp,
            packet->id.id.srcaddr,
            packet->id.id.dstaddr,
            1,
            0,
            0,
            can_packet_data.raw);
    }
}

static void can_rmp_handle_cmd2_set_req(const can_packet_t *packet)
{
    can_rmp_msgtype_cmd2_set_req_byte2_7 = *(can_rmp_msgtype_cmd2_set_req_byte2_7_t *)&packet->data.raw[2];
    can_packet_data_u can_packet_data = {0};
    static const uint8_t cmd2_order[] = CAN_PACK_MSGTYPE_CMD2_SET_REQ_REDATA;
    for (uint8_t i = 0; i < sizeof(cmd2_order); i++)
    {
        can_packet_data.data.err = 0;
        can_packet_data.data.msgtype = packet->data.data.msgtype;
        can_packet_data.data.errtype = 0;
        can_packet_data.data.data_info.valueinfo.valuetype.byte2 = 0x00;
        can_packet_data.data.data_info.valueinfo.valuetype.byte3 = cmd2_order[i];
        can_packet_data.data.data_info.valueinfo.value.u32 = valuetype_rmp_buffer[cmd2_order[i]].u32;
        can_packet_transmit_data(
            PROTNO_RMP,
            packet->id.id.ptp,
            packet->id.id.srcaddr,
            packet->id.id.dstaddr,
            1,
            0,
            0,
            can_packet_data.raw);
    }
}

static void can_rmp_handle_byte_read_req(const can_packet_t *packet) { (void)packet; }
static void can_rmp_handle_bit_read_req(const can_packet_t *packet) { (void)packet; }
static void can_rmp_handle_byte_write_req(const can_packet_t *packet) { (void)packet; }
