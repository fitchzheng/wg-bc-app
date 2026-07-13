#include "parallel_mode.h"
#include "app_features.h"
#include "section.h"
#include "wg_com_v2.h"
#include "ctrl_app.h"
#include "fault.h"
#include "gpio.h"
#include "string.h"

#if (APP_PARALLEL_MODE_FEATURES == 1)

#define PARALLEL_PROTOCOL_VERSION             1U
#define PARALLEL_CAP_CAN                      0x0001UL
#define PARALLEL_CAP_RS485                    0x0002UL
#define PARALLEL_CAP_LOCAL_DROOP              0x0004UL
#define PARALLEL_HEARTBEAT_TICKS              50U
#define PARALLEL_MAX_NODE_COUNT               30U
#define PARALLEL_NODE_TIMEOUT_TICKS           30U

#define PARALLEL_NODE_FLAG_USED               0x01U
#define PARALLEL_NODE_FLAG_LOCAL              0x02U
#define PARALLEL_NODE_FLAG_READY              0x04U

#define PARALLEL_ADDR_RANK_MASTER             1U
#define PARALLEL_RS485_TEMP_ADDR_MASTER       2U
#define PARALLEL_CAN_TEMP_ADDR_MASTER         75U

static parallel_mode_status_t parallel_status;
static uint16_t parallel_tick_10ms;
static uint8_t parallel_node_tick_10ms;
static uint32_t parallel_confirmed_params_crc32;
static uint32_t parallel_requested_params_crc32;

typedef struct
{
    uint32_t uid32;
    uint32_t params_crc32;
    uint16_t ready_flags;
    uint8_t temp_addr;
    uint8_t role_state;
    uint8_t timeout;
    uint8_t flags;
} parallel_node_t;

static parallel_node_t parallel_nodes[PARALLEL_MAX_NODE_COUNT];

static uint16_t parallel_read_u16_be(const uint8_t *data, uint8_t offset);
static uint32_t parallel_read_u32_be(const uint8_t *data, uint8_t offset);
static uint32_t parallel_calc_params_crc(void);
static uint16_t parallel_get_state(void);
static void parallel_apply_params_crc(uint32_t params_crc32);

static uint8_t parallel_pg_is_active(void)
{
    return (get_key_pg_val() != 0U) ? 1U : 0U;
}

static uint8_t parallel_rank_to_rs485_addr(uint8_t rank)
{
    return (uint8_t)(PARALLEL_RS485_TEMP_ADDR_MASTER + rank - 1U);
}

static uint8_t parallel_rank_to_can_addr(uint8_t rank)
{
    return (uint8_t)(PARALLEL_CAN_TEMP_ADDR_MASTER + rank - 1U);
}

static uint8_t parallel_can_addr_to_rank(uint8_t addr)
{
    uint8_t rank = PARALLEL_ADDR_RANK_MASTER;

    if ((addr >= PARALLEL_CAN_TEMP_ADDR_MASTER) &&
        (addr < (PARALLEL_CAN_TEMP_ADDR_MASTER + PARALLEL_MAX_NODE_COUNT)))
    {
        rank = (uint8_t)(addr - PARALLEL_CAN_TEMP_ADDR_MASTER + 1U);
    }

    return rank;
}

