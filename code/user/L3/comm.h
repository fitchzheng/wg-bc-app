#ifndef __COMM_H__
#define __COMM_H__

#include "section.h"
#include <stddef.h>
#include <stdint.h>

#define CRC16_CCITT_POLY 0x1021u
#define CRC16_CCITT_INIT 0xFFFFu

uint16_t crc16_init(void);
uint16_t crc16_update(uint16_t crc, uint8_t data);
uint16_t crc16_final(uint16_t crc);
uint16_t section_crc16(uint8_t *p_data, uint32_t len);
uint16_t section_crc16_with_crc(uint8_t *p_data, uint32_t len, uint16_t crc_in);

#pragma pack(push, 1)
typedef struct
{
    uint8_t sop;
    uint8_t version;
    uint8_t src;
    uint8_t d_src;
    uint8_t dst;
    uint8_t d_dst;
    uint8_t cmd_set;
    uint8_t cmd_word;
    uint8_t is_ack;
    uint16_t len;
    uint8_t *p_data;
    uint16_t crc;
    uint16_t eop;
} section_packform_t;
#pragma pack(pop)

typedef enum
{
    SECTION_PACKFORM_STA_SOP = 0,
    SECTION_PACKFORM_STA_VER,
    SECTION_PACKFORM_STA_SRC,
    SECTION_PACKFORM_STA_DST,
    SECTION_PACKFORM_STA_CMD,
    SECTION_PACKFORM_STA_ACK,
    SECTION_PACKFORM_STA_LEN,
    SECTION_PACKFORM_STA_DATA,
    SECTION_PACKFORM_STA_CRC,
    SECTION_PACKFORM_STA_EOP,
    SECTION_PACKFORM_STA_ROUTE,
} SECTION_PACKFORM_STA_E;

typedef struct
{
    uint8_t *p_data_buffer;
    uint16_t buffer_size;
    uint16_t index;
    uint8_t status;
    uint16_t crc;
    section_packform_t pack;
    void (*func)(section_packform_t *p_pack, DEC_MY_PRINTF);
    uint16_t len;
    const uint8_t src;
    uint8_t d_src;
    uint8_t src_flag : 1;
    uint8_t dst_flag : 1;
    uint8_t cmd_flag : 1;
    uint8_t len_flag : 1;
    uint8_t eop_flag : 1;
    uint8_t is_route : 1;
    uint8_t link_id;
} comm_ctx_t;

#define DECLARE_COMM_CTX(name, payload_size, _src, _link_id) \
    static uint8_t name##_payload_buf[(payload_size)] = {0}; \
    static comm_ctx_t name = {                               \
        .p_data_buffer = name##_payload_buf,                 \
        .buffer_size = (uint16_t)sizeof(name##_payload_buf), \
        .index = 0,                                          \
        .status = SECTION_PACKFORM_STA_SOP,                  \
        .crc = 0,                                            \
        .pack = {0},                                         \
        .func = NULL,                                        \
        .len = 0,                                            \
        .src = (uint8_t)(_src),                              \
        .d_src = 0,                                          \
        .src_flag = 0,                                       \
        .dst_flag = 0,                                       \
        .cmd_flag = 0,                                       \
        .len_flag = 0,                                       \
        .eop_flag = 0,                                       \
        .is_route = 0,                                       \
        .link_id = (uint8_t)(_link_id),                      \
    };

typedef struct section_com_t
{
    uint8_t cmd_set;
    uint8_t cmd_word;
    void (*func)(section_packform_t *p_pack, DEC_MY_PRINTF);
    struct section_com_t *p_next;
} section_com_t;

#define _REG_COMM(_cmd_set, _cmd_word, _func)              \
    section_com_t section_com_##_cmd_set##_##_cmd_word = { \
        .cmd_set = (_cmd_set),                             \
        .cmd_word = (_cmd_word),                           \
        .func = (_func),                                   \
        .p_next = NULL,                                    \
    };                                                     \
    REG_SECTION_FUNC(SECTION_COMM, section_com_##_cmd_set##_##_cmd_word)

#define REG_COMM(_cmd_set, _cmd_word, _func) _REG_COMM(_cmd_set, _cmd_word, _func)

typedef struct comm_route_t
{
    uint8_t src_link_id;
    uint8_t dst_link_id;
    uint8_t dst_addr;
    struct comm_route_t *p_next;
} comm_route_t;

#define _REG_COMM_ROUTE(_src_link_id, _dst_link_id, _dst_addr)          \
    comm_route_t comm_route_##_src_link_id##_dst_link_id##_dst_addr = { \
        .src_link_id = (_src_link_id),                                  \
        .dst_link_id = (_dst_link_id),                                  \
        .dst_addr = (_dst_addr),                                        \
        .p_next = NULL,                                                 \
    };                                                                  \
    REG_SECTION_FUNC(SECTION_COMM_ROUTE, comm_route_##_src_link_id##_dst_link_id##_dst_addr)

#define REG_COMM_ROUTE(_src_link_id, _dst_link_id, _dst_addr) \
    _REG_COMM_ROUTE(_src_link_id, _dst_link_id, _dst_addr)

void comm_run(uint8_t data, DEC_MY_PRINTF, void *ctx);
void comm_send_data(section_packform_t *p_pack, DEC_MY_PRINTF);

#endif
