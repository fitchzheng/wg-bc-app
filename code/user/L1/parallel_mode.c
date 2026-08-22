#include "parallel_mode.h"
#include "app_features.h"
#include "section.h"
#include "wg_com_v2.h"
#include "ctrl_app.h"
#include "fault.h"
#include "eeprom_cfg.h"
#include "gpio.h"
#include "power_sw.h"
#include "hc32_ll_efm.h"
#include "string.h"

#if (APP_PARALLEL_MODE_FEATURES == 1)

#if (APP_PARALLEL_CAN_FEATURES == 1)
#include "bsp_can.h"
#include "rvc_address.h"
#endif

#define PARALLEL_PROTOCOL_VERSION             1U
#define PARALLEL_CAP_CAN                      0x0001UL
#define PARALLEL_CAP_RS485                    0x0002UL
#define PARALLEL_CAP_LOCAL_DROOP              0x0004UL
#define PARALLEL_HEARTBEAT_TICKS              50U
#define PARALLEL_MAX_NODE_COUNT               30U
#define PARALLEL_NODE_TIMEOUT_TICKS           300U

#define PARALLEL_NODE_FLAG_USED               0x01U
#define PARALLEL_NODE_FLAG_LOCAL              0x02U
#define PARALLEL_NODE_FLAG_READY              0x04U

#define PARALLEL_ADDR_RANK_MASTER             1U
#define PARALLEL_RS485_TEMP_ADDR_MASTER       2U
#define PARALLEL_CAN_TEMP_ADDR_MASTER         75U
#define PARALLEL_RS485_MAX_ADDR               31U
#define PARALLEL_RS485_POLL_TICKS             100U
#define PARALLEL_RS485_RUN_IDLE_TICKS         500U
#define PARALLEL_RS485_ELECTION_WAIT_TICKS    1500U
#define PARALLEL_RS485_UID_ANNOUNCE_FUNC      0x41U
#define PARALLEL_RS485_UID_ASSIGN_FUNC        0x42U
#define PARALLEL_RS485_READY_OK_FUNC          0x43U
#define PARALLEL_RS485_UID_FRAME_DATA_LEN     8U
#define PARALLEL_RS485_UID_FRAME_LEN          13U
#define PARALLEL_RS485_UID_ANNOUNCE_ROUNDS    3U
#define PARALLEL_RS485_UID_ANNOUNCE_SLOTS     30U
#define PARALLEL_RS485_UID_ANNOUNCE_SLOT_TICKS 10U
#define PARALLEL_RS485_UID_ANNOUNCE_ACTIVE_TICKS \
    (PARALLEL_RS485_UID_ANNOUNCE_ROUNDS * \
     PARALLEL_RS485_UID_ANNOUNCE_SLOTS * \
     PARALLEL_RS485_UID_ANNOUNCE_SLOT_TICKS)
#define PARALLEL_RS485_ASSIGN_REG             0x1206U
#define PARALLEL_RS485_CONTROL_REG            0x120CU
#define PARALLEL_RS485_CRC_CONFIRM_REG        0x120AU
#define PARALLEL_RS485_CRC_CONFIRM_COUNT      0x0002U
#define PARALLEL_RS485_CTRL_SYNC_START        0x0402U
#define PARALLEL_RS485_CTRL_SYNC_COUNT        0x000DU
#define PARALLEL_RS485_PARAM_SYNC_START       0x081AU
#define PARALLEL_RS485_PARAM_SYNC_COUNT       0x001AU
#define PARALLEL_RS485_CRC_CONFIRM_SETTLE_POLLS 2U
#define PARALLEL_RS485_STATUS_REPLY_WAIT_POLLS 1U
#define PARALLEL_RS485_BACKEND_READ_TIMEOUT_POLLS 1U
#define PARALLEL_RS485_MASTER_POLL_REPLY_GUARD_POLLS 1U
#define PARALLEL_RS485_BROADCAST_REPLY_INTERVAL_MS 200U
#define PARALLEL_RS485_BROADCAST_REPLY_INTERVAL_TICKS \
    (PARALLEL_RS485_BROADCAST_REPLY_INTERVAL_MS / 10U)

#define PARALLEL_TRANSPORT_RS485             0x01U
#define PARALLEL_TRANSPORT_CAN               0x02U
#define PARALLEL_CAN_POLL_TICKS              50U
#define PARALLEL_CAN_PARAM_SYNC_ITEM_COUNT   \
    (PARALLEL_RS485_CTRL_SYNC_COUNT + PARALLEL_RS485_PARAM_SYNC_COUNT + PARALLEL_RS485_CRC_CONFIRM_COUNT)
#define PARALLEL_CAN_PARAM_SYNC_FULL_LO_MASK 0xFFFFFFFFUL
#define PARALLEL_CAN_PARAM_SYNC_FULL_HI_MASK 0x01FFU
#define PARALLEL_CAN_DGN_UID_ANNOUNCE        0xEF60UL
#define PARALLEL_CAN_DGN_UID_ASSIGN          0xEF62UL
#define PARALLEL_CAN_DGN_READY_OK            0xEF64UL
#define PARALLEL_CAN_DGN_STATUS              0xEF65UL
#define PARALLEL_CAN_DGN_CONTROL             0xEF66UL
#define PARALLEL_CAN_DGN_PARAM_SUMMARY       0xEF68UL
#define PARALLEL_CAN_DGN_PARAM_SYNC          0xEF69UL
#define PARALLEL_CAN_DGN_REQUEST             0xEA00UL

static parallel_mode_status_t parallel_status;
static uint16_t parallel_tick_10ms;
static uint8_t parallel_node_tick_10ms;
static uint32_t parallel_confirmed_params_crc32;
static uint32_t parallel_requested_params_crc32;
static uint8_t parallel_requested_params_crc_mask;
static uint32_t parallel_master_uid32;
static uint8_t parallel_master_locked;
static uint8_t parallel_rs485_active;
static uint16_t parallel_rs485_poll_tick;
static uint8_t parallel_rs485_sync_phase;
static uint8_t parallel_rs485_sync_addr;
static uint8_t parallel_rs485_sync_wait_polls;
static uint8_t parallel_rs485_prepare_ready_latched;
static uint8_t parallel_rs485_prepare_ok_sent;
static uint8_t parallel_rs485_assigned_slave;
static uint8_t parallel_rs485_backend_read_quiet_polls;
static uint8_t parallel_rs485_master_poll_reply_guard_polls;
static uint16_t parallel_rs485_election_wait_tick;
static uint8_t parallel_rs485_uid_announce_sent_mask;
static uint8_t parallel_can_active;
static uint16_t parallel_can_poll_tick;
static uint8_t parallel_can_run_request_phase;

static uint8_t parallel_can_sync_index;
static uint8_t parallel_can_prepare_ready_latched;
static uint8_t parallel_can_prepare_ok_sent;
static uint8_t parallel_can_assigned_slave;
static uint8_t parallel_can_uid_announce_sent_mask;
static uint8_t parallel_can_pc_control_seen;
static uint32_t parallel_can_param_sync_mask_low;
static uint16_t parallel_can_param_sync_mask_high;

typedef struct
{
    uint32_t uid32;
    uint32_t params_crc32;
    uint16_t ready_flags;
    uint16_t block_fault;
    uint16_t timeout;
    uint8_t temp_addr;
    uint8_t role_state;
    uint8_t flags;
} parallel_node_t;

static parallel_node_t parallel_nodes[PARALLEL_MAX_NODE_COUNT];

#if (APP_PARALLEL_CAN_FEATURES != 1)
static void parallel_can_feature_off_keep_refs(void)
{
    (void)parallel_can_poll_tick;
    (void)parallel_can_run_request_phase;
    (void)parallel_can_sync_index;
    (void)parallel_can_prepare_ok_sent;
    (void)parallel_can_uid_announce_sent_mask;
    (void)parallel_can_param_sync_mask_low;
    (void)parallel_can_param_sync_mask_high;
}
#endif

#define parallel_get_state() \
    ((uint16_t)(parallel_status.status_role & 0x00FFU))
#define parallel_get_role() \
    ((uint16_t)((parallel_status.status_role >> 8U) & 0x00FFU))
#define parallel_pg_is_active() \
    ((get_key_pg_val() == 0U) ? 1U : 0U)
#define parallel_set_state(state_) \
    do \
    { \
        parallel_status.status_role = (uint16_t)((parallel_status.status_role & 0xFF00U) | \
                                                ((uint16_t)(state_) & 0x00FFU)); \
    } while (0)
#define parallel_set_role(role_) \
    do \
    { \
        parallel_status.status_role = (uint16_t)((parallel_status.status_role & 0x00FFU) | \
                                                (((uint16_t)(role_) & 0x00FFU) << 8U)); \
    } while (0)
#define parallel_rank_is_valid(rank_) \
    ((((uint8_t)(rank_) >= PARALLEL_ADDR_RANK_MASTER) && \
      ((uint8_t)(rank_) <= PARALLEL_MAX_NODE_COUNT)) ? 1U : 0U)
#define parallel_rs485_addr_to_rank(addr_) \
    ((((uint8_t)(addr_) >= PARALLEL_RS485_TEMP_ADDR_MASTER) && \
      ((uint8_t)(addr_) <= PARALLEL_RS485_MAX_ADDR)) ? \
     (uint8_t)((uint8_t)(addr_) - PARALLEL_RS485_TEMP_ADDR_MASTER + 1U) : \
     PARALLEL_ADDR_RANK_MASTER)
#define parallel_rank_to_rs485_addr(rank_) \
    ((uint8_t)(PARALLEL_RS485_TEMP_ADDR_MASTER + (uint8_t)(rank_) - 1U))
#define parallel_rank_to_can_addr(rank_) \
    ((uint8_t)(PARALLEL_CAN_TEMP_ADDR_MASTER + (uint8_t)(rank_) - 1U))
#define parallel_write_u16_be(data_, offset_, value_) \
    do \
    { \
        (data_)[(uint8_t)(offset_)] = (uint8_t)(((uint16_t)(value_) >> 8U) & 0x00FFU); \
        (data_)[(uint8_t)((uint8_t)(offset_) + 1U)] = (uint8_t)((uint16_t)(value_) & 0x00FFU); \
    } while (0)
#define parallel_read_u16_be(data_, offset_) \
    ((uint16_t)(((uint16_t)(data_)[(uint8_t)(offset_)] << 8U) | \
                (uint16_t)(data_)[(uint8_t)((uint8_t)(offset_) + 1U)]))
#define parallel_write_u32_be(data_, offset_, value_) \
    do \
    { \
        (data_)[(uint8_t)(offset_)] = (uint8_t)(((uint32_t)(value_) >> 24U) & 0x000000FFUL); \
        (data_)[(uint8_t)((uint8_t)(offset_) + 1U)] = (uint8_t)(((uint32_t)(value_) >> 16U) & 0x000000FFUL); \
        (data_)[(uint8_t)((uint8_t)(offset_) + 2U)] = (uint8_t)(((uint32_t)(value_) >> 8U) & 0x000000FFUL); \
        (data_)[(uint8_t)((uint8_t)(offset_) + 3U)] = (uint8_t)((uint32_t)(value_) & 0x000000FFUL); \
    } while (0)
#define parallel_read_u32_be(data_, offset_) \
    ((((uint32_t)(data_)[(uint8_t)(offset_)]) << 24U) | \
     (((uint32_t)(data_)[(uint8_t)((uint8_t)(offset_) + 1U)]) << 16U) | \
     (((uint32_t)(data_)[(uint8_t)((uint8_t)(offset_) + 2U)]) << 8U) | \
     ((uint32_t)(data_)[(uint8_t)((uint8_t)(offset_) + 3U)]))
#define parallel_mode_range_inside(addr_, count_, allowed_addr_, allowed_count_) \
    ((((count_) != 0U) && \
      ((allowed_count_) != 0U) && \
      ((uint32_t)(addr_) >= (uint32_t)(allowed_addr_)) && \
      (((uint32_t)(addr_) + (uint32_t)(count_)) <= \
       ((uint32_t)(allowed_addr_) + (uint32_t)(allowed_count_)))) ? 1U : 0U)

static uint32_t parallel_calc_params_crc(void);
static void parallel_update_params_crc(void);
static void parallel_apply_params_crc(uint32_t params_crc32);
static void parallel_update_local_ready_flags(void);
static void parallel_update_prepare_block(void);
static uint8_t parallel_rs485_prepare_ready_hold(void);
static uint8_t parallel_node_find_by_rank(uint8_t rank);
static void parallel_note_rs485_runtime_activity(uint8_t addr);
static void parallel_note_rs485_broadcast_status_request(const uint8_t *frame, uint16_t len);
static uint8_t parallel_node_rank_under_master(uint32_t uid32);
static uint8_t parallel_node_sync_rank(uint8_t index);
static void parallel_node_mark_remote_run_allowed(void);
static void parallel_node_refresh_remote_timeouts(void);
static void parallel_stop_running_group(uint16_t block_fault);
static void parallel_rs485_clear_control_state(void);
static void parallel_can_clear_control_state(void);
static void parallel_rs485_latch_prepare_ready(void);
static void parallel_can_latch_prepare_ready(void);
static void parallel_rs485_run_10ms(void);
static void parallel_can_run_10ms(void);
static uint16_t parallel_rs485_get_run_refresh_ticks(void);
static uint8_t parallel_rs485_poll_run_status(void);
static uint8_t parallel_rs485_send_short(uint8_t addr, uint8_t cmd, uint16_t reg_addr, uint16_t value);
static uint8_t parallel_can_send_control(uint16_t control);
static uint8_t parallel_rs485_uid_announce_active(void);
static uint8_t parallel_rs485_send_prepare_ready_ok(void);
static uint8_t parallel_can_send_prepare_ready_ok(void);
static uint8_t parallel_rs485_status_payload_is_valid(uint8_t addr, const uint8_t *data);
static uint8_t parallel_rs485_node_run_status_is_valid(uint8_t index);
static uint8_t parallel_mode_should_reply_rs485_broadcast_read(uint16_t addr, uint16_t count);
static uint8_t parallel_mode_should_reply_rs485_broadcast_write(uint16_t addr, uint16_t count);

