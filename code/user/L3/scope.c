#include "scope.h"

#include "comm.h"
#include "section.h"
#include <stddef.h>
#include <string.h>

#if (APP_DEBUG_FEATURES == 1)

#define SCOPE_SERVICE_VAR_COUNT_MAX 10u
#define SCOPE_SERVICE_NAME_SIZE_MAX 32u

static scope_service_obj_t *g_scope_service_first = NULL;
static uint8_t g_scope_service_count = 0u;

static scope_service_obj_t *scope_service_find_by_id(uint8_t scope_id)
{
    scope_service_obj_t *p_obj = g_scope_service_first;

    while (p_obj != NULL)
    {
        if (p_obj->scope_id == scope_id)
        {
            return p_obj;
        }
        p_obj = p_obj->p_next;
    }

    return NULL;
}

static uint32_t scope_service_get_trigger_display_index(scope_t *p_scope)
{
    if ((p_scope == NULL) || (p_scope->buffer_size == 0u) || (p_scope->trigger_post_cnt >= p_scope->buffer_size))
    {
        return 0u;
    }

    return p_scope->buffer_size - p_scope->trigger_post_cnt - 1u;
}

static uint32_t scope_service_get_logical_start_index(scope_t *p_scope, uint8_t read_mode)
{
    if ((p_scope == NULL) || (p_scope->buffer_size == 0u))
    {
        return 0u;
    }

    if ((read_mode == SCOPE_READ_MODE_FORCE) &&
        (p_scope->state == SCOPE_STATE_RUNNING) &&
        (p_scope->in_trigger == 0u))
    {
        return p_scope->write_index % p_scope->buffer_size;
    }

    return (p_scope->trigger_index + p_scope->trigger_post_cnt + 1u) % p_scope->buffer_size;
}

static uint32_t scope_service_logical_to_physical_index(scope_t *p_scope, uint8_t read_mode, uint32_t logical_index)
{
    uint32_t start_index = scope_service_get_logical_start_index(p_scope, read_mode);

    if ((p_scope == NULL) || (p_scope->buffer_size == 0u))
    {
        return 0u;
    }

    return (start_index + logical_index) % p_scope->buffer_size;
}

static void scope_service_fill_ctrl_ack(scope_service_obj_t *p_obj, scope_tool_status_e status, scope_ctrl_ack_t *p_ack)
{
    memset((uint8_t *)p_ack, 0, sizeof(*p_ack));
    if (p_obj == NULL)
    {
        p_ack->status = (uint8_t)status;
        return;
    }

    p_ack->scope_id = p_obj->scope_id;
    p_ack->status = (uint8_t)status;
    p_ack->state = (uint8_t)p_obj->p_scope->state;
    p_ack->data_ready = p_obj->data_ready;
    p_ack->capture_tag = p_obj->capture_tag;
}

static void scope_service_reply(section_packform_t *p_req_pack,
                                DEC_MY_PRINTF,
                                uint8_t cmd_word,
                                uint8_t is_ack,
                                uint8_t *p_data,
                                uint16_t len)
{
    section_packform_t packform = {0};

    packform.cmd_set = CMD_SET_SCOPE;
    packform.cmd_word = cmd_word;
    packform.dst = p_req_pack->src;
    packform.d_dst = p_req_pack->d_src;
    packform.src = p_req_pack->dst;
    packform.d_src = p_req_pack->d_dst;
    packform.is_ack = is_ack;
    packform.len = len;
    packform.p_data = p_data;
    comm_send_data(&packform, my_printf);
}

