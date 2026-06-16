#ifndef __BSP_CAN_H__
#define __BSP_CAN_H__ 

#include "hc32_ll.h"
#include "hc32_ll_mcan.h"


#define MCAN_UNIT1                      (1U)

#define MCAN_UNIT_SEL                   MCAN_UNIT1

#define MCAN_PERIPH_CLK                 (FCG1_PERIPH_MCAN1)

#define MCAN_UNIT                       (CM_MCAN1)
#define MCAN_CLK_UNIT                   (CLK_MCAN1)
#define MCAN_CLK_SRC                    (CLK_MCANCLK_SYSCLK_DIV3)
/* Pin */
#define MCAN_TX_PORT                    (GPIO_PORT_B)  //注意改成PB8和PB9
#define MCAN_TX_PIN                     (GPIO_PIN_09)
#define MCAN_TX_PIN_FUNC                (GPIO_FUNC_54)
#define MCAN_RX_PORT                    (GPIO_PORT_B)
#define MCAN_RX_PIN                     (GPIO_PIN_08)
#define MCAN_RX_PIN_FUNC                (GPIO_FUNC_55)
#define MCAN_PHY_STBY_PORT              (GPIO_PORT_B)
#define MCAN_PHY_STBY_PIN               (GPIO_PIN_02)
/* IRQ */



/* Interrupt */
#define MCAN_RX_INT_SEL                 (MCAN_INT_RX_FIFO0_NEW_MSG | \
                                         MCAN_INT_RX_FIFO0_WATERMARK | \
                                         MCAN_INT_RX_FIFO0_FULL | \
                                         MCAN_INT_RX_FIFO0_MSG_LOST | \
                                         MCAN_INT_RX_FIFO1_NEW_MSG | \
                                         MCAN_INT_RX_FIFO1_WATERMARK | \
                                         MCAN_INT_RX_FIFO1_FULL | \
                                         MCAN_INT_RX_FIFO1_MSG_LOST | \
                                         MCAN_INT_RX_BUF_NEW_MSG)
#define MCAN_TX_INT_SEL                 (MCAN_INT_TX_CPLT | \
                                         MCAN_INT_TX_EVT_FIFO_NEW_DATA | \
                                         MCAN_INT_BUS_OFF)

#define MCAN_INT0_SEL                   MCAN_RX_INT_SEL
#define MCAN_INT1_SEL                   MCAN_TX_INT_SEL

/* Message RAM */
/* Each standard filter element size is 4 bytes */
#define MCAN_STD_FILTER_NUM             (1U)
/* Each extended filter element size is 8 bytes */
#define MCAN_EXT_FILTER_NUM             (1U)
/* Each Rx FIFO0 element size is 8+8 bytes */
#define MCAN_RX_FIFO0_NUM               (10U)
#define MCAN_RX_FIFO0_WATERMARK         (8U)
#define MCAN_RX_FIFO0_DATA_FIELD_SIZE   MCAN_DATA_SIZE_8BYTE
/* Each Rx FIFO1 element size is 8+8 bytes */
#define MCAN_RX_FIFO1_NUM               (10U)
#define MCAN_RX_FIFO1_WATERMARK         (7U)
#define MCAN_RX_FIFO1_DATA_FIELD_SIZE   MCAN_DATA_SIZE_8BYTE
/* Each Rx buffer element size is 8+8 bytes */
#define MCAN_RX_BUF_NUM                 (6U)
#define MCAN_RX_BUF_DATA_FIELD_SIZE     MCAN_DATA_SIZE_8BYTE
/* Each Tx buffer element size is 8+8 bytes */
#define MCAN_TX_BUF_NUM                 (12U)
#define MCAN_TX_FIFO_NUM                (6U)
#define MCAN_TX_BUF_DATA_FIELD_SIZE     MCAN_DATA_SIZE_8BYTE
#define MCAN_TX_NOTIFICATION_BUF        ((1UL << (MCAN_TX_BUF_NUM + MCAN_TX_FIFO_NUM)) - 1U)
/* Each extended filter element size is 8 bytes */
#define MCAN_TX_EVT_NUM                 (12U)

