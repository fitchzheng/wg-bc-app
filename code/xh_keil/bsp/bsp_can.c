#include "bsp_can.h"



void McanCommClockConfig(void);
void McanInitConfig(void);
void McanIrqConfig(void);
void McanPinConfig(void);
void McanPhyEnable(void);
void McanSampleTx(void);
void McanLoadTxMsg(stc_mcan_tx_msg_t *pstcTxMsg, stc_mcan_rx_msg_t *pstcRxMsg);



/**
 * @brief  Specifies communication clock.
 * @param  None
 * @retval None
 */
void McanCommClockConfig(void)
{
    CLK_SetCANClockSrc(MCAN_CLK_UNIT, MCAN_CLK_SRC);
}

/**
 * @brief  MCAN initial configuration.
 * @param  None
 * @retval None
 */
void McanInitConfig(void)
{
    stc_mcan_init_t stcMcanInit;
    stc_mcan_filter_t stcStdFilterList[MCAN_STD_FILTER_NUM] = {
        MCAN_STD_FILTER0, //MCAN_STD_FILTER1, MCAN_STD_FILTER2, MCAN_STD_FILTER3
    };

    stc_mcan_filter_t stcExtFilterList[MCAN_EXT_FILTER_NUM] = {
        MCAN_EXT_FILTER0, //MCAN_EXT_FILTER1, MCAN_EXT_FILTER2, MCAN_EXT_FILTER3
    };

    (void)MCAN_StructInit(&stcMcanInit);
    stcMcanInit.u32Mode = MCAN_MD_NORMAL;
    stcMcanInit.u32FrameFormat = MCAN_FRAME_CLASSIC;
    /* Classic CAN. Baudrate 1Mbps, sample point 80% */
    stcMcanInit.stcBitTime.u32NominalPrescaler     = 8U;  //250K
    stcMcanInit.stcBitTime.u32NominalTimeSeg1      = 16U;
    stcMcanInit.stcBitTime.u32NominalTimeSeg2      = 4U;
    stcMcanInit.stcBitTime.u32NominalSyncJumpWidth = 4U;
    /* Message RAM */
    stcMcanInit.stcMsgRam.u32AddrOffset        = 0U;
    stcMcanInit.stcMsgRam.u32StdFilterNum      = MCAN_STD_FILTER_NUM;
    stcMcanInit.stcMsgRam.u32ExtFilterNum      = MCAN_EXT_FILTER_NUM;
    stcMcanInit.stcMsgRam.u32RxFifo0Num        = MCAN_RX_FIFO0_NUM;
    stcMcanInit.stcMsgRam.u32RxFifo0DataSize   = MCAN_RX_FIFO0_DATA_FIELD_SIZE;
    stcMcanInit.stcMsgRam.u32RxFifo1Num        = MCAN_RX_FIFO1_NUM;
    stcMcanInit.stcMsgRam.u32RxFifo1DataSize   = MCAN_RX_FIFO1_DATA_FIELD_SIZE;
    stcMcanInit.stcMsgRam.u32RxBufferNum       = MCAN_RX_BUF_NUM;
    stcMcanInit.stcMsgRam.u32RxBufferDataSize  = MCAN_RX_BUF_DATA_FIELD_SIZE;
    stcMcanInit.stcMsgRam.u32TxBufferNum       = MCAN_TX_BUF_NUM;
    stcMcanInit.stcMsgRam.u32TxFifoQueueNum    = MCAN_TX_FIFO_NUM;
    stcMcanInit.stcMsgRam.u32TxFifoQueueMode   = MCAN_TX_FIFO_MD;
    stcMcanInit.stcMsgRam.u32TxDataSize        = MCAN_TX_BUF_DATA_FIELD_SIZE;
    stcMcanInit.stcMsgRam.u32TxEventNum        = MCAN_TX_EVT_NUM;
    /* Acceptance filter */
    stcMcanInit.stcFilter.pstcStdFilterList     = stcStdFilterList;
    stcMcanInit.stcFilter.pstcExtFilterList     = stcExtFilterList;
    stcMcanInit.stcFilter.u32StdFilterConfigNum = stcMcanInit.stcMsgRam.u32StdFilterNum;
    stcMcanInit.stcFilter.u32ExtFilterConfigNum = stcMcanInit.stcMsgRam.u32ExtFilterNum;

    FCG_Fcg1PeriphClockCmd(MCAN_PERIPH_CLK, ENABLE);
    (void)MCAN_Init(MCAN_UNIT, &stcMcanInit);

    /* Watermark if needed */
    MCAN_SetFifoWatermark(MCAN_UNIT, MCAN_WATERMARK_RX_FIFO0, MCAN_RX_FIFO0_WATERMARK);
    MCAN_SetFifoWatermark(MCAN_UNIT, MCAN_WATERMARK_RX_FIFO1, MCAN_RX_FIFO1_WATERMARK);

    /* Configure Rx FIFO0 operation mode if needed.
       If new message received when Rx FIFO0 is full, the new message
       will not be stored and Message Lost Interrupt will be generated. */
    MCAN_RxFifoOperationModeConfig(MCAN_UNIT, MCAN_RX_FIFO0, MCAN_RX_FIFO_BLOCKING);
    /* Configure Rx FIFO1 operation mode if needed.
       If new message received when Rx FIFO1 is full, the new received message
       will overwrite the oldest received message and RF1N Interrupt generated. */
    MCAN_RxFifoOperationModeConfig(MCAN_UNIT, MCAN_RX_FIFO1, MCAN_RX_FIFO_OVERWRITE);  //FIFO1使用覆盖模式

    /* Configures timestamp if needed */
    MCAN_TimestampCounterConfig(MCAN_UNIT, 1U);
    MCAN_TimestampCounterCmd(MCAN_UNIT, ENABLE);

    /* The Tx buffer can cause transmission completed completed interrupt
       only when its own transmission completed interrupt is enabled. */
    MCAN_TxBufferNotificationCmd(MCAN_UNIT, MCAN_TX_NOTIFICATION_BUF, MCAN_INT_TX_CPLT, ENABLE);

//  不开启中断，使用轮询方式
//    MCAN_IntCmd(MCAN_UNIT, MCAN_INT0_SEL, MCAN_INT_LINE0, ENABLE);  
//    MCAN_IntCmd(MCAN_UNIT, MCAN_INT1_SEL, MCAN_INT_LINE1, ENABLE);
}