#define parallel_rs485_send_read(addr_, reg_addr_, count_) \
    parallel_rs485_send_short((addr_), WG_COM_V2_CMD_READ, (reg_addr_), (count_))
#define parallel_rs485_send_write_one(addr_, reg_addr_, value_) \
    parallel_rs485_send_short((addr_), WG_COM_V2_CMD_WRITE_DATA, (reg_addr_), (value_))
#define parallel_sanitize_block_fault(block_fault_) \
    ((((uint16_t)(block_fault_)) <= PARALLEL_BLOCK_NODE_NOT_READY) ? \
     (uint16_t)(block_fault_) : PARALLEL_BLOCK_NONE)

static uint8_t parallel_rs485_status_payload_is_valid(uint8_t addr, const uint8_t *data)
{
    uint8_t rank;
    uint8_t role;
    uint8_t state;
    uint8_t runtime_addr;
    uint16_t status_role;

    rank = parallel_rs485_addr_to_rank(addr);
    if (parallel_read_u16_be(data, 0U) != 0U)
    {
        return 0U;
    }

    status_role = parallel_read_u16_be(data, 2U);
    role = (uint8_t)((status_role >> 8U) & 0x00FFU);
    state = (uint8_t)(status_role & 0x00FFU);
    if ((role != PARALLEL_ROLE_MASTER) && (role != PARALLEL_ROLE_SLAVE))
    {
        return 0U;
    }

    if (state > PARALLEL_STATE_RUN_ALLOWED)
    {
        return 0U;
    }

    runtime_addr = (uint8_t)parallel_read_u16_be(data, 12U);
    if (runtime_addr != addr)
    {
        return 0U;
    }

    if (runtime_addr != parallel_rank_to_rs485_addr(rank))
    {
        return 0U;
    }

    if (parallel_read_u32_be(data, 8U) == 0UL)
    {
        return 0U;
    }

    return 1U;
}

static uint8_t parallel_is_requested(void)
{
    return (((parallel_pg_is_active() != 0U) ||
             (parallel_status.control == PARALLEL_CONTROL_PREPARE_START) ||
             (parallel_status.control == PARALLEL_CONTROL_RUN_ENABLE)) ? 1U : 0U);
}

static uint8_t parallel_rs485_pre_election_active(void)
{
    return (((parallel_master_locked == 0U) &&
             (parallel_rs485_uid_announce_active() != 0U)) ? 1U : 0U);
}

static uint8_t parallel_get_local_rank(void)
{
    uint8_t rank = (uint8_t)(parallel_status.temp_addr & 0x00FFU);

    if ((rank == 0U) || (rank > PARALLEL_MAX_NODE_COUNT))
    {
        rank = PARALLEL_ADDR_RANK_MASTER;
    }

    return rank;
}

static uint32_t parallel_hash_bytes(uint32_t hash, const uint8_t *data, uint16_t length)
{
    uint16_t index;

    for (index = 0U; index < length; index++)
    {
        hash ^= data[index];
        hash *= 16777619UL;
    }

    return hash;
}

static uint32_t parallel_build_uid32(void)
{
    uint32_t hash = 2166136261UL;
    stc_efm_unique_id_t unique_id = {0UL, 0UL, 0UL};

    EFM_GetUID(&unique_id);
    hash = parallel_hash_bytes(hash, (const uint8_t *)&unique_id, (uint16_t)sizeof(unique_id));

    if (hash == 0UL)
    {
        hash = 1UL;
    }

    return hash;
}

static void parallel_init_state_if_needed(void)
{
    if (parallel_status.protocol_version != PARALLEL_PROTOCOL_VERSION)
    {
        memset(&parallel_status, 0, sizeof(parallel_status));
        parallel_status.protocol_version = PARALLEL_PROTOCOL_VERSION;
        parallel_status.uid32 = parallel_build_uid32();
        parallel_status.temp_addr = PARALLEL_ADDR_RANK_MASTER;
        parallel_status.discovered_count = 1U;
        parallel_status.block_fault = PARALLEL_BLOCK_CONFIG_MISSING;
        parallel_status.capability = PARALLEL_CAP_LOCAL_DROOP;
#if (APP_PARALLEL_CAN_FEATURES == 1)
        parallel_status.capability |= PARALLEL_CAP_CAN;
#endif
#if (APP_PARALLEL_RS485_FEATURES == 1)
        parallel_status.capability |= PARALLEL_CAP_RS485;
#endif
        parallel_set_role(PARALLEL_ROLE_UNKNOWN);
        parallel_set_state(PARALLEL_STATE_IDLE);
    }
}

static uint8_t parallel_node_find(uint32_t uid32)
{
    uint8_t index;

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            (parallel_nodes[index].uid32 == uid32))
        {
            return index;
        }
    }

    return PARALLEL_MAX_NODE_COUNT;
}

static uint8_t parallel_node_find_sync_target(void)
{
    uint8_t index;
    uint8_t rank;
    uint8_t best_index = PARALLEL_MAX_NODE_COUNT;
    uint8_t best_rank = (uint8_t)(PARALLEL_MAX_NODE_COUNT + 1U);

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_LOCAL) == 0U) &&
            (parallel_nodes[index].uid32 != parallel_master_uid32))
        {
            rank = parallel_node_sync_rank(index);
            if (parallel_rank_is_valid(rank) == 0U)
            {
                continue;
            }

            if ((parallel_nodes[index].temp_addr <= PARALLEL_ADDR_RANK_MASTER) ||
                ((parallel_nodes[index].ready_flags & PARALLEL_READY_PARAMS_CRC) == 0U) ||
                (parallel_nodes[index].params_crc32 != parallel_status.params_crc32))
            {
                if (rank < best_rank)
                {
                    best_rank = rank;
                    best_index = index;
                }
            }
        }
    }

    return best_index;
}

static void parallel_node_ack_matching_crc_nodes(void)
{
    uint8_t index;

    if (parallel_status.params_crc32 == 0UL)
    {
        return;
    }

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_LOCAL) == 0U) &&
            (parallel_rank_is_valid(parallel_nodes[index].temp_addr) != 0U) &&
            (parallel_nodes[index].params_crc32 == parallel_status.params_crc32))
        {
            parallel_nodes[index].ready_flags |= PARALLEL_READY_PARAMS_CRC;
            if (parallel_nodes[index].block_fault == PARALLEL_BLOCK_PARAMS_MISMATCH)
            {
                parallel_nodes[index].block_fault = PARALLEL_BLOCK_NONE;
            }
        }
    }
}

static void parallel_node_mark_remote_run_allowed(void)
{
    uint8_t index;
    uint8_t role;

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_LOCAL) == 0U) &&
            (parallel_nodes[index].temp_addr > PARALLEL_ADDR_RANK_MASTER) &&
            (parallel_rank_is_valid(parallel_nodes[index].temp_addr) != 0U))
        {
            role = (uint8_t)(parallel_nodes[index].role_state & 0xF0U);
            parallel_nodes[index].role_state = (uint8_t)(role | PARALLEL_STATE_RUN_ALLOWED);
            parallel_nodes[index].block_fault = PARALLEL_BLOCK_NONE;
            parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
        }
    }
}

static void parallel_node_refresh_remote_timeouts(void)
{
    uint8_t index;

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_LOCAL) == 0U))
        {
            parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
            parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;
        }
    }
}

static uint8_t parallel_node_alloc(uint32_t uid32)
{
    uint8_t index;

    if (uid32 == 0UL)
    {
        return PARALLEL_MAX_NODE_COUNT;
    }

    index = parallel_node_find(uid32);
    if (index < PARALLEL_MAX_NODE_COUNT)
    {
        return index;
    }

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) == 0U)
        {
            memset(&parallel_nodes[index], 0, sizeof(parallel_nodes[index]));
            parallel_nodes[index].uid32 = uid32;
            parallel_nodes[index].flags = PARALLEL_NODE_FLAG_USED;
            parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
            return index;
        }
    }

    return PARALLEL_MAX_NODE_COUNT;
}

static void parallel_block_if_running(uint16_t block_fault)
{
    if (parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED)
    {
        parallel_stop_running_group(block_fault);
    }
}

static void parallel_node_refresh_local(void)
{
    uint8_t index;

    index = parallel_node_alloc(parallel_status.uid32);
    if (index < PARALLEL_MAX_NODE_COUNT)
    {
        parallel_nodes[index].params_crc32 = parallel_status.params_crc32;
        parallel_nodes[index].ready_flags = parallel_status.ready_flags;
        parallel_nodes[index].block_fault = parallel_status.block_fault;
        parallel_nodes[index].temp_addr = (uint8_t)(parallel_status.temp_addr & 0x00FFU);
        parallel_nodes[index].role_state = (uint8_t)((parallel_get_role() << 4) | parallel_get_state());
        parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
        parallel_nodes[index].flags = PARALLEL_NODE_FLAG_USED |
                                      PARALLEL_NODE_FLAG_LOCAL |
                                      PARALLEL_NODE_FLAG_READY;
    }
}

static uint8_t parallel_node_count_used(void)
{
    uint8_t index;
    uint8_t count = 0U;

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            (parallel_rank_is_valid(parallel_nodes[index].temp_addr) != 0U))
        {
            count++;
        }
    }

    return count;
}

static uint8_t parallel_node_find_by_rank(uint8_t rank)
{
    uint8_t index;

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            (parallel_nodes[index].temp_addr == rank))
        {
            return index;
        }
    }

    return PARALLEL_MAX_NODE_COUNT;
}

static void parallel_note_rs485_runtime_activity(uint8_t addr)
{
    uint8_t rank;
    uint8_t index;

    if (parallel_get_state() != PARALLEL_STATE_RUN_ALLOWED)
    {
        return;
    }

    rank = parallel_rs485_addr_to_rank(addr);
    if ((parallel_rank_is_valid(rank) == 0U) ||
        (rank == parallel_get_local_rank()))
    {
        return;
    }

    index = parallel_node_find_by_rank(rank);
    if ((index < PARALLEL_MAX_NODE_COUNT) &&
        ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_LOCAL) == 0U))
    {
        parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
        parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;
    }
}

static void parallel_note_rs485_broadcast_status_request(const uint8_t *frame, uint16_t len)
{
    uint16_t reg_addr;
    uint16_t count;

    if ((parallel_get_state() != PARALLEL_STATE_RUN_ALLOWED) ||
        (parallel_get_role() != PARALLEL_ROLE_SLAVE) ||
        (frame == NULL) ||
        (len < 8U) ||
        (frame[0] != WG_COM_V2_BROADCAST_ADDR) ||
        (frame[1] != WG_COM_V2_CMD_READ))
    {
        return;
    }

    reg_addr = parallel_read_u16_be(frame, 2U);
    count = parallel_read_u16_be(frame, 4U);
    if ((reg_addr == PARALLEL_RS485_BASE_ADDR) &&
        (count == PARALLEL_RS485_REG_COUNT))
    {
        parallel_node_refresh_remote_timeouts();
    }
}

static uint8_t parallel_rs485_node_run_status_is_valid(uint8_t index)
{
    uint16_t required_flags;
    uint8_t state;

    required_flags = (PARALLEL_READY_PG_OFF |
                      PARALLEL_READY_HW_VALID |
                      PARALLEL_READY_NO_FAULT |
                      PARALLEL_READY_HEARTBEAT_FRESH |
                      PARALLEL_READY_PARAMS_CRC);
    state = (uint8_t)(parallel_nodes[index].role_state & 0x0FU);

    if (state != PARALLEL_STATE_RUN_ALLOWED)
    {
        return 0U;
    }

    if ((parallel_nodes[index].ready_flags & required_flags) != required_flags)
    {
        return 0U;
    }

    if (parallel_nodes[index].block_fault != PARALLEL_BLOCK_NONE)
    {
        return 0U;
    }

    if ((parallel_status.params_crc32 == 0UL) ||
        (parallel_nodes[index].params_crc32 != parallel_status.params_crc32))
    {
        return 0U;
    }

    return 1U;
}

static uint32_t parallel_node_min_uid(void)
{
    uint8_t index;
    uint32_t min_uid = 0UL;

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U)
        {
            if ((min_uid == 0UL) || (parallel_nodes[index].uid32 < min_uid))
            {
                min_uid = parallel_nodes[index].uid32;
            }
        }
    }

    return min_uid;
}

static uint8_t parallel_master_is_present(void)
{
    return (((parallel_master_uid32 != 0UL) &&
             (parallel_node_find(parallel_master_uid32) < PARALLEL_MAX_NODE_COUNT)) ? 1U : 0U);
}

static uint8_t parallel_node_rank_under_master(uint32_t uid32)
{
    uint8_t index;
    uint8_t rank = 2U;

    if ((parallel_master_uid32 == 0UL) || (uid32 == parallel_master_uid32))
    {
        return PARALLEL_ADDR_RANK_MASTER;
    }

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            (parallel_nodes[index].uid32 != parallel_master_uid32) &&
            (parallel_nodes[index].uid32 < uid32))
        {
            rank++;
        }
    }

    return (rank <= PARALLEL_MAX_NODE_COUNT) ? rank : PARALLEL_MAX_NODE_COUNT;
}

