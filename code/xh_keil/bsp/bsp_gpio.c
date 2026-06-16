#include "bsp_gpio.h"  

const bsp_gpio_parm_t bsp_gpio_parm_table[] = {
    GPIO_REG_PARM(PIN_TEMP3,       GPIO_PORT_C, 00, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_ACCVS,       GPIO_PORT_C, 01, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_ADDRS,       GPIO_PORT_C, 02, IPD,            0),
    GPIO_REG_PARM(PIN_ILV,         GPIO_PORT_C, 03, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_FVS48,       GPIO_PORT_A, 00, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_RVS12,       GPIO_PORT_A, 01, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_TEMP1,       GPIO_PORT_A, 02, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_TEMP2,       GPIO_PORT_A, 03, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_ILA,         GPIO_PORT_A, 04, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_ILB,         GPIO_PORT_A, 05, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_IHV,         GPIO_PORT_A, 06, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_VCC_8VD,     GPIO_PORT_C, 04, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_INTC25K,     GPIO_PORT_C, 05, OUT_PP,         0),
    GPIO_REG_PARM(PIN_RMT,         GPIO_PORT_B, 01, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_I2_SCL,      GPIO_PORT_B, 10, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_I2_SDA,      GPIO_PORT_B, 11, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_BUCK_PWMH_A, GPIO_PORT_B, 12, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_BUCK_PWML_A, GPIO_PORT_B, 13, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_BOOST_PWML_A,GPIO_PORT_B, 14, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_BOOST_PWMH_A,GPIO_PORT_B, 15, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_LED2,        GPIO_PORT_C, 06, OUT_PP,         1),
    GPIO_REG_PARM(PIN_LED3,        GPIO_PORT_C, 07, OUT_PP,         1),
    GPIO_REG_PARM(PIN_LG,          GPIO_PORT_C, 08, OUT_PP,         0),
    GPIO_REG_PARM(PIN_LR,          GPIO_PORT_C, 09, OUT_PP,         0),
    GPIO_REG_PARM(PIN_BOOST_PWMH_B,GPIO_PORT_A, 08, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_BOOST_PWML_B,GPIO_PORT_A, 09, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_BUCK_PWMH_B, GPIO_PORT_A, 10, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_BUCK_PWML_B, GPIO_PORT_A, 11, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_PG_EN,       GPIO_PORT_A, 15, IPD,            0),
    GPIO_REG_PARM(PIN_LED1,        GPIO_PORT_A, 12, OUT_PP,         1),    //PA12
    GPIO_REG_PARM(PIN_TX_BLE,      GPIO_PORT_C, 10, DEFAULT_OUT_PP, 1),
    GPIO_REG_PARM(PIN_RX_BLE,      GPIO_PORT_C, 11, DEFAULT_IPD,    1),
    GPIO_REG_PARM(PIN_AUXOFF,      GPIO_PORT_C, 12, OUT_PP,         0),
    GPIO_REG_PARM(PIN_POFF,        GPIO_PORT_D, 02, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_LIST_RX,     GPIO_PORT_B, 03, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_LIST_TX,     GPIO_PORT_B, 04, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_RE,          GPIO_PORT_B, 05, OUT_PP,         0),
    GPIO_REG_PARM(PIN_USART0_TX,   GPIO_PORT_B, 06, DEFAULT_OUT_PP, 0),
    GPIO_REG_PARM(PIN_USART0_RX,   GPIO_PORT_B, 07, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_CAN0_RX,     GPIO_PORT_B, 08, DEFAULT_IPD,    0),
    GPIO_REG_PARM(PIN_CAN0_TX,     GPIO_PORT_B, 09, DEFAULT_OUT_PP, 0),
};