static uint8_t parallel_is_requested(void)
{
    uint16_t state = parallel_get_state();

    if ((state != PARALLEL_STATE_IDLE) || (parallel_pg_is_active() != 0U))
    {
        return 1U;
    }

    return 0U;
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

static uint16_t parallel_get_state(void)
{
    return (uint16_t)(parallel_status.status_role & 0x00FFU);
}

static void parallel_set_state(uint16_t state)
{
    parallel_status.status_role &= 0xFF00U;
    parallel_status.status_role |= (uint16_t)(state & 0x00FFU);
}

static void parallel_set_role(uint16_t role)
{
    parallel_status.status_role &= 0x00FFU;
    parallel_status.status_role |= (uint16_t)((role & 0x00FFU) << 8);
}

static uint16_t parallel_get_role(void)
{
    return (uint16_t)((parallel_status.status_role >> 8) & 0x00FFU);
}

static void parallel_mark_prepare_seen(void)
{
    uint16_t state = parallel_get_state();

    if (state == PARALLEL_STATE_IDLE)
    {
        parallel_status.control = PARALLEL_CONTROL_PREPARE_START;
        parallel_status.block_fault = PARALLEL_BLOCK_LOCAL_NOT_READY;
        parallel_set_state(PARALLEL_STATE_PREPARE);
    }
}

static uint32_t parallel_build_uid32(void)
{
    uint32_t hash = 2166136261UL;
    uint8_t index;

    for (index = 0U; index < 10U; index++)
    {
        uint16_t word = wg_com_v2_product_info.SnSerial[index];
        hash ^= (uint32_t)(word & 0x00FFU);
        hash *= 16777619UL;
        hash ^= (uint32_t)((word >> 8) & 0x00FFU);
        hash *= 16777619UL;
    }

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
        parallel_status.temp_addr = 0U;
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

static uint8_t parallel_node_find_addr(uint8_t temp_addr)
{
    uint8_t index;

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            (parallel_nodes[index].temp_addr == temp_addr))
        {
            return index;
        }
    }

    return PARALLEL_MAX_NODE_COUNT;
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

static void parallel_node_remove(uint32_t uid32)
{
    uint8_t index;

    index = parallel_node_find(uid32);
    if ((index < PARALLEL_MAX_NODE_COUNT) &&
        ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_LOCAL) == 0U))
    {
        memset(&parallel_nodes[index], 0, sizeof(parallel_nodes[index]));
    }
}

static void parallel_block_if_running(uint16_t block_fault)
{
    if (parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED)
    {
        parallel_status.block_fault = block_fault;
        parallel_set_state(PARALLEL_STATE_BLOCKED);
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
        if ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U)
        {
            count++;
        }
    }

    return count;
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

static uint8_t parallel_node_rank(uint32_t uid32)
{
    uint8_t index;
    uint8_t rank = 1U;

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if (((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U) &&
            (parallel_nodes[index].uid32 < uid32))
        {
            rank++;
        }
    }

    return rank;
}

static void parallel_update_role_and_addr(void)
{
    uint32_t min_uid;
    uint8_t rank;

    parallel_node_refresh_local();
    parallel_status.discovered_count = parallel_node_count_used();

    min_uid = parallel_node_min_uid();
    if ((min_uid != 0UL) && (parallel_status.uid32 == min_uid))
    {
        parallel_set_role(PARALLEL_ROLE_MASTER);
    }
    else
    {
        parallel_set_role(PARALLEL_ROLE_SLAVE);
    }

    rank = parallel_node_rank(parallel_status.uid32);
    parallel_status.temp_addr = (uint16_t)((rank <= PARALLEL_MAX_NODE_COUNT) ? rank : PARALLEL_ADDR_RANK_MASTER);
    parallel_node_refresh_local();
}

static void parallel_node_age_100ms(void)
{
    uint8_t index;
    uint8_t removed = 0U;

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
                memset(&parallel_nodes[index], 0, sizeof(parallel_nodes[index]));
                removed = 1U;
            }
        }
    }

    if ((removed != 0U) && (parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED))
    {
        parallel_status.ready_flags |= PARALLEL_READY_GROUP_LIMITING;
        if (parallel_status.expected_node_count != 0U)
        {
            parallel_block_if_running(PARALLEL_BLOCK_NODE_COUNT);
        }
    }
}

static void parallel_node_update_addr_frame(const uint8_t *data)
{
    uint32_t uid32;
    uint8_t index;

    uid32 = parallel_read_u32_be(data, 1U);
    if ((uid32 == 0UL) || (uid32 == parallel_status.uid32))
    {
        if (uid32 == parallel_status.uid32)
        {
            parallel_status.temp_addr = parallel_can_addr_to_rank(data[5]);
        }
        return;
    }

    index = parallel_node_alloc(uid32);
    if (index < PARALLEL_MAX_NODE_COUNT)
    {
        parallel_nodes[index].temp_addr = parallel_can_addr_to_rank(data[5]);
        parallel_nodes[index].role_state = (uint8_t)(((uint8_t)data[7] << 4) | (data[6] & 0x0FU));
        parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
        parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;
    }
}

