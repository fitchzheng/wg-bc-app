#include "bsp_fwdgt.h"

// 看门狗初始化配置
// 10K时钟，32分配，计数256，即32*256/10K = 0.8192秒
// 休眠时看门狗停止
// 看门狗溢出后直接复位芯片
void SWDT_Config(void)
{
    stc_swdt_init_t stcSwdtInit;

    /* SWDT configuration */
    stcSwdtInit.u32CountPeriod = SWDT_CNT_PERIOD65536;
    stcSwdtInit.u32ClockDiv = SWDT_CLK_DIV32;
    stcSwdtInit.u32RefreshRange = SWDT_RANGE_0TO100PCT;
    stcSwdtInit.u32LPMCount = SWDT_LPM_CNT_STOP;
    stcSwdtInit.u32ExceptionType = SWDT_EXP_TYPE_RST;
    (void)SWDT_Init(&stcSwdtInit);
}

void bsp_fwdgt_init(void)
{
    SWDT_Config();
}

void bsp_fwdgt_feed(void)
{
    SWDT_FeedDog();//看门狗喂狗
}