void bsp_gpio_init(void)
{
    uint8_t gpio_num = 0;
    gpio_num = sizeof(bsp_gpio_parm_table) / sizeof(bsp_gpio_parm_t);
    uint8_t i;

    stc_gpio_init_t stcGpioInit;
    (void)GPIO_StructInit(&stcGpioInit);

    //rcu_periph_clock_enable(RCU_AF);
    //gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
    GPIO_SetDebugPort(GPIO_PIN_DEBUG_JTAG,DISABLE);
    for (i = 0; i < gpio_num; i++)
    {
        //rcu_periph_clock_enable(bsp_gpio_parm_table[i].rcu_periph);

        if(bsp_gpio_parm_table[i].def_lv == 0)
        {
            stcGpioInit.u16PinState = PIN_STAT_RST;
        }
        else
        {
            stcGpioInit.u16PinState = PIN_STAT_SET;
        }

        switch (bsp_gpio_parm_table[i].mode)
        {
            case GPIO_MODE_OUT_PP:
                stcGpioInit.u16PinDir = PIN_DIR_OUT;
                stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
                stcGpioInit.u16PinOutputType = PIN_OUT_TYPE_CMOS;
                break;
            case GPIO_MODE_IPD:
                stcGpioInit.u16PinDir =   PIN_DIR_IN;
                stcGpioInit.u16PullUp = PIN_PU_ON;
                stcGpioInit.u16PullDown = PIN_PD_OFF;
                break;
            case GPIO_MODE_DEFAULT_OUT_PP:
                stcGpioInit.u16PinDir         = PIN_DIR_IN;
                stcGpioInit.u16PinDrv         = PIN_LOW_DRV;
                stcGpioInit.u16PinAttr        = PIN_ATTR_DIGITAL;
                stcGpioInit.u16PullDown       = PIN_PD_OFF;
                stcGpioInit.u16InputMos       = PIN_IN_MOS_OFF;
                stcGpioInit.u16Latch          = PIN_LATCH_OFF;
                stcGpioInit.u16PullUp         = PIN_PU_OFF;
                stcGpioInit.u16Invert         = PIN_INVT_OFF;
                stcGpioInit.u16ExtInt         = PIN_EXTINT_OFF;
                stcGpioInit.u16PinOutputType  = PIN_OUT_TYPE_CMOS;
                stcGpioInit.u16PinInputType   = PIN_IN_TYPE_SMT;
                break;
            case GPIO_MODE_DEFAULT_IPD:
                stcGpioInit.u16PinDir         = PIN_DIR_OUT;
                stcGpioInit.u16PinDrv         = PIN_LOW_DRV;
                stcGpioInit.u16PinAttr        = PIN_ATTR_DIGITAL;
                stcGpioInit.u16PullDown       = PIN_PD_OFF;
                stcGpioInit.u16InputMos       = PIN_IN_MOS_OFF;
                stcGpioInit.u16Latch          = PIN_LATCH_OFF;
                stcGpioInit.u16PullUp         = PIN_PU_OFF;
                stcGpioInit.u16Invert         = PIN_INVT_OFF;
                stcGpioInit.u16ExtInt         = PIN_EXTINT_OFF;
                stcGpioInit.u16PinOutputType  = PIN_OUT_TYPE_CMOS;
                stcGpioInit.u16PinInputType   = PIN_IN_TYPE_SMT;
                break;
            default:
                break;
        }

        GPIO_Init(bsp_gpio_parm_table[i].gpio_periph, bsp_gpio_parm_table[i].pin, &stcGpioInit);
       
    }
}


void bsp_gpio_set_bit(bsp_gpio_table_e num, uint8_t val)
{
    if (val)
    {
        GPIO_SetPins(bsp_gpio_parm_table[num].gpio_periph, bsp_gpio_parm_table[num].pin);
    }
    else
    {
        GPIO_ResetPins(bsp_gpio_parm_table[num].gpio_periph, bsp_gpio_parm_table[num].pin);
    }
}

void bsp_gpio_get_bit(bsp_gpio_table_e num, uint8_t *val)
{
    *val = GPIO_ReadInputPins(bsp_gpio_parm_table[num].gpio_periph, bsp_gpio_parm_table[num].pin);
}

