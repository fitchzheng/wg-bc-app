#include "bsp_pwm.h"
#include "section.h"
uint32_t timer_pwma_chctl2;
uint32_t timer_pwmb_chctl2;

// pwm输出端口使能
void hrpwm_port_init(void)
{
    GPIO_HrpwmPinCmd(HRPWM_HA1_PORT, ENABLE);
    GPIO_HrpwmPinCmd(HRPWM_LA1_PORT, ENABLE);
    GPIO_HrpwmPinCmd(HRPWM_LA2_PORT, ENABLE);
    GPIO_HrpwmPinCmd(HRPWM_HA2_PORT, ENABLE);
    GPIO_HrpwmPinCmd(HRPWM_HB1_PORT, ENABLE);
    GPIO_HrpwmPinCmd(HRPWM_LB1_PORT, ENABLE);
    GPIO_HrpwmPinCmd(HRPWM_HB2_PORT, ENABLE);
    GPIO_HrpwmPinCmd(HRPWM_LB2_PORT, ENABLE);
}

uint32_t RAMFUNC HRPWM_GetPeriodValue_Buff(const CM_HRPWM_TypeDef *HRPWMx)
{
    DDL_ASSERT(IS_HRPWM_UNIT(HRPWMx));
    return READ_REG32(HRPWMx->HRPERBR);
}

// HRPWM4极性 HA1 LA1
void A1PWM_polarity_set(void)
{
    stc_hrpwm_init_t stcHrpwmInit;
    stc_hrpwm_pwm_init_t stcPwmInit;

    (void)HRPWM_StructInit(&stcHrpwmInit);
    /* HRPWM general count function configure */
    stcHrpwmInit.u32CountMode = HRPWM_MD_TRIANGLE;     // 三角波计数
    stcHrpwmInit.u32CountReload = HRPWM_CNT_RELOAD_ON; // 计数器自动重载
    stcHrpwmInit.u32PeriodValue = CTRL_PERIOD;         // 周期值
    (void)HRPWM_Init(A1_PWM_UNIT, &stcHrpwmInit);

    (void)HRPWM_PWM_StructInit(&stcPwmInit);
    /* Configure PWM output */
    stcPwmInit.u32CompareValue = CTRL_PERIOD / 2;               // 默认比较值，50%
    stcPwmInit.u32StartPolarity = HRPWM4_HA1_START;             // 起始电平
    stcPwmInit.u32PeakPolarity = HRPWM_PWM_PEAK_HOLD;           // 峰值电平保持
    stcPwmInit.u32ValleyPolarity = HRPWM_PWM_VALLEY_HOLD;       // 谷值电平保持
    stcPwmInit.u32UpMatchAPolarity = HRPWM4_HA1_UP_MATCH;       // 上升沿A比较匹配电平
    stcPwmInit.u32DownMatchAPolarity = HRPWM4_HA1_DOWN_MATCH;   // 下降沿A比较匹配电平
    stcPwmInit.u32UpMatchBPolarity = HRPWM_PWM_UP_MATCH_B_HOLD; // 其他保持电平不变
    stcPwmInit.u32DownMatchBPolarity = HRPWM_PWM_DOWN_MATCH_B_HOLD;
    stcPwmInit.u32UpMatchEPolarity = HRPWM_PWM_UP_MATCH_E_HOLD;
    stcPwmInit.u32DownMatchEPolarity = HRPWM_PWM_DOWN_MATCH_E_HOLD;
    stcPwmInit.u32UpMatchFPolarity = HRPWM_PWM_UP_MATCH_F_HOLD;
    stcPwmInit.u32DownMatchFPolarity = HRPWM_PWM_DOWN_MATCH_F_HOLD;
    stcPwmInit.u32UpMatchSpecialAPolarity = HRPWM_PWM_UP_MATCH_SPECIAL_A_HOLD;
    stcPwmInit.u32DownMatchSpecialAPolarity = HRPWM_PWM_DOWN_MATCH_SPECIAL_A_HOLD;
    stcPwmInit.u32UpMatchSpecialBPolarity = HRPWM_PWM_UP_MATCH_SPECIAL_B_HOLD;
    stcPwmInit.u32DownMatchSpecialBPolarity = HRPWM_PWM_DOWN_MATCH_SPECIAL_B_HOLD;
    (void)HRPWM_PWM_ChAInit_Buf(A1_PWM_UNIT, &stcPwmInit);    // 初始化通道A
    stcPwmInit.u32StartPolarity = HRPWM4_LA1_START;           // 起始电平高
    stcPwmInit.u32UpMatchAPolarity = HRPWM4_LA1_UP_MATCH;     // 上升沿A比较匹配电平
    stcPwmInit.u32DownMatchAPolarity = HRPWM4_LA1_DOWN_MATCH; // 下降沿A比较匹配电平
    (void)HRPWM_PWM_ChBInit_Buf(A1_PWM_UNIT, &stcPwmInit);
    /* PWM output enable */
    //    HRPWM_PWM_ChAOutputEnable(A1_PWM_UNIT);
    //    HRPWM_PWM_ChBOutputEnable(A1_PWM_UNIT);

    //    HRPWM_PWM_ChAOutputEnable_Buf(A1_PWM_UNIT);
    //    HRPWM_PWM_ChBOutputEnable_Buf(A1_PWM_UNIT);
}

// HRPWM5极性 HA2 LA2
void A2PWM_polarity_set(void)
{
    stc_hrpwm_init_t stcHrpwmInit;
    stc_hrpwm_pwm_init_t stcPwmInit;
    stc_hrpwm_pwm_output_init_t pstcPwmOutputInit;

    (void)HRPWM_StructInit(&stcHrpwmInit);
    /* HRPWM general count function configure */
    stcHrpwmInit.u32CountMode = HRPWM_MD_TRIANGLE;     // 三角波计数
    stcHrpwmInit.u32CountReload = HRPWM_CNT_RELOAD_ON; // 计数器自动重载
    stcHrpwmInit.u32PeriodValue = CTRL_PERIOD;         // 周期值
    (void)HRPWM_Init(A2_PWM_UNIT, &stcHrpwmInit);

    HRPWM_PWM_OutputStructInit(&pstcPwmOutputInit);
    pstcPwmOutputInit.u32ChSwap = HRPWM_PWM_CH_SWAP_ON;
    HRPWM_PWM_OutputInit_Buf(A2_PWM_UNIT, &pstcPwmOutputInit);

    (void)HRPWM_PWM_StructInit(&stcPwmInit);
    /* Configure PWM output */
    stcPwmInit.u32CompareValue = CTRL_PERIOD / 2;               // 默认比较值，50%
    stcPwmInit.u32StartPolarity = HRPWM5_HA2_START;             // 起始电平
    stcPwmInit.u32PeakPolarity = HRPWM_PWM_PEAK_HOLD;           // 峰值电平保持
    stcPwmInit.u32ValleyPolarity = HRPWM_PWM_VALLEY_HOLD;       // 谷值电平保持
    stcPwmInit.u32UpMatchAPolarity = HRPWM5_HA2_UP_MATCH;       // 上升沿A比较匹配电平
    stcPwmInit.u32DownMatchAPolarity = HRPWM5_HA2_DOWN_MATCH;   // 下降沿A比较匹配电平
    stcPwmInit.u32UpMatchBPolarity = HRPWM_PWM_UP_MATCH_B_HOLD; // 其他保持电平不变
    stcPwmInit.u32DownMatchBPolarity = HRPWM_PWM_DOWN_MATCH_B_HOLD;
    stcPwmInit.u32UpMatchEPolarity = HRPWM_PWM_UP_MATCH_E_HOLD;
    stcPwmInit.u32DownMatchEPolarity = HRPWM_PWM_DOWN_MATCH_E_HOLD;
    stcPwmInit.u32UpMatchFPolarity = HRPWM_PWM_UP_MATCH_F_HOLD;
    stcPwmInit.u32DownMatchFPolarity = HRPWM_PWM_DOWN_MATCH_F_HOLD;
    stcPwmInit.u32UpMatchSpecialAPolarity = HRPWM_PWM_UP_MATCH_SPECIAL_A_HOLD;
    stcPwmInit.u32DownMatchSpecialAPolarity = HRPWM_PWM_DOWN_MATCH_SPECIAL_A_HOLD;
    stcPwmInit.u32UpMatchSpecialBPolarity = HRPWM_PWM_UP_MATCH_SPECIAL_B_HOLD;
    stcPwmInit.u32DownMatchSpecialBPolarity = HRPWM_PWM_DOWN_MATCH_SPECIAL_B_HOLD;
    (void)HRPWM_PWM_ChAInit_Buf(A2_PWM_UNIT, &stcPwmInit);    // 初始化通道A
    stcPwmInit.u32StartPolarity = HRPWM5_LA2_START;           // 起始电平高
    stcPwmInit.u32UpMatchAPolarity = HRPWM5_LA2_UP_MATCH;     // 上升沿A比较匹配电平
    stcPwmInit.u32DownMatchAPolarity = HRPWM5_LA2_DOWN_MATCH; // 下降沿A比较匹配电平
    (void)HRPWM_PWM_ChBInit_Buf(A2_PWM_UNIT, &stcPwmInit);

    /* PWM output enable */
    //    HRPWM_PWM_ChAOutputEnable(A2_PWM_UNIT);
    //    HRPWM_PWM_ChBOutputEnable(A2_PWM_UNIT);
    //    HRPWM_PWM_ChAOutputEnable_Buf(A2_PWM_UNIT);
    //    HRPWM_PWM_ChBOutputEnable_Buf(A2_PWM_UNIT);
}

