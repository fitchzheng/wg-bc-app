#ifndef __CAN_PACKET_H__
#define __CAN_PACKET_H__

#include <stdint.h>

#pragma pack(push, 1)
typedef struct
{
    uint32_t res2 : 1;
    uint32_t res1 : 1;
    uint32_t cnt : 1;
    uint32_t srcaddr : 8;
    uint32_t dstaddr : 8;
    uint32_t ptp : 1;
    uint32_t protno : 9;
    uint32_t rsvd : 3;
} can_packet_id_t;

typedef union
{
    can_packet_id_t id;
    uint32_t raw;
} can_packet_id_u;

typedef union
{
    float f;
    uint32_t u32;
    uint8_t bytes[4];
} can_packet_data_value_u;

typedef struct
{
    uint8_t byte2;
    uint8_t byte3;
} can_packet_data_valuetype_t;

typedef struct
{
    can_packet_data_valuetype_t valuetype;
    can_packet_data_value_u value;
} can_packet_value_info_t;

typedef union
{
    uint8_t bytes[6];
    can_packet_value_info_t valueinfo;
} can_packet_data_info_u;

typedef struct
{
    uint8_t msgtype : 7;
    uint8_t err : 1;
    uint8_t errtype;
    can_packet_data_info_u data_info;
} can_packet_data_t;

typedef union
{
    can_packet_data_t data;
    uint8_t raw[8];
} can_packet_data_u;

typedef struct
{
    can_packet_id_u id;
    can_packet_data_u data;
} can_packet_t;
#pragma pack(pop)

// 消息类型
#define CAN_MSG_SET_DATA 0x00
#define CAN_MSG_READ_DATA 0x02

// 帧命令类型（举例）
#define CMD_VOUT_REF 0x02

can_packet_id_t can_build_ext_id(uint16_t protno, uint8_t ptp, uint8_t dstaddr, uint8_t srcaddr, uint8_t cnt, uint8_t res1, uint8_t res2);

void can_build_payload(uint8_t *data, uint8_t group, uint8_t msg_type, uint8_t cmd_type, uint32_t cmd_data);

void can_packet_transmit_data(uint16_t protno, uint8_t ptp, uint8_t dstaddr, uint8_t srcaddr, uint8_t cnt, uint8_t res1, uint8_t res2, uint8_t *data);

int can_packet_receive(can_packet_t *packet);

#endif