static uint8_t parallel_node_sync_rank(uint8_t index)
{
    uint8_t rank;

    if ((parallel_get_role() == PARALLEL_ROLE_MASTER) &&
        (parallel_status.control == PARALLEL_CONTROL_PREPARE_START) &&
        (parallel_get_state() == PARALLEL_STATE_PREPARE))
    {
        return parallel_node_rank_under_master(parallel_nodes[index].uid32);
    }

    rank = parallel_nodes[index].temp_addr;
    if ((rank <= PARALLEL_ADDR_RANK_MASTER) || (rank > PARALLEL_MAX_NODE_COUNT))
    {
        rank = parallel_node_rank_under_master(parallel_nodes[index].uid32);
    }
    return rank;
}

static void parallel_select_master_if_needed(void)
{
    if (parallel_master_is_present() == 0U)
    {
        parallel_master_uid32 = parallel_node_min_uid();
    }
}

static void parallel_note_remote_role(uint32_t uid32, uint8_t role)
{
    if ((uid32 == 0UL) || (uid32 == parallel_status.uid32))
    {
        return;
    }

    if (role == PARALLEL_ROLE_MASTER)
    {
        if (parallel_master_uid32 == 0UL)
        {
            parallel_master_uid32 = uid32;
        }
        else if ((parallel_master_uid32 == parallel_status.uid32) &&
                 (parallel_master_locked == 0U) &&
                 (uid32 < parallel_status.uid32))
        {
            parallel_master_uid32 = uid32;
        }
    }
}

static void parallel_update_role_and_addr(void)
{
    uint8_t rank;

    parallel_node_refresh_local();
    parallel_status.discovered_count = parallel_node_count_used();

    if (parallel_rs485_pre_election_active() != 0U)
    {
        parallel_status.temp_addr = PARALLEL_ADDR_RANK_MASTER;
        parallel_set_role(PARALLEL_ROLE_UNKNOWN);
        parallel_node_refresh_local();
        return;
    }

    rank = (uint8_t)(parallel_status.temp_addr & 0x00FFU);
    if (((parallel_rs485_assigned_slave != 0U) ||
         (parallel_can_assigned_slave != 0U)) &&
        (rank > PARALLEL_ADDR_RANK_MASTER) &&
        (rank <= PARALLEL_MAX_NODE_COUNT))
    {
        parallel_set_role(PARALLEL_ROLE_SLAVE);
        parallel_status.temp_addr = rank;
        parallel_node_refresh_local();
        return;
    }

    parallel_select_master_if_needed();

    if (parallel_master_uid32 == 0UL)
    {
        parallel_master_uid32 = parallel_status.uid32;
    }

    if ((parallel_status.discovered_count > 1U) && (parallel_master_uid32 != 0UL))
    {
        parallel_master_locked = 1U;
    }

    if (parallel_status.uid32 == parallel_master_uid32)
    {
        parallel_set_role(PARALLEL_ROLE_MASTER);
        parallel_status.temp_addr = PARALLEL_ADDR_RANK_MASTER;
    }
    else
    {
        parallel_set_role(PARALLEL_ROLE_SLAVE);
        if ((parallel_status.control == PARALLEL_CONTROL_PREPARE_START) &&
            (parallel_master_locked != 0U) &&
            (parallel_rs485_assigned_slave == 0U) &&
            (parallel_can_assigned_slave == 0U) &&
            (rank <= PARALLEL_ADDR_RANK_MASTER))
        {
            rank = PARALLEL_ADDR_RANK_MASTER;
        }
        else if ((rank <= PARALLEL_ADDR_RANK_MASTER) || (rank > PARALLEL_MAX_NODE_COUNT))
        {
            rank = parallel_node_rank_under_master(parallel_status.uid32);
        }
        if (parallel_rank_is_valid(rank) == 0U)
        {
            rank = parallel_node_rank_under_master(parallel_status.uid32);
        }
        parallel_status.temp_addr = rank;
    }

    parallel_node_refresh_local();
}

static void parallel_node_age_100ms(void)
{
    uint8_t index;
    uint8_t removed = 0U;

    if ((parallel_status.control == PARALLEL_CONTROL_PREPARE_START) &&
        (parallel_get_role() == PARALLEL_ROLE_MASTER) &&
        (parallel_get_state() == PARALLEL_STATE_PREPARE))
    {
        return;
    }

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_LOCAL) == 0U))
        {
            if (parallel_nodes[index].timeout > 0U)
            {
                parallel_nodes[index].timeout--;
            }

            if (parallel_nodes[index].timeout == 0U)
            {
                if (parallel_nodes[index].uid32 == parallel_master_uid32)
                {
                    parallel_master_uid32 = 0UL;
                    parallel_master_locked = 0U;
                }
                memset(&parallel_nodes[index], 0, sizeof(parallel_nodes[index]));
                removed = 1U;
            }
        }
    }

    if ((removed != 0U) && (parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED))
    {
        parallel_status.ready_flags |= PARALLEL_READY_GROUP_LIMITING;
        parallel_block_if_running(PARALLEL_BLOCK_NODE_COUNT);
    }
}

static void parallel_update_local_ready_flags(void)
{
    uint16_t ready_flags = 0U;

    if (parallel_pg_is_active() != 0U)
    {
        ready_flags |= PARALLEL_READY_PG_OFF;
        ready_flags |= PARALLEL_READY_HW_VALID;
    }

    if (ctrl_app_get_is_run() == 0U)
    {
        ready_flags |= PARALLEL_READY_OUTPUT_OFF;
    }

    if (fault_get_all_fault() == 0UL)
    {
        ready_flags |= PARALLEL_READY_NO_FAULT;
    }

    if (parallel_status.heartbeat != 0U)
    {
        ready_flags |= PARALLEL_READY_HEARTBEAT_FRESH;
    }

    if ((parallel_status.params_crc32 != 0UL) &&
        ((parallel_confirmed_params_crc32 == parallel_status.params_crc32) ||
         (parallel_requested_params_crc32 == parallel_status.params_crc32)))
    {
        parallel_confirmed_params_crc32 = parallel_status.params_crc32;
        ready_flags |= PARALLEL_READY_PARAMS_CRC;
    }

    if ((parallel_status.ready_flags & PARALLEL_READY_GROUP_LIMITING) != 0U)
    {
        ready_flags |= PARALLEL_READY_GROUP_LIMITING;
    }

    parallel_status.ready_flags = ready_flags;
}

static void parallel_confirm_single_node_params(void)
{
    if ((parallel_status.discovered_count <= 1U) &&
        (parallel_status.params_crc32 != 0UL))
    {
        parallel_confirmed_params_crc32 = parallel_status.params_crc32;
        parallel_update_local_ready_flags();
        parallel_node_refresh_local();
    }
}

static uint8_t parallel_group_is_ready(uint16_t *block_fault, uint8_t require_output_off)
{
    uint8_t index;
    uint16_t required_flags;

    required_flags = (PARALLEL_READY_PG_OFF |
                      PARALLEL_READY_HW_VALID |
                      PARALLEL_READY_NO_FAULT |
                      PARALLEL_READY_HEARTBEAT_FRESH |
                      PARALLEL_READY_PARAMS_CRC);
    if (require_output_off != 0U)
    {
        required_flags |= PARALLEL_READY_OUTPUT_OFF;
    }

    if (parallel_status.discovered_count == 0U)
    {
        *block_fault = PARALLEL_BLOCK_NODE_COUNT;
        return 0U;
    }

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            (parallel_rank_is_valid(parallel_nodes[index].temp_addr) != 0U))
        {
            if ((parallel_status.params_crc32 == 0UL) ||
                ((parallel_nodes[index].ready_flags & PARALLEL_READY_PARAMS_CRC) == 0U) ||
                (parallel_nodes[index].params_crc32 != parallel_status.params_crc32))
            {
                *block_fault = PARALLEL_BLOCK_PARAMS_MISMATCH;
                return 0U;
            }

            if ((parallel_nodes[index].ready_flags & required_flags) != required_flags)
            {
                *block_fault = PARALLEL_BLOCK_NODE_NOT_READY;
                return 0U;
            }
        }
    }

    *block_fault = PARALLEL_BLOCK_NONE;
    return 1U;
}

static uint8_t parallel_local_is_ready(uint16_t *block_fault)
{
    uint16_t required_flags;

    required_flags = (PARALLEL_READY_PG_OFF |
                      PARALLEL_READY_HW_VALID |
                      PARALLEL_READY_OUTPUT_OFF |
                      PARALLEL_READY_NO_FAULT |
                      PARALLEL_READY_HEARTBEAT_FRESH |
                      PARALLEL_READY_PARAMS_CRC);

    if ((parallel_status.ready_flags & required_flags) != required_flags)
    {
        *block_fault = PARALLEL_BLOCK_LOCAL_NOT_READY;
        return 0U;
    }

    if (parallel_status.params_crc32 == 0UL)
    {
        *block_fault = PARALLEL_BLOCK_CONFIG_MISSING;
        return 0U;
    }

    *block_fault = PARALLEL_BLOCK_NONE;
    return 1U;
}

static void parallel_rs485_latch_prepare_ready(void)
{
    parallel_rs485_prepare_ready_latched = 1U;
    parallel_rs485_active = 0U;
    parallel_rs485_sync_phase = 0U;
    parallel_rs485_sync_addr = 0U;
    parallel_rs485_sync_wait_polls = 0U;
}

static void parallel_can_latch_prepare_ready(void)
{
    parallel_can_prepare_ready_latched = 1U;
    parallel_can_active = 0U;

    parallel_can_sync_index = 0U;
}

static void parallel_rs485_clear_control_state(void)
{
    parallel_rs485_prepare_ready_latched = 0U;
    parallel_rs485_prepare_ok_sent = 0U;
    parallel_rs485_active = 0U;
    parallel_rs485_sync_phase = 0U;
    parallel_rs485_sync_addr = 0U;
    parallel_rs485_sync_wait_polls = 0U;
    parallel_rs485_assigned_slave = 0U;
    parallel_rs485_backend_read_quiet_polls = 0U;
    parallel_rs485_master_poll_reply_guard_polls = 0U;
}

static void parallel_can_clear_control_state(void)
{
    parallel_can_prepare_ready_latched = 0U;
    parallel_can_prepare_ok_sent = 0U;
    parallel_can_active = 0U;
    parallel_can_poll_tick = 0U;

    parallel_can_sync_index = 0U;
    parallel_can_assigned_slave = 0U;
    parallel_can_uid_announce_sent_mask = 0U;
    parallel_can_param_sync_mask_low = 0UL;
    parallel_can_param_sync_mask_high = 0U;
}

static void parallel_update_prepare_block(void)
{
    uint16_t block_fault;
    uint16_t state;
    uint8_t sync_target;

    state = parallel_get_state();
    if ((parallel_status.control != PARALLEL_CONTROL_PREPARE_START) ||
        ((state != PARALLEL_STATE_PREPARE) && (state != PARALLEL_STATE_BLOCKED)))
    {
        return;
    }

    parallel_node_ack_matching_crc_nodes();
    sync_target = parallel_node_find_sync_target();

    if ((sync_target >= PARALLEL_MAX_NODE_COUNT) &&
        (parallel_rs485_prepare_ready_hold() != 0U))
    {
        parallel_status.block_fault = PARALLEL_BLOCK_NONE;
        parallel_set_state(PARALLEL_STATE_PREPARE);
        if (((parallel_rs485_active != 0U) && (parallel_rs485_send_prepare_ready_ok() == 0U)) ||
            ((parallel_can_active != 0U) && (parallel_can_send_prepare_ready_ok() == 0U)))
        {
            return;
        }
        if (parallel_rs485_active != 0U)
        {
            parallel_rs485_latch_prepare_ready();
        }
        if (parallel_can_active != 0U)
        {
            parallel_can_latch_prepare_ready();
        }
        parallel_node_refresh_local();
        return;
    }

    if (((parallel_get_role() == PARALLEL_ROLE_MASTER) &&
         (sync_target >= PARALLEL_MAX_NODE_COUNT) &&
         (parallel_group_is_ready(&block_fault, 1U) != 0U)) ||
        ((parallel_get_role() != PARALLEL_ROLE_MASTER) &&
         (parallel_local_is_ready(&block_fault) != 0U)))
    {
        parallel_status.block_fault = PARALLEL_BLOCK_NONE;
        parallel_set_state(PARALLEL_STATE_PREPARE);
        if (parallel_get_role() == PARALLEL_ROLE_MASTER)
        {
            if (((parallel_rs485_active != 0U) && (parallel_rs485_send_prepare_ready_ok() == 0U)) ||
                ((parallel_can_active != 0U) && (parallel_can_send_prepare_ready_ok() == 0U)))
            {
                return;
            }
            if (parallel_rs485_active != 0U)
            {
                parallel_rs485_latch_prepare_ready();
            }
            if (parallel_can_active != 0U)
            {
                parallel_can_latch_prepare_ready();
            }
        }
        parallel_node_refresh_local();
    }
    else if ((parallel_status.block_fault != PARALLEL_BLOCK_FORCE_STOP) &&
             (parallel_status.block_fault != PARALLEL_BLOCK_FAULT))
    {
        parallel_status.block_fault = block_fault;
        parallel_set_state(PARALLEL_STATE_PREPARE);
    }
}

static uint8_t parallel_rs485_uid_announce_active(void)
{
    if ((parallel_status.control == PARALLEL_CONTROL_PREPARE_START) &&
        (parallel_master_locked == 0U) &&
        (parallel_get_state() == PARALLEL_STATE_PREPARE) &&
        (parallel_rs485_election_wait_tick < PARALLEL_RS485_UID_ANNOUNCE_ACTIVE_TICKS))
    {
        return 1U;
    }

    return 0U;
}