// HRPWM3极性 HB1 LB1
void B1PWM_polarity_set(void)
{
    stc_hrpwm_init_t stcHrpwmInit;
    stc_hrpwm_pwm_init_t stcPwmInit;

    (void)HRPWM_StructInit(&stcHrpwmInit);
    /* HRPWM general count function configure */
    stcHrpwmInit.u32CountMode = HRPWM_MD_TRIANGLE;     // 三角波计数
    stcHrpwmInit.u32CountReload = HRPWM_CNT_RELOAD_ON; // 计数器自动重载
    stcHrpwmInit.u32PeriodValue = CTRL_PERIOD;         // 周期值
    (void)HRPWM_Init(B1_PWM_UNIT, &stcHrpwmInit);

    (void)HRPWM_PWM_StructInit(&stcPwmInit);
    /* Configure PWM output */
    stcPwmInit.u32CompareValue = CTRL_PERIOD / 2;               // 默认比较值，50%
    stcPwmInit.u32StartPolarity = HRPWM3_HB1_START;             // 起始电平高
    stcPwmInit.u32PeakPolarity = HRPWM_PWM_PEAK_HOLD;           // 峰值电平保持
    stcPwmInit.u32ValleyPolarity = HRPWM_PWM_VALLEY_HOLD;       // 谷值电平保持
    stcPwmInit.u32UpMatchAPolarity = HRPWM3_HB1_UP_MATCH;       // 上升沿A比较匹配低电平
    stcPwmInit.u32DownMatchAPolarity = HRPWM3_HB1_DOWN_MATCH;   // 下降沿A比较匹配高电平
    stcPwmInit.u32UpMatchBPolarity = HRPWM_PWM_UP_MATCH_B_HOLD; // 其他保持电平不变
    stcPwmInit.u32DownMatchBPolarity = HRPWM_PWM_DOWN_MATCH_B_HOLD;
    stcPwmInit.u32UpMatchEPolarity = HRPWM_PWM_UP_MATCH_E_HOLD;
    stcPwmInit.u32DownMatchEPolarity = HRPWM_PWM_DOWN_MATCH_E_HOLD;
    stcPwmInit.u32UpMatchFPolarity = HRPWM_PWM_UP_MATCH_F_HOLD;
    stcPwmInit.u32DownMatchFPolarity = HRPWM_PWM_DOWN_MATCH_F_HOLD;
    stcPwmInit.u32UpMatchSpecialAPolarity = HRPWM_PWM_UP_MATCH_SPECIAL_A_HOLD;
    stcPwmInit.u32DownMatchSpecialAPolarity = HRPWM_PWM_DOWN_MATCH_SPECIAL_A_HOLD;
    stcPwmInit.u32UpMatchSpecialBPolarity = HRPWM_PWM_UP_MATCH_SPECIAL_B_HOLD;
    stcPwmInit.u32DownMatchSpecialBPolarity = HRPWM_PWM_DOWN_MATCH_SPECIAL_B_HOLD;
    (void)HRPWM_PWM_ChAInit_Buf(B1_PWM_UNIT, &stcPwmInit);    // 初始化通道A
    stcPwmInit.u32StartPolarity = HRPWM3_LB1_START;           // 起始电平低
    stcPwmInit.u32UpMatchAPolarity = HRPWM3_LB1_UP_MATCH;     // 上升沿A比较匹配高电平
    stcPwmInit.u32DownMatchAPolarity = HRPWM3_LB1_DOWN_MATCH; // 下降沿A比较匹配低电平
    (void)HRPWM_PWM_ChBInit_Buf(B1_PWM_UNIT, &stcPwmInit);
    /* PWM output enable */
    //    HRPWM_PWM_ChAOutputEnable(B1_PWM_UNIT);
    //    HRPWM_PWM_ChBOutputEnable(B1_PWM_UNIT);
    //    HRPWM_PWM_ChAOutputEnable_Buf(B1_PWM_UNIT);
    //    HRPWM_PWM_ChBOutputEnable_Buf(B1_PWM_UNIT);
}

// HRPWM2极性 HB2 LB2
void B2PWM_polarity_set(void)
{
    stc_hrpwm_init_t stcHrpwmInit;
    stc_hrpwm_pwm_init_t stcPwmInit;

    (void)HRPWM_StructInit(&stcHrpwmInit);
    /* HRPWM general count function configure */
    stcHrpwmInit.u32CountMode = HRPWM_MD_TRIANGLE;     // 三角波计数
    stcHrpwmInit.u32CountReload = HRPWM_CNT_RELOAD_ON; // 计数器自动重载
    stcHrpwmInit.u32PeriodValue = CTRL_PERIOD;         // 周期值
    (void)HRPWM_Init(B2_PWM_UNIT, &stcHrpwmInit);

    (void)HRPWM_PWM_StructInit(&stcPwmInit);
    /* Configure PWM output */
    stcPwmInit.u32CompareValue = CTRL_PERIOD / 2;               // 默认比较值，50%
    stcPwmInit.u32StartPolarity = HRPWM2_HB2_START;             // 起始电平
    stcPwmInit.u32PeakPolarity = HRPWM_PWM_PEAK_HOLD;           // 峰值电平保持
    stcPwmInit.u32ValleyPolarity = HRPWM_PWM_VALLEY_HOLD;       // 谷值电平保持
    stcPwmInit.u32UpMatchAPolarity = HRPWM2_HB2_UP_MATCH;       // 上升沿A比较匹配电平
    stcPwmInit.u32DownMatchAPolarity = HRPWM2_HB2_DOWN_MATCH;   // 下降沿A比较匹配电平
    stcPwmInit.u32UpMatchBPolarity = HRPWM_PWM_UP_MATCH_B_HOLD; // 其他保持电平不变
    stcPwmInit.u32DownMatchBPolarity = HRPWM_PWM_DOWN_MATCH_B_HOLD;
    stcPwmInit.u32UpMatchEPolarity = HRPWM_PWM_UP_MATCH_E_HOLD;
    stcPwmInit.u32DownMatchEPolarity = HRPWM_PWM_DOWN_MATCH_E_HOLD;
    stcPwmInit.u32UpMatchFPolarity = HRPWM_PWM_UP_MATCH_F_HOLD;
    stcPwmInit.u32DownMatchFPolarity = HRPWM_PWM_DOWN_MATCH_F_HOLD;
    stcPwmInit.u32UpMatchSpecialAPolarity = HRPWM_PWM_UP_MATCH_SPECIAL_A_HOLD;
    stcPwmInit.u32DownMatchSpecialAPolarity = HRPWM_PWM_DOWN_MATCH_SPECIAL_A_HOLD;
    stcPwmInit.u32UpMatchSpecialBPolarity = HRPWM_PWM_UP_MATCH_SPECIAL_B_HOLD;
    stcPwmInit.u32DownMatchSpecialBPolarity = HRPWM_PWM_DOWN_MATCH_SPECIAL_B_HOLD;
    (void)HRPWM_PWM_ChAInit_Buf(B2_PWM_UNIT, &stcPwmInit);    // 初始化通道A
    stcPwmInit.u32StartPolarity = HRPWM2_LB2_START;           // 起始电平高
    stcPwmInit.u32UpMatchAPolarity = HRPWM2_LB2_UP_MATCH;     // 上升沿A比较匹配电平
    stcPwmInit.u32DownMatchAPolarity = HRPWM2_LB2_DOWN_MATCH; // 下降沿A比较匹配电平
    (void)HRPWM_PWM_ChBInit_Buf(B2_PWM_UNIT, &stcPwmInit);
    /* PWM output enable */
    //    HRPWM_PWM_ChAOutputEnable(B2_PWM_UNIT);
    //    HRPWM_PWM_ChBOutputEnable(B2_PWM_UNIT);
    //    HRPWM_PWM_ChAOutputEnable_Buf(B2_PWM_UNIT);
    //    HRPWM_PWM_ChBOutputEnable_Buf(B2_PWM_UNIT);
}

// HRPWM1极性
void PWM_polarity_set(void)
{
    stc_hrpwm_init_t stcHrpwmInit;
    stc_hrpwm_pwm_init_t stcPwmInit;

    (void)HRPWM_StructInit(&stcHrpwmInit);
    /* HRPWM general count function configure */
    stcHrpwmInit.u32CountMode = HRPWM_MD_TRIANGLE;     // 三角波计数
    stcHrpwmInit.u32CountReload = HRPWM_CNT_RELOAD_ON; // 计数器自动重载
    stcHrpwmInit.u32PeriodValue = CTRL_PERIOD * 4;     // 周期值
    (void)HRPWM_Init(PWM_UNIT, &stcHrpwmInit);

    (void)HRPWM_PWM_StructInit(&stcPwmInit);
    /* Configure PWM output */
    stcPwmInit.u32CompareValue = CTRL_PERIOD / 8;               // 默认比较值，50%
    stcPwmInit.u32StartPolarity = HRPWM1_HB1_START;             // 起始电平高
    stcPwmInit.u32PeakPolarity = HRPWM_PWM_PEAK_HOLD;           // 峰值电平保持
    stcPwmInit.u32ValleyPolarity = HRPWM_PWM_VALLEY_HOLD;       // 谷值电平保持
    stcPwmInit.u32UpMatchAPolarity = HRPWM1_HB1_UP_MATCH;       // 上升沿A比较匹配低电平
    stcPwmInit.u32DownMatchAPolarity = HRPWM1_HB1_DOWN_MATCH;   // 下降沿A比较匹配高电平
    stcPwmInit.u32UpMatchBPolarity = HRPWM_PWM_UP_MATCH_B_HOLD; // 其他保持电平不变
    stcPwmInit.u32DownMatchBPolarity = HRPWM_PWM_DOWN_MATCH_B_HOLD;
    stcPwmInit.u32UpMatchEPolarity = HRPWM_PWM_UP_MATCH_E_HOLD;
    stcPwmInit.u32DownMatchEPolarity = HRPWM_PWM_DOWN_MATCH_E_HOLD;
    stcPwmInit.u32UpMatchFPolarity = HRPWM_PWM_UP_MATCH_F_HOLD;
    stcPwmInit.u32DownMatchFPolarity = HRPWM_PWM_DOWN_MATCH_F_HOLD;
    stcPwmInit.u32UpMatchSpecialAPolarity = HRPWM_PWM_UP_MATCH_SPECIAL_A_HOLD;
    stcPwmInit.u32DownMatchSpecialAPolarity = HRPWM_PWM_DOWN_MATCH_SPECIAL_A_HOLD;
    stcPwmInit.u32UpMatchSpecialBPolarity = HRPWM_PWM_UP_MATCH_SPECIAL_B_HOLD;
    stcPwmInit.u32DownMatchSpecialBPolarity = HRPWM_PWM_DOWN_MATCH_SPECIAL_B_HOLD;
    (void)HRPWM_PWM_ChAInit_Buf(PWM_UNIT, &stcPwmInit);       // 初始化通道A
    stcPwmInit.u32StartPolarity = HRPWM1_LB1_START;           // 起始电平低
    stcPwmInit.u32UpMatchAPolarity = HRPWM1_LB1_UP_MATCH;     // 上升沿A比较匹配高电平
    stcPwmInit.u32DownMatchAPolarity = HRPWM1_LB1_DOWN_MATCH; // 下降沿A比较匹配低电平
    (void)HRPWM_PWM_ChBInit_Buf(PWM_UNIT, &stcPwmInit);
    /* PWM output enable */
    //    HRPWM_PWM_ChAOutputEnable(PWM_UNIT);
    //    HRPWM_PWM_ChBOutputEnable(PWM_UNIT);
    //    HRPWM_PWM_ChAOutputEnable_Buf(PWM_UNIT);
    //    HRPWM_PWM_ChBOutputEnable_Buf(PWM_UNIT);
}

