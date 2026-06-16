/**
 * @file section.c
 * @brief Runtime implementation of the section-based auto-registration framework.
 */

#include "section.h"

#include <stddef.h>
#include <string.h>

#ifndef SECTION_LINK_PROCESS_MAX_BYTES
#define SECTION_LINK_PROCESS_MAX_BYTES 100U
#endif

reg_task_t *p_task_first = NULL;
reg_interrupt_t *p_interrupt_first = NULL;
section_link_t *p_link_first = NULL;
reg_init_t *p_init_first = NULL;
reg_wg_com_t *p_wg_com_first = NULL;

static void task_insert(reg_task_t *task)
{
    static reg_task_t *s_task_tail = NULL;

    if (!task)
        return;

    task->time_last = SECTION_SYS_TICK;
    task->p_next = NULL;

    if (p_task_first == NULL)
    {
        p_task_first = task;
        s_task_tail = task;
    }
    else
    {
        s_task_tail->p_next = task;
        s_task_tail = task;
    }
}

static void interrupt_insert(reg_interrupt_t *intr)
{
    if (!intr)
        return;

    intr->p_next = NULL;

    if (!p_interrupt_first || intr->priority < p_interrupt_first->priority)
    {
        intr->p_next = p_interrupt_first;
        p_interrupt_first = intr;
    }
    else
    {
        reg_interrupt_t *prev = p_interrupt_first;
        while (prev->p_next && prev->p_next->priority < intr->priority)
        {
            prev = prev->p_next;
        }
        intr->p_next = prev->p_next;
        prev->p_next = intr;
    }
}

static void link_insert(section_link_t *link)
{
    static section_link_t *s_link_tail = NULL;

    if (!link)
        return;

    link->p_next = NULL;

    if (!p_link_first)
    {
        p_link_first = link;
        s_link_tail = link;
    }
    else
    {
        s_link_tail->p_next = link;
        s_link_tail = link;
    }
}

static void init_insert(reg_init_t *init)
{
    if (!init)
        return;

    init->p_next = NULL;

    if (!p_init_first || init->priority < p_init_first->priority)
    {
        init->p_next = p_init_first;
        p_init_first = init;
    }
    else
    {
        reg_init_t *prev = p_init_first;
        while (prev->p_next && prev->p_next->priority <= init->priority)
        {
            prev = prev->p_next;
        }
        init->p_next = prev->p_next;
        prev->p_next = init;
    }
}

static void wg_com_insert(reg_wg_com_t *wg_com)
{
    static reg_wg_com_t *s_wg_com_tail = NULL;

    if (!wg_com)
        return;

    wg_com->p_next = NULL;

    if (!p_wg_com_first)
    {
        p_wg_com_first = wg_com;
        s_wg_com_tail = wg_com;
    }
    else
    {
        s_wg_com_tail->p_next = wg_com;
        s_wg_com_tail = wg_com;
    }
}

void section_init(void)
{
    for (reg_section_t *p = (reg_section_t *)&SECTION_START;
         p < (reg_section_t *)&SECTION_STOP;
         ++p)
    {
        switch (p->section_type)
        {
        case SECTION_INIT:
        case SECTION_INIT_TP:
            init_insert((reg_init_t *)p->p_str);
            break;
        case SECTION_TASK:
            task_insert((reg_task_t *)p->p_str);
            break;
        case SECTION_INTERRUPT:
            interrupt_insert((reg_interrupt_t *)p->p_str);
            break;
        case SECTION_LINK:
            link_insert((section_link_t *)p->p_str);
            break;
        case SECTION_WG_COM:
            wg_com_insert((reg_wg_com_t *)p->p_str);
            break;
        default:
            break;
        }
    }

    for (reg_init_t *init = p_init_first; init != NULL; init = init->p_next)
    {
        if (init->p_func)
            init->p_func();
    }
}

void run_task(void)
{
    const uint32_t now = SECTION_SYS_TICK;

    for (reg_task_t *task = p_task_first; task; task = task->p_next)
    {
        if (!task->p_func)
            continue;

        const uint32_t period = task->t_period;
        if (period == 0u)
            continue;

        const uint32_t elapsed = (uint32_t)(now - task->time_last);
        if (elapsed < period)
            continue;

        section_perf_record_t *rec = task->p_perf_record;
        const uint32_t *perf_cnt = g_section_perf_cnt;
        uint32_t perf_start = 0u;
        if (rec && perf_cnt)
            perf_start = *perf_cnt;

        task->p_func();

        if (rec && perf_cnt)
        {
            const uint32_t perf_end = *perf_cnt;
            const uint32_t delta = (uint32_t)(perf_end - perf_start);

            rec->time = (uint16_t)delta;
            if (delta > rec->max_time)
                rec->max_time = delta;
        }

        const uint32_t k = elapsed / period;
        task->time_last += k * period;
    }
}

void RAMFUNC section_interrupt(void)
{
    for (reg_interrupt_t *p = p_interrupt_first; p != NULL; p = p->p_next)
    {
        if (p->p_func)
            p->p_func();
    }
}

static void link_process(section_link_t *link)
{
    uint8_t data = 0u;
    uint32_t process_count = 0U;

    if (!link || !link->rx_get_byte || !link->handler_arr)
        return;

    while ((process_count < SECTION_LINK_PROCESS_MAX_BYTES) && (link->rx_get_byte(&data) != 0u))
    {
        process_count++;
        for (uint32_t i = 0; i < link->handler_num; ++i)
        {
            const section_link_handler_item_t *it = &link->handler_arr[i];
            if (it->func)
                it->func(data, link->my_printf, it->ctx);
        }
    }
}

static void section_link_task(void)
{
    for (section_link_t *p = p_link_first; p != NULL; p = p->p_next)
    {
        link_process(p);
    }
}

REG_TASK_MS(1, section_link_task)

void section_fsm_func(reg_fsm_t *fsm)
{
    if (!fsm->p_fsm_func_table || !fsm->p_fsm_ev)
        return;

    for (uint32_t i = 0; i < fsm->fsm_table_size; ++i)
    {
        reg_fsm_func_t *entry = &fsm->p_fsm_func_table[i];
        if (fsm->fsm_sta == entry->fsm_sta)
        {
            if (fsm->fsm_sta_is_change)
            {
                fsm->fsm_sta_is_change = 0;
                PLECS_LOG("%s\n", entry->p_name);
                if (entry->func_in)
                    entry->func_in();
            }

            if (entry->func_exe)
                entry->func_exe();

            if (*fsm->p_fsm_ev)
            {
                uint32_t next = entry->func_chk ? entry->func_chk(*fsm->p_fsm_ev) : 0u;
                if (next && next != entry->fsm_sta)
                {
                    PLECS_LOG("%s-chk_ev:%d\n", entry->p_name, *fsm->p_fsm_ev);
                    if (entry->func_out)
                        entry->func_out();
                    fsm->fsm_sta = next;
                    fsm->fsm_sta_is_change = 1;
                }
                *fsm->p_fsm_ev = 0;
            }
            break;
        }
    }
}