void RAMFUNC scope_run(scope_t *scope)
{
    float *buffer;
    float **var_ptrs;
    float *buf_base;
    uint32_t buf_size;
    uint32_t var_count;
    uint32_t write_idx;
    uint32_t i;

    if ((scope == NULL) ||
        (scope->buffer == NULL) ||
        (scope->var_ptrs == NULL) ||
        (scope->buffer_size == 0u) ||
        (scope->var_count == 0u))
    {
        return;
    }

    buffer = scope->buffer;
    buf_size = scope->buffer_size;
    var_count = scope->var_count;

    if (scope->state == SCOPE_STATE_IDLE)
    {
        if (scope->is_running)
        {
            scope->state = SCOPE_STATE_RUNNING;
            scope->write_index = 0u;
            scope->trigger_counter = 0u;
        }
        else
        {
            return;
        }
    }

    write_idx = scope->write_index;
    var_ptrs = scope->var_ptrs;
    buf_base = buffer + write_idx;

    for (i = 0u; i < var_count; ++i)
    {
        if (var_ptrs[i] != NULL)
        {
            buf_base[i * buf_size] = *(var_ptrs[i]);
        }
    }

    if ((scope->state == SCOPE_STATE_RUNNING) && scope->is_triggered)
    {
        scope->trigger_index = write_idx;
        scope->is_triggered = 0u;
        scope->in_trigger = 1u;
        scope->state = SCOPE_STATE_TRIGGERED;
    }
    else if (scope->state == SCOPE_STATE_TRIGGERED)
    {
        if (++scope->trigger_counter >= scope->trigger_post_cnt)
        {
            scope->trigger_counter = 0u;
            scope->is_running = 0u;
            scope->in_trigger = 0u;
            scope->state = SCOPE_STATE_IDLE;
        }
    }

    write_idx++;
    if (write_idx >= buf_size)
    {
        write_idx = 0u;
    }
    scope->write_index = write_idx;
}

void scope_start(scope_t *scope)
{
    if ((scope != NULL) && (scope->state == SCOPE_STATE_IDLE))
    {
        scope->is_running = 1u;
        scope->write_index = 0u;
        scope->trigger_counter = 0u;
        scope->in_trigger = 0u;
        scope->is_triggered = 0u;
    }
}

void scope_stop(scope_t *scope)
{
    if (scope != NULL)
    {
        scope->is_running = 0u;
        scope->state = SCOPE_STATE_IDLE;
        scope->in_trigger = 0u;
    }
}

void RAMFUNC scope_trigger(scope_t *scope)
{
    if ((scope != NULL) && (scope->state == SCOPE_STATE_RUNNING))
    {
        scope->is_triggered = 1u;
    }
}

void scope_reset(scope_t *scope)
{
    if (scope != NULL)
    {
        scope->write_index = 0u;
        scope->trigger_counter = 0u;
        scope->is_triggered = 0u;
        scope->is_running = 0u;
        scope->in_trigger = 0u;
        scope->state = SCOPE_STATE_IDLE;
    }
}

void scope_service_register(scope_service_obj_t *p_obj)
{
    scope_service_obj_t *p_tail;

    if ((p_obj == NULL) || (p_obj->p_scope == NULL) || (p_obj->p_name == NULL))
    {
        return;
    }

    p_obj->scope_id = g_scope_service_count;
    p_obj->capture_tag = 0u;
    p_obj->data_ready = 0u;
    p_obj->last_state = p_obj->p_scope->state;
    p_obj->p_next = NULL;

    if (g_scope_service_first == NULL)
    {
        g_scope_service_first = p_obj;
    }
    else
    {
        p_tail = g_scope_service_first;
        while (p_tail->p_next != NULL)
        {
            p_tail = p_tail->p_next;
        }
        p_tail->p_next = p_obj;
    }

    g_scope_service_count++;
}

scope_service_obj_t *scope_service_first(void)
{
    return g_scope_service_first;
}

uint8_t scope_service_count(void)
{
    return g_scope_service_count;
}

static void scope_service_list_query_act(section_packform_t *p_pack, DEC_MY_PRINTF)
{
    scope_service_obj_t *p_obj = g_scope_service_first;

    if ((p_pack == NULL) || (p_pack->is_ack == 1u))
    {
        return;
    }

    while (p_obj != NULL)
    {
        uint8_t tx_buffer[sizeof(scope_list_item_t) + SCOPE_SERVICE_NAME_SIZE_MAX] = {0};
        scope_list_item_t item = {0};
        uint8_t name_len = (uint8_t)strlen(p_obj->p_name);

        if (name_len > SCOPE_SERVICE_NAME_SIZE_MAX)
        {
            name_len = SCOPE_SERVICE_NAME_SIZE_MAX;
        }

        item.scope_id = p_obj->scope_id;
        item.is_last = (p_obj->p_next == NULL) ? 1u : 0u;
        item.name_len = name_len;
        memcpy((uint8_t *)tx_buffer, (uint8_t *)&item, sizeof(item));
        memcpy((uint8_t *)&tx_buffer[sizeof(item)], (const uint8_t *)p_obj->p_name, name_len);
        scope_service_reply(p_pack,
                            my_printf,
                            CMD_WORD_SCOPE_LIST_QUERY,
                            0u,
                            (uint8_t *)tx_buffer,
                            (uint16_t)(sizeof(item) + name_len));
        p_obj = p_obj->p_next;
    }

    if (g_scope_service_first == NULL)
    {
        scope_list_item_t item = {0};

        item.scope_id = 0xFFu;
        item.is_last = 1u;
        item.name_len = 0u;
        scope_service_reply(p_pack,
                            my_printf,
                            CMD_WORD_SCOPE_LIST_QUERY,
                            0u,
                            (uint8_t *)&item,
                            sizeof(item));
    }
}