// 使用RPWM1的专用比较事件做周期响应功能(SMCA 向上匹配)
void bsp_hrpwm_ValidPeriod_cfg(void)
{
    stc_hrpwm_valid_period_config_t stcValidPeriodConfig;
    HRPWM_ValidPeriodStructInit(&stcValidPeriodConfig);

    HRPWM_MatchSpecialEventEnable(PWM_UNIT, HRPWM_EVT_UP_MATCH_SPECIAL_A);     // 使能专用A匹配事件
    HRPWM_MatchSpecialEventEnable_Buf(PWM_UNIT, HRPWM_EVT_UP_MATCH_SPECIAL_A); // 使能专用A匹配事件，缓存寄存器
    /* Set special compare register */
    HRPWM_SetSpecialCompareAValue(PWM_UNIT, CTRL_PERIOD / 4);     // 专用比较器初始值
    HRPWM_SetSpecialCompareAValue_Buf(PWM_UNIT, CTRL_PERIOD / 4); // 专用比较器缓存初始值
}

// 配置HRPWM的缓存功能
void bsp_hrpwm_buffer_cfg(void)
{
    stc_hrpwm_buf_config_t stcBufConfig;

    (void)HRPWM_BufStructInit(&stcBufConfig);
    /* General compare buffer function configure */
    stcBufConfig.enBufTransU1Single = ENABLE;
    //    stcBufConfig.enBufTransAfterU1Single = ENABLE;  //传送条件，hrpwm1单次缓存
    (void)HRPWM_GeneralAEBufConfig(A1_PWM_UNIT, &stcBufConfig); // 使能比较器缓存
    (void)HRPWM_GeneralAEBufConfig(A2_PWM_UNIT, &stcBufConfig);
    (void)HRPWM_GeneralAEBufConfig(B1_PWM_UNIT, &stcBufConfig);
    (void)HRPWM_GeneralAEBufConfig(B2_PWM_UNIT, &stcBufConfig);
    HRPWM_GeneralAEBufEnable(A1_PWM_UNIT);
    HRPWM_GeneralAEBufEnable(A2_PWM_UNIT);
    HRPWM_GeneralAEBufEnable(B1_PWM_UNIT);
    HRPWM_GeneralAEBufEnable(B2_PWM_UNIT);

    HRPWM_PeriodBufConfig(A1_PWM_UNIT, &stcBufConfig);
    HRPWM_PeriodBufConfig(A2_PWM_UNIT, &stcBufConfig);
    HRPWM_PeriodBufConfig(B1_PWM_UNIT, &stcBufConfig);
    HRPWM_PeriodBufConfig(B2_PWM_UNIT, &stcBufConfig);
    HRPWM_PeriodBufEnable(A1_PWM_UNIT);
    HRPWM_PeriodBufEnable(A2_PWM_UNIT);
    HRPWM_PeriodBufEnable(B1_PWM_UNIT);
    HRPWM_PeriodBufEnable(B2_PWM_UNIT);

    HRPWM_DeadTimeBufConfig(A1_PWM_UNIT, &stcBufConfig);
    HRPWM_DeadTimeBufConfig(A2_PWM_UNIT, &stcBufConfig);
    HRPWM_DeadTimeBufConfig(B1_PWM_UNIT, &stcBufConfig);
    HRPWM_DeadTimeBufConfig(B2_PWM_UNIT, &stcBufConfig);
    HRPWM_DeadTimeUpBufEnable(A1_PWM_UNIT);
    HRPWM_DeadTimeUpBufEnable(A2_PWM_UNIT);
    HRPWM_DeadTimeUpBufEnable(B1_PWM_UNIT);
    HRPWM_DeadTimeUpBufEnable(B2_PWM_UNIT);
    HRPWM_DeadTimeDownBufEnable(A1_PWM_UNIT);
    HRPWM_DeadTimeDownBufEnable(A2_PWM_UNIT);
    HRPWM_DeadTimeDownBufEnable(B1_PWM_UNIT);
    HRPWM_DeadTimeDownBufEnable(B2_PWM_UNIT);

    HRPWM_ControlRegBufConfig(A1_PWM_UNIT, &stcBufConfig); // 使能控制寄存器缓存
    HRPWM_ControlRegBufConfig(A2_PWM_UNIT, &stcBufConfig);
    HRPWM_ControlRegBufConfig(B1_PWM_UNIT, &stcBufConfig);
    HRPWM_ControlRegBufConfig(B2_PWM_UNIT, &stcBufConfig);

    HRPWM_ControlRegBufEnable(A1_PWM_UNIT);
    HRPWM_ControlRegBufEnable(A2_PWM_UNIT);
    HRPWM_ControlRegBufEnable(B1_PWM_UNIT);
    HRPWM_ControlRegBufEnable(B2_PWM_UNIT);

    HRPWM_PH_SetBufCond(HRPWM_BUF_TRANS_VALLEY);
    HRPWM_PH_BufEnable();

    (void)HRPWM_BufStructInit(&stcBufConfig);
    stcBufConfig.enBufTransU1Single = ENABLE;
    stcBufConfig.enBufTransAfterU1Single = DISABLE; // 传送条件，hrpwm1单次缓存
    (void)HRPWM_GeneralAEBufConfig(PWM_UNIT, &stcBufConfig);
    HRPWM_GeneralAEBufEnable(PWM_UNIT);
    HRPWM_PeriodBufConfig(PWM_UNIT, &stcBufConfig);
    HRPWM_PeriodBufEnable(PWM_UNIT);
    HRPWM_DeadTimeBufConfig(PWM_UNIT, &stcBufConfig);
    HRPWM_DeadTimeUpBufEnable(PWM_UNIT);
    HRPWM_DeadTimeDownBufEnable(PWM_UNIT);
    HRPWM_ControlRegBufConfig(PWM_UNIT, &stcBufConfig);
    HRPWM_ControlRegBufEnable(PWM_UNIT);

    HRPWM_SpecialABufEnable(PWM_UNIT); // 使能专用比较器缓存，只有通道1需要

    HRPWM_U1SingleTransBufConfig(HRPWM_BUF_U1_SINGLE_TRANS_ON); // 使能hrpwm1单次缓存模式
    //   HRPWM_U1SingleTransBufTrigger(); //触发U1单次缓存
}

__STATIC_INLINE void RAMFUNC HRPWM_IDLE_Exit_32bit(uint32_t u32Unit, uint32_t u32Ch)
{
    WRITE_REG32(CM_HRPWM_COMMON->SSTARUNR1, u32Unit & u32Ch);
}

void Set_channelA_outen(uint8_t set)
{
    if (set & HA1_EN_BIT)
    {
        bCM_HRPWM4->BPCNAR1_b.OUTENA = 1;
    }
    else
    {
        bCM_HRPWM4->BPCNAR1_b.OUTENA = 0;
    }
    if (set & LA1_EN_BIT)
    {
        bCM_HRPWM4->BPCNBR1_b.OUTENB = 1;
    }
    else
    {
        bCM_HRPWM4->BPCNBR1_b.OUTENB = 0;
    }
    if (set & HA2_EN_BIT)
    {
        bCM_HRPWM5->BPCNAR1_b.OUTENA = 1;
    }
    else
    {
        bCM_HRPWM5->BPCNAR1_b.OUTENA = 0;
    }
    if (set & LA2_EN_BIT)
    {
        bCM_HRPWM5->BPCNBR1_b.OUTENB = 1;
    }
    else
    {
        bCM_HRPWM5->BPCNBR1_b.OUTENB = 0;
    }
}

void Set_channelB_outen(uint8_t set)
{
    if (set & HB1_EN_BIT)
    {
        bCM_HRPWM3->BPCNAR1_b.OUTENA = 1;
    }
    else
    {
        bCM_HRPWM3->BPCNAR1_b.OUTENA = 0;
    }
    if (set & LB1_EN_BIT)
    {
        bCM_HRPWM3->BPCNBR1_b.OUTENB = 1;
    }
    else
    {
        bCM_HRPWM3->BPCNBR1_b.OUTENB = 0;
    }
    if (set & HB2_EN_BIT)
    {
        bCM_HRPWM2->BPCNAR1_b.OUTENA = 1;
    }
    else
    {
        bCM_HRPWM2->BPCNAR1_b.OUTENA = 0;
    }
    if (set & LB2_EN_BIT)
    {
        bCM_HRPWM2->BPCNBR1_b.OUTENB = 1;
    }
    else
    {
        bCM_HRPWM2->BPCNBR1_b.OUTENB = 0;
    }
}

// A1 IDLE处理
void RAMFUNC bsp_idleA1_process(uint8_t set)
{
    static uint8_t Last_Set = 0;
    static uint8_t wait_fresh_CHA = 0; // 每次进入/退出IDLE操作 都等待占空比或极性更新后再进行
    static uint8_t wait_fresh_CHB = 0;
    if ((Last_Set & (HA1_EN_BIT | LA1_EN_BIT)) == (set & (HA1_EN_BIT | LA1_EN_BIT)))
    {
        return;
    }
    if (set & HA1_EN_BIT) // HA1输出状态，退出IDLE
    {
        if (HRPWM_IDLE_GetChStatus(A1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA) == RESET) // 判断当前通道处于IDLE状态
        {
            if (wait_fresh_CHA == 0) // 等待一个中断周期，等待占空比信息更新
            {
                wait_fresh_CHA = 1;
            }
            else if (wait_fresh_CHA == 1)
            {
                wait_fresh_CHA = 0;
                HRPWM_IDLE_Exit_32bit(A1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA); // 退出IDLE状态
            }
        }
    }
    else // HA1进入IDLE
    {
        if (HRPWM_IDLE_GetChStatus(A1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA) == SET)
        {
            HRPWM_IDLE_EnterImmediate(A1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA);
        }
    }
    if (set & LA1_EN_BIT) // LA1输出状态
    {
        if (HRPWM_IDLE_GetChStatus(A1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB) == RESET) // 判断当前通道处于IDLE状态
        {
            if (wait_fresh_CHB == 0)
            {
                wait_fresh_CHB = 1;
            }
            else if (wait_fresh_CHB == 1)
            {
                wait_fresh_CHB = 0;
                HRPWM_IDLE_Exit_32bit(A1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB); // 退出IDLE状态
            }
        }
    }
    else
    {
        if (HRPWM_IDLE_GetChStatus(A1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB) == SET)
        {
            HRPWM_IDLE_EnterImmediate(A1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB);
        }
    }
    if ((wait_fresh_CHA == 0) && (wait_fresh_CHB == 0))
    {
        Last_Set = set;
    }
}