/**
 * @brief  CAN interrupt configuration.
 * @param  None
 * @retval None
 */
void McanIrqConfig(void)
{
    // NVIC_ClearPendingIRQ(MCAN_INT0_IRQn);
    // NVIC_SetPriority(MCAN_INT0_IRQn, MCAN_INT0_PRIO);
    // NVIC_EnableIRQ(MCAN_INT0_IRQn);

    // NVIC_ClearPendingIRQ(MCAN_INT1_IRQn);
    // NVIC_SetPriority(MCAN_INT1_IRQn, MCAN_INT0_PRIO);
    // NVIC_EnableIRQ(MCAN_INT1_IRQn);
}

/**
 * @brief  Specifies pin function for TXD and RXD.
 * @param  None
 * @retval None
 */
void McanPinConfig(void)
{
    GPIO_SetFunc(MCAN_TX_PORT, MCAN_TX_PIN, MCAN_TX_PIN_FUNC);
    GPIO_SetFunc(MCAN_RX_PORT, MCAN_RX_PIN, MCAN_RX_PIN_FUNC);
}

/**
 * @brief  Set CAN PHY STB pin as low.
 * @param  None
 * @retval None
 */
void McanPhyEnable(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinState = PIN_STAT_RST;
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    GPIO_Init(MCAN_PHY_STBY_PORT, MCAN_PHY_STBY_PIN, &stcGpioInit);

    /* Set PYH STB pin as low. */
    GPIO_ResetPins(MCAN_PHY_STBY_PORT, MCAN_PHY_STBY_PIN);
    GPIO_OutputCmd(MCAN_PHY_STBY_PORT, MCAN_PHY_STBY_PIN, ENABLE);
}

/**
 * @brief  MCAN transmit.
 * @param  None
 * @retval None
 */
void McanSampleTx(void)
{
    stc_mcan_tx_msg_t stcTxMsg = {
        .ID = 0x777UL,
        .IDE = 0U,
        .DLC = MCAN_DLC8,
        .u32TxBuffer = MCAN_TX_BUF0,
        .au8Data = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U},
    };
    (void)MCAN_AddMsgToTxBuffer(MCAN_UNIT, &stcTxMsg);
    MCAN_EnableTxBufferRequest(MCAN_UNIT, stcTxMsg.u32TxBuffer);

//    stcTxMsg.ID = 0x17777777UL;
//    stcTxMsg.IDE = 1U;
//    (void)MCAN_AddMsgToTxFifoQueue(MCAN_UNIT, &stcTxMsg);
}