REG_COMM(CMD_SET_SCOPE, CMD_WORD_SCOPE_LIST_QUERY, scope_service_list_query_act)

static void scope_service_info_query_act(section_packform_t *p_pack, DEC_MY_PRINTF)
{
    scope_info_ack_t ack = {0};
    scope_info_query_t query = {0};
    scope_service_obj_t *p_obj = NULL;

    if ((p_pack == NULL) || (p_pack->is_ack == 1u))
    {
        return;
    }

    if (p_pack->len == sizeof(query))
    {
        memcpy((uint8_t *)&query, p_pack->p_data, sizeof(query));
        p_obj = scope_service_find_by_id(query.scope_id);
    }

    if (p_obj == NULL)
    {
        ack.status = SCOPE_TOOL_STATUS_SCOPE_ID_INVALID;
    }
    else
    {
        ack.scope_id = p_obj->scope_id;
        ack.status = SCOPE_TOOL_STATUS_OK;
        ack.state = (uint8_t)p_obj->p_scope->state;
        ack.data_ready = p_obj->data_ready;
        ack.var_count = p_obj->p_scope->var_count;
        ack.sample_count = p_obj->p_scope->buffer_size;
        ack.write_index = p_obj->p_scope->write_index;
        ack.trigger_index = p_obj->p_scope->trigger_index;
        ack.trigger_post_cnt = p_obj->p_scope->trigger_post_cnt;
        ack.trigger_display_index = scope_service_get_trigger_display_index(p_obj->p_scope);
        ack.sample_period_us = p_obj->sample_period_us;
        ack.capture_tag = p_obj->capture_tag;
    }

    scope_service_reply(p_pack,
                        my_printf,
                        CMD_WORD_SCOPE_INFO_QUERY,
                        1u,
                        (uint8_t *)&ack,
                        sizeof(ack));
}

REG_COMM(CMD_SET_SCOPE, CMD_WORD_SCOPE_INFO_QUERY, scope_service_info_query_act)

static void scope_service_var_query_act(section_packform_t *p_pack, DEC_MY_PRINTF)
{
    scope_var_ack_t ack = {0};
    scope_var_query_t query = {0};
    scope_service_obj_t *p_obj = NULL;
    uint8_t tx_buffer[sizeof(scope_var_ack_t) + SCOPE_SERVICE_NAME_SIZE_MAX] = {0};

    if ((p_pack == NULL) || (p_pack->is_ack == 1u))
    {
        return;
    }

    if (p_pack->len == sizeof(query))
    {
        memcpy((uint8_t *)&query, p_pack->p_data, sizeof(query));
        p_obj = scope_service_find_by_id(query.scope_id);
    }

    if (p_obj == NULL)
    {
        ack.status = SCOPE_TOOL_STATUS_SCOPE_ID_INVALID;
        memcpy((uint8_t *)tx_buffer, (uint8_t *)&ack, sizeof(ack));
        scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_VAR_QUERY, 1u, (uint8_t *)tx_buffer, sizeof(ack));
        return;
    }

    ack.scope_id = p_obj->scope_id;
    ack.var_index = query.var_index;
    if (query.var_index >= p_obj->p_scope->var_count)
    {
        ack.status = SCOPE_TOOL_STATUS_VAR_INDEX_INVALID;
        memcpy((uint8_t *)tx_buffer, (uint8_t *)&ack, sizeof(ack));
        scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_VAR_QUERY, 1u, (uint8_t *)tx_buffer, sizeof(ack));
        return;
    }

    ack.status = SCOPE_TOOL_STATUS_OK;
    ack.is_last = (query.var_index >= (uint8_t)(p_obj->p_scope->var_count - 1u)) ? 1u : 0u;
    ack.name_len = (uint8_t)strlen(p_obj->p_scope->var_names[query.var_index]);
    if (ack.name_len > SCOPE_SERVICE_NAME_SIZE_MAX)
    {
        ack.name_len = SCOPE_SERVICE_NAME_SIZE_MAX;
    }
    memcpy((uint8_t *)tx_buffer, (uint8_t *)&ack, sizeof(ack));
    memcpy((uint8_t *)&tx_buffer[sizeof(ack)],
           (const uint8_t *)p_obj->p_scope->var_names[query.var_index],
           ack.name_len);
    scope_service_reply(p_pack,
                        my_printf,
                        CMD_WORD_SCOPE_VAR_QUERY,
                        1u,
                        (uint8_t *)tx_buffer,
                        (uint16_t)(sizeof(ack) + ack.name_len));
}