#if (APP_PARALLEL_CAN_FEATURES == 1)
static uint8_t parallel_can_uid_announce_active(void)
{
    if ((parallel_status.control == PARALLEL_CONTROL_PREPARE_START) &&
        (parallel_can_active != 0U) &&
        (parallel_can_pc_control_seen != 0U) &&
        (parallel_master_locked == 0U) &&
        (parallel_get_state() == PARALLEL_STATE_PREPARE) &&
        (parallel_rs485_election_wait_tick < PARALLEL_RS485_UID_ANNOUNCE_ACTIVE_TICKS))
    {
        return 1U;
    }

    return 0U;
}
#endif

static uint8_t parallel_rs485_prepare_ready_hold(void)
{
    if ((parallel_get_role() != PARALLEL_ROLE_MASTER) ||
        (parallel_get_state() != PARALLEL_STATE_PREPARE) ||
        (parallel_status.control != PARALLEL_CONTROL_PREPARE_START))
    {
        return 0U;
    }

    if ((parallel_rs485_prepare_ready_latched != 0U) ||
        (parallel_status.block_fault == PARALLEL_BLOCK_NONE))
    {
        return 1U;
    }

    return 0U;
}

static uint16_t parallel_modbus_crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;
    uint16_t index;
    uint8_t bit;

    for (index = 0U; index < length; index++)
    {
        crc ^= data[index];
        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc = (uint16_t)((crc >> 1U) ^ 0xA001U);
            }
            else
            {
                crc = (uint16_t)(crc >> 1U);
            }
        }
    }

    return (uint16_t)((crc >> 8U) | (crc << 8U));
}

static uint8_t parallel_rs485_send_short(uint8_t addr, uint8_t cmd, uint16_t reg_addr, uint16_t value)
{
    uint8_t frame[8];
    uint16_t crc;

    frame[0] = addr;
    frame[1] = cmd;
    parallel_write_u16_be(frame, 2U, reg_addr);
    parallel_write_u16_be(frame, 4U, value);
    crc = parallel_modbus_crc16(frame, 6U);
    parallel_write_u16_be(frame, 6U, crc);

    return wg_com_v2_send_rs485_raw(frame, (uint16_t)sizeof(frame));
}

static uint8_t parallel_rs485_poll_run_status(void)
{
    if ((parallel_get_role() != PARALLEL_ROLE_MASTER) ||
        (parallel_get_state() != PARALLEL_STATE_RUN_ALLOWED))
    {
        return 0U;
    }

    return parallel_rs485_send_read(WG_COM_V2_BROADCAST_ADDR,
                                    PARALLEL_RS485_BASE_ADDR,
                                    PARALLEL_RS485_REG_COUNT);
}

static uint16_t parallel_rs485_get_run_refresh_ticks(void)
{
    uint16_t node_count;

    node_count = parallel_status.discovered_count;
    if (node_count < 2U)
    {
        node_count = 2U;
    }
    if (node_count > PARALLEL_MAX_NODE_COUNT)
    {
        node_count = PARALLEL_MAX_NODE_COUNT;
    }

    return (uint16_t)(PARALLEL_RS485_RUN_IDLE_TICKS +
                      (node_count * PARALLEL_RS485_BROADCAST_REPLY_INTERVAL_TICKS));
}

static void parallel_stop_running_group(uint16_t block_fault)
{
    if (parallel_get_state() != PARALLEL_STATE_RUN_ALLOWED)
    {
        return;
    }

    if (parallel_get_role() == PARALLEL_ROLE_MASTER)
    {
        if (parallel_rs485_active != 0U)
        {
            (void)parallel_rs485_send_write_one(WG_COM_V2_BROADCAST_ADDR,
                                                PARALLEL_RS485_CONTROL_REG,
                                                PARALLEL_CONTROL_STOP);
        }
        if (parallel_can_active != 0U)
        {
            (void)parallel_can_send_control(PARALLEL_CONTROL_STOP);
        }
    }

    WG_COM_V2_SET_DATA_UINT(1U, wg_com_v2_ctrl.PowerOnOff);
    parallel_status.block_fault = block_fault;
    parallel_rs485_active = 0U;
    parallel_rs485_backend_read_quiet_polls = 0U;
    parallel_rs485_master_poll_reply_guard_polls = 0U;
    parallel_can_active = 0U;
    parallel_set_state(PARALLEL_STATE_BLOCKED);
    parallel_node_refresh_local();
}

static uint8_t parallel_rs485_send_write_block(uint8_t addr,
                                               uint16_t reg_addr,
                                               uint16_t count,
                                               const uint8_t *data)
{
    uint8_t frame[80];
    uint16_t byte_count;
    uint16_t crc;

    if ((data == NULL) || (count == 0U) || ((7U + (count * 2U) + 2U) > sizeof(frame)))
    {
        return 0U;
    }

    byte_count = (uint16_t)(count * 2U);
    frame[0] = addr;
    frame[1] = WG_COM_V2_CMD_WRITE_STR;
    parallel_write_u16_be(frame, 2U, reg_addr);
    parallel_write_u16_be(frame, 4U, count);
    frame[6] = (uint8_t)(byte_count & 0x00FFU);
    memcpy(&frame[7], data, byte_count);
    crc = parallel_modbus_crc16(frame, (uint16_t)(7U + byte_count));
    parallel_write_u16_be(frame, (uint8_t)(7U + byte_count), crc);

    return wg_com_v2_send_rs485_raw(frame, (uint16_t)(9U + byte_count));
}

static uint8_t parallel_rs485_send_uid_frame(uint8_t function_code,
                                             uint8_t round_or_rank,
                                             uint32_t uid32)
{
    uint8_t frame[PARALLEL_RS485_UID_FRAME_LEN];
    uint16_t crc;

    frame[0] = 0xFFU;
    frame[1] = function_code;
    frame[2] = PARALLEL_RS485_UID_FRAME_DATA_LEN;
    frame[3] = PARALLEL_PROTOCOL_VERSION;
    frame[4] = round_or_rank;
    parallel_write_u32_be(frame, 5U, uid32);
    frame[9] = (uint8_t)(parallel_get_role() & 0x00FFU);
    frame[10] = (uint8_t)(parallel_get_state() & 0x00FFU);
    crc = parallel_modbus_crc16(frame, 11U);
    parallel_write_u16_be(frame, 11U, crc);

    return wg_com_v2_send_rs485_raw(frame, (uint16_t)sizeof(frame));
}

#if (APP_PARALLEL_CAN_FEATURES == 1)
static uint32_t parallel_can_build_id(uint32_t dgn)
{
    return (((uint32_t)6U << 26U) | ((dgn & 0x0001FFFFUL) << 8U) | rvc_address_get_current());
}

static uint8_t parallel_can_send_dgn(uint32_t dgn, uint8_t *data)
{
    if (data == NULL)
    {
        return 0U;
    }

    return bsp_rvc_can_tx(parallel_can_build_id(dgn), data, 8U);
}

static uint8_t parallel_can_request_dgn(uint32_t dgn)
{
    uint8_t data[8];

    memset(data, 0xFF, sizeof(data));
    data[0] = (uint8_t)(dgn & 0x000000FFUL);
    data[1] = (uint8_t)((dgn >> 8U) & 0x000000FFUL);
    data[2] = (uint8_t)((dgn >> 16U) & 0x000000FFUL);

    return parallel_can_send_dgn(PARALLEL_CAN_DGN_REQUEST, data);
}

static uint8_t parallel_can_make_param_sync_item(uint8_t index, uint8_t *data)
{
    const uint8_t *src;
    uint16_t param_index;

    if ((data == NULL) || (index >= PARALLEL_CAN_PARAM_SYNC_ITEM_COUNT))
    {
        return 0U;
    }

    memset(data, 0xFF, 8U);
    data[0] = 0xFFU;
    data[1] = 0xFFU;
    data[2] = index;

    if (index < PARALLEL_RS485_CTRL_SYNC_COUNT)
    {
        src = (const uint8_t *)&wg_com_v2_ctrl.SetPowerMode;
        data[3] = src[(uint16_t)index * 2U];
        data[4] = src[((uint16_t)index * 2U) + 1U];
    }
    else if (index < (PARALLEL_RS485_CTRL_SYNC_COUNT + PARALLEL_RS485_PARAM_SYNC_COUNT))
    {
        param_index = (uint16_t)(index - PARALLEL_RS485_CTRL_SYNC_COUNT);
        src = &((const uint8_t *)&wg_com_v2_param)
              [(PARALLEL_RS485_PARAM_SYNC_START - WG_COM_V2_PARAM_ADDR) * 2U];
        data[3] = src[param_index * 2U];
        data[4] = src[(param_index * 2U) + 1U];
    }
    else if (index == (PARALLEL_CAN_PARAM_SYNC_ITEM_COUNT - 2U))
    {
        parallel_write_u16_be(data, 3U, (uint16_t)((parallel_status.params_crc32 >> 16U) & 0x0000FFFFUL));
    }
    else
    {
        parallel_write_u16_be(data, 3U, (uint16_t)(parallel_status.params_crc32 & 0x0000FFFFUL));
    }

    parallel_write_u16_be(data, 5U, (uint16_t)(parallel_status.params_crc32 & 0x0000FFFFUL));
    data[7] = PARALLEL_PROTOCOL_VERSION;
    return 1U;
}

static void parallel_can_mark_param_sync_item(uint8_t index)
{
    if (index < 32U)
    {
        parallel_can_param_sync_mask_low |= (1UL << index);
    }
    else if (index < PARALLEL_CAN_PARAM_SYNC_ITEM_COUNT)
    {
        parallel_can_param_sync_mask_high |= (uint16_t)(1U << (index - 32U));
    }
}

static uint8_t parallel_can_param_sync_is_complete(void)
{
    return (((parallel_can_param_sync_mask_low == PARALLEL_CAN_PARAM_SYNC_FULL_LO_MASK) &&
             (parallel_can_param_sync_mask_high == PARALLEL_CAN_PARAM_SYNC_FULL_HI_MASK)) ? 1U : 0U);
}

static void parallel_can_apply_param_sync_item(const uint8_t *data)
{
    uint8_t *dst;
    uint8_t index;
    uint16_t param_index;
    uint32_t crc32;

    if ((data == NULL) ||
        (data[7] != PARALLEL_PROTOCOL_VERSION) ||
        (parallel_get_role() != PARALLEL_ROLE_SLAVE))
    {
        return;
    }

    index = data[2];
    if (index >= PARALLEL_CAN_PARAM_SYNC_ITEM_COUNT)
    {
        return;
    }

    if (index < PARALLEL_RS485_CTRL_SYNC_COUNT)
    {
        dst = (uint8_t *)&wg_com_v2_ctrl.SetPowerMode;
        dst[(uint16_t)index * 2U] = data[3];
        dst[((uint16_t)index * 2U) + 1U] = data[4];
    }
    else if (index < (PARALLEL_RS485_CTRL_SYNC_COUNT + PARALLEL_RS485_PARAM_SYNC_COUNT))
    {
        param_index = (uint16_t)(index - PARALLEL_RS485_CTRL_SYNC_COUNT);
        dst = &((uint8_t *)&wg_com_v2_param)
              [(PARALLEL_RS485_PARAM_SYNC_START - WG_COM_V2_PARAM_ADDR) * 2U];
        dst[param_index * 2U] = data[3];
        dst[(param_index * 2U) + 1U] = data[4];
    }
    else if (index == (PARALLEL_CAN_PARAM_SYNC_ITEM_COUNT - 2U))
    {
        parallel_requested_params_crc32 &= 0x0000FFFFUL;
        parallel_requested_params_crc32 |= ((uint32_t)parallel_read_u16_be(data, 3U) << 16U);
        parallel_requested_params_crc_mask |= 0x01U;
    }
    else
    {
        parallel_requested_params_crc32 &= 0xFFFF0000UL;
        parallel_requested_params_crc32 |= (uint32_t)parallel_read_u16_be(data, 3U);
        parallel_requested_params_crc_mask |= 0x02U;
    }

    parallel_can_mark_param_sync_item(index);
    if ((parallel_requested_params_crc_mask == 0x03U) &&
        (parallel_can_param_sync_is_complete() != 0U))
    {
        crc32 = parallel_requested_params_crc32;
        parallel_update_params_crc();
        if (parallel_status.params_crc32 == crc32)
        {
            (void)eeprom_commit_current_pages_for_range(PARALLEL_RS485_CTRL_SYNC_START,
                                                        PARALLEL_RS485_CTRL_SYNC_COUNT);
            (void)eeprom_commit_current_pages_for_range(PARALLEL_RS485_PARAM_SYNC_START,
                                                        PARALLEL_RS485_PARAM_SYNC_COUNT);
            parallel_apply_params_crc(crc32);
            parallel_update_local_ready_flags();
            parallel_node_refresh_local();
        }
        parallel_requested_params_crc_mask = 0U;
    }
}

static uint8_t parallel_can_send_uid_frame(uint32_t dgn, uint8_t round_or_rank, uint32_t uid32)
{
    uint8_t data[8];

    memset(data, 0xFF, sizeof(data));
    data[0] = 0xFFU;
    data[1] = PARALLEL_PROTOCOL_VERSION;
    data[2] = round_or_rank;
    parallel_write_u32_be(data, 3U, uid32);
    data[7] = (uint8_t)(((parallel_get_role() & 0x0FU) << 4U) |
                         (parallel_get_state() & 0x0FU));

    return parallel_can_send_dgn(dgn, data);
}

static uint8_t parallel_can_send_control(uint16_t control)
{
    uint8_t data[8];

    memset(data, 0xFF, sizeof(data));
    data[0] = 0xFFU;
    parallel_write_u16_be(data, 1U, control);
    parallel_write_u32_be(data, 3U, parallel_status.uid32);
    data[7] = PARALLEL_PROTOCOL_VERSION;
    return parallel_can_send_dgn(PARALLEL_CAN_DGN_CONTROL, data);
}
#else
static uint8_t parallel_can_send_control(uint16_t control)
{
    (void)control;
    return 0U;
}
#endif

