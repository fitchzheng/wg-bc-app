#include "bsp_stop.h"


/**
 * @brief  Whether ready to entry stop mode.
 * @param  None
 * @retval int32_t:
 * @note Ensure DMA stops transmit and no flash erase/program operation.
 */
int32_t STOP_IsReady(void)
{
    int32_t i32Ret = LL_OK;
    uint8_t tmp1;
    uint8_t tmp2;

    tmp1 = (uint8_t)((READ_REG32(CM_EFM->FSR) & EFM_FSR_RDY) == EFM_FSR_RDY);

    tmp2 = (uint8_t)((READ_REG32(CM_DMA->CHSTAT) & DMA_CHSTAT_DMAACT) == 0x00U);

    if (0U == (tmp1 & tmp2)) {
        i32Ret = LL_ERR_NOT_RDY;
    }
    return i32Ret;
}

/**
 * @brief  MCU behavior config for stop mode.
 * @param  None
 * @retval None
 */ 
void STOP_Config(void)
{
    stc_pwc_stop_mode_config_t stcStopConfig;

    (void)PWC_STOP_StructInit(&stcStopConfig);

    stcStopConfig.u16Clock = PWC_STOP_CLK_KEEP;
    stcStopConfig.u16FlashWait = PWC_STOP_FLASH_WAIT_ON;

    (void)PWC_STOP_Config(&stcStopConfig);

    /* Wake-up source config (EXTINT Ch.3 here) */
    INTC_WakeupSrcCmd(INTC_STOP_WKUP_TMR0_CMP, ENABLE);
}

void TMR0_1_Handler(void)
{
    TMR0_ClearStatus(TMR0_UNIT, TMR0_CH_FLAG);
}

/**
 * @brief  Configure TMR0.
 * @note   In asynchronous clock, If you want to write a TMR0 register, you need to wait for
 *         at least 6 asynchronous clock cycles after the last write operation!
 * @param  None
 * @retval None
 */
void TMR0_Config(void)
{
    stc_tmr0_init_t stcTmr0Init;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Enable TIMER0 and AOS clock */
    FCG_Fcg2PeriphClockCmd(TMR0_CLK, ENABLE);
//    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);

    /* TIMER0 initialize */
    (void)TMR0_StructInit(&stcTmr0Init);
    stcTmr0Init.u32ClockSrc = TMR0_CLK_SRC_LRC;
    stcTmr0Init.u32ClockDiv = TMR0_CLK_DIV1024;
    stcTmr0Init.u32Func     = TMR0_FUNC_CMP;
    stcTmr0Init.u16CompareValue = (uint16_t)TMR0_CMP_VALUE;
    (void)TMR0_Init(TMR0_UNIT, TMR0_CH, &stcTmr0Init);
    TMR0_IntCmd(TMR0_UNIT, TMR0_CH_INT, ENABLE);
//    TMR0_HWStopCondCmd(TMR0_UNIT, TMR0_CH, ENABLE);
//    AOS_SetTriggerEventSrc(TMR0_TRIG_CH, BSP_KEY1_INT_EVT);

    /* Interrupt configuration */
    stcIrqSignConfig.enIntSrc    = TMR0_INT_SRC;
    stcIrqSignConfig.enIRQn      = TMR0_IRQn;
    stcIrqSignConfig.pfnCallback = &TMR0_1_Handler;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);
}


