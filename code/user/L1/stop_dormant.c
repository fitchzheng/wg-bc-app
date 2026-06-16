#include "stop_dormant.h"
#include "bsp_stop.h"
#include "wg_com_v2.h"
#include "fault.h"
#include "adc.h"
#include "gpio.h"
#include "bsp_pwm.h"
#include "bsp_dma.h"
#include "bsp_can.h"
#include "bsp_interrupt.h"
#include "bsp_iic.h"
#include "bsp_init.h"
#include "fwdgt.h"
#include "adc_check.h"
#include "ctrl_app.h"
#include "get_com_data.h"
#include "bat_charge_pattern.h"

const bsp_gpio_parm_pwc_t bsp_gpio_parm_pwc[] = {
    PWC_GPIO_REG_PARM(PWC_PC13,	GPIO_PORT_C	,	13	,	ANALOG_PD	, 	0),
	PWC_GPIO_REG_PARM(PWC_PC14,	GPIO_PORT_C	,	14	,	ANALOG_PD	, 	0),
	PWC_GPIO_REG_PARM(PWC_PC15,	GPIO_PORT_C	,	15	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PD0,	GPIO_PORT_D	,	00	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PD1,	GPIO_PORT_D	,	01	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC0,	GPIO_PORT_C	,	00	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC1,	GPIO_PORT_C	,	01	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC2,	GPIO_PORT_C	,	02	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC3,	GPIO_PORT_C	,	03	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA0,	GPIO_PORT_A	,	00	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA1,	GPIO_PORT_A	,	01	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA2,	GPIO_PORT_A	,	02	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA3,	GPIO_PORT_A	,	03	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA4,	GPIO_PORT_A	,	04	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA5,	GPIO_PORT_A	,	05	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA6,	GPIO_PORT_A	,	06	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA7,	GPIO_PORT_A	,	07	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC4,	GPIO_PORT_C	,	04	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC5,	GPIO_PORT_C	,	05	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB0,	GPIO_PORT_B	,	00	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB1,	GPIO_PORT_B	,	01	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB2,	GPIO_PORT_B	,	02	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB10,	GPIO_PORT_B	,	10	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB11,	GPIO_PORT_B	,	11	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB12,	GPIO_PORT_B	,	12	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB13,	GPIO_PORT_B	,	13	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB14,	GPIO_PORT_B	,	14	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB15,	GPIO_PORT_B	,	15	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC6,	GPIO_PORT_C	,	06	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC7,	GPIO_PORT_C	,	07	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC8,	GPIO_PORT_C	,	08	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC9,	GPIO_PORT_C	,	09	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA8,	GPIO_PORT_A	,	08	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA9,	GPIO_PORT_A	,	09	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA10,	GPIO_PORT_A	,	10	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA11,	GPIO_PORT_A	,	11	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA12,	GPIO_PORT_A	,	12	,	ANALOG_PD	,	0),

	PWC_GPIO_REG_PARM(PWC_PA13,	GPIO_PORT_A	,	13	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PA14,	GPIO_PORT_A	,	14	,	ANALOG_PD	,	0),

	PWC_GPIO_REG_PARM(PWC_PA15,	GPIO_PORT_A	,	15	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC10,	GPIO_PORT_C	,	10	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PC11,	GPIO_PORT_C	,	11	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PD2,	GPIO_PORT_D	,	02	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB3,	GPIO_PORT_B	,	03	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB4,	GPIO_PORT_B	,	04	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB5,	GPIO_PORT_B	,	05	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB6,	GPIO_PORT_B	,	06	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB7,	GPIO_PORT_B	,	07	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB8,	GPIO_PORT_B	,	08	,	ANALOG_PD	,	0),
	PWC_GPIO_REG_PARM(PWC_PB9,	GPIO_PORT_B	,	09	,	ANALOG_PD	,	0),
};

void bsp_gpio_pwc_init(void)
{
    uint8_t gpio_num = 0;
    gpio_num = sizeof(bsp_gpio_parm_pwc) / sizeof(bsp_gpio_parm_pwc_t);
    uint8_t i;

    stc_gpio_init_t stcGpioInit;
    (void)GPIO_StructInit(&stcGpioInit);

    GPIO_SetDebugPort(GPIO_PIN_SWCLK|GPIO_PIN_SWDIO,DISABLE);
    for (i = 0; i < gpio_num; i++)
    {
        stcGpioInit.u16PinDir = PIN_DIR_IN;
        stcGpioInit.u16PullUp = PIN_PU_OFF;
        stcGpioInit.u16PullDown = PIN_PD_ON;
        stcGpioInit.u16PinAttr = PIN_ATTR_ANALOG;

        GPIO_Init(bsp_gpio_parm_pwc[i].gpio_periph, bsp_gpio_parm_pwc[i].pin, &stcGpioInit);
    }
}