/* Filter */
#define MCAN_CFG_IGNOR                  (0U)
/* Accept standard frames with ID from 0x110 to 0x11F and store to Rx FIFO0 */
#define MCAN_STD_FILTER0                {.u32IdType = MCAN_STD_ID, .u32FilterType = MCAN_FILTER_RANGE, \
                                         .u32FilterConfig = MCAN_FILTER_TO_RX_FIFO1, .u32FilterId1 = 0, \
                                         .u32FilterId2 = 0x600UL,/*0x7FFUL,*/}
/* Accept standard frames with ID 0x130~0x132 and store to dedicated Rx buffer 0~2 */
// #define MCAN_STD_FILTER1                {.u32IdType = MCAN_STD_ID, .u32FilterType = MCAN_CFG_IGNOR, \
//                                          .u32FilterConfig = MCAN_FILTER_TO_RX_BUF, .u32FilterId1 = 0x130UL, \
//                                          .u32FilterId2 = MCAN_CFG_IGNOR, .u32RxBufferIndex = MCAN_RX_BUF0}
// #define MCAN_STD_FILTER2                {.u32IdType = MCAN_STD_ID, .u32FilterType = MCAN_CFG_IGNOR, \
//                                          .u32FilterConfig = MCAN_FILTER_TO_RX_BUF, .u32FilterId1 = 0x131UL, \
//                                          .u32FilterId2 = MCAN_CFG_IGNOR, .u32RxBufferIndex = MCAN_RX_BUF1}
// #define MCAN_STD_FILTER3                {.u32IdType = MCAN_STD_ID, .u32FilterType = MCAN_CFG_IGNOR, \
//                                          .u32FilterConfig = MCAN_FILTER_TO_RX_BUF, .u32FilterId1 = 0x132UL, \
//                                          .u32FilterId2 = MCAN_CFG_IGNOR, .u32RxBufferIndex = MCAN_RX_BUF2}

/* Accept extended frames with ID from 0x0 to 0x1FFFFFF0UL and store to Rx FIFO1 */
#define MCAN_EXT_FILTER0                {.u32IdType = MCAN_EXT_ID, .u32FilterType = MCAN_FILTER_RANGE, \
                                         .u32FilterConfig = MCAN_FILTER_TO_RX_FIFO1, .u32FilterId1 = 0, \
                                         .u32FilterId2 = 0x1FFFFFF0UL,}
/* Accept extended frames with ID 0x12345130~0x12345132 and store to dedicated Rx buffer 3~5 */
// #define MCAN_EXT_FILTER1                {.u32IdType = MCAN_EXT_ID, .u32FilterType = MCAN_CFG_IGNOR, \
//                                          .u32FilterConfig = MCAN_FILTER_TO_RX_BUF, .u32FilterId1 = 0x12345130UL, \
//                                          .u32FilterId2 = MCAN_CFG_IGNOR, .u32RxBufferIndex = MCAN_RX_BUF3}
// #define MCAN_EXT_FILTER2                {.u32IdType = MCAN_EXT_ID, .u32FilterType = MCAN_CFG_IGNOR, \
//                                          .u32FilterConfig = MCAN_FILTER_TO_RX_BUF, .u32FilterId1 = 0x12345131UL, \
//                                          .u32FilterId2 = MCAN_CFG_IGNOR, .u32RxBufferIndex = MCAN_RX_BUF4}
// #define MCAN_EXT_FILTER3                {.u32IdType = MCAN_EXT_ID, .u32FilterType = MCAN_CFG_IGNOR, \
//                                          .u32FilterConfig = MCAN_FILTER_TO_RX_BUF, .u32FilterId1 = 0x12345132UL, \
                                         .u32FilterId2 = MCAN_CFG_IGNOR, .u32RxBufferIndex = MCAN_RX_BUF5}

                                         

void bsp_can_init(void);
void McanSampleTx(void);      
void can_test(void);                                   
            
void bsp_can_tx(uint32_t id, uint8_t *p_data);
int bsp_can_rx(uint32_t *p_raw, uint8_t *p_data);
void bsp_can_deinit(void);
uint8_t bsp_rvc_can_tx(uint32_t id, const uint8_t *p_data, uint8_t len);
#endif

