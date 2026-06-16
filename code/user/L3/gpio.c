#include "gpio.h"
#include "bsp_gpio.h"
#include "section.h"

void gpio_set_auxoff(uint8_t val)
{
    bsp_gpio_set_bit(PIN_AUXOFF, val);
}

void gpio_set_lg(uint8_t val)
{
    #if (LED_ON_OFF == 1)
        bsp_gpio_set_bit(PIN_LG, val);
    #endif
}

void gpio_set_lr(uint8_t val)
{
    #if (LED_ON_OFF == 1)
        bsp_gpio_set_bit(PIN_LR, val);
    #endif
}

void gpio_set_led1_green(uint8_t val)
{
    bsp_gpio_set_bit(PIN_LED1, val);
}

void gpio_set_led2_red(uint8_t val)
{
    bsp_gpio_set_bit(PIN_LED2, val);
}

void gpio_set_led3_green(uint8_t val)
{
    bsp_gpio_set_bit(PIN_LED3, val);
}

uint8_t gpio_get_pg(void)
{
    uint8_t temp = 0;
    bsp_gpio_get_bit(PIN_PG_EN, &temp);
    return temp;
}


void gpio_set_re(uint8_t val)
{
    bsp_gpio_set_bit(PIN_RE, val);
}

uint8_t bsp_get_addrs(void)
{
    static uint8_t Value = 0;
    bsp_gpio_get_bit(PIN_ADDRS, &Value);
    return Value;
}

uint8_t mos_g150_flag = 0;
uint8_t mos_g300_flag = 0;
void mos_on_off_G150(uint8_t model)
{
    #if (LED_ON_OFF == 0)
        if(model){// G150
            bsp_gpio_set_bit(PIN_LG, 1);
            mos_g150_flag = 1;
        }else{
            bsp_gpio_set_bit(PIN_LG, 0);
            mos_g150_flag = 0;
        }
    #endif
}

void mos_on_off_G300(uint8_t model)
{
    #if (LED_ON_OFF == 0)
    if(model){// G300
        bsp_gpio_set_bit(PIN_LR, 1);
        mos_g300_flag = 1;
    }else{
        bsp_gpio_set_bit(PIN_LR, 0);
        mos_g300_flag = 0;
    }
    #endif
}



static uint8_t key_state = 1;
void key_pg(void)
{
	static uint16_t key_up = 0;
	static uint16_t key_du = 0;
	if(gpio_get_pg() == 1){
		if(++key_up >= 30){
            key_up = 0;
			key_state = 1;
		}
        key_du = 0;
	}else{
		if(++key_du >= 30){
            key_du = 0;
			key_state = 0;
		}
        key_up = 0;
	}
}

uint8_t get_key_pg_val(void)
{
	return key_state;
}

REG_TASK(10, key_pg)