static uint8_t parallel_rs485_send_prepare_ready_ok(void)
{
    uint16_t block_fault;

    if ((parallel_get_role() != PARALLEL_ROLE_MASTER) ||
        (parallel_status.discovered_count < 2U))
    {
        return 0U;
    }

    if (parallel_group_is_ready(&block_fault, 1U) == 0U)
    {
        parallel_status.block_fault = block_fault;
        parallel_rs485_prepare_ok_sent = 0U;
        return 0U;
    }

    parallel_status.block_fault = PARALLEL_BLOCK_NONE;
    parallel_set_role(PARALLEL_ROLE_MASTER);
    parallel_status.temp_addr = PARALLEL_ADDR_RANK_MASTER;
    parallel_master_uid32 = parallel_status.uid32;
    parallel_master_locked = 1U;
    parallel_node_refresh_local();

    if (parallel_rs485_prepare_ok_sent == 0U)
    {
        (void)parallel_rs485_send_uid_frame(PARALLEL_RS485_READY_OK_FUNC,
                                            (uint8_t)parallel_status.discovered_count,
                                            parallel_status.uid32);
        parallel_rs485_prepare_ok_sent = 1U;
    }

    return 1U;
}

static uint8_t parallel_local_can_enter_run(uint16_t *block_fault)
{
    uint16_t required_flags;

    parallel_update_local_ready_flags();

    required_flags = (PARALLEL_READY_PG_OFF |
                      PARALLEL_READY_HW_VALID |
                      PARALLEL_READY_NO_FAULT |
                      PARALLEL_READY_HEARTBEAT_FRESH |
                      PARALLEL_READY_PARAMS_CRC);

    if ((parallel_status.ready_flags & required_flags) != required_flags)
    {
        *block_fault = PARALLEL_BLOCK_LOCAL_NOT_READY;
        return 0U;
    }

    if (parallel_status.params_crc32 == 0UL)
    {
        *block_fault = PARALLEL_BLOCK_CONFIG_MISSING;
        return 0U;
    }

    *block_fault = PARALLEL_BLOCK_NONE;
    return 1U;
}

static uint8_t parallel_rs485_uid_announce_slot(uint8_t round)
{
    uint32_t mixed_uid;

    mixed_uid = parallel_status.uid32 ^ (0x9E3779B9UL * ((uint32_t)round + 1UL));
    mixed_uid ^= (mixed_uid >> 16);
    mixed_uid ^= (mixed_uid >> 8);

    return (uint8_t)(mixed_uid % PARALLEL_RS485_UID_ANNOUNCE_SLOTS);
}

static void parallel_rs485_uid_announce_10ms(void)
{
    uint16_t slot_ticks;
    uint8_t round;
    uint8_t slot;
    uint8_t own_slot;
    uint8_t round_mask;

    if (parallel_rs485_uid_announce_active() == 0U)
    {
        return;
    }

    slot_ticks = (uint16_t)(parallel_rs485_election_wait_tick /
                            PARALLEL_RS485_UID_ANNOUNCE_SLOT_TICKS);
    round = (uint8_t)(slot_ticks / PARALLEL_RS485_UID_ANNOUNCE_SLOTS);
    if (round >= PARALLEL_RS485_UID_ANNOUNCE_ROUNDS)
    {
        return;
    }

    slot = (uint8_t)(slot_ticks % PARALLEL_RS485_UID_ANNOUNCE_SLOTS);
    own_slot = parallel_rs485_uid_announce_slot(round);
    round_mask = (uint8_t)(1U << round);
    if ((slot == own_slot) &&
        ((parallel_rs485_uid_announce_sent_mask & round_mask) == 0U))
    {
        (void)parallel_rs485_send_uid_frame(PARALLEL_RS485_UID_ANNOUNCE_FUNC,
                                            round,
                                            parallel_status.uid32);
        parallel_rs485_uid_announce_sent_mask |= round_mask;
    }
}

static uint32_t parallel_calc_params_crc(void)
{
    uint32_t hash = 2166136261UL;

    hash = parallel_hash_bytes(hash, (const uint8_t *)&wg_com_v2_ctrl.SetPowerMode, 16U);
    hash = parallel_hash_bytes(hash, (const uint8_t *)&wg_com_v2_ctrl.BatModeFR, 6U);
    hash = parallel_hash_bytes(hash,
                               (const uint8_t *)&wg_com_v2_param.SetInpVolt,
                               (uint16_t)(PARALLEL_RS485_PARAM_SYNC_COUNT * 2U));
    return (hash == 0UL) ? 1UL : hash;
}

static void parallel_update_params_crc(void)
{
    uint32_t params_crc32;

    params_crc32 = parallel_calc_params_crc();
    if (parallel_status.params_crc32 != params_crc32)
    {
        parallel_status.params_crc32 = params_crc32;
        if (parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED)
        {
            parallel_confirmed_params_crc32 = params_crc32;
            parallel_requested_params_crc32 = params_crc32;
            parallel_requested_params_crc_mask = 0U;
            parallel_rs485_prepare_ready_latched = 0U;
        }
        else
        {
            parallel_confirmed_params_crc32 = 0UL;
            parallel_requested_params_crc_mask = 0U;
            parallel_rs485_prepare_ready_latched = 0U;
            if ((parallel_status.block_fault == PARALLEL_BLOCK_PARAMS_MISMATCH) ||
                (parallel_status.block_fault == PARALLEL_BLOCK_CONFIG_MISSING))
            {
                parallel_status.block_fault = PARALLEL_BLOCK_LOCAL_NOT_READY;
                parallel_set_state(PARALLEL_STATE_PREPARE);
                parallel_update_prepare_block();
            }
        }
    }
}

static void parallel_apply_params_crc(uint32_t params_crc32)
{
    parallel_update_params_crc();

    if (params_crc32 == 0UL)
    {
        parallel_status.block_fault = PARALLEL_BLOCK_CONFIG_MISSING;
        parallel_set_state(PARALLEL_STATE_BLOCKED);
        return;
    }

    if (parallel_status.params_crc32 != params_crc32)
    {
        parallel_status.block_fault = PARALLEL_BLOCK_PARAMS_MISMATCH;
        parallel_set_state(PARALLEL_STATE_BLOCKED);
        return;
    }

    parallel_confirmed_params_crc32 = params_crc32;

    if (parallel_status.block_fault == PARALLEL_BLOCK_PARAMS_MISMATCH)
    {
        parallel_status.block_fault = PARALLEL_BLOCK_LOCAL_NOT_READY;
        parallel_set_state(PARALLEL_STATE_PREPARE);
        parallel_update_prepare_block();
    }
}

static void parallel_apply_control(uint16_t control, uint8_t transport)
{
    uint16_t block_fault;
    uint8_t prepare_ready_latched;

    prepare_ready_latched = ((((parallel_rs485_prepare_ready_latched != 0U) &&
                              ((transport & PARALLEL_TRANSPORT_RS485) != 0U)) ||
                             ((parallel_can_prepare_ready_latched != 0U) &&
                              ((transport & PARALLEL_TRANSPORT_CAN) != 0U))) &&
                             (parallel_get_role() == PARALLEL_ROLE_MASTER) &&
                             (parallel_get_state() == PARALLEL_STATE_PREPARE)) ? 1U : 0U;
    parallel_status.control = control;

    if (control == PARALLEL_CONTROL_STOP)
    {
        WG_COM_V2_SET_DATA_UINT(1U, wg_com_v2_ctrl.PowerOnOff);
        parallel_rs485_clear_control_state();
        parallel_can_clear_control_state();
        parallel_status.block_fault = PARALLEL_BLOCK_NONE;
        parallel_set_state(PARALLEL_STATE_IDLE);
        return;
    }

    if (control == PARALLEL_CONTROL_FORCE_STOP)
    {
        WG_COM_V2_SET_DATA_UINT(1U, wg_com_v2_ctrl.PowerOnOff);
        parallel_rs485_clear_control_state();
        parallel_can_clear_control_state();
        parallel_status.block_fault = PARALLEL_BLOCK_FORCE_STOP;
        parallel_set_state(PARALLEL_STATE_BLOCKED);
        return;
    }

    if (control == PARALLEL_CONTROL_PREPARE_START)
    {
        memset(parallel_nodes, 0, sizeof(parallel_nodes));
        parallel_status.reserved0 = 0U;
        parallel_status.temp_addr = PARALLEL_ADDR_RANK_MASTER;
        parallel_master_uid32 = 0UL;
        parallel_master_locked = 0U;
        parallel_rs485_active = ((transport & PARALLEL_TRANSPORT_RS485) != 0U) ? 1U : 0U;
        parallel_rs485_poll_tick = 0U;
        parallel_rs485_sync_phase = 0U;
        parallel_rs485_sync_addr = 0U;
        parallel_rs485_sync_wait_polls = 0U;
        parallel_rs485_election_wait_tick = 0U;
        parallel_rs485_uid_announce_sent_mask = 0U;
        parallel_rs485_prepare_ready_latched = 0U;
        parallel_rs485_prepare_ok_sent = 0U;
        parallel_rs485_assigned_slave = 0U;

        parallel_rs485_backend_read_quiet_polls = 0U;
        parallel_rs485_master_poll_reply_guard_polls = 0U;
        parallel_can_active = ((transport & PARALLEL_TRANSPORT_CAN) != 0U) ? 1U : 0U;
        parallel_can_poll_tick = 0U;
        parallel_can_run_request_phase = 0U;

        parallel_can_sync_index = 0U;
        parallel_can_prepare_ready_latched = 0U;
        parallel_can_prepare_ok_sent = 0U;
        parallel_can_assigned_slave = 0U;
        parallel_can_uid_announce_sent_mask = 0U;
        parallel_can_pc_control_seen = ((transport & PARALLEL_TRANSPORT_CAN) != 0U) ? 1U : 0U;
        parallel_can_param_sync_mask_low = 0UL;
        parallel_can_param_sync_mask_high = 0U;
        parallel_status.block_fault = PARALLEL_BLOCK_LOCAL_NOT_READY;
        parallel_set_role(PARALLEL_ROLE_UNKNOWN);
        parallel_set_state(PARALLEL_STATE_PREPARE);
        parallel_node_refresh_local();
        return;
    }

    if (control == PARALLEL_CONTROL_RUN_ENABLE)
    {
        if (parallel_get_role() == PARALLEL_ROLE_MASTER)
        {
            if ((prepare_ready_latched != 0U) ||
                (parallel_group_is_ready(&block_fault, 1U) != 0U))
            {
                parallel_status.block_fault = PARALLEL_BLOCK_NONE;
                if ((transport & PARALLEL_TRANSPORT_RS485) != 0U)
                {
                    parallel_rs485_prepare_ready_latched = 0U;
                    parallel_rs485_active = 1U;
                    parallel_rs485_poll_tick = PARALLEL_RS485_POLL_TICKS;
                    parallel_rs485_sync_phase = 0U;
                    parallel_rs485_sync_addr = 0U;
                    parallel_rs485_sync_wait_polls = 0U;

                    parallel_rs485_backend_read_quiet_polls = PARALLEL_RS485_BACKEND_READ_TIMEOUT_POLLS;
                    parallel_rs485_master_poll_reply_guard_polls = 0U;
                    (void)parallel_rs485_send_write_one(WG_COM_V2_BROADCAST_ADDR,
                                                        PARALLEL_RS485_CONTROL_REG,
                                                        PARALLEL_CONTROL_RUN_ENABLE);
                }
                if ((transport & PARALLEL_TRANSPORT_CAN) != 0U)
                {
                    parallel_can_prepare_ready_latched = 0U;
                    parallel_can_active = 1U;
                    (void)parallel_can_send_control(PARALLEL_CONTROL_RUN_ENABLE);
                }
                parallel_node_mark_remote_run_allowed();
                parallel_node_refresh_remote_timeouts();
                WG_COM_V2_SET_DATA_UINT(0U, wg_com_v2_ctrl.PowerOnOff);
                power_sw_force_power_on();
                parallel_set_state(PARALLEL_STATE_RUN_ALLOWED);
                parallel_node_refresh_local();
            }
            else
            {
                parallel_status.block_fault = block_fault;
                parallel_set_state(PARALLEL_STATE_BLOCKED);
            }
        }
        else if (parallel_local_can_enter_run(&block_fault) != 0U)
        {
            parallel_status.block_fault = PARALLEL_BLOCK_NONE;
            parallel_node_refresh_remote_timeouts();
            WG_COM_V2_SET_DATA_UINT(0U, wg_com_v2_ctrl.PowerOnOff);
            power_sw_force_power_on();
            parallel_set_state(PARALLEL_STATE_RUN_ALLOWED);
        }
        else
        {
            parallel_status.block_fault = block_fault;
            parallel_set_state(PARALLEL_STATE_BLOCKED);
        }
        return;
    }

    parallel_status.block_fault = PARALLEL_BLOCK_LOCAL_NOT_READY;
    parallel_set_state(PARALLEL_STATE_BLOCKED);
}