static void parallel_node_update_ack_frame(const uint8_t *data)
{
    uint32_t uid32;
    uint8_t index;

    uid32 = parallel_read_u32_be(data, 1U);
    if ((uid32 == 0UL) || (uid32 == parallel_status.uid32))
    {
        return;
    }

    index = parallel_node_alloc(uid32);
    if (index < PARALLEL_MAX_NODE_COUNT)
    {
        parallel_nodes[index].temp_addr = parallel_can_addr_to_rank(data[5]);
        parallel_nodes[index].ready_flags = data[6];
        parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
        parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;

        if (((uint8_t)(parallel_status.params_crc32 & 0x000000FFUL) == data[7]) &&
            (parallel_status.params_crc32 != 0UL))
        {
            parallel_nodes[index].params_crc32 = parallel_status.params_crc32;
        }
        else
        {
            parallel_nodes[index].params_crc32 = (uint32_t)data[7];
        }
    }
}

static void parallel_node_update_heartbeat_frame(const uint8_t *data)
{
    uint8_t index;
    uint8_t rank;

    rank = parallel_can_addr_to_rank(data[1]);
    index = parallel_node_find_addr(rank);
    if (index < PARALLEL_MAX_NODE_COUNT)
    {
        parallel_nodes[index].ready_flags = parallel_read_u16_be(data, 5U);
        parallel_nodes[index].timeout = PARALLEL_NODE_TIMEOUT_TICKS;
        parallel_nodes[index].flags |= PARALLEL_NODE_FLAG_READY;
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
        (parallel_confirmed_params_crc32 == parallel_status.params_crc32))
    {
        ready_flags |= PARALLEL_READY_PARAMS_CRC;
    }

    if ((parallel_status.ready_flags & PARALLEL_READY_GROUP_LIMITING) != 0U)
    {
        ready_flags |= PARALLEL_READY_GROUP_LIMITING;
    }

    parallel_status.ready_flags = ready_flags;
}