// A2 IDLE处理
// 如果有一路空闲，就开启软件触发事件
// 如果有一路输出，就退出相应的通道空闲
void RAMFUNC bsp_idleA2_process(uint8_t set)
{
    static uint8_t Last_Set = 0;
    static uint8_t wait_fresh_CHA = 0; // 每次进入/退出IDLE操作 都等待占空比或极性更新后再进行
    static uint8_t wait_fresh_CHB = 0;
    if ((Last_Set & (HA2_EN_BIT | LA2_EN_BIT)) == (set & (HA2_EN_BIT | LA2_EN_BIT)))
    {
        return;
    }
    if (set & HA2_EN_BIT) // HA1输出状态，退出IDLE
    {
        if (HRPWM_IDLE_GetChStatus(A2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA) == RESET) // 判断当前通道处于IDLE状态
        {
            if (wait_fresh_CHA == 0) // 等待一个中断周期，等待占空比信息更新
            {
                wait_fresh_CHA = 1;
            }
            else if (wait_fresh_CHA == 1)
            {
                wait_fresh_CHA = 0;
                HRPWM_IDLE_Exit_32bit(A2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA); // 退出IDLE状态
            }
        }
    }
    else // HA1进入IDLE
    {
        if (HRPWM_IDLE_GetChStatus(A2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA) == SET)
        {
            HRPWM_IDLE_EnterImmediate(A2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA);
        }
    }
    if (set & LA2_EN_BIT) // LA1输出状态
    {
        if (HRPWM_IDLE_GetChStatus(A2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB) == RESET) // 判断当前通道处于IDLE状态
        {
            if (wait_fresh_CHB == 0)
            {
                wait_fresh_CHB = 1;
            }
            else if (wait_fresh_CHB == 1)
            {
                wait_fresh_CHB = 0;
                HRPWM_IDLE_Exit_32bit(A2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB); // 退出IDLE状态
            }
        }
    }
    else
    {
        if (HRPWM_IDLE_GetChStatus(A2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB) == SET)
        {
            HRPWM_IDLE_EnterImmediate(A2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB);
        }
    }
    if ((wait_fresh_CHA == 0) && (wait_fresh_CHB == 0))
    {
        Last_Set = set;
    }
}

// B1 IDLE处理
// 如果有一路空闲，就开启软件触发事件
// 如果有一路输出，就退出相应的通道空闲
void RAMFUNC bsp_idleB1_process(uint8_t set)
{
    static uint8_t Last_Set = 0;
    static uint8_t wait_fresh_CHA = 0; // 每次进入/退出IDLE操作 都等待占空比或极性更新后再进行
    static uint8_t wait_fresh_CHB = 0;
    if ((Last_Set & (HB1_EN_BIT | LB1_EN_BIT)) == (set & (HB1_EN_BIT | LB1_EN_BIT)))
    {
        return;
    }
    if (set & HB1_EN_BIT) // HA1输出状态，退出IDLE
    {
        if (HRPWM_IDLE_GetChStatus(B1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA) == RESET) // 判断当前通道处于IDLE状态
        {
            if (wait_fresh_CHA == 0) // 等待一个中断周期，等待占空比信息更新
            {
                wait_fresh_CHA = 1;
            }
            else if (wait_fresh_CHA == 1)
            {
                wait_fresh_CHA = 0;
                HRPWM_IDLE_Exit_32bit(B1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA); // 退出IDLE状态
            }
        }
    }
    else // HA1进入IDLE
    {
        if (HRPWM_IDLE_GetChStatus(B1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA) == SET)
        {
            HRPWM_IDLE_EnterImmediate(B1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA);
        }
    }
    if (set & LB1_EN_BIT) // LA1输出状态
    {
        if (HRPWM_IDLE_GetChStatus(B1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB) == RESET) // 判断当前通道处于IDLE状态
        {
            if (wait_fresh_CHB == 0)
            {
                wait_fresh_CHB = 1;
            }
            else if (wait_fresh_CHB == 1)
            {
                wait_fresh_CHB = 0;
                HRPWM_IDLE_Exit_32bit(B1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB); // 退出IDLE状态
            }
        }
    }
    else
    {
        if (HRPWM_IDLE_GetChStatus(B1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB) == SET)
        {
            HRPWM_IDLE_EnterImmediate(B1_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB);
        }
    }
    if ((wait_fresh_CHA == 0) && (wait_fresh_CHB == 0))
    {
        Last_Set = set;
    }
}

// B2 IDLE处理
// 如果有一路空闲，就开启软件触发事件
// 如果有一路输出，就退出相应的通道空闲
void RAMFUNC bsp_idleB2_process(uint8_t set)
{
    static uint8_t Last_Set = 0;
    static uint8_t wait_fresh_CHA = 0; // 每次进入/退出IDLE操作 都等待占空比或极性更新后再进行
    static uint8_t wait_fresh_CHB = 0;
    if ((Last_Set & (HB2_EN_BIT | LB2_EN_BIT)) == (set & (HB2_EN_BIT | LB2_EN_BIT)))
    {
        return;
    }
    if (set & HB2_EN_BIT) // HA1输出状态，退出IDLE
    {
        if (HRPWM_IDLE_GetChStatus(B2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA) == RESET) // 判断当前通道处于IDLE状态
        {
            if (wait_fresh_CHA == 0) // 等待一个中断周期，等待占空比信息更新
            {
                wait_fresh_CHA = 1;
            }
            else if (wait_fresh_CHA == 1)
            {
                wait_fresh_CHA = 0;
                HRPWM_IDLE_Exit_32bit(B2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA); // 退出IDLE状态
            }
        }
    }
    else // HA1进入IDLE
    {
        if (HRPWM_IDLE_GetChStatus(B2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA) == SET)
        {
            HRPWM_IDLE_EnterImmediate(B2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHA);
        }
    }
    if (set & LB2_EN_BIT) // LA1输出状态
    {
        if (HRPWM_IDLE_GetChStatus(B2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB) == RESET) // 判断当前通道处于IDLE状态
        {
            if (wait_fresh_CHB == 0)
            {
                wait_fresh_CHB = 1;
            }
            else if (wait_fresh_CHB == 1)
            {
                wait_fresh_CHB = 0;
                HRPWM_IDLE_Exit_32bit(B2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB); // 退出IDLE状态
            }
        }
    }
    else
    {
        if (HRPWM_IDLE_GetChStatus(B2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB) == SET)
        {
            HRPWM_IDLE_EnterImmediate(B2_SW_PWM_UNIT, HRPWM_SW_SYNC_CHB);
        }
    }
    if ((wait_fresh_CHA == 0) && (wait_fresh_CHB == 0))
    {
        Last_Set = set;
    }
}

void bsp_DeadTime_Config(void)
{
    stc_hrpwm_deadtime_config_t stcDeadTimeConfig;
    (void)HRPWM_DeadTimeStructInit(&stcDeadTimeConfig);
    /* Set dead time value (up count) */
    HRPWM_SetDeadTimeUpValue_Buf(PWM_UNIT, PWM_DEAD_TIME);
    HRPWM_SetDeadTimeUpValue_Buf(A1_PWM_UNIT, PWM_DEAD_TIME);
    HRPWM_SetDeadTimeUpValue_Buf(A2_PWM_UNIT, PWM_DEAD_TIME);
    HRPWM_SetDeadTimeUpValue_Buf(B1_PWM_UNIT, PWM_DEAD_TIME);
    HRPWM_SetDeadTimeUpValue_Buf(B2_PWM_UNIT, PWM_DEAD_TIME);

    /* DeadTime function configure */
    stcDeadTimeConfig.u32EqualUpDown = HRPWM_DEADTIME_EQUAL_ON;
    stcDeadTimeConfig.u32BufUp = HRPWM_DEADTIME_CNT_UP_BUF_OFF;
    stcDeadTimeConfig.u32BufDown = HRPWM_DEADTIME_CNT_DOWN_BUF_OFF;
    (void)HRPWM_DeadTimeConfig(PWM_UNIT, &stcDeadTimeConfig);
    (void)HRPWM_DeadTimeConfig(A1_PWM_UNIT, &stcDeadTimeConfig);
    (void)HRPWM_DeadTimeConfig(A2_PWM_UNIT, &stcDeadTimeConfig);
    (void)HRPWM_DeadTimeConfig(B1_PWM_UNIT, &stcDeadTimeConfig);
    (void)HRPWM_DeadTimeConfig(B2_PWM_UNIT, &stcDeadTimeConfig);

    HRPWM_DeadTimeEnable(PWM_UNIT);
    HRPWM_DeadTimeEnable(A1_PWM_UNIT);
    HRPWM_DeadTimeEnable(A2_PWM_UNIT);
    HRPWM_DeadTimeEnable(B1_PWM_UNIT);
    HRPWM_DeadTimeEnable(B2_PWM_UNIT);
}

void bsp_phase_config(void)
{
    stc_hrpwm_ph_config_t stcPHConfig;

    HRPWM_PH_SetCompareValue(HRPWM_PH_MATCH_IDX1, HRPWM_PHSCMP1_VALUE); // 0
    HRPWM_PH_SetCompareValue_Buf(HRPWM_PH_MATCH_IDX1, HRPWM_PHSCMP1_VALUE);

    (void)HRPWM_PH_StructInit(&stcPHConfig);
    stcPHConfig.u32PhaseIndex = HRPWM_PH_MATCH_IDX1;
    stcPHConfig.u32ForceChA = HRPWM_PH_MATCH_FORCE_CHA_OFF;
    stcPHConfig.u32ForceChB = HRPWM_PH_MATCH_FORCE_CHB_OFF;
    stcPHConfig.u32PeriodLink = HRPWM_PH_PERIOD_LINK_OFF;
    (void)HRPWM_PH_Config(B1_PWM_UNIT, &stcPHConfig);
    (void)HRPWM_PH_Config(B2_PWM_UNIT, &stcPHConfig);
    (void)HRPWM_PH_Config(A1_PWM_UNIT, &stcPHConfig);
    (void)HRPWM_PH_Config(A2_PWM_UNIT, &stcPHConfig);

    HRPWM_PH_Enable(A2_PWM_UNIT);
    HRPWM_PH_Enable(A1_PWM_UNIT);
    HRPWM_PH_Enable(B2_PWM_UNIT);
    HRPWM_PH_Enable(B1_PWM_UNIT);
}