static void parallel_rs485_run_10ms(void)
{
    uint8_t index;
    uint8_t addr;
    uint8_t rank;
    uint8_t crc_data[4];
    const uint8_t *param_data;
    uint16_t block_fault;
    uint8_t sync_target;

    if ((parallel_rs485_active == 0U) ||
        (parallel_is_requested() == 0U))
    {
        return;
    }

    if ((parallel_get_role() != PARALLEL_ROLE_MASTER) &&
        (parallel_rs485_pre_election_active() == 0U))
    {
        return;
    }

    if (parallel_rs485_uid_announce_active() != 0U)
    {
        parallel_rs485_uid_announce_10ms();
        return;
    }

    parallel_node_ack_matching_crc_nodes();
    sync_target = parallel_node_find_sync_target();

    if ((sync_target >= PARALLEL_MAX_NODE_COUNT) &&
        (parallel_rs485_prepare_ready_hold() != 0U))
    {
        if (parallel_rs485_send_prepare_ready_ok() == 0U)
        {
            return;
        }
        parallel_rs485_latch_prepare_ready();
        return;
    }

    if ((parallel_get_role() == PARALLEL_ROLE_MASTER) &&
        (parallel_get_state() == PARALLEL_STATE_PREPARE) &&
        (sync_target >= PARALLEL_MAX_NODE_COUNT) &&
        (parallel_group_is_ready(&block_fault, 1U) != 0U))
    {
        parallel_status.block_fault = PARALLEL_BLOCK_NONE;
        if (parallel_rs485_send_prepare_ready_ok() == 0U)
        {
            return;
        }
        parallel_rs485_latch_prepare_ready();
        parallel_node_refresh_local();
        return;
    }

    parallel_rs485_poll_tick++;
    if (parallel_rs485_poll_tick < ((parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED) ?
                                    parallel_rs485_get_run_refresh_ticks() :
                                    PARALLEL_RS485_POLL_TICKS))
    {
        return;
    }
    parallel_rs485_poll_tick = 0U;

    if ((parallel_rs485_sync_phase == 5U) && (parallel_rs485_sync_addr != 0U))
    {
        if (parallel_rs485_sync_wait_polls < PARALLEL_RS485_CRC_CONFIRM_SETTLE_POLLS)
        {
            parallel_rs485_sync_wait_polls++;
            return;
        }

        (void)parallel_rs485_send_read(parallel_rs485_sync_addr,
                                       PARALLEL_RS485_BASE_ADDR,
                                       PARALLEL_RS485_REG_COUNT);
        parallel_rs485_sync_phase = 6U;
        parallel_rs485_sync_wait_polls = 0U;
        return;
    }

    if ((parallel_rs485_sync_phase == 6U) && (parallel_rs485_sync_addr != 0U))
    {
        if (parallel_rs485_sync_wait_polls < PARALLEL_RS485_STATUS_REPLY_WAIT_POLLS)
        {
            parallel_rs485_sync_wait_polls++;
            return;
        }

        parallel_rs485_sync_phase = 0U;
        parallel_rs485_sync_addr = 0U;
        parallel_rs485_sync_wait_polls = 0U;
        return;
    }

    if (parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED)
    {
        if (parallel_get_role() != PARALLEL_ROLE_MASTER)
        {
            return;
        }

        if (parallel_rs485_master_poll_reply_guard_polls > 0U)
        {
            parallel_rs485_master_poll_reply_guard_polls--;
        }

        if (parallel_rs485_backend_read_quiet_polls < PARALLEL_RS485_BACKEND_READ_TIMEOUT_POLLS)
        {
            parallel_rs485_backend_read_quiet_polls++;
            return;
        }

        if (parallel_rs485_poll_run_status() != 0U)
        {
            parallel_rs485_master_poll_reply_guard_polls = PARALLEL_RS485_MASTER_POLL_REPLY_GUARD_POLLS;
        }
        return;
    }

    if ((parallel_rs485_sync_phase != 0U) && (parallel_rs485_sync_addr == 0U))
    {
        parallel_rs485_sync_phase = 0U;
        parallel_rs485_sync_wait_polls = 0U;
    }

    parallel_node_ack_matching_crc_nodes();
    if ((parallel_rs485_sync_phase != 0U) && (parallel_rs485_sync_addr != 0U))
    {
        addr = parallel_rs485_sync_addr;
    }
    else
    {
        index = parallel_node_find_sync_target();
        if (index >= PARALLEL_MAX_NODE_COUNT)
        {
            parallel_rs485_sync_phase = 0U;
            parallel_rs485_sync_addr = 0U;
            parallel_rs485_sync_wait_polls = 0U;
            return;
        }

        rank = parallel_node_sync_rank(index);
        if ((rank <= PARALLEL_ADDR_RANK_MASTER) || (rank > PARALLEL_MAX_NODE_COUNT))
        {
            parallel_rs485_sync_phase = 0U;
            parallel_rs485_sync_addr = 0U;
            parallel_rs485_sync_wait_polls = 0U;
            return;
        }
        addr = parallel_rank_to_rs485_addr(rank);

        (void)parallel_rs485_send_uid_frame(PARALLEL_RS485_UID_ASSIGN_FUNC,
                                            rank,
                                            parallel_nodes[index].uid32);
        parallel_nodes[index].temp_addr = rank;
        parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
        parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;
        parallel_rs485_sync_addr = addr;
        parallel_rs485_sync_phase = 1U;
        parallel_rs485_sync_wait_polls = 0U;
        return;
    }

    if (parallel_rs485_sync_phase == 1U)
    {
        rank = parallel_rs485_addr_to_rank(addr);
        (void)parallel_rs485_send_write_one(addr,
                                            PARALLEL_RS485_ASSIGN_REG,
                                            (uint16_t)rank);
        parallel_rs485_sync_phase = 2U;
    }
    else if (parallel_rs485_sync_phase == 2U)
    {
        (void)parallel_rs485_send_write_block(addr,
                                              PARALLEL_RS485_CTRL_SYNC_START,
                                              PARALLEL_RS485_CTRL_SYNC_COUNT,
                                              (const uint8_t *)&wg_com_v2_ctrl.SetPowerMode);
        parallel_rs485_sync_phase = 3U;
    }
    else if (parallel_rs485_sync_phase == 3U)
    {
        param_data = &((const uint8_t *)&wg_com_v2_param)[(PARALLEL_RS485_PARAM_SYNC_START - WG_COM_V2_PARAM_ADDR) * 2U];
        (void)parallel_rs485_send_write_block(addr,
                                              PARALLEL_RS485_PARAM_SYNC_START,
                                              PARALLEL_RS485_PARAM_SYNC_COUNT,
                                              param_data);
        parallel_rs485_sync_phase = 4U;
    }
    else
    {
        parallel_write_u32_be(crc_data, 0U, parallel_status.params_crc32);
        (void)parallel_rs485_send_write_block(addr,
                                              PARALLEL_RS485_CRC_CONFIRM_REG,
                                              PARALLEL_RS485_CRC_CONFIRM_COUNT,
                                              crc_data);
        parallel_rs485_sync_phase = 5U;
        parallel_rs485_sync_wait_polls = 0U;
    }
}

void parallel_mode_run_10ms(void)
{
    parallel_init_state_if_needed();
    parallel_update_params_crc();
    parallel_update_local_ready_flags();

    if ((parallel_pg_is_active() == 0U) &&
        (parallel_is_requested() == 0U))
    {
        memset(parallel_nodes, 0, sizeof(parallel_nodes));
        parallel_confirmed_params_crc32 = 0UL;
        parallel_requested_params_crc32 = 0UL;
        parallel_requested_params_crc_mask = 0U;
        parallel_master_uid32 = 0UL;
        parallel_master_locked = 0U;
        parallel_rs485_active = 0U;
        parallel_rs485_poll_tick = 0U;
        parallel_rs485_sync_phase = 0U;
        parallel_rs485_sync_addr = 0U;
        parallel_rs485_sync_wait_polls = 0U;
        parallel_rs485_prepare_ready_latched = 0U;
        parallel_rs485_prepare_ok_sent = 0U;
        parallel_rs485_election_wait_tick = 0U;
        parallel_rs485_uid_announce_sent_mask = 0U;
        parallel_rs485_assigned_slave = 0U;

        parallel_rs485_backend_read_quiet_polls = 0U;
        parallel_rs485_master_poll_reply_guard_polls = 0U;
        parallel_can_active = 0U;
        parallel_can_poll_tick = 0U;

        parallel_can_sync_index = 0U;
        parallel_can_prepare_ready_latched = 0U;
        parallel_can_prepare_ok_sent = 0U;
        parallel_can_assigned_slave = 0U;
        parallel_can_uid_announce_sent_mask = 0U;
        parallel_can_param_sync_mask_low = 0UL;
        parallel_can_param_sync_mask_high = 0U;
        parallel_status.control = PARALLEL_CONTROL_STOP;
        parallel_status.reserved0 = 0U;
        parallel_status.temp_addr = 0U;
        parallel_status.discovered_count = 1U;
        parallel_status.block_fault = PARALLEL_BLOCK_NONE;
        parallel_set_role(PARALLEL_ROLE_UNKNOWN);
        parallel_set_state(PARALLEL_STATE_IDLE);
        return;
    }

    parallel_tick_10ms++;
    if (parallel_tick_10ms >= PARALLEL_HEARTBEAT_TICKS)
    {
        parallel_tick_10ms = 0U;
        parallel_status.heartbeat++;
    }

    parallel_node_tick_10ms++;
    if (parallel_node_tick_10ms >= 10U)
    {
        parallel_node_tick_10ms = 0U;
        parallel_node_age_100ms();
    }

    if ((parallel_status.control == PARALLEL_CONTROL_PREPARE_START) &&
        (parallel_master_locked == 0U) &&
        (parallel_rs485_election_wait_tick < PARALLEL_RS485_ELECTION_WAIT_TICKS))
    {
        parallel_rs485_election_wait_tick++;
    }

    parallel_update_role_and_addr();
    parallel_confirm_single_node_params();
    parallel_update_prepare_block();
    parallel_rs485_run_10ms();
    parallel_can_run_10ms();
}

void parallel_mode_on_rvc_rx(uint32_t dgn, const uint8_t *data, uint8_t len, uint8_t source_addr)
{
    uint32_t uid32;
    uint8_t index;
    uint8_t rank;
    uint8_t role;
    uint8_t state;
    uint16_t block_fault;

    parallel_init_state_if_needed();

    if ((data == NULL) || (len < 8U))
    {
        return;
    }

    if (dgn == PARALLEL_CAN_DGN_CONTROL)
    {
        parallel_apply_control(parallel_read_u16_be(data, 1U), PARALLEL_TRANSPORT_CAN);
        return;
    }

    if (parallel_is_requested() == 0U)
    {
        return;
    }

    if (parallel_can_pc_control_seen == 0U)
    {
        return;
    }

    if (dgn == PARALLEL_CAN_DGN_UID_ANNOUNCE)
    {
        if (data[1] != PARALLEL_PROTOCOL_VERSION)
        {
            return;
        }

        uid32 = parallel_read_u32_be(data, 3U);
        if ((uid32 == 0UL) || (uid32 == parallel_status.uid32))
        {
            return;
        }

        role = (uint8_t)((data[7] >> 4U) & 0x0FU);
        state = (uint8_t)(data[7] & 0x0FU);
        parallel_note_remote_role(uid32, role);
        index = parallel_node_alloc(uid32);
        if (index < PARALLEL_MAX_NODE_COUNT)
        {
            parallel_nodes[index].temp_addr = PARALLEL_ADDR_RANK_MASTER;
            parallel_nodes[index].role_state = (uint8_t)((role << 4U) | state);
            parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
            parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;
        }
        parallel_update_prepare_block();
        return;
    }

    if (dgn == PARALLEL_CAN_DGN_UID_ASSIGN)
    {
        if (data[1] != PARALLEL_PROTOCOL_VERSION)
        {
            return;
        }

        rank = data[2];
        uid32 = parallel_read_u32_be(data, 3U);
        if ((rank < PARALLEL_ADDR_RANK_MASTER) || (rank > PARALLEL_MAX_NODE_COUNT))
        {
            return;
        }

        if (uid32 == parallel_status.uid32)
        {
            parallel_status.temp_addr = rank;
            parallel_status.control = PARALLEL_CONTROL_PREPARE_START;
            parallel_can_active = 1U;
            if (rank == PARALLEL_ADDR_RANK_MASTER)
            {
                parallel_can_assigned_slave = 0U;
                parallel_master_uid32 = parallel_status.uid32;
                parallel_set_role(PARALLEL_ROLE_MASTER);
            }
            else
            {
                parallel_can_assigned_slave = 1U;
                parallel_master_locked = 1U;
                parallel_set_role(PARALLEL_ROLE_SLAVE);
            }
            parallel_set_state(PARALLEL_STATE_PREPARE);
            parallel_node_refresh_local();
            parallel_update_prepare_block();
            return;
        }

        if (uid32 == 0UL)
        {
            return;
        }

        index = parallel_node_find(uid32);
        if (index < PARALLEL_MAX_NODE_COUNT)
        {
            parallel_nodes[index].temp_addr = rank;
            parallel_nodes[index].role_state = (uint8_t)((PARALLEL_ROLE_SLAVE << 4U) |
                                                         PARALLEL_STATE_PREPARE);
            parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
            parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;
        }
        parallel_update_prepare_block();
        return;
    }

    if (dgn == PARALLEL_CAN_DGN_READY_OK)
    {
        if (data[1] != PARALLEL_PROTOCOL_VERSION)
        {
            return;
        }

        uid32 = parallel_read_u32_be(data, 3U);
        if ((uid32 != 0UL) && (uid32 != parallel_status.uid32))
        {
            parallel_master_uid32 = uid32;
            parallel_master_locked = 1U;
            parallel_status.control = PARALLEL_CONTROL_PREPARE_START;
            parallel_can_active = 1U;
            parallel_can_prepare_ready_latched = 1U;
            if (parallel_status.uid32 != uid32)
            {
                parallel_set_role(PARALLEL_ROLE_SLAVE);
            }
            parallel_set_state(PARALLEL_STATE_PREPARE);
            parallel_update_prepare_block();
        }
        return;
    }

#if (APP_PARALLEL_CAN_FEATURES == 1)
    if (dgn == PARALLEL_CAN_DGN_PARAM_SYNC)
    {
        parallel_can_apply_param_sync_item(data);
        return;
    }
#endif

    if ((source_addr < PARALLEL_CAN_TEMP_ADDR_MASTER) ||
        (source_addr >= (PARALLEL_CAN_TEMP_ADDR_MASTER + PARALLEL_MAX_NODE_COUNT)))
    {
        return;
    }

    rank = (uint8_t)(source_addr - PARALLEL_CAN_TEMP_ADDR_MASTER + 1U);
    if ((rank == parallel_get_local_rank()) || (parallel_rank_is_valid(rank) == 0U))
    {
        return;
    }

    index = parallel_node_find_by_rank(rank);
    if (index >= PARALLEL_MAX_NODE_COUNT)
    {
        return;
    }

    if (dgn == PARALLEL_CAN_DGN_STATUS)
    {
        state = data[1];
        role = data[2];
        if ((role != PARALLEL_ROLE_MASTER) && (role != PARALLEL_ROLE_SLAVE))
        {
            return;
        }
        parallel_nodes[index].role_state = (uint8_t)((role << 4U) | (state & 0x0FU));
        parallel_nodes[index].block_fault = parallel_sanitize_block_fault(parallel_read_u16_be(data, 5U));
        parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
        parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;
    }
    else if (dgn == PARALLEL_CAN_DGN_PARAM_SUMMARY)
    {
        parallel_nodes[index].params_crc32 = parallel_read_u32_be(data, 1U);
        parallel_nodes[index].ready_flags = parallel_read_u16_be(data, 5U);
        parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
        parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;
    }
    else
    {
        return;
    }

    if ((parallel_get_role() == PARALLEL_ROLE_MASTER) &&
        (parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED) &&
        (parallel_rs485_node_run_status_is_valid(index) == 0U))
    {
        if ((parallel_status.params_crc32 == 0UL) ||
            (parallel_nodes[index].params_crc32 != parallel_status.params_crc32))
        {
            block_fault = PARALLEL_BLOCK_PARAMS_MISMATCH;
        }
        else
        {
            block_fault = PARALLEL_BLOCK_NODE_NOT_READY;
        }
        parallel_stop_running_group(block_fault);
    }
}