REG_COMM(CMD_SET_SCOPE, CMD_WORD_SCOPE_VAR_QUERY, scope_service_var_query_act)

static void scope_service_start_act(section_packform_t *p_pack, DEC_MY_PRINTF)
{
    scope_ctrl_ack_t ack = {0};
    scope_info_query_t query = {0};
    scope_service_obj_t *p_obj = NULL;
    scope_tool_status_e status = SCOPE_TOOL_STATUS_SCOPE_ID_INVALID;

    if ((p_pack == NULL) || (p_pack->is_ack == 1u))
    {
        return;
    }

    if (p_pack->len == sizeof(query))
    {
        memcpy((uint8_t *)&query, p_pack->p_data, sizeof(query));
        p_obj = scope_service_find_by_id(query.scope_id);
    }

    if (p_obj != NULL)
    {
        if (p_obj->p_scope->state == SCOPE_STATE_IDLE)
        {
            scope_start(p_obj->p_scope);
            p_obj->data_ready = 0u;
            p_obj->capture_tag++;
            p_obj->last_state = p_obj->p_scope->state;
            status = SCOPE_TOOL_STATUS_OK;
        }
        else
        {
            status = SCOPE_TOOL_STATUS_BUSY;
        }
    }

    scope_service_fill_ctrl_ack(p_obj, status, &ack);
    scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_START, 1u, (uint8_t *)&ack, sizeof(ack));
}

REG_COMM(CMD_SET_SCOPE, CMD_WORD_SCOPE_START, scope_service_start_act)

static void scope_service_trigger_act(section_packform_t *p_pack, DEC_MY_PRINTF)
{
    scope_ctrl_ack_t ack = {0};
    scope_info_query_t query = {0};
    scope_service_obj_t *p_obj = NULL;
    scope_tool_status_e status = SCOPE_TOOL_STATUS_SCOPE_ID_INVALID;

    if ((p_pack == NULL) || (p_pack->is_ack == 1u))
    {
        return;
    }

    if (p_pack->len == sizeof(query))
    {
        memcpy((uint8_t *)&query, p_pack->p_data, sizeof(query));
        p_obj = scope_service_find_by_id(query.scope_id);
    }

    if (p_obj != NULL)
    {
        if (p_obj->p_scope->state == SCOPE_STATE_RUNNING)
        {
            scope_trigger(p_obj->p_scope);
            p_obj->last_state = p_obj->p_scope->state;
            status = SCOPE_TOOL_STATUS_OK;
        }
        else
        {
            status = SCOPE_TOOL_STATUS_BUSY;
        }
    }

    scope_service_fill_ctrl_ack(p_obj, status, &ack);
    scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_TRIGGER, 1u, (uint8_t *)&ack, sizeof(ack));
}

REG_COMM(CMD_SET_SCOPE, CMD_WORD_SCOPE_TRIGGER, scope_service_trigger_act)

static void scope_service_stop_act(section_packform_t *p_pack, DEC_MY_PRINTF)
{
    scope_ctrl_ack_t ack = {0};
    scope_info_query_t query = {0};
    scope_service_obj_t *p_obj = NULL;
    scope_tool_status_e status = SCOPE_TOOL_STATUS_SCOPE_ID_INVALID;

    if ((p_pack == NULL) || (p_pack->is_ack == 1u))
    {
        return;
    }

    if (p_pack->len == sizeof(query))
    {
        memcpy((uint8_t *)&query, p_pack->p_data, sizeof(query));
        p_obj = scope_service_find_by_id(query.scope_id);
    }

    if (p_obj != NULL)
    {
        scope_stop(p_obj->p_scope);
        p_obj->data_ready = 1u;
        p_obj->last_state = p_obj->p_scope->state;
        status = SCOPE_TOOL_STATUS_OK;
    }

    scope_service_fill_ctrl_ack(p_obj, status, &ack);
    scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_STOP, 1u, (uint8_t *)&ack, sizeof(ack));
}

