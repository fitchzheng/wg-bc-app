#include "bsp_pwc.h"
#include "section.h"
#include "hc32_ll_rmu.h"
#include "hc32_ll_pwc.h"
#include "hc32_ll_rtc.h"
#include "hc32_ll_clk.h"
#include "hc32_ll_utility.h"
#include "hc32_ll_interrupts.h"

static void PowerDownModeConfig(void)
{
    stc_pwc_pd_mode_config_t stcPDModeConfig;

    (void)PWC_PD_StructInit(&stcPDModeConfig);

    stcPDModeConfig.u8IOState = PWC_PD_IO_KEEP1;
    stcPDModeConfig.u8Mode = PWC_PD_MD1;

    (void)PWC_PD_Config(&stcPDModeConfig);
    PWC_PD_ClearWakeupStatus(PWC_PD_WKUP_FLAG_ALL);

    /* Wake up by WKTM */
    PWC_PD_WakeupCmd(PWC_PD_WKUP_WKTM, ENABLE);

    /* Disable WKTM inadvance */
    PWC_WKT_Cmd(DISABLE);
    /* LRC for WKTM */
    CLK_LrcCmd(ENABLE);
    /* WKTM init */
    PWC_WKT_Config(PWC_WKT_CLK_SRC_64HZ, 0x400U);

    NVIC_ClearPendingIRQ(WKTM_IRQn);
    NVIC_SetPriority(WKTM_IRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(WKTM_IRQn);
}
/**
 * @brief  RTC calendar configuration.
 * @param  None
 * @retval None
 */
static void RTC_CalendarConfig(void)
{
    stc_rtc_date_t stcRtcDate;
    stc_rtc_time_t stcRtcTime;

    /* Date configuration */
    stcRtcDate.u8Year    = 20U;
    stcRtcDate.u8Month   = RTC_MONTH_JANUARY;
    stcRtcDate.u8Day     = 1U;
    stcRtcDate.u8Weekday = RTC_WEEKDAY_WEDNESDAY;

    /* Time configuration */
    stcRtcTime.u8Hour   = 23U;
    stcRtcTime.u8Minute = 59U;
    stcRtcTime.u8Second = 55U;
    stcRtcTime.u8AmPm   = RTC_HOUR_12H_AM;

    if (LL_OK != RTC_SetDate(RTC_DATA_FMT_DEC, &stcRtcDate)) {
        // Set Date failed!
    }

    if (LL_OK != RTC_SetTime(RTC_DATA_FMT_DEC, &stcRtcTime)) {
        // Set Time failed!
    }
}

/**
 * @brief  RTC configuration.
 * @param  None
 * @retval None
 */
static void RTC_Config(void)
{
    stc_rtc_init_t stcRtcInit;

    /* Reset RTC counter */
    if (LL_ERR_TIMEOUT == RTC_DeInit()) {
        //RTC重置失败
    } else {
        /* Stop RTC */
        RTC_Cmd(DISABLE);
        /* Configure structure initialization */
        (void)RTC_StructInit(&stcRtcInit);

        /* Configuration RTC structure */
        stcRtcInit.u8ClockSrc   = RTC_CLK_SRC_LRC;
        stcRtcInit.u8HourFormat = RTC_HOUR_FMT_24H;
        stcRtcInit.u8IntPeriod  = RTC_INT_PERIOD_PER_MINUTE;
        (void)RTC_Init(&stcRtcInit);

        /* Update date and time */
        RTC_CalendarConfig();
        /* Startup RTC count */
        RTC_Cmd(ENABLE);
    }
}

void PWC_WKTM_Handler(void)
{
    if (SET == PWC_WKT_GetStatus()) {
        PWC_WKT_ClearStatus();
    }

    __DSB();  /* Arm Errata 838869 */
}

//            PWC_WKT_Cmd(ENABLE);//32
//            PWC_PD_Enter();


void bsp_pwc_init(void)
{
    RTC_Config();
    PowerDownModeConfig();
    RMU_ClearStatus();
}

//REG_INIT_TP(pwc_init);