#if (APP_PARALLEL_CAN_FEATURES == 1)
static void parallel_can_send_periodic_status(void)
{
    uint8_t data[8];

    if (parallel_can_pc_control_seen == 0U)
    {
        return;
    }

    memset(data, 0xFF, sizeof(data));
    data[0] = 0xFFU;
    if (parallel_mode_make_rvc_response(PARALLEL_CAN_DGN_STATUS, data, 8U) != 0U)
    {
        (void)parallel_can_send_dgn(PARALLEL_CAN_DGN_STATUS, data);
    }

    memset(data, 0xFF, sizeof(data));
    data[0] = 0xFFU;
    if (parallel_mode_make_rvc_response(PARALLEL_CAN_DGN_PARAM_SUMMARY, data, 8U) != 0U)
    {
        (void)parallel_can_send_dgn(PARALLEL_CAN_DGN_PARAM_SUMMARY, data);
    }
}

static void parallel_can_uid_announce_10ms(void)
{
    uint16_t slot_ticks;
    uint8_t round;
    uint8_t slot;
    uint8_t own_slot;
    uint8_t round_mask;

    if (parallel_can_uid_announce_active() == 0U)
    {
        return;
    }

    slot_ticks = (uint16_t)(parallel_rs485_election_wait_tick /
                            PARALLEL_RS485_UID_ANNOUNCE_SLOT_TICKS);
    round = (uint8_t)(slot_ticks / PARALLEL_RS485_UID_ANNOUNCE_SLOTS);
    if (round >= PARALLEL_RS485_UID_ANNOUNCE_ROUNDS)
    {
        return;
    }

    slot = (uint8_t)(slot_ticks % PARALLEL_RS485_UID_ANNOUNCE_SLOTS);
    own_slot = parallel_rs485_uid_announce_slot(round);
    round_mask = (uint8_t)(1U << round);
    if ((slot == own_slot) &&
        ((parallel_can_uid_announce_sent_mask & round_mask) == 0U))
    {
        (void)parallel_can_send_uid_frame(PARALLEL_CAN_DGN_UID_ANNOUNCE,
                                          round,
                                          parallel_status.uid32);
        parallel_can_uid_announce_sent_mask |= round_mask;
    }
}
#endif

static void parallel_can_run_10ms(void)
{
#if (APP_PARALLEL_CAN_FEATURES == 1)
    uint8_t index;
    uint8_t rank;
    uint8_t data[8];
    uint16_t block_fault;
    uint8_t sync_target;

    if ((parallel_can_active == 0U) ||
        (parallel_is_requested() == 0U))
    {
        return;
    }

    if (parallel_can_pc_control_seen == 0U)
    {
        return;
    }

    if (parallel_can_uid_announce_active() != 0U)
    {
        parallel_can_uid_announce_10ms();
        return;
    }

    parallel_can_poll_tick++;
    if (parallel_can_poll_tick < PARALLEL_CAN_POLL_TICKS)
    {
        return;
    }
    parallel_can_poll_tick = 0U;

    if (parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED)
    {
        if (parallel_get_role() != PARALLEL_ROLE_MASTER)
        {
            return;
        }

        if (parallel_can_run_request_phase == 0U)
        {
            (void)parallel_can_request_dgn(PARALLEL_CAN_DGN_STATUS);
            parallel_can_run_request_phase = 1U;
        }
        else
        {
            (void)parallel_can_request_dgn(PARALLEL_CAN_DGN_PARAM_SUMMARY);
            parallel_can_run_request_phase = 0U;
        }
        return;
    }

    if (parallel_can_prepare_ready_latched == 0U)
    {
        parallel_can_send_periodic_status();
    }

    if (parallel_get_role() != PARALLEL_ROLE_MASTER)
    {
        return;
    }

    parallel_node_ack_matching_crc_nodes();
    sync_target = parallel_node_find_sync_target();
    if ((sync_target >= PARALLEL_MAX_NODE_COUNT) &&
        (parallel_group_is_ready(&block_fault, 1U) != 0U))
    {
        parallel_status.block_fault = PARALLEL_BLOCK_NONE;
        if (parallel_can_send_prepare_ready_ok() != 0U)
        {
            parallel_can_latch_prepare_ready();
            parallel_node_refresh_local();
        }
        return;
    }

    if (sync_target < PARALLEL_MAX_NODE_COUNT)
    {
        rank = parallel_node_sync_rank(sync_target);
        if ((rank > PARALLEL_ADDR_RANK_MASTER) && (rank <= PARALLEL_MAX_NODE_COUNT))
        {
            (void)parallel_can_send_uid_frame(PARALLEL_CAN_DGN_UID_ASSIGN,
                                              rank,
                                              parallel_nodes[sync_target].uid32);
            parallel_nodes[sync_target].temp_addr = rank;
            parallel_nodes[sync_target].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
            parallel_nodes[sync_target].flags |= PARALLEL_NODE_FLAG_READY;
        }
    }

    for (index = 0U; index < 2U; index++)
    {
        if (parallel_can_make_param_sync_item(parallel_can_sync_index, data) != 0U)
        {
            (void)parallel_can_send_dgn(PARALLEL_CAN_DGN_PARAM_SYNC, data);
        }
        parallel_can_sync_index++;
        if (parallel_can_sync_index >= PARALLEL_CAN_PARAM_SYNC_ITEM_COUNT)
        {
            parallel_can_sync_index = 0U;
        }
    }
#else
    parallel_can_feature_off_keep_refs();
#endif
}

void parallel_mode_on_rs485_frame(const uint8_t *frame, uint16_t len)
{
    const uint8_t *data;
    uint8_t addr;
    uint8_t rank;
    uint8_t index;
    uint16_t byte_count;
    uint16_t status_role;
    uint32_t uid32;

    parallel_init_state_if_needed();

    if ((frame == NULL) || (len < 5U) || (parallel_is_requested() == 0U))
    {
        return;
    }

    if ((len >= PARALLEL_RS485_UID_FRAME_LEN) &&
        (frame[0] == 0xFFU) &&
        (frame[2] >= PARALLEL_RS485_UID_FRAME_DATA_LEN) &&
        ((frame[1] == PARALLEL_RS485_UID_ANNOUNCE_FUNC) ||
         (frame[1] == PARALLEL_RS485_UID_ASSIGN_FUNC)))
    {
        uid32 = parallel_read_u32_be(frame, 5U);
        if ((uid32 == 0UL) || (uid32 == parallel_status.uid32))
        {
            if ((frame[1] == PARALLEL_RS485_UID_ASSIGN_FUNC) &&
                (uid32 == parallel_status.uid32))
            {
                rank = frame[4];
                if ((rank >= PARALLEL_ADDR_RANK_MASTER) &&
                    (rank <= PARALLEL_MAX_NODE_COUNT))
                {
                    parallel_status.temp_addr = rank;
                    if (rank == PARALLEL_ADDR_RANK_MASTER)
                    {
                        parallel_rs485_assigned_slave = 0U;
                        parallel_master_uid32 = parallel_status.uid32;
                        parallel_set_role(PARALLEL_ROLE_MASTER);
                    }
                    else
                    {
                        parallel_rs485_assigned_slave = 1U;
                        parallel_master_locked = 1U;
                        parallel_set_role(PARALLEL_ROLE_SLAVE);
                    }
                    parallel_status.control = PARALLEL_CONTROL_PREPARE_START;
                    parallel_set_state(PARALLEL_STATE_PREPARE);
                    parallel_rs485_active = 1U;
                    parallel_node_refresh_local();
                    parallel_update_prepare_block();
                }
            }
            return;
        }

        index = parallel_node_alloc(uid32);
        if (index < PARALLEL_MAX_NODE_COUNT)
        {
            if (frame[1] == PARALLEL_RS485_UID_ANNOUNCE_FUNC)
            {
                parallel_nodes[index].temp_addr = PARALLEL_ADDR_RANK_MASTER;
                parallel_nodes[index].role_state = (uint8_t)(((frame[9] & 0x0FU) << 4) |
                                                             (frame[10] & 0x0FU));
            }
            else
            {
                rank = frame[4];
                if ((rank >= PARALLEL_ADDR_RANK_MASTER) &&
                    (rank <= PARALLEL_MAX_NODE_COUNT))
                {
                    parallel_nodes[index].temp_addr = rank;
                    parallel_nodes[index].role_state = (uint8_t)((PARALLEL_ROLE_SLAVE << 4) |
                                                                 PARALLEL_STATE_PREPARE);
                }
            }
            parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
            parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;
        }

        parallel_update_prepare_block();
        return;
    }

    addr = frame[0];
    if (addr == WG_COM_V2_BROADCAST_ADDR)
    {
        parallel_note_rs485_broadcast_status_request(frame, len);
        return;
    }

    if ((addr < PARALLEL_RS485_TEMP_ADDR_MASTER) || (addr > PARALLEL_RS485_MAX_ADDR))
    {
        return;
    }
    parallel_note_rs485_runtime_activity(addr);

    if (((frame[1] == WG_COM_V2_CMD_WRITE_DATA) ||
         (frame[1] == WG_COM_V2_CMD_WRITE_STR)) &&
        (len >= 8U))
    {
        return;
    }

    if ((frame[1] != WG_COM_V2_CMD_READ) || (len < 39U))
    {
        return;
    }

    byte_count = frame[2];
    if (byte_count < (PARALLEL_RS485_REG_COUNT * 2U))
    {
        return;
    }

    data = &frame[3];
    if (parallel_rs485_status_payload_is_valid(addr, data) == 0U)
    {
        return;
    }

    uid32 = parallel_read_u32_be(data, 8U);
    if ((uid32 == 0UL) || (uid32 == parallel_status.uid32))
    {
        return;
    }

    rank = parallel_rs485_addr_to_rank(addr);
    if (parallel_rank_is_valid(rank) == 0U)
    {
        return;
    }

    status_role = parallel_read_u16_be(data, 2U);
    parallel_note_remote_role(uid32, (uint8_t)((status_role >> 8U) & 0x00FFU));

    index = parallel_node_alloc(uid32);
    if (index < PARALLEL_MAX_NODE_COUNT)
    {
        parallel_nodes[index].temp_addr = rank;
        parallel_nodes[index].role_state = (uint8_t)((((uint8_t)((status_role >> 8U) & 0x00FFU)) << 4) |
                                                     ((uint8_t)status_role & 0x0FU));
        parallel_nodes[index].ready_flags = parallel_read_u16_be(data, 16U);
        parallel_nodes[index].block_fault = parallel_read_u16_be(data, 18U);
        parallel_nodes[index].params_crc32 = parallel_read_u32_be(data, 20U);
        parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
        parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;

        if ((parallel_get_role() == PARALLEL_ROLE_MASTER) &&
            (parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED))
        {
            if (parallel_rs485_master_poll_reply_guard_polls == 0U)
            {
                parallel_rs485_backend_read_quiet_polls = 0U;
            }
            if (parallel_rs485_node_run_status_is_valid(index) == 0U)
            {
                if ((parallel_status.params_crc32 == 0UL) ||
                    (parallel_nodes[index].params_crc32 != parallel_status.params_crc32))
                {
                    parallel_stop_running_group(PARALLEL_BLOCK_PARAMS_MISMATCH);
                }
                else
                {
                    parallel_stop_running_group(PARALLEL_BLOCK_NODE_NOT_READY);
                }
            }
        }
    }

    parallel_update_prepare_block();
}

