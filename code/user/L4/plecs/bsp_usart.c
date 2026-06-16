#include "bsp_usart.h"
#include "stdio.h"
#include "stdint.h"

volatile uint8_t usart2_rx_buffer[USART2_RX_BUFFER_SIZE];
volatile uint8_t usart2_tx_buffer[USART2_TX_BUFFER_SIZE];