//检测是否接收到新数据
uint8_t bsp_can_rev_check(void)
{
    if (MCAN_GetStatus(MCAN_UNIT, MCAN_FLAG_RX_FIFO1_NEW_MSG) == SET) 
    {
        MCAN_ClearStatus(MCAN_UNIT, MCAN_FLAG_RX_FIFO1_NEW_MSG);
        return 1;
    }
    return 0;
}
//获取接收数据
uint8_t bsp_can_get_msg(stc_mcan_rx_msg_t* stcRxMsg)
{
    return MCAN_GetRxMsg(MCAN_UNIT, MCAN_RX_FIFO1,stcRxMsg);

}
//CAN接收数据处理，用于测试，将接收到的数据修改一下再发送回去
//stcRxMsg  接收数据指针
//stcTxMsg  发送数据指针
void can_data_process(stc_mcan_tx_msg_t *stcTxMsg,stc_mcan_rx_msg_t* stcRxMsg)
{
    stcTxMsg->ID = stcRxMsg->ID + 1; //ID+1
    stcTxMsg->IDE = stcRxMsg->IDE;   //帧格式不变
    stcTxMsg->DLC = stcRxMsg->DLC;   //数据长度不变
    stcTxMsg->RTR = stcRxMsg->RTR;
    stcTxMsg->au8Data[0] = stcRxMsg->au8Data[0] + 1;  //前两个数据+1
    stcTxMsg->au8Data[1] = stcRxMsg->au8Data[1] + 1;
    MCAN_AddMsgToTxFifoQueue(MCAN_UNIT, stcTxMsg);
}


//CAN总线测试
void can_test(void)
{
    stc_mcan_tx_msg_t stcTxMsg={0}; //发送数据结构体
    stc_mcan_rx_msg_t stcRxMsg={0}; //接收数据结构体
    if(bsp_can_rev_check()) //是否有收到新数据
    {
        if(bsp_can_get_msg(&stcRxMsg) == LL_OK) //获取数据
        {
            can_data_process(&stcTxMsg,&stcRxMsg); //处理数据
        }

    }    

}

/**
 * @brief  Load Tx message from received message
 * @param  [in]  pstcTxMsg              Pointer to the message to be transmitted.
 * @param  [in]  pstcRxMsg              Pointer to the received message.
 * @retval None
 */
void McanLoadTxMsg(stc_mcan_tx_msg_t *pstcTxMsg, stc_mcan_rx_msg_t *pstcRxMsg)
{
    static uint8_t u8TxMarker = 0U;
    *pstcTxMsg = *((stc_mcan_tx_msg_t *)pstcRxMsg);
    pstcTxMsg->au8Data[0U] = u8TxMarker;
    pstcTxMsg->ID &= 0xFFFFF0FFUL;
    pstcTxMsg->ID |= 0x300UL;
    pstcTxMsg->u32StoreTxEvent = 1U;
    pstcTxMsg->u32MsgMarker = u8TxMarker++;
}



void bsp_can_init(void)
{
    McanCommClockConfig();
    McanInitConfig();
//    McanIrqConfig();
    McanPinConfig();
    McanPhyEnable();
    MCAN_Start(MCAN_UNIT);
}

void bsp_can_tx(uint32_t id, uint8_t *p_data)
{
    stc_mcan_tx_msg_t stcTxMsg = {
        .ID = id,
        .IDE = 0U,//1U,
        .DLC = MCAN_DLC8,
        .u32TxBuffer = MCAN_TX_BUF0,
    };

    memcpy(&stcTxMsg.au8Data[0], p_data, 8);

    (void)MCAN_AddMsgToTxFifoQueue(MCAN_UNIT, &stcTxMsg);
}

uint8_t bsp_rvc_can_tx(uint32_t id, const uint8_t *p_data, uint8_t len)
{
    stc_mcan_tx_msg_t stcTxMsg = {
        .ID = id,
        .IDE = 1U,
        .DLC = MCAN_DLC8,
        .u32TxBuffer = MCAN_TX_BUF0,
    };

    memcpy(&stcTxMsg.au8Data[0], p_data, len);

    if(MCAN_AddMsgToTxFifoQueue(MCAN_UNIT, &stcTxMsg) == LL_OK)
    {
        return 1;
    }

    return 0;
}

int bsp_can_rx(uint32_t *p_raw, uint8_t *p_data)
{
    stc_mcan_rx_msg_t stcRxMsg = {0};
    uint8_t scan_count = 0;

    while (scan_count < 16U)
    {
        scan_count++;
        if (bsp_can_get_msg(&stcRxMsg) != LL_OK)
        {
            *p_raw = 0;
            return -1;
        }

        if ((stcRxMsg.IDE == 1U) && (stcRxMsg.DLC == MCAN_DLC8))
        {
            *p_raw = stcRxMsg.ID;
            memcpy(p_data, &stcRxMsg.au8Data[0], 8);
            return 0;
        }
    }

    *p_raw = 0;
    memset(p_data, 0, 8);
    return -1;
}

void bsp_can_deinit(void)
{
//    MCAN_TimestampCounterCmd(MCAN_UNIT, DISABLE);
//    MCAN_Stop(MCAN_UNIT);

    FCG_Fcg1PeriphClockCmd(MCAN_PERIPH_CLK, DISABLE);
}