void parallel_mode_on_rvc_control_rx(uint32_t dgn, const uint8_t *data, uint8_t len, uint8_t source_addr, uint8_t pc_source)
{
    uint16_t control;

    if ((data == NULL) || (len < 8U))
    {
        return;
    }

    if (dgn != PARALLEL_CAN_DGN_CONTROL)
    {
        parallel_mode_on_rvc_rx(dgn, data, len, source_addr);
        return;
    }

    control = parallel_read_u16_be(data, 1U);
    if (pc_source == 0U)
    {
        return;
    }

    if ((control == PARALLEL_CONTROL_PREPARE_START) ||
        (control == PARALLEL_CONTROL_RUN_ENABLE))
    {
        parallel_can_pc_control_seen = 1U;
    }
    else if ((control == PARALLEL_CONTROL_STOP) ||
             (control == PARALLEL_CONTROL_FORCE_STOP))
    {
        parallel_can_pc_control_seen = 1U;
    }
    else
    {
        return;
    }

    parallel_mode_on_rvc_rx(dgn, data, len, source_addr);
}

uint8_t parallel_mode_make_rvc_response(uint32_t dgn, uint8_t *data, uint8_t len)
{
    uint8_t local_rank;
    uint8_t can_temp_addr;

    parallel_init_state_if_needed();

    if ((data == NULL) || (len < 8U))
    {
        return 0U;
    }

    local_rank = parallel_get_local_rank();
    can_temp_addr = parallel_rank_to_can_addr(local_rank);

    if (dgn == 0xEF65U)
    {
        uint16_t block_fault;

        if (parallel_can_pc_control_seen == 0U)
        {
            return 0U;
        }

        block_fault = parallel_sanitize_block_fault(parallel_status.block_fault);
        parallel_status.block_fault = block_fault;
        data[1] = (uint8_t)(parallel_get_state() & 0x00FFU);
        data[2] = (uint8_t)(parallel_get_role() & 0x00FFU);
        data[3] = 0U;
        data[4] = (uint8_t)(parallel_status.discovered_count & 0x00FFU);
        parallel_write_u16_be(data, 5U, block_fault);
        data[7] = can_temp_addr;
        return 1U;
    }

    if (dgn == 0xEF68U)
    {
        if (parallel_can_pc_control_seen == 0U)
        {
            return 0U;
        }

        parallel_write_u32_be(data, 1U, parallel_status.params_crc32);
        parallel_write_u16_be(data, 5U, parallel_status.ready_flags);
        data[7] = (uint8_t)(parallel_status.protocol_version & 0x00FFU);
        return 1U;
    }

    return 0U;
}

uint8_t parallel_mode_is_run_allowed(void)
{
    parallel_init_state_if_needed();
    return (parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED) ? 1U : 0U;
}

uint8_t parallel_mode_should_block_local_run(void)
{
    uint16_t state;

    parallel_init_state_if_needed();
    state = parallel_get_state();

    return (((state != PARALLEL_STATE_IDLE) &&
             (state != PARALLEL_STATE_RUN_ALLOWED)) ? 1U : 0U);
}

uint8_t parallel_mode_get_rs485_runtime_addr(void)
{
    parallel_init_state_if_needed();

    if (parallel_is_requested() == 0U)
    {
        return 0U;
    }

    if (parallel_get_role() == PARALLEL_ROLE_MASTER)
    {
        return PARALLEL_RS485_TEMP_ADDR_MASTER;
    }

    return parallel_rank_to_rs485_addr(parallel_get_local_rank());
}

uint8_t parallel_mode_get_can_runtime_addr(void)
{
    parallel_init_state_if_needed();

    if (parallel_is_requested() == 0U)
    {
        return 0U;
    }

    if ((parallel_status.control == PARALLEL_CONTROL_PREPARE_START) &&
        (parallel_master_locked == 0U))
    {
        return 0U;
    }

    return parallel_rank_to_can_addr(parallel_get_local_rank());
}

static uint8_t parallel_mode_should_reply_rs485_broadcast_read(uint16_t addr, uint16_t count)
{
    return ((((addr == WG_COM_V2_REALTIME_DATA_ADDR) && (count == 0x0011U)) ||
             ((addr == WG_COM_V2_CTRL_ADDR) && (count == 0x000FU)) ||
             ((addr == (WG_COM_V2_PRUCUCT_INFO_ADDR + 0x0008U)) && (count == 0x000AU)) ||
             ((addr == PARALLEL_RS485_BASE_ADDR) && (count == PARALLEL_RS485_REG_COUNT))) ? 1U : 0U);
}

static uint8_t parallel_mode_should_reply_rs485_broadcast_write(uint16_t addr, uint16_t count)
{
    return (((parallel_mode_range_inside(addr, count, PARALLEL_RS485_CTRL_SYNC_START, PARALLEL_RS485_CTRL_SYNC_COUNT) != 0U) ||
             (parallel_mode_range_inside(addr, count, PARALLEL_RS485_PARAM_SYNC_START, PARALLEL_RS485_PARAM_SYNC_COUNT) != 0U)) ?
            1U :
            0U);
}

uint8_t parallel_mode_should_reply_rs485_broadcast(uint8_t cmd, uint16_t addr, uint16_t count)
{
    parallel_init_state_if_needed();

    if (parallel_get_state() != PARALLEL_STATE_RUN_ALLOWED)
    {
        return 0U;
    }

    if (parallel_is_requested() == 0U)
    {
        return 0U;
    }

    if (cmd == WG_COM_V2_CMD_READ)
    {
        return parallel_mode_should_reply_rs485_broadcast_read(addr, count);
    }

    if ((cmd == WG_COM_V2_CMD_WRITE_DATA) || (cmd == WG_COM_V2_CMD_WRITE_STR))
    {
        return parallel_mode_should_reply_rs485_broadcast_write(addr, count);
    }

    return 0U;
}

uint16_t parallel_mode_get_rs485_broadcast_reply_delay_ms(void)
{
    uint8_t runtime_addr = 0U;
    uint16_t delay_ms = 0U;

    runtime_addr = parallel_mode_get_rs485_runtime_addr();
    if (runtime_addr > PARALLEL_RS485_TEMP_ADDR_MASTER)
    {
        delay_ms = (uint16_t)(((uint16_t)runtime_addr - (uint16_t)PARALLEL_RS485_TEMP_ADDR_MASTER) *
                              PARALLEL_RS485_BROADCAST_REPLY_INTERVAL_MS);
    }

    return delay_ms;
}

uint8_t parallel_mode_read_registers(uint16_t addr, uint16_t count, uint8_t *data)
{
    uint16_t values[PARALLEL_RS485_REG_COUNT];
    uint16_t offset;
    uint16_t index;

    parallel_init_state_if_needed();

    if (data == NULL)
    {
        return 0U;
    }

    if (parallel_rs485_uid_announce_active() != 0U)
    {
        return 0U;
    }

    if ((addr < PARALLEL_RS485_BASE_ADDR) ||
        ((uint32_t)addr + (uint32_t)count) > ((uint32_t)PARALLEL_RS485_BASE_ADDR + PARALLEL_RS485_REG_COUNT))
    {
        return 0U;
    }

    parallel_update_local_ready_flags();
    parallel_update_prepare_block();

    values[0] = parallel_status.reserved0;
    values[1] = parallel_status.status_role;
    values[2] = (uint16_t)((parallel_status.session_id >> 16) & 0x0000FFFFUL);
    values[3] = (uint16_t)(parallel_status.session_id & 0x0000FFFFUL);
    values[4] = (uint16_t)((parallel_status.uid32 >> 16) & 0x0000FFFFUL);
    values[5] = (uint16_t)(parallel_status.uid32 & 0x0000FFFFUL);
    values[6] = parallel_rank_to_rs485_addr(parallel_get_local_rank());
    values[7] = parallel_status.discovered_count;
    values[8] = parallel_status.ready_flags;
    values[9] = parallel_status.block_fault;
    values[10] = (uint16_t)((parallel_status.params_crc32 >> 16) & 0x0000FFFFUL);
    values[11] = (uint16_t)(parallel_status.params_crc32 & 0x0000FFFFUL);
    values[12] = parallel_status.control;
    values[13] = parallel_status.heartbeat;
    values[14] = (uint16_t)((parallel_status.capability >> 16) & 0x0000FFFFUL);
    values[15] = (uint16_t)(parallel_status.capability & 0x0000FFFFUL);
    values[16] = parallel_status.protocol_version;

    offset = (uint16_t)(addr - PARALLEL_RS485_BASE_ADDR);
    for (index = 0U; index < count; index++)
    {
        parallel_write_u16_be(data, (uint8_t)(index * 2U), values[(uint16_t)(offset + index)]);
    }

    return 1U;
}

static uint8_t parallel_can_send_prepare_ready_ok(void)
{
#if (APP_PARALLEL_CAN_FEATURES == 1)
    uint16_t block_fault;

    if ((parallel_get_role() != PARALLEL_ROLE_MASTER) ||
        (parallel_status.discovered_count < 2U))
    {
        return 0U;
    }

    if (parallel_group_is_ready(&block_fault, 1U) == 0U)
    {
        parallel_status.block_fault = block_fault;
        parallel_can_prepare_ok_sent = 0U;
        return 0U;
    }

    parallel_status.block_fault = PARALLEL_BLOCK_NONE;
    parallel_set_role(PARALLEL_ROLE_MASTER);
    parallel_status.temp_addr = PARALLEL_ADDR_RANK_MASTER;
    parallel_master_uid32 = parallel_status.uid32;
    parallel_master_locked = 1U;
    parallel_node_refresh_local();

    if (parallel_can_prepare_ok_sent == 0U)
    {
        (void)parallel_can_send_uid_frame(PARALLEL_CAN_DGN_READY_OK,
                                          (uint8_t)parallel_status.discovered_count,
                                          parallel_status.uid32);
        parallel_can_prepare_ok_sent = 1U;
    }

    return 1U;
#else
    return 0U;
#endif
}

uint8_t parallel_mode_write_registers(uint16_t addr, uint16_t count, const uint8_t *data)
{
    uint16_t offset;
    uint16_t index;
    uint16_t reg_offset;
    uint16_t value;

    parallel_init_state_if_needed();

    if ((data == NULL) ||
        (addr < PARALLEL_RS485_BASE_ADDR) ||
        ((uint32_t)addr + (uint32_t)count) > ((uint32_t)PARALLEL_RS485_BASE_ADDR + PARALLEL_RS485_REG_COUNT))
    {
        return 0U;
    }

    offset = (uint16_t)(addr - PARALLEL_RS485_BASE_ADDR);
    for (index = 0U; index < count; index++)
    {
        reg_offset = (uint16_t)(offset + index);
        value = parallel_read_u16_be(data, (uint8_t)(index * 2U));
        if (reg_offset == 0x0006U)
        {
            if ((value >= PARALLEL_ADDR_RANK_MASTER) &&
                (value <= PARALLEL_MAX_NODE_COUNT) &&
                (parallel_rank_is_valid((uint8_t)value) != 0U))
            {
                parallel_status.temp_addr = value;
                if (value == PARALLEL_ADDR_RANK_MASTER)
                {
                    parallel_master_uid32 = parallel_status.uid32;
                }
            }
        }
        else if (reg_offset == 0x000AU)
        {
            parallel_requested_params_crc32 &= 0x0000FFFFUL;
            parallel_requested_params_crc32 |= ((uint32_t)value << 16);
            parallel_requested_params_crc_mask |= 0x01U;
            if (parallel_requested_params_crc_mask == 0x03U)
            {
                parallel_apply_params_crc(parallel_requested_params_crc32);
                parallel_requested_params_crc_mask = 0U;
            }
        }
        else if (reg_offset == 0x000BU)
        {
            parallel_requested_params_crc32 &= 0xFFFF0000UL;
            parallel_requested_params_crc32 |= (uint32_t)value;
            parallel_requested_params_crc_mask |= 0x02U;
            if (parallel_requested_params_crc_mask == 0x03U)
            {
                parallel_apply_params_crc(parallel_requested_params_crc32);
                parallel_requested_params_crc_mask = 0U;
            }
        }
        else if (reg_offset == 0x000CU)
        {
            parallel_apply_control(value, PARALLEL_TRANSPORT_RS485);
        }
    }

    return 1U;
}

#else

void parallel_mode_run_10ms(void)
{
}

void parallel_mode_on_rvc_rx(uint32_t dgn, const uint8_t *data, uint8_t len, uint8_t source_addr)
{
    (void)dgn;
    (void)data;
    (void)len;
    (void)source_addr;
}

void parallel_mode_on_rs485_frame(const uint8_t *frame, uint16_t len)
{
    (void)frame;
    (void)len;
}

uint8_t parallel_mode_make_rvc_response(uint32_t dgn, uint8_t *data, uint8_t len)
{
    (void)dgn;
    (void)data;
    (void)len;
    return 0U;
}

uint8_t parallel_mode_is_run_allowed(void)
{
    return 0U;
}

uint8_t parallel_mode_should_block_local_run(void)
{
    return 0U;
}

uint8_t parallel_mode_get_rs485_runtime_addr(void)
{
    return 0U;
}

uint8_t parallel_mode_get_can_runtime_addr(void)
{
    return 0U;
}

uint8_t parallel_mode_should_reply_rs485_broadcast(uint8_t cmd, uint16_t addr, uint16_t count)
{
    (void)cmd;
    (void)addr;
    (void)count;
    return 0U;
}

uint16_t parallel_mode_get_rs485_broadcast_reply_delay_ms(void)
{
    return 0U;
}

uint8_t parallel_mode_read_registers(uint16_t addr, uint16_t count, uint8_t *data)
{
    (void)addr;
    (void)count;
    (void)data;
    return 0U;
}

uint8_t parallel_mode_write_registers(uint16_t addr, uint16_t count, const uint8_t *data)
{
    (void)addr;
    (void)count;
    (void)data;
    return 0U;
}

#endif

#if (APP_PARALLEL_MODE_FEATURES == 1)
REG_TASK(10, parallel_mode_run_10ms)
#endif
