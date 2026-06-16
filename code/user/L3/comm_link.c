#include "comm_link.h"

#include "bsp_usart.h"
#include "comm.h"
#include "comm_addr.h"
#include "section.h"
#include "shell.h"

#ifndef USART_DBG_COMM_PAYLOAD_SIZE
#define USART_DBG_COMM_PAYLOAD_SIZE 128U
#endif

static section_link_tx_func_t s_usart_dbg_tx_func = {
    .my_printf = bsp_usart_dbg_printf,
    .tx_by_dma = bsp_usart_dbg_tx,
};

DECLARE_COMM_CTX(s_usart_dbg_comm_ctx, USART_DBG_COMM_PAYLOAD_SIZE, LOCAL_ADDR, USART_DBG_LINK)
DECLARE_SHELL_CTX(s_usart_dbg_shell_ctx);

static const section_link_handler_item_t s_usart_dbg_handler_arr[] = {
    {.func = comm_run, .ctx = (void *)&s_usart_dbg_comm_ctx},
    {.func = shell_run, .ctx = (void *)&s_usart_dbg_shell_ctx},
};

REG_LINK(USART_DBG_LINK,
         s_usart_dbg_tx_func,
         bsp_usart_dbg_rx_get_byte,
         s_usart_dbg_handler_arr,
         sizeof(s_usart_dbg_handler_arr) / sizeof(s_usart_dbg_handler_arr[0]))