void bsp_pwm_idle_Level(void)
{
    // 开启延时空闲
    stc_hrpwm_idle_delay_init_t stcIdleDelay;
    (void)HRPWM_IDLE_DELAY_StructInit(&stcIdleDelay);
    stcIdleDelay.u32TriggerSrc = HRPWM_IDLE_DELAY_TRIG_SW;  // 软件触发延时空闲
    stcIdleDelay.u32PeriodPoint = HRPWM_CPLT_PERIOD_VALLEY; // 波谷匹配
    stcIdleDelay.u32IdleOutputChAStatus = HRPWM_IDLE_OUTPUT_CHA_ON;
    stcIdleDelay.u32IdleOutputChBStatus = HRPWM_IDLE_OUTPUT_CHB_ON;
    (void)HRPWM_IDLE_DELAY_Init(A1_PWM_UNIT, &stcIdleDelay);
    (void)HRPWM_IDLE_DELAY_Init(A2_PWM_UNIT, &stcIdleDelay);
    (void)HRPWM_IDLE_DELAY_Init(B1_PWM_UNIT, &stcIdleDelay);
    (void)HRPWM_IDLE_DELAY_Init(B2_PWM_UNIT, &stcIdleDelay);

    HRPWM_IDLE_SetChAIdleLevel(A1_PWM_UNIT, HRPWM_IDLE_CHA_LVL_LOW);
    HRPWM_IDLE_SetChBIdleLevel(A1_PWM_UNIT, HRPWM_IDLE_CHB_LVL_LOW);

    HRPWM_IDLE_SetChAIdleLevel(A2_PWM_UNIT, HRPWM_IDLE_CHA_LVL_LOW);
    HRPWM_IDLE_SetChBIdleLevel(A2_PWM_UNIT, HRPWM_IDLE_CHB_LVL_LOW);

    HRPWM_IDLE_SetChAIdleLevel(B1_PWM_UNIT, HRPWM_IDLE_CHA_LVL_LOW);
    HRPWM_IDLE_SetChBIdleLevel(B1_PWM_UNIT, HRPWM_IDLE_CHB_LVL_LOW);

    HRPWM_IDLE_SetChAIdleLevel(B2_PWM_UNIT, HRPWM_IDLE_CHA_LVL_LOW);
    HRPWM_IDLE_SetChBIdleLevel(B2_PWM_UNIT, HRPWM_IDLE_CHB_LVL_LOW);

    HRPWM_IDLE_SetChAIdleLevel(PWM_UNIT, HRPWM_IDLE_CHA_LVL_LOW);
    HRPWM_IDLE_SetChBIdleLevel(PWM_UNIT, HRPWM_IDLE_CHB_LVL_LOW);

    HRPWM_IDLE_DELAY_Enable(A1_PWM_UNIT);
    HRPWM_IDLE_DELAY_Enable(A2_PWM_UNIT);
    HRPWM_IDLE_DELAY_Enable(B1_PWM_UNIT);
    HRPWM_IDLE_DELAY_Enable(B2_PWM_UNIT);
}

void bsp_hrpwm_init_pwc(void)
{
    FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_HRPWM_1, ENABLE);
    hrpwm_port_init();
    HRPWM_CommonDeInit();
    HRPWM_DeInit(CM_HRPWM1);

    if (LL_OK != HRPWM_CALIB_ProcessSingle())
    {
        // add code here if calibration failed
    }

    PWM_polarity_set();

    bsp_DeadTime_Config();       // 死区时间配置
    bsp_phase_config();          // 相位同步配置
    bsp_hrpwm_buffer_cfg();      // 缓存配置（影子寄存器）
    bsp_hrpwm_ValidPeriod_cfg(); // 周期响应配置（重复次数）
    bsp_pwm_idle_Level();        // 指定IDLE电平
    //    HRPWM_IDLE_Exit(HRPWM_PWM_SYNC_UNIT, HRPWM_SW_SYNC_CH_ALL);  //所有通道都退出空闲
    HRPWM_CountStart(PWM_UNIT); // 启动定时器必须从HRPWM1开始，1是主通道

    HRPWM_IntEnable(PWM_UNIT, HRPWM_INT_UP_MATCH_SPECIAL_A); // 使能定时器中断
    HRPWM_IntEnable_Buf(PWM_UNIT, HRPWM_INT_UP_MATCH_SPECIAL_A);
}

extern void PWM_para_config(void);

// 设置PWMA通道占空比
// 左上 PA8 HA1
// 左下 PB13 LA1
// 右上 PA9 HA2
// 右下 PB14 LA2
void RAMFUNC bsp_pwm_set_a_duty(bsp_pwm_t *str)
{
    CM_HRPWM4->BPCNAR1 &= ~(3 << 16);
    CM_HRPWM4->BPCNBR1 &= ~(3 << 16);

    static uint8_t left_up_on_flg = 0;
    static uint8_t left_dn_on_flg = 0;
    static uint8_t right_up_on_flg = 0;
    static uint8_t right_dn_on_flg = 0;

    if (str->left_up_en == 0)
    {
        left_up_on_flg = 0;
        CM_HRPWM4->BPCNAR1 |= 2 << 16;
    }

    if (str->left_dn_en == 0)
    {
        left_dn_on_flg = 0;
        CM_HRPWM4->BPCNBR1 |= 2 << 16;
    }

    if (str->left_duty < 0.012f)
    {
        left_up_on_flg = 0;
        CM_HRPWM4->BPCNAR1 |= 2 << 16;
        if (str->left_dn_en == 1)
        {
            if (left_dn_on_flg == 0)
            {
                left_dn_on_flg = 1;
                CM_HRPWM4->HRGCMDR = (6 * 64);
            }
            else if (left_dn_on_flg == 1)
            {
                CM_HRPWM4->BPCNBR1 |= 3 << 16;
            }
        }
    }
    else if (str->left_duty > 0.988f)
    {
        left_dn_on_flg = 0;
        if (str->left_up_en == 1)
        {
            if (left_up_on_flg == 0)
            {
                left_up_on_flg = 1;
                CM_HRPWM4->HRGCMCR = CM_HRPWM4->HRPERBR - 64;
            }
            else if (left_up_on_flg == 1)
            {
                CM_HRPWM4->BPCNAR1 |= 3 << 16;
            }
        }
        CM_HRPWM4->BPCNBR1 |= 2 << 16;
    }
    else
    {
        left_up_on_flg = 0;
        left_dn_on_flg = 0;
        CM_HRPWM4->HRGCMCR = str->left_duty * CM_HRPWM4->HRPERBR - (6 * 64);
        CM_HRPWM4->HRGCMDR = str->left_duty * CM_HRPWM4->HRPERBR + (6 * 64);
    }

    CM_HRPWM5->BPCNAR1 &= ~(3 << 16);
    CM_HRPWM5->BPCNBR1 &= ~(3 << 16);

    if (str->right_up_en == 0)
    {
        right_up_on_flg = 0;
        CM_HRPWM5->BPCNAR1 |= 2 << 16;
    }

    if (str->right_dn_en == 0)
    {
        right_dn_on_flg = 0;
        CM_HRPWM5->BPCNBR1 |= 2 << 16;
    }

    if (str->right_duty < 0.012f)
    {
        right_up_on_flg = 0;
        CM_HRPWM5->BPCNAR1 |= 2 << 16;
        if (str->right_dn_en == 1)
        {
            if (right_dn_on_flg == 0)
            {
                right_dn_on_flg = 1;
                CM_HRPWM5->HRGCMDR = CM_HRPWM5->HRPERBR - 64;
            }
            else if (right_dn_on_flg == 1)
            {
                CM_HRPWM5->BPCNBR1 |= 3 << 16;
            }
        }
    }
    else if (str->right_duty > 0.988f)
    {
        right_dn_on_flg = 0;
        if (str->right_up_en == 1)
        {
            if (right_up_on_flg == 0)
            {
                right_up_on_flg = 1;
                CM_HRPWM5->HRGCMCR = (6 * 64);
            }
            else if (right_up_on_flg == 1)
            {
                CM_HRPWM5->BPCNAR1 |= 3 << 16;
            }
        }
        CM_HRPWM5->BPCNBR1 |= 2 << 16;
    }
    else
    {
        right_up_on_flg = 0;
        right_dn_on_flg = 0;
        CM_HRPWM5->HRGCMCR = (1.0f - str->right_duty) * CM_HRPWM5->HRPERBR + (6 * 64);
        CM_HRPWM5->HRGCMDR = (1.0f - str->right_duty) * CM_HRPWM5->HRPERBR - (6 * 64);
    }
}