REG_COMM(CMD_SET_SCOPE, CMD_WORD_SCOPE_STOP, scope_service_stop_act)

static void scope_service_reset_act(section_packform_t *p_pack, DEC_MY_PRINTF)
{
    scope_ctrl_ack_t ack = {0};
    scope_info_query_t query = {0};
    scope_service_obj_t *p_obj = NULL;
    scope_tool_status_e status = SCOPE_TOOL_STATUS_SCOPE_ID_INVALID;

    if ((p_pack == NULL) || (p_pack->is_ack == 1u))
    {
        return;
    }

    if (p_pack->len == sizeof(query))
    {
        memcpy((uint8_t *)&query, p_pack->p_data, sizeof(query));
        p_obj = scope_service_find_by_id(query.scope_id);
    }

    if (p_obj != NULL)
    {
        scope_reset(p_obj->p_scope);
        p_obj->data_ready = 0u;
        p_obj->last_state = p_obj->p_scope->state;
        status = SCOPE_TOOL_STATUS_OK;
    }

    scope_service_fill_ctrl_ack(p_obj, status, &ack);
    scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_RESET, 1u, (uint8_t *)&ack, sizeof(ack));
}

REG_COMM(CMD_SET_SCOPE, CMD_WORD_SCOPE_RESET, scope_service_reset_act)

static void scope_service_sample_query_act(section_packform_t *p_pack, DEC_MY_PRINTF)
{
    scope_sample_query_t query = {0};
    scope_sample_ack_t ack = {0};
    scope_service_obj_t *p_obj = NULL;
    uint8_t tx_buffer[sizeof(scope_sample_ack_t) + SCOPE_SERVICE_VAR_COUNT_MAX * sizeof(float)] = {0};

    if ((p_pack == NULL) || (p_pack->is_ack == 1u))
    {
        return;
    }

    if (p_pack->len == sizeof(query))
    {
        memcpy((uint8_t *)&query, p_pack->p_data, sizeof(query));
        p_obj = scope_service_find_by_id(query.scope_id);
    }

    if (p_obj == NULL)
    {
        ack.status = SCOPE_TOOL_STATUS_SCOPE_ID_INVALID;
        memcpy((uint8_t *)tx_buffer, (uint8_t *)&ack, sizeof(ack));
        scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_SAMPLE_QUERY, 1u, (uint8_t *)tx_buffer, sizeof(ack));
        return;
    }

    ack.scope_id = p_obj->scope_id;
    ack.read_mode = query.read_mode;
    ack.var_count = p_obj->p_scope->var_count;
    ack.sample_index = query.sample_index;
    ack.capture_tag = p_obj->capture_tag;
    if (query.sample_index >= p_obj->p_scope->buffer_size)
    {
        ack.status = SCOPE_TOOL_STATUS_SAMPLE_INDEX_INVALID;
        memcpy((uint8_t *)tx_buffer, (uint8_t *)&ack, sizeof(ack));
        scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_SAMPLE_QUERY, 1u, (uint8_t *)tx_buffer, sizeof(ack));
        return;
    }

    if ((query.read_mode == SCOPE_READ_MODE_NORMAL) && (p_obj->p_scope->state != SCOPE_STATE_IDLE))
    {
        ack.status = SCOPE_TOOL_STATUS_RUNNING_DENIED;
        memcpy((uint8_t *)tx_buffer, (uint8_t *)&ack, sizeof(ack));
        scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_SAMPLE_QUERY, 1u, (uint8_t *)tx_buffer, sizeof(ack));
        return;
    }

    if ((query.read_mode == SCOPE_READ_MODE_NORMAL) && (p_obj->data_ready == 0u))
    {
        ack.status = SCOPE_TOOL_STATUS_DATA_NOT_READY;
        memcpy((uint8_t *)tx_buffer, (uint8_t *)&ack, sizeof(ack));
        scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_SAMPLE_QUERY, 1u, (uint8_t *)tx_buffer, sizeof(ack));
        return;
    }

    if ((query.expected_capture_tag != 0u) && (query.expected_capture_tag != p_obj->capture_tag))
    {
        ack.status = SCOPE_TOOL_STATUS_CAPTURE_CHANGED;
        memcpy((uint8_t *)tx_buffer, (uint8_t *)&ack, sizeof(ack));
        scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_SAMPLE_QUERY, 1u, (uint8_t *)tx_buffer, sizeof(ack));
        return;
    }

    if (p_obj->p_scope->var_count > SCOPE_SERVICE_VAR_COUNT_MAX)
    {
        ack.status = SCOPE_TOOL_STATUS_BUSY;
        memcpy((uint8_t *)tx_buffer, (uint8_t *)&ack, sizeof(ack));
        scope_service_reply(p_pack, my_printf, CMD_WORD_SCOPE_SAMPLE_QUERY, 1u, (uint8_t *)tx_buffer, sizeof(ack));
        return;
    }

    ack.status = SCOPE_TOOL_STATUS_OK;
    ack.is_last_sample = (query.sample_index >= (p_obj->p_scope->buffer_size - 1u)) ? 1u : 0u;
    memcpy((uint8_t *)tx_buffer, (uint8_t *)&ack, sizeof(ack));
    if (p_obj->p_scope->var_count > 0u)
    {
        uint32_t physical_index = scope_service_logical_to_physical_index(p_obj->p_scope,
                                                                          query.read_mode,
                                                                          query.sample_index);
        for (uint8_t var_index = 0u; var_index < p_obj->p_scope->var_count; var_index++)
        {
            float value = p_obj->p_scope->buffer[var_index * p_obj->p_scope->buffer_size + physical_index];
            memcpy((uint8_t *)&tx_buffer[sizeof(ack) + var_index * sizeof(float)],
                   (uint8_t *)&value,
                   sizeof(float));
        }
    }

    scope_service_reply(p_pack,
                        my_printf,
                        CMD_WORD_SCOPE_SAMPLE_QUERY,
                        1u,
                        (uint8_t *)tx_buffer,
                        (uint16_t)(sizeof(ack) + p_obj->p_scope->var_count * sizeof(float)));
}