static uint8_t parallel_group_is_ready(uint16_t *block_fault)
{
    uint8_t index;
    uint16_t required_flags;

    required_flags = (PARALLEL_READY_PG_OFF |
                      PARALLEL_READY_HW_VALID |
                      PARALLEL_READY_OUTPUT_OFF |
                      PARALLEL_READY_NO_FAULT |
                      PARALLEL_READY_HEARTBEAT_FRESH |
                      PARALLEL_READY_PARAMS_CRC);

    if (parallel_status.discovered_count == 0U)
    {
        *block_fault = PARALLEL_BLOCK_NODE_COUNT;
        return 0U;
    }

    if ((parallel_status.expected_node_count != 0U) &&
        (parallel_status.discovered_count != parallel_status.expected_node_count))
    {
        *block_fault = PARALLEL_BLOCK_NODE_COUNT;
        return 0U;
    }

    for (index = 0U; index < PARALLEL_MAX_NODE_COUNT; index++)
    {
        if ((parallel_nodes[index].flags & PARALLEL_NODE_FLAG_USED) != 0U)
        {
            if ((parallel_nodes[index].ready_flags & required_flags) != required_flags)
            {
                *block_fault = PARALLEL_BLOCK_NODE_NOT_READY;
                return 0U;
            }

            if ((parallel_status.params_crc32 == 0UL) ||
                (parallel_nodes[index].params_crc32 != parallel_status.params_crc32))
            {
                *block_fault = PARALLEL_BLOCK_PARAMS_MISMATCH;
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

static void parallel_update_prepare_block(void)
{
    uint16_t block_fault;
    uint16_t state;

    state = parallel_get_state();
    if ((parallel_status.control != PARALLEL_CONTROL_PREPARE_START) ||
        ((state != PARALLEL_STATE_PREPARE) && (state != PARALLEL_STATE_BLOCKED)))
    {
        return;
    }

    if (parallel_group_is_ready(&block_fault) != 0U)
    {
        parallel_status.block_fault = PARALLEL_BLOCK_NONE;
        parallel_set_state(PARALLEL_STATE_PREPARE);
    }
    else if ((parallel_status.block_fault != PARALLEL_BLOCK_FORCE_STOP) &&
             (parallel_status.block_fault != PARALLEL_BLOCK_FAULT))
    {
        parallel_status.block_fault = block_fault;
        parallel_set_state(PARALLEL_STATE_PREPARE);
    }
}

static void parallel_write_u16_be(uint8_t *data, uint8_t offset, uint16_t value)
{
    data[offset] = (uint8_t)((value >> 8) & 0x00FFU);
    data[(uint8_t)(offset + 1U)] = (uint8_t)(value & 0x00FFU);
}

static uint16_t parallel_read_u16_be(const uint8_t *data, uint8_t offset)
{
    return (uint16_t)(((uint16_t)data[offset] << 8) | (uint16_t)data[(uint8_t)(offset + 1U)]);
}

static void parallel_write_u32_be(uint8_t *data, uint8_t offset, uint32_t value)
{
    data[offset] = (uint8_t)((value >> 24) & 0x000000FFUL);
    data[(uint8_t)(offset + 1U)] = (uint8_t)((value >> 16) & 0x000000FFUL);
    data[(uint8_t)(offset + 2U)] = (uint8_t)((value >> 8) & 0x000000FFUL);
    data[(uint8_t)(offset + 3U)] = (uint8_t)(value & 0x000000FFUL);
}

static uint32_t parallel_read_u32_be(const uint8_t *data, uint8_t offset)
{
    return (((uint32_t)data[offset]) << 24) |
           (((uint32_t)data[(uint8_t)(offset + 1U)]) << 16) |
           (((uint32_t)data[(uint8_t)(offset + 2U)]) << 8) |
           ((uint32_t)data[(uint8_t)(offset + 3U)]);
}

static uint32_t parallel_hash_u16(uint32_t hash, uint16_t value)
{
    hash ^= (uint32_t)(value & 0x00FFU);
    hash *= 16777619UL;
    hash ^= (uint32_t)((value >> 8) & 0x00FFU);
    hash *= 16777619UL;
    return hash;
}

static uint32_t parallel_calc_params_crc(void)
{
    uint32_t hash = 2166136261UL;

    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.SetPowerMode);
    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.SetChargMode);
    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.InpBatyType);
    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.OutBatyType);
    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.SetBootTimeA);
    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.SetBootTimeB);
    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.SetOnCurrStartTimeA);
    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.SetOnCurrStartTimeB);
    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.BatModeFR);
    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.MpptSwitch);
    hash = parallel_hash_u16(hash, wg_com_v2_ctrl.SleepModeOnOff);

    hash = parallel_hash_u16(hash, wg_com_v2_param.SetInpVolt);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetInpCurr);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetInpCurrPower);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetOutVolt);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetOutCurr);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetOutCurrPower);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetInpUvlo);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetInpUvloRecover);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetInpOVP);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetInpOVPRecover);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetOutUvlo);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetOutUvloRecover);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetOutOVP);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetOutOVPRecover);
    hash = parallel_hash_u16(hash, (uint16_t)wg_com_v2_param.SetInsideTemp);
    hash = parallel_hash_u16(hash, (uint16_t)wg_com_v2_param.SetOutsideTemp);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetInpChargLedCurr);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetInpFullLedCurr);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetOutChargLedCurr);
    hash = parallel_hash_u16(hash, wg_com_v2_param.SetOutFullLedCurr);
    hash = parallel_hash_u16(hash, wg_com_v2_param.AuotForwardOpenVoltA);
    hash = parallel_hash_u16(hash, wg_com_v2_param.AuotForwardVeerVoltA);
    hash = parallel_hash_u16(hash, wg_com_v2_param.AuotForwardShutVoltA);
    hash = parallel_hash_u16(hash, wg_com_v2_param.AuotReverseOpenVoltB);
    hash = parallel_hash_u16(hash, wg_com_v2_param.AuotReverseShutVoltB);
    hash = parallel_hash_u16(hash, (uint16_t)wg_com_v2_param.SetTemp2);
    hash = parallel_hash_u16(hash, wg_com_v2_param.VoltCompensationAK);
    hash = parallel_hash_u16(hash, wg_com_v2_param.VoltCompensationAB);
    hash = parallel_hash_u16(hash, wg_com_v2_param.VoltCompensationBK);
    hash = parallel_hash_u16(hash, wg_com_v2_param.VoltCompensationBB);

    return (hash == 0UL) ? 1UL : hash;
}