// 设置PWMB通道占空比
// 左上 PB8 HA1
// 左下 PB9 LA1
// 右上 PB10 HA2
// 右下 PB11 LA2
void RAMFUNC bsp_pwm_set_b_duty(bsp_pwm_t *str)
{
    CM_HRPWM3->BPCNAR1 &= ~(3 << 16);
    CM_HRPWM3->BPCNBR1 &= ~(3 << 16);

    static uint8_t left_up_on_flg = 0;
    static uint8_t left_dn_on_flg = 0;
    static uint8_t right_up_on_flg = 0;
    static uint8_t right_dn_on_flg = 0;

    if (str->left_up_en == 0)
    {
        left_up_on_flg = 0;
        CM_HRPWM3->BPCNAR1 |= 2 << 16;
    }

    if (str->left_dn_en == 0)
    {
        left_dn_on_flg = 0;
        CM_HRPWM3->BPCNBR1 |= 2 << 16;
    }

    if (str->left_duty < 0.012f)
    {
        left_up_on_flg = 0;
        CM_HRPWM3->BPCNAR1 |= 2 << 16;
        if (str->left_dn_en == 1)
        {
            if (left_dn_on_flg == 0)
            {
                left_dn_on_flg = 1;
                CM_HRPWM3->HRGCMDR = CM_HRPWM3->HRPERBR - 64;
            }
            else if (left_dn_on_flg == 1)
            {
                CM_HRPWM3->BPCNBR1 |= 3 << 16;
            }
        }
    }
    else if (str->left_duty > 0.988f)
    {
        left_dn_on_flg = 0;
        if (str->left_up_en == 1)
        {
            if (left_up_on_flg == 0)
            {
                left_up_on_flg = 1;
                CM_HRPWM3->HRGCMCR = (6 * 64);
            }
            else if (left_up_on_flg == 1)
            {
                CM_HRPWM3->BPCNAR1 |= 3 << 16;
            }
        }
        CM_HRPWM3->BPCNBR1 |= 2 << 16;
    }
    else
    {
        left_up_on_flg = 0;
        left_dn_on_flg = 0;
        CM_HRPWM3->HRGCMCR = (1.0f - str->left_duty) * CM_HRPWM3->HRPERBR + (6 * 64);
        CM_HRPWM3->HRGCMDR = (1.0f - str->left_duty) * CM_HRPWM3->HRPERBR - (6 * 64);
    }

    CM_HRPWM2->BPCNAR1 &= ~(3 << 16);
    CM_HRPWM2->BPCNBR1 &= ~(3 << 16);

    if (str->right_up_en == 0)
    {
        right_up_on_flg = 0;
        CM_HRPWM2->BPCNAR1 |= 2 << 16;
    }
    if (str->right_dn_en == 0)
    {
        right_dn_on_flg = 0;
        CM_HRPWM2->BPCNBR1 |= 2 << 16;
    }

    if (str->right_duty < 0.012f)
    {
        right_up_on_flg = 0;
        CM_HRPWM2->BPCNAR1 |= 2 << 16;
        if (str->right_dn_en == 1)
        {
            if (right_dn_on_flg == 0)
            {
                right_dn_on_flg = 1;
                CM_HRPWM2->HRGCMDR = (6 * 64);
            }
            else if (right_dn_on_flg == 1)
            {
                CM_HRPWM2->BPCNBR1 |= 3 << 16;
            }
        }
    }
    else if (str->right_duty > 0.988f)
    {
        right_dn_on_flg = 0;
        if (str->right_up_en == 1)
        {
            if (right_up_on_flg == 0)
            {
                right_up_on_flg = 1;
                CM_HRPWM2->HRGCMCR = CM_HRPWM2->HRPERBR - 64;
            }
            else if (right_up_on_flg == 1)
            {
                CM_HRPWM2->BPCNAR1 |= 3 << 16;
            }
        }
        CM_HRPWM2->BPCNBR1 |= 2 << 16;
    }
    else
    {
        right_up_on_flg = 0;
        right_dn_on_flg = 0;
        CM_HRPWM2->HRGCMCR = str->right_duty * CM_HRPWM2->HRPERBR - (6 * 64);
        CM_HRPWM2->HRGCMDR = str->right_duty * CM_HRPWM2->HRPERBR + (6 * 64);
    }
}

void RAMFUNC bsp_pwm_update_chclt2(void)
{
    //    bsp_idleA1_process(timer_pwma_chctl2);
    //    bsp_idleA2_process(timer_pwma_chctl2);
    //    bsp_idleB1_process(timer_pwmb_chctl2);
    //    bsp_idleB2_process(timer_pwmb_chctl2);

    //    bsp_A1_output_process(timer_pwma_chctl2);
    //    bsp_A2_output_process(timer_pwma_chctl2);
    //    bsp_B1_output_process(timer_pwmb_chctl2);
    //    bsp_B2_output_process(timer_pwmb_chctl2);

    Set_channelA_outen(timer_pwma_chctl2);
    Set_channelB_outen(timer_pwmb_chctl2);

    HRPWM_IDLE_Exit_32bit(SW_PWM_UNIT, HRPWM_SW_SYNC_CHA); // 退出IDLE状态
    HRPWM_IDLE_Exit_32bit(SW_PWM_UNIT, HRPWM_SW_SYNC_CHB); // 退出IDLE状态
}

// 设定PWM1的重载值为全频率
void RAMFUNC bsp_pwm_change_freq_pwma(void)
{
    HRPWM_SetPeriodValue_Buf(PWM_UNIT, CTRL_PERIOD * 4);
}

// 设定PWMA的重载值为全频率
void RAMFUNC bsp_pwm_change_full_freq_pwma(void)
{
    HRPWM_SetPeriodValue_Buf(A1_PWM_UNIT, CTRL_PERIOD);
    HRPWM_SetPeriodValue_Buf(A2_PWM_UNIT, CTRL_PERIOD);
}

// 设定PWMB的重载值为全频率
void RAMFUNC bsp_pwm_change_full_freq_pwmb(void)
{
    HRPWM_SetPeriodValue_Buf(B1_PWM_UNIT, CTRL_PERIOD);
    HRPWM_SetPeriodValue_Buf(B2_PWM_UNIT, CTRL_PERIOD);
}

// 设定PWMA的重载值为半频率
void RAMFUNC bsp_pwm_change_half_freq_pwma(void)
{
    HRPWM_SetPeriodValue_Buf(A1_PWM_UNIT, ((CTRL_PERIOD) << 1));
    HRPWM_SetPeriodValue_Buf(A2_PWM_UNIT, ((CTRL_PERIOD) << 1));
}

// 设定PWMB的重载值为半频率
void RAMFUNC bsp_pwm_change_half_freq_pwmb(void)
{
    HRPWM_SetPeriodValue_Buf(B1_PWM_UNIT, ((CTRL_PERIOD) << 1));
    HRPWM_SetPeriodValue_Buf(B2_PWM_UNIT, ((CTRL_PERIOD) << 1));
}

