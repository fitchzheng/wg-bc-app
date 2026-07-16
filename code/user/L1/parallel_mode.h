#ifndef __PARALLEL_MODE_H__
#define __PARALLEL_MODE_H__

#include "stdint.h"
#include "app_features.h"

#define PARALLEL_RS485_BASE_ADDR              0x1200U
#define PARALLEL_RS485_REG_COUNT              0x0011U
#define PARALLEL_RS485_SUMMARY_ADDR           0x1220U
#define PARALLEL_RS485_SUMMARY_MAX_NODES      16U
#define PARALLEL_RS485_SUMMARY_HEADER_COUNT   4U
#define PARALLEL_RS485_SUMMARY_NODE_REG_COUNT 3U
#define PARALLEL_RS485_SUMMARY_REG_COUNT      (PARALLEL_RS485_SUMMARY_HEADER_COUNT + \
                                               (PARALLEL_RS485_SUMMARY_MAX_NODES * PARALLEL_RS485_SUMMARY_NODE_REG_COUNT))

#define PARALLEL_STATE_IDLE                   0U
#define PARALLEL_STATE_PREPARE                1U
#define PARALLEL_STATE_BLOCKED                2U
#define PARALLEL_STATE_RUN_ALLOWED            3U
#define PARALLEL_STATE_FAULT                  4U

#define PARALLEL_ROLE_UNKNOWN                 0U
#define PARALLEL_ROLE_MASTER                  1U
#define PARALLEL_ROLE_SLAVE                   2U

#define PARALLEL_CONTROL_STOP                 0U
#define PARALLEL_CONTROL_PREPARE_START        1U
#define PARALLEL_CONTROL_FORCE_STOP           2U
#define PARALLEL_CONTROL_RUN_ENABLE           3U

#define PARALLEL_READY_PG_OFF                 0x0001U
#define PARALLEL_READY_HW_VALID               0x0002U
#define PARALLEL_READY_OUTPUT_OFF             0x0004U
#define PARALLEL_READY_NO_FAULT               0x0008U
#define PARALLEL_READY_HEARTBEAT_FRESH        0x0010U
#define PARALLEL_READY_PARAMS_CRC             0x0020U
#define PARALLEL_READY_GROUP_LIMITING         0x0080U

#define PARALLEL_BLOCK_NONE                   0x0000U
#define PARALLEL_BLOCK_CONFIG_MISSING         0x0001U
#define PARALLEL_BLOCK_LOCAL_NOT_READY        0x0002U
#define PARALLEL_BLOCK_FAULT                  0x0003U
#define PARALLEL_BLOCK_FORCE_STOP             0x0004U
#define PARALLEL_BLOCK_PARAMS_MISMATCH        0x0005U
#define PARALLEL_BLOCK_NODE_COUNT             0x0006U
#define PARALLEL_BLOCK_NODE_NOT_READY         0x0007U

typedef struct
{
    uint16_t reserved0;
    uint16_t status_role;
    uint32_t session_id;
    uint32_t uid32;
    uint16_t temp_addr;
    uint16_t discovered_count;
    uint16_t ready_flags;
    uint16_t block_fault;
    uint32_t params_crc32;
    uint16_t control;
    uint16_t heartbeat;
    uint32_t capability;
    uint16_t protocol_version;
} parallel_mode_status_t;

void parallel_mode_run_10ms(void);
void parallel_mode_on_rvc_rx(uint32_t dgn, const uint8_t *data, uint8_t len, uint8_t source_addr);
void parallel_mode_on_rs485_frame(const uint8_t *frame, uint16_t len);
uint8_t parallel_mode_make_rvc_response(uint32_t dgn, uint8_t *data, uint8_t len);
uint8_t parallel_mode_is_run_allowed(void);
uint8_t parallel_mode_should_block_local_run(void);
uint8_t parallel_mode_get_rs485_runtime_addr(void);
uint8_t parallel_mode_get_can_runtime_addr(void);
uint8_t parallel_mode_read_registers(uint16_t addr, uint16_t count, uint8_t *data);
uint8_t parallel_mode_write_registers(uint16_t addr, uint16_t count, const uint8_t *data);

#endif