static void parallel_update_params_crc(void)
{
    uint32_t params_crc32;

    params_crc32 = parallel_calc_params_crc();
    if (parallel_status.params_crc32 != params_crc32)
    {
        parallel_status.params_crc32 = params_crc32;
        parallel_confirmed_params_crc32 = 0UL;
        if ((parallel_status.block_fault == PARALLEL_BLOCK_PARAMS_MISMATCH) ||
            (parallel_status.block_fault == PARALLEL_BLOCK_CONFIG_MISSING))
        {
            parallel_status.block_fault = PARALLEL_BLOCK_LOCAL_NOT_READY;
            parallel_set_state(PARALLEL_STATE_PREPARE);
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
    }
}

static void parallel_apply_control(uint16_t control)
{
    uint16_t block_fault;

    parallel_status.control = control;

    if (control == PARALLEL_CONTROL_STOP)
    {
        parallel_status.block_fault = PARALLEL_BLOCK_NONE;
        parallel_set_state(PARALLEL_STATE_IDLE);
        return;
    }

    if (control == PARALLEL_CONTROL_FORCE_STOP)
    {
        parallel_status.block_fault = PARALLEL_BLOCK_FORCE_STOP;
        parallel_set_state(PARALLEL_STATE_BLOCKED);
        return;
    }

    if (control == PARALLEL_CONTROL_PREPARE_START)
    {
        parallel_status.block_fault = PARALLEL_BLOCK_LOCAL_NOT_READY;
        parallel_set_state(PARALLEL_STATE_PREPARE);
        return;
    }

    if (control == PARALLEL_CONTROL_RUN_ENABLE)
    {
        if (parallel_get_role() == PARALLEL_ROLE_MASTER)
        {
            if (parallel_group_is_ready(&block_fault) != 0U)
            {
                parallel_status.block_fault = PARALLEL_BLOCK_NONE;
                parallel_set_state(PARALLEL_STATE_RUN_ALLOWED);
            }
            else
            {
                parallel_status.block_fault = block_fault;
                parallel_set_state(PARALLEL_STATE_BLOCKED);
            }
        }
        else if (parallel_local_is_ready(&block_fault) != 0U)
        {
            parallel_status.block_fault = PARALLEL_BLOCK_NONE;
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

void parallel_mode_run_10ms(void)
{
    uint16_t block_fault;

    parallel_init_state_if_needed();
    parallel_update_params_crc();
    parallel_update_local_ready_flags();

    parallel_tick_10ms++;
    if (parallel_tick_10ms >= PARALLEL_HEARTBEAT_TICKS)
    {
        parallel_tick_10ms = 0U;
        parallel_status.heartbeat++;
        parallel_status.uid32 = parallel_build_uid32();
    }

    parallel_node_tick_10ms++;
    if (parallel_node_tick_10ms >= 10U)
    {
        parallel_node_tick_10ms = 0U;
        parallel_node_age_100ms();
    }

    parallel_update_role_and_addr();
    parallel_update_prepare_block();

    if ((parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED) &&
        (parallel_get_role() == PARALLEL_ROLE_MASTER) &&
        (parallel_group_is_ready(&block_fault) == 0U))
    {
        parallel_status.block_fault = block_fault;
        parallel_set_state(PARALLEL_STATE_BLOCKED);
    }
}

void parallel_mode_on_rvc_rx(uint32_t dgn, const uint8_t *data, uint8_t len)
{
    parallel_init_state_if_needed();

    if ((data == NULL) || (len < 8U))
    {
        return;
    }

    switch (dgn)
    {
        case 0xEF66U:
            parallel_apply_control(parallel_read_u16_be(data, 1U));
            break;

        case 0xEF67U:
            parallel_status.block_fault = parallel_read_u16_be(data, 1U);
            parallel_set_state(PARALLEL_STATE_BLOCKED);
            break;

        case 0xEF69U:
            parallel_apply_params_crc(parallel_read_u32_be(data, 1U));
            break;

        case 0xEF6BU:
            parallel_apply_control(PARALLEL_CONTROL_RUN_ENABLE);
            break;

        case 0xEF60U:
        case 0xEF61U:
        case 0xEF62U:
        case 0xEF6AU:
            parallel_node_update_addr_frame(data);
            parallel_status.block_fault = PARALLEL_BLOCK_LOCAL_NOT_READY;
            parallel_set_state(PARALLEL_STATE_PREPARE);
            break;

        case 0xEF63U:
            parallel_node_update_ack_frame(data);
            parallel_update_prepare_block();
            break;

        case 0xEF64U:
            parallel_node_update_heartbeat_frame(data);
            parallel_update_prepare_block();
            break;

        case 0xEF6CU:
            parallel_node_remove(parallel_read_u32_be(data, 1U));
            if ((parallel_get_state() == PARALLEL_STATE_RUN_ALLOWED) &&
                (parallel_status.expected_node_count != 0U))
            {
                parallel_block_if_running(PARALLEL_BLOCK_NODE_COUNT);
            }
            break;

        default:
            break;
    }
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

    switch (dgn)
    {
        case 0xEF60U:
        case 0xEF61U:
        case 0xEF6AU:
        case 0xEF6BU:
        case 0xEF6CU:
            parallel_write_u32_be(data, 1U, parallel_status.uid32);
            data[5] = can_temp_addr;
            data[6] = (uint8_t)(parallel_status.status_role & 0x00FFU);
            data[7] = (uint8_t)((parallel_status.status_role >> 8) & 0x00FFU);
            return 1U;

        case 0xEF63U:
            parallel_write_u32_be(data, 1U, parallel_status.uid32);
            data[5] = can_temp_addr;
            data[6] = (uint8_t)(parallel_status.ready_flags & 0x00FFU);
            data[7] = (uint8_t)(parallel_status.params_crc32 & 0x000000FFUL);
            return 1U;

        case 0xEF62U:
            parallel_write_u32_be(data, 1U, parallel_status.uid32);
            data[5] = can_temp_addr;
            data[6] = (uint8_t)(parallel_status.expected_node_count & 0x00FFU);
            data[7] = (uint8_t)(parallel_status.discovered_count & 0x00FFU);
            return 1U;

        case 0xEF64U:
            data[1] = can_temp_addr;
            data[2] = (uint8_t)(parallel_get_state() & 0x00FFU);
            parallel_write_u16_be(data, 3U, parallel_status.heartbeat);
            parallel_write_u16_be(data, 5U, parallel_status.ready_flags);
            data[7] = (uint8_t)(parallel_status.block_fault & 0x00FFU);
            return 1U;

        case 0xEF65U:
        case 0xEF66U:
        case 0xEF67U:
            data[1] = (uint8_t)(parallel_get_state() & 0x00FFU);
            data[2] = (uint8_t)((parallel_status.status_role >> 8) & 0x00FFU);
            data[3] = (uint8_t)(parallel_status.expected_node_count & 0x00FFU);
            data[4] = (uint8_t)(parallel_status.discovered_count & 0x00FFU);
            parallel_write_u16_be(data, 5U, parallel_status.block_fault);
            data[7] = can_temp_addr;
            return 1U;

        case 0xEF68U:
        case 0xEF69U:
            parallel_write_u32_be(data, 1U, parallel_status.params_crc32);
            parallel_write_u16_be(data, 5U, parallel_status.ready_flags);
            data[7] = (uint8_t)(parallel_status.protocol_version & 0x00FFU);
            return 1U;

        default:
            break;
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

    if ((state == PARALLEL_STATE_IDLE) ||
        (state == PARALLEL_STATE_RUN_ALLOWED))
    {
        return 0U;
    }

    return 1U;
}

void parallel_mode_get_status(parallel_mode_status_t *status)
{
    parallel_init_state_if_needed();

    if (status != NULL)
    {
        *status = parallel_status;
    }
}

uint8_t parallel_mode_get_rs485_runtime_addr(void)
{
    parallel_init_state_if_needed();

    if (parallel_is_requested() == 0U)
    {
        return 0U;
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

    return parallel_rank_to_can_addr(parallel_get_local_rank());
}

uint8_t parallel_mode_read_registers(uint16_t addr, uint16_t count, uint8_t *data)
{
    uint16_t offset;
    uint16_t index;
    uint16_t value;

    parallel_init_state_if_needed();

    if ((data == NULL) ||
        (addr < PARALLEL_RS485_BASE_ADDR) ||
        ((uint32_t)addr + (uint32_t)count) > ((uint32_t)PARALLEL_RS485_BASE_ADDR + PARALLEL_RS485_REG_COUNT))
    {
        return 0U;
    }

    parallel_mark_prepare_seen();

    offset = (uint16_t)(addr - PARALLEL_RS485_BASE_ADDR);
    for (index = 0U; index < count; index++)
    {
        switch ((uint16_t)(offset + index))
        {
            case 0x0000U:
                value = parallel_status.expected_node_count;
                break;
            case 0x0001U:
                value = parallel_status.status_role;
                break;
            case 0x0002U:
                value = (uint16_t)((parallel_status.session_id >> 16) & 0x0000FFFFUL);
                break;
            case 0x0003U:
                value = (uint16_t)(parallel_status.session_id & 0x0000FFFFUL);
                break;
            case 0x0004U:
                value = (uint16_t)((parallel_status.uid32 >> 16) & 0x0000FFFFUL);
                break;
            case 0x0005U:
                value = (uint16_t)(parallel_status.uid32 & 0x0000FFFFUL);
                break;
            case 0x0006U:
                value = parallel_rank_to_rs485_addr(parallel_get_local_rank());
                break;
            case 0x0007U:
                value = parallel_status.discovered_count;
                break;
            case 0x0008U:
                value = parallel_status.ready_flags;
                break;
            case 0x0009U:
                value = parallel_status.block_fault;
                break;
            case 0x000AU:
                value = (uint16_t)((parallel_status.params_crc32 >> 16) & 0x0000FFFFUL);
                break;
            case 0x000BU:
                value = (uint16_t)(parallel_status.params_crc32 & 0x0000FFFFUL);
                break;
            case 0x000CU:
                value = parallel_status.control;
                break;
            case 0x000DU:
                value = parallel_status.heartbeat;
                break;
            case 0x000EU:
                value = (uint16_t)((parallel_status.capability >> 16) & 0x0000FFFFUL);
                break;
            case 0x000FU:
                value = (uint16_t)(parallel_status.capability & 0x0000FFFFUL);
                break;
            default:
                value = parallel_status.protocol_version;
                break;
        }
        parallel_write_u16_be(data, (uint8_t)(index * 2U), value);
    }

    return 1U;
}

uint8_t parallel_mode_write_registers(uint16_t addr, uint16_t count, const uint8_t *data)
{
    uint16_t offset;
    uint16_t index;
    uint16_t value;

    parallel_init_state_if_needed();

    if ((data == NULL) ||
        (addr < PARALLEL_RS485_BASE_ADDR) ||
        ((uint32_t)addr + (uint32_t)count) > ((uint32_t)PARALLEL_RS485_BASE_ADDR + PARALLEL_RS485_REG_COUNT))
    {
        return 0U;
    }

    parallel_mark_prepare_seen();

    offset = (uint16_t)(addr - PARALLEL_RS485_BASE_ADDR);
    for (index = 0U; index < count; index++)
    {
        value = parallel_read_u16_be(data, (uint8_t)(index * 2U));
        switch ((uint16_t)(offset + index))
        {
            case 0x0000U:
                if (value <= PARALLEL_MAX_NODE_COUNT)
                {
                    parallel_status.expected_node_count = value;
                }
                break;
            case 0x000AU:
                parallel_requested_params_crc32 &= 0x0000FFFFUL;
                parallel_requested_params_crc32 |= ((uint32_t)value << 16);
                parallel_apply_params_crc(parallel_requested_params_crc32);
                break;
            case 0x000BU:
                parallel_requested_params_crc32 &= 0xFFFF0000UL;
                parallel_requested_params_crc32 |= (uint32_t)value;
                parallel_apply_params_crc(parallel_requested_params_crc32);
                break;
            case 0x000CU:
                parallel_apply_control(value);
                break;
            default:
                break;
        }
    }

    return 1U;
}

#else

void parallel_mode_run_10ms(void)
{
}

void parallel_mode_on_rvc_rx(uint32_t dgn, const uint8_t *data, uint8_t len)
{
    (void)dgn;
    (void)data;
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

void parallel_mode_get_status(parallel_mode_status_t *status)
{
    (void)status;
}

uint8_t parallel_mode_get_rs485_runtime_addr(void)
{
    return 0U;
}

uint8_t parallel_mode_get_can_runtime_addr(void)
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