extern void ctrl_app_disable(void);
extern void bsp_hrpwm_init_pwc(void);
void bsp_adc_init_pwc(void);
uint8_t stop_time_flag = 0;
static uint8_t stop = 0;
static uint8_t InitFlag = 0;
uint8_t bsp_pwc_stop_rum(void)
{
    float fvs48 = 0;
    uint8_t re_state = 0;
    
    if (LL_OK == STOP_IsReady() && (stop == 1))
    {

        if(InitFlag == 0)
        {
            ctrl_app_disable();
            //关闭外设
            bsp_adc_deinit();    

            bsp_pwm_deinit();

            bsp_dma_deinit();

            bsp_can_deinit();

            bsp_interrupt_deinit();

            bsp_iic_deinit();  

            STOP_Config(); 
            
            CLK_SetClockDiv(CLK_BUS_CLK_ALL,
            (CLK_PCLK0_DIV8 | CLK_PCLK1_DIV16| CLK_PCLK2_DIV16 |
            CLK_PCLK3_DIV16 | CLK_PCLK4_DIV16 | CLK_HCLK_DIV8));  //配置主频分频 CLK_HCLK_DIV16决定主频
            CLK_SetSysClockSrc(CLK_SYSCLK_SRC_MRC); //配置时钟源 ,MRC 8M
            
            CLK_PLLCmd(DISABLE);
            TMR0_Config();
            
            SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                             SysTick_CTRL_TICKINT_Msk   |
                             SysTick_CTRL_ENABLE_Pos;                         /* Enable SysTick IRQ and SysTick Timer */
                             
            InitFlag = 1;
        }

        bsp_gpio_pwc_init();
        gpio_set_auxoff(1);
        for(uint16_t i = 0;i < 0xffff;i++)
        {
            __NOP();__NOP();__NOP();__NOP();__NOP();
            fwdgt_feed_task();
        }
        TMR0_Start(TMR0_UNIT, TMR0_CH);
        TMR0_ClearStatus(TMR0_UNIT, TMR0_CH_FLAG);
        PWC_STOP_Enter(PWC_STOP_WFE_EVT);
        fwdgt_feed_task();
        stop = 0;
        re_state = 1;
    }
    else
    {

        bsp_adc_init_pwc();
        bsp_hrpwm_init_pwc();
        for(uint16_t i = 0;i < 0xffff;i++)
        {
            __NOP();__NOP();__NOP();__NOP();__NOP();
            fwdgt_feed_task();
        }
        fwdgt_feed_task();
        WG_COM_V2_GET_DATA_UINT(fvs48, wg_com_v2_param.SetInpUvlo);
        static float pwc_fvs48 = 0;
        static float pwc_vccvs = 0;
        pwc_fvs48 = 0;
        pwc_vccvs = 0;

        uint32_t u32TimeCount = 0;
        for(uint16_t i = 0;i < 8;i++)
        {
            u32TimeCount = 0;
            do {
                if (ADC_GetStatus(CM_ADC1, ADC_FLAG_EOCA) == SET) {
                    ADC_ClearStatus(CM_ADC1, ADC_FLAG_EOCA);
                    pwc_fvs48 += get_show_fvs48_show();
                    break;
                }
                fwdgt_feed_task();
            } while (u32TimeCount++ < 1000U);
            
            u32TimeCount = 0;
            
            do {
                if (ADC_GetStatus(CM_ADC2, ADC_FLAG_EOCA) == SET) {
                    ADC_ClearStatus(CM_ADC2, ADC_FLAG_EOCA);
                    pwc_vccvs += adc_get_accvs();
                    break;
                }
                fwdgt_feed_task();
            } while (u32TimeCount++ < 1000U);
        }
        fwdgt_feed_task();
        pwc_fvs48 = pwc_fvs48 / 8;
        pwc_vccvs = pwc_vccvs / 8;

        bsp_adc_deinit();    

        bsp_pwm_deinit();
 
        if((pwc_fvs48 < (fvs48+0.5f)) || (pwc_vccvs > 9.0f))
        { 
            NVIC_SystemReset();
        }
        else
        {
            re_state = 1;
            stop = 1;
        }
    }

    return re_state;
}

void bat_stop_charge_flag(void)
{
    static uint16_t PowerMode = 0;
    static uint16_t ChargMode = 0;
    static uint16_t InpBatyType = 0;
    WG_COM_V2_GET_DATA_UINT(PowerMode, wg_com_v2_realtime_data.PowerMode);
    WG_COM_V2_GET_DATA_UINT(ChargMode, wg_com_v2_realtime_data.ChargMode);
    WG_COM_V2_GET_DATA_UINT(InpBatyType, wg_com_v2_ctrl.InpBatyType);
    static uint16_t delay = 0;
    uint16_t bat_type = Get_Charge_State();
    if((PowerMode == eSET_BAT_MODE)                                 && 
       (fault_get_fault() == 0)                                     &&
       ((charge_state_data.SetCharState == eFLOAT_CHARGE)           &&
       (get_wg_com_v2_data.com_ctrl.SleepModeOnOff == 1)            &&
       ((bat_type == eBAT_LA_AGM)||(bat_type == eBAT_LA_GEL))       &&
       (get_check_state_data() == ADDRS_BACKWARD)                   &&
       (get_wg_com_v2_data.BatModeFRState == 0)                     &&
       (ctrl_app_get_is_run() == 1)))
    {
        if(++delay >= 20)
        {
            delay = 0;
            stop_time_flag = 1;
            InitFlag = 0;
            stop = 1;
        }
    }
    else
    {
        delay = 0;
    }
}

REG_TASK(1000, bat_stop_charge_flag);