void RAMFUNC bsp_hrpwm1_update_trig(void)
{
    HRPWM_U1SingleTransBufTrigger(); // 触发U1单次缓存
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// 以下代码暂时没有用到

void set_channelA_inv(uint8_t set)
{
    if (set & HA1_INV_BIT)
    {
        bCM_HRPWM4->BGCONR1_b.INVCAEN = 1;
    }
    else
    {
        bCM_HRPWM4->BGCONR1_b.INVCAEN = 0;
    }
    if (set & LA1_INV_BIT)
    {
        bCM_HRPWM4->BGCONR1_b.INVCBEN = 1;
    }
    else
    {
        bCM_HRPWM4->BGCONR1_b.INVCBEN = 0;
    }
    if (set & HA2_INV_BIT)
    {
        bCM_HRPWM5->BGCONR1_b.INVCBEN = 1;
    }
    else
    {
        bCM_HRPWM5->BGCONR1_b.INVCBEN = 0;
    }
    if (set & LA2_INV_BIT)
    {
        bCM_HRPWM5->BGCONR1_b.INVCAEN = 1;
    }
    else
    {
        bCM_HRPWM5->BGCONR1_b.INVCAEN = 0;
    }
}

void set_channelB_inv(uint8_t set)
{
    if (set & HB1_INV_BIT)
    {
        bCM_HRPWM1->BGCONR1_b.INVCAEN = 1;
    }
    else
    {
        bCM_HRPWM1->BGCONR1_b.INVCAEN = 0;
    }
    if (set & LB1_INV_BIT)
    {
        bCM_HRPWM1->BGCONR1_b.INVCBEN = 1;
    }
    else
    {
        bCM_HRPWM1->BGCONR1_b.INVCBEN = 0;
    }
    if (set & HB2_INV_BIT)
    {
        bCM_HRPWM2->BGCONR1_b.INVCAEN = 1;
    }
    else
    {
        bCM_HRPWM2->BGCONR1_b.INVCAEN = 0;
    }
    if (set & LB2_INV_BIT)
    {
        bCM_HRPWM2->BGCONR1_b.INVCBEN = 1;
    }
    else
    {
        bCM_HRPWM2->BGCONR1_b.INVCBEN = 0;
    }
}

// 获取A1,A2通道的输出状态
// 1使能 0不使能
// BIT0: HA1   PWM4_A
// BIT1: LA1   PWM4_B
// BIT2: HA2   PWM5_B  //注意此处硬件PWMA/B和H/L是反的
// BIT3: LA2   PWM5_A
uint8_t get_channelA_outen(void)
{
    uint8_t temp = 0;
    if (bCM_HRPWM4->BPCNAR1_b.OUTENA)
    {
        temp |= HA1_EN_BIT;
    }
    if (bCM_HRPWM4->BPCNBR1_b.OUTENB)
    {
        temp |= LA1_EN_BIT;
    }

    if (bCM_HRPWM5->BPCNAR1_b.OUTENA)
    {
        temp |= LA2_EN_BIT;
    }
    if (bCM_HRPWM5->BPCNBR1_b.OUTENB)
    {
        temp |= HA2_EN_BIT;
    }

    if (bCM_HRPWM4->BGCONR1_b.INVCAEN)
    {
        temp |= HA1_INV_BIT;
    }
    if (bCM_HRPWM4->BGCONR1_b.INVCBEN)
    {
        temp |= LA1_INV_BIT;
    }

    if (bCM_HRPWM5->BGCONR1_b.INVCAEN)
    {
        temp |= LA2_INV_BIT;
    }
    if (bCM_HRPWM5->BGCONR1_b.INVCBEN)
    {
        temp |= HA2_INV_BIT;
    }

    return temp;
}

// 获取B1,B2通道的输出状态
// 1使能 0不使能
// BIT0: HB1   PWM1_A
// BIT1: LB1   PWM1_B
// BIT2: HB2   PWM2_A
// BIT3: LB2   PWM2_B
uint8_t get_channelB_outen(void)
{
    uint8_t temp = 0;
    if (bCM_HRPWM1->BPCNAR1_b.OUTENA)
    {
        temp |= HB1_EN_BIT;
    }
    if (bCM_HRPWM1->BPCNBR1_b.OUTENB)
    {
        temp |= LB1_EN_BIT;
    }

    if (bCM_HRPWM2->BPCNAR1_b.OUTENA)
    {
        temp |= HB2_EN_BIT;
    }
    if (bCM_HRPWM2->BPCNBR1_b.OUTENB)
    {
        temp |= LB2_EN_BIT;
    }

    if (bCM_HRPWM1->BGCONR1_b.INVCAEN)
    {
        temp |= HB1_INV_BIT;
    }
    if (bCM_HRPWM1->BGCONR1_b.INVCBEN)
    {
        temp |= LB1_INV_BIT;
    }

    if (bCM_HRPWM2->BGCONR1_b.INVCAEN)
    {
        temp |= HB2_INV_BIT;
    }
    if (bCM_HRPWM2->BGCONR1_b.INVCBEN)
    {
        temp |= LB2_INV_BIT;
    }

    return temp;
}

void bsp_pwm_deinit(void)
{
    GPIO_HrpwmPinCmd(HRPWM_HA1_PORT, DISABLE);
    GPIO_HrpwmPinCmd(HRPWM_LA1_PORT, DISABLE);
    GPIO_HrpwmPinCmd(HRPWM_LA2_PORT, DISABLE);
    GPIO_HrpwmPinCmd(HRPWM_HA2_PORT, DISABLE);
    GPIO_HrpwmPinCmd(HRPWM_HB1_PORT, DISABLE);
    GPIO_HrpwmPinCmd(HRPWM_LB1_PORT, DISABLE);
    GPIO_HrpwmPinCmd(HRPWM_HB2_PORT, DISABLE);
    GPIO_HrpwmPinCmd(HRPWM_LB2_PORT, DISABLE);

    HRPWM_CommonDeInit();
    HRPWM_DeInit(CM_HRPWM1);
    HRPWM_DeInit(CM_HRPWM4);
    HRPWM_DeInit(CM_HRPWM5);
    HRPWM_DeInit(CM_HRPWM3);
    HRPWM_DeInit(CM_HRPWM2);

    HRPWM_CountStop(PWM_UNIT);

    HRPWM_IntDisable(PWM_UNIT, HRPWM_INT_UP_MATCH_SPECIAL_A); // 使能定时器中断
    HRPWM_IntDisable_Buf(PWM_UNIT, HRPWM_INT_UP_MATCH_SPECIAL_A);

    FCG_Fcg2PeriphClockCmd(HRPWM_PWM_FCG, DISABLE);
}

void bsp_pwm_init(void)
{
    FCG_Fcg2PeriphClockCmd(HRPWM_PWM_FCG, ENABLE);
    hrpwm_port_init();
    HRPWM_CommonDeInit();
    HRPWM_DeInit(CM_HRPWM1);
    HRPWM_DeInit(CM_HRPWM4);
    HRPWM_DeInit(CM_HRPWM5);
    HRPWM_DeInit(CM_HRPWM3);
    HRPWM_DeInit(CM_HRPWM2);

    if (LL_OK != HRPWM_CALIB_ProcessSingle())
    {
        // add code here if calibration failed
    }

    CM_HRPWM1->CR |= 1 << 3; /* 1：使能高精度HRPWM */
    CM_HRPWM2->CR |= 1 << 3; /* 1：使能高精度HRPWM */
    CM_HRPWM3->CR |= 1 << 3; /* 1：使能高精度HRPWM */
    CM_HRPWM4->CR |= 1 << 3; /* 1：使能高精度HRPWM */
    CM_HRPWM5->CR |= 1 << 3; /* 1：使能高精度HRPWM */

    CM_HRPWM1->HRPERAR = 0xFFFF * 64;
    CM_HRPWM1->HRGCMAR = (CTRL_PERIOD * 4 * 2) - 64;
    CM_HRPWM1->HCLRR2 |= 1 << 10;

    CM_HRPWM1->SCMAR = (CTRL_PERIOD * 4 * 2) - 64 - (24 << 6); // 200ns
    CM_HRPWM1->GCONR1 |= 1 << 16;                              // 触发ADC采样

    CM_HRPWM2->HRPERBR = CTRL_PERIOD - 64;
    CM_HRPWM3->HRPERBR = CTRL_PERIOD - 64;
    CM_HRPWM4->HRPERBR = CTRL_PERIOD - 64;
    CM_HRPWM5->HRPERBR = CTRL_PERIOD - 64;

    CM_HRPWM2->HCLRR2 |= 1 << 10;
    CM_HRPWM3->HCLRR2 |= 1 << 10;
    CM_HRPWM4->HCLRR2 |= 1 << 10;
    CM_HRPWM5->HCLRR2 |= 1 << 10;

    bCM_HRPWM2->BCONR1_b.BENP = 1;
    bCM_HRPWM3->BCONR1_b.BENP = 1;
    bCM_HRPWM4->BCONR1_b.BENP = 1;
    bCM_HRPWM5->BCONR1_b.BENP = 1;

    bCM_HRPWM2->BCONR1_b.BENAE = 1;
    bCM_HRPWM3->BCONR1_b.BENAE = 1;
    bCM_HRPWM4->BCONR1_b.BENAE = 1;
    bCM_HRPWM5->BCONR1_b.BENAE = 1;

    bCM_HRPWM2->BCONR1_b.BENBF = 1;
    bCM_HRPWM3->BCONR1_b.BENBF = 1;
    bCM_HRPWM4->BCONR1_b.BENBF = 1;
    bCM_HRPWM5->BCONR1_b.BENBF = 1;

    bCM_HRPWM2->BCONR2_b.BENCTL = 1;
    bCM_HRPWM3->BCONR2_b.BENCTL = 1;
    bCM_HRPWM4->BCONR2_b.BENCTL = 1;
    bCM_HRPWM5->BCONR2_b.BENCTL = 1;

    /* 设置缓存传送条件 */
    bCM_HRPWM2->BCONR2_b.BTRU0PCTL = 1; /* 控制寄存器缓存传送U0PCTL 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM2->BCONR2_b.BTRU0PAE = 1;  /* 控制寄存器缓存传送U0PAE 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM2->BCONR2_b.BTRU0PBF = 1;  /* 控制寄存器缓存传送U0PBF 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM2->BCONR2_b.BTRU0PP = 1;   /* 周期值缓存传送 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM3->BCONR2_b.BTRU0PCTL = 1; /* 控制寄存器缓存传送U0PCTL 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM3->BCONR2_b.BTRU0PAE = 1;  /* 控制寄存器缓存传送U0PAE 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM3->BCONR2_b.BTRU0PBF = 1;  /* 控制寄存器缓存传送U0PBF 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM3->BCONR2_b.BTRU0PP = 1;   /* 周期值缓存传送 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM4->BCONR2_b.BTRU0PCTL = 1; /* 控制寄存器缓存传送U0PCTL 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM4->BCONR2_b.BTRU0PAE = 1;  /* 控制寄存器缓存传送U0PAE 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM4->BCONR2_b.BTRU0PBF = 1;  /* 控制寄存器缓存传送U0PBF 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM4->BCONR2_b.BTRU0PP = 1;   /* 周期值缓存传送 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM5->BCONR2_b.BTRU0PCTL = 1; /* 控制寄存器缓存传送U0PCTL 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM5->BCONR2_b.BTRU0PAE = 1;  /* 控制寄存器缓存传送U0PAE 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM5->BCONR2_b.BTRU0PBF = 1;  /* 控制寄存器缓存传送U0PBF 1：在单元1单次缓存传送点，发生一次缓存值传送*/
    bCM_HRPWM5->BCONR2_b.BTRU0PP = 1;   /* 周期值缓存传送 1：在单元1单次缓存传送点，发生一次缓存值传送*/

    bCM_HRPWM5->BGCONR1_b.SWAPEN = 1;

    CM_HRPWM2->GCONR |= 1 << 2; /* 1：三角波模式 */
    CM_HRPWM3->GCONR |= 1 << 2; /* 1：三角波模式 */
    CM_HRPWM4->GCONR |= 1 << 2; /* 1：三角波模式 */
    CM_HRPWM5->GCONR |= 1 << 2; /* 1：三角波模式 */

    CM_HRPWM2->BPCNAR1 = 0;

    CM_HRPWM2->BPCNAR1 |= (2 << 0) |  /* 10：计数开始时，HRPWM_<t>_PWMA端口输出保持先前状态 */
                          (0 << 2) |  /* 00：计数停止时，HRPWM_<t>_PWMA端口输出设定为低电平 */
                          (2 << 4) |  /* 10：计数值等于周期值或者锯齿波硬件清零时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 6) |  /* 10：计数值等于0时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (0 << 8) |  /* 00：在向上计数期间，定时器计数值与GCMAR相等时，HRPWM_<t>_PWMA端口输出设定为低电平 */
                          (1 << 10) | /* 01：在向下计数期间，定时器计数值与GCMAR相等时，HRPWM_<t>_PWMA端口输出设定为高电平 */
                          (2 << 12) | /* 10：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 14) | /* 10：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 16) | /* 10：下周期开始，HRPWM_<t>_PWMA端口输出设定为低电平 注2：该寄存器位可用于实现PWM输出占空比0%或100%的控制 */
                          (0 << 20) | /* 00：被选择的通道发生EMB事件时，HRPWM_<t>_PWMA端口正常输出 */
                          (0 << 22) | /* 00：被选择的通道EMB事件无效时，立即释放HRPWM_<t>_PWMA端口（One Shot） */
                          (0 << 24) | /* 000：选择EMB事件通道0有效 */
                          (1 << 28)   /* 0：HRPWM功能时的HRPWM_<t>_PWMA端口输出无效 */
        ;

    CM_HRPWM2->BPCNBR1 = 0;

    CM_HRPWM2->BPCNBR1 |= (2 << 0) |  /* 10：计数开始时，HRPWM_<t>_PWMB端口输出保持先前状态 */
                          (0 << 2) |  /* 00：计数停止时，HRPWM_<t>_PWMB端口输出设定为低电平 */
                          (2 << 4) |  /* 10：计数值等于周期值或者锯齿波硬件清零时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 6) |  /* 10：计数值等于0时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 8) |  /* 00：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 10) | /* 01：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (1 << 12) | /* 10：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出设定为高电平 */
                          (0 << 14) | /* 10：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出设定为低电平 */
                          (2 << 16) | /* 10：下周期开始，HRPWM_<t>_PWMB端口输出设定为低电平 注2：该寄存器位可用于实现PWM输出占空比0%或100%的控制 */
                          (0 << 20) | /* 00：被选择的通道发生EMB事件时，HRPWM_<t>_PWMB端口正常输出 */
                          (0 << 22) | /* 00：被选择的通道EMB事件无效时，立即释放HRPWM_<t>_PWMB端口（One Shot） */
                          (0 << 24) | /* 000：选择EMB事件通道0有效 */
                          (1 << 28)   /* 0：HRPWM功能时的HRPWM_<t>_PWMB端口输出无效 */
        ;

    CM_HRPWM3->BPCNAR1 = 0;

    CM_HRPWM3->BPCNAR1 |= (2 << 0) |  /* 10：计数开始时，HRPWM_<t>_PWMA端口输出保持先前状态 */
                          (0 << 2) |  /* 00：计数停止时，HRPWM_<t>_PWMA端口输出设定为低电平 */
                          (2 << 4) |  /* 10：计数值等于周期值或者锯齿波硬件清零时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 6) |  /* 10：计数值等于0时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (1 << 8) |  /* 00：在向上计数期间，定时器计数值与GCMAR相等时，HRPWM_<t>_PWMA端口输出设定为高电平 */
                          (0 << 10) | /* 01：在向下计数期间，定时器计数值与GCMAR相等时，HRPWM_<t>_PWMA端口输出设定为低电平 */
                          (2 << 12) | /* 10：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 14) | /* 10：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 16) | /* 10：下周期开始，HRPWM_<t>_PWMA端口输出设定为低电平 注2：该寄存器位可用于实现PWM输出占空比0%或100%的控制 */
                          (0 << 20) | /* 00：被选择的通道发生EMB事件时，HRPWM_<t>_PWMA端口正常输出 */
                          (0 << 22) | /* 00：被选择的通道EMB事件无效时，立即释放HRPWM_<t>_PWMA端口（One Shot） */
                          (0 << 24) | /* 000：选择EMB事件通道0有效 */
                          (1 << 28)   /* 0：HRPWM功能时的HRPWM_<t>_PWMA端口输出无效 */
        ;

    CM_HRPWM3->BPCNBR1 = 0;

    CM_HRPWM3->BPCNBR1 |= (2 << 0) |  /* 10：计数开始时，HRPWM_<t>_PWMB端口输出保持先前状态 */
                          (0 << 2) |  /* 00：计数停止时，HRPWM_<t>_PWMB端口输出设定为低电平 */
                          (2 << 4) |  /* 10：计数值等于周期值或者锯齿波硬件清零时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 6) |  /* 10：计数值等于0时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 8) |  /* 00：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 10) | /* 01：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (0 << 12) | /* 10：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出设定为低电平 */
                          (1 << 14) | /* 10：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出设定为高电平 */
                          (2 << 16) | /* 10：下周期开始，HRPWM_<t>_PWMB端口输出设定为低电平 注2：该寄存器位可用于实现PWM输出占空比0%或100%的控制 */
                          (0 << 20) | /* 00：被选择的通道发生EMB事件时，HRPWM_<t>_PWMB端口正常输出 */
                          (0 << 22) | /* 00：被选择的通道EMB事件无效时，立即释放HRPWM_<t>_PWMB端口（One Shot） */
                          (0 << 24) | /* 000：选择EMB事件通道0有效 */
                          (1 << 28)   /* 0：HRPWM功能时的HRPWM_<t>_PWMB端口输出无效 */
        ;

    CM_HRPWM4->BPCNAR1 = 0;

    CM_HRPWM4->BPCNAR1 |= (2 << 0) |  /* 10：计数开始时，HRPWM_<t>_PWMA端口输出保持先前状态 */
                          (0 << 2) |  /* 00：计数停止时，HRPWM_<t>_PWMA端口输出设定为低电平 */
                          (2 << 4) |  /* 10：计数值等于周期值或者锯齿波硬件清零时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 6) |  /* 10：计数值等于0时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (0 << 8) |  /* 00：在向上计数期间，定时器计数值与GCMAR相等时，HRPWM_<t>_PWMA端口输出设定为低电平 */
                          (1 << 10) | /* 01：在向下计数期间，定时器计数值与GCMAR相等时，HRPWM_<t>_PWMA端口输出设定为高电平 */
                          (2 << 12) | /* 10：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 14) | /* 10：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 16) | /* 10：下周期开始，HRPWM_<t>_PWMA端口输出设定为低电平 注2：该寄存器位可用于实现PWM输出占空比0%或100%的控制 */
                          (0 << 20) | /* 00：被选择的通道发生EMB事件时，HRPWM_<t>_PWMA端口正常输出 */
                          (0 << 22) | /* 00：被选择的通道EMB事件无效时，立即释放HRPWM_<t>_PWMA端口（One Shot） */
                          (0 << 24) | /* 000：选择EMB事件通道0有效 */
                          (1 << 28)   /* 0：HRPWM功能时的HRPWM_<t>_PWMA端口输出无效 */
        ;

    CM_HRPWM4->BPCNBR1 = 0;

    CM_HRPWM4->BPCNBR1 |= (2 << 0) |  /* 10：计数开始时，HRPWM_<t>_PWMB端口输出保持先前状态 */
                          (0 << 2) |  /* 00：计数停止时，HRPWM_<t>_PWMB端口输出设定为低电平 */
                          (2 << 4) |  /* 10：计数值等于周期值或者锯齿波硬件清零时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 6) |  /* 10：计数值等于0时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 8) |  /* 00：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 10) | /* 01：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (1 << 12) | /* 10：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出设定为高电平 */
                          (0 << 14) | /* 10：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出设定为低电平 */
                          (2 << 16) | /* 10：下周期开始，HRPWM_<t>_PWMB端口输出设定为低电平 注2：该寄存器位可用于实现PWM输出占空比0%或100%的控制 */
                          (0 << 20) | /* 00：被选择的通道发生EMB事件时，HRPWM_<t>_PWMB端口正常输出 */
                          (0 << 22) | /* 00：被选择的通道EMB事件无效时，立即释放HRPWM_<t>_PWMB端口（One Shot） */
                          (0 << 24) | /* 000：选择EMB事件通道0有效 */
                          (1 << 28)   /* 0：HRPWM功能时的HRPWM_<t>_PWMB端口输出无效 */
        ;

    CM_HRPWM5->BPCNAR1 = 0;

    CM_HRPWM5->BPCNAR1 |= (2 << 0) |  /* 10：计数开始时，HRPWM_<t>_PWMA端口输出保持先前状态 */
                          (0 << 2) |  /* 00：计数停止时，HRPWM_<t>_PWMA端口输出设定为低电平 */
                          (2 << 4) |  /* 10：计数值等于周期值或者锯齿波硬件清零时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 6) |  /* 10：计数值等于0时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (1 << 8) |  /* 00：在向上计数期间，定时器计数值与GCMAR相等时，HRPWM_<t>_PWMA端口输出设定为高电平 */
                          (0 << 10) | /* 01：在向下计数期间，定时器计数值与GCMAR相等时，HRPWM_<t>_PWMA端口输出设定为低电平 */
                          (2 << 12) | /* 10：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 14) | /* 10：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMA端口输出不受影响 */
                          (2 << 16) | /* 10：下周期开始，HRPWM_<t>_PWMA端口输出设定为低电平 注2：该寄存器位可用于实现PWM输出占空比0%或100%的控制 */
                          (0 << 20) | /* 00：被选择的通道发生EMB事件时，HRPWM_<t>_PWMA端口正常输出 */
                          (0 << 22) | /* 00：被选择的通道EMB事件无效时，立即释放HRPWM_<t>_PWMA端口（One Shot） */
                          (0 << 24) | /* 000：选择EMB事件通道0有效 */
                          (1 << 28)   /* 0：HRPWM功能时的HRPWM_<t>_PWMA端口输出无效 */
        ;

    CM_HRPWM5->BPCNBR1 = 0;

    CM_HRPWM5->BPCNBR1 |= (2 << 0) |  /* 10：计数开始时，HRPWM_<t>_PWMB端口输出保持先前状态 */
                          (0 << 2) |  /* 00：计数停止时，HRPWM_<t>_PWMB端口输出设定为低电平 */
                          (2 << 4) |  /* 10：计数值等于周期值或者锯齿波硬件清零时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 6) |  /* 10：计数值等于0时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 8) |  /* 00：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (2 << 10) | /* 01：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出不受影响 */
                          (0 << 12) | /* 10：在向上计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出设定为低电平 */
                          (1 << 14) | /* 10：在向下计数期间，定时器计数值与GCMBR相等时，HRPWM_<t>_PWMB端口输出设定为高电平 */
                          (2 << 16) | /* 10：下周期开始，HRPWM_<t>_PWMB端口输出设定为低电平 注2：该寄存器位可用于实现PWM输出占空比0%或100%的控制 */
                          (0 << 20) | /* 00：被选择的通道发生EMB事件时，HRPWM_<t>_PWMB端口正常输出 */
                          (0 << 22) | /* 00：被选择的通道EMB事件无效时，立即释放HRPWM_<t>_PWMB端口（One Shot） */
                          (0 << 24) | /* 000：选择EMB事件通道0有效 */
                          (1 << 28)   /* 0：HRPWM功能时的HRPWM_<t>_PWMB端口输出无效 */
        ;

    CM_HRPWM2->HRGCMCR = 0;
    CM_HRPWM4->HRGCMCR = 0;

    CM_HRPWM3->HRGCMCR = CM_HRPWM3->HRPERBR;
    CM_HRPWM5->HRGCMCR = CM_HRPWM3->HRPERBR;

    CM_HRPWM2->GCONR |= 1 << 2; /* 1：三角波模式 */

    CM_HRPWM3->GCONR |= 1 << 2; /* 1：三角波模式 */

    CM_HRPWM4->GCONR |= 1 << 2; /* 1：三角波模式 */

    CM_HRPWM5->GCONR |= 1 << 2; /* 1：三角波模式 */

    // bsp_DeadTime_Config(); // 死区时间配置

    CM_HRPWM_COMMON->SSTARUNR2 |= 0xFFFF << 2;
    CM_HRPWM_COMMON->SSTARUNR1 |= 0xFFFF << 2;

    CM_HRPWM1->ICONR |= 1 << 7; /* 1：计数值等于0时，该中断使能 */

    NVIC_ClearPendingIRQ(HRPWM_1_OVF_UDF_IRQn);
    NVIC_SetPriority(HRPWM_1_OVF_UDF_IRQn, DDL_IRQ_PRIO_00);
    NVIC_EnableIRQ(HRPWM_1_OVF_UDF_IRQn);

    bCM_HRPWM_COMMON->GBCONR_b.OSTENU1 = 1; /* 1：单元1单次缓存使能 */

    bCM_HRPWM2->GCONR_b.START = 1;
    bCM_HRPWM3->GCONR_b.START = 1;
    bCM_HRPWM4->GCONR_b.START = 1;
    bCM_HRPWM5->GCONR_b.START = 1;
    bCM_HRPWM1->GCONR_b.START = 1;

    volatile uint32_t i = 1000;
    while (i--)
        ;

    bCM_HRPWM1->HCLRR1_b.CLES = 1;
    bCM_HRPWM2->HCLRR1_b.CLES = 1;
    bCM_HRPWM3->HCLRR1_b.CLES = 1;
    bCM_HRPWM4->HCLRR1_b.CLES = 1;
    bCM_HRPWM5->HCLRR1_b.CLES = 1;
}