REG_COMM(CMD_SET_SCOPE, CMD_WORD_SCOPE_SAMPLE_QUERY, scope_service_sample_query_act)

static void scope_service_state_task(void)
{
    scope_service_obj_t *p_obj = g_scope_service_first;

    while (p_obj != NULL)
    {
        if ((p_obj->last_state != SCOPE_STATE_IDLE) &&
            (p_obj->p_scope->state == SCOPE_STATE_IDLE))
        {
            p_obj->data_ready = 1u;
        }
        else if (p_obj->p_scope->state != SCOPE_STATE_IDLE)
        {
            p_obj->data_ready = 0u;
        }
        p_obj->last_state = p_obj->p_scope->state;
        p_obj = p_obj->p_next;
    }
}

REG_TASK_MS(1, scope_service_state_task)

#ifndef SCOPE_TEST_ENABLE
#define SCOPE_TEST_ENABLE 0
#endif

#if (SCOPE_TEST_ENABLE == 1)
REG_SCOPE(test, 128, 32, scope_·test_ramp, scope_test_square, scope_test_saw)

static void scope_test_task(void)
{
    static uint32_t tick = 0u;

    scope_test_ramp = (float)(tick & 0xFFu) * 0.01f;
    scope_test_square = (((tick / 20u) & 0x01u) != 0u) ? 1.0f : -1.0f;
    scope_test_saw = (float)(tick % 100u) - 50.0f;

    if ((scope_test.state == SCOPE_STATE_IDLE) && (scope_test.is_running == 0u))
    {
        scope_start(&scope_test);
    }

    if ((scope_test.state == SCOPE_STATE_RUNNING) && ((tick % 80u) == 40u))
    {
        scope_trigger(&scope_test);
    }

    scope_run(&scope_test);
    tick++;
}

REG_TASK_MS(1, scope_test_task)
#endif

#else

void scope_run(scope_t *scope)
{
    (void)scope;
}

void scope_start(scope_t *scope)
{
    (void)scope;
}

void scope_stop(scope_t *scope)
{
    (void)scope;
}

void scope_trigger(scope_t *scope)
{
    (void)scope;
}

void scope_reset(scope_t *scope)
{
    (void)scope;
}

void scope_service_register(scope_service_obj_t *p_obj)
{
    (void)p_obj;
}

scope_service_obj_t *scope_service_first(void)
{
    return NULL;
}

uint8_t scope_service_count(void)
{
    return 0u;
}

#endif
