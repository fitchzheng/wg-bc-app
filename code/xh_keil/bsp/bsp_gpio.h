#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__ 

#include "hc32_ll.h"
#include "hc32_ll_gpio.h"

#define GPIO_MODE_OUT_PP            0
#define GPIO_MODE_IPD               1
#define GPIO_MODE_DEFAULT_OUT_PP    2
#define GPIO_MODE_DEFAULT_IPD       3
//其他类型定义按实际需要增加



//以下定义中，speed与rcu_periph用不到，直接给0
#define GPIO_REG_PARM(_name, _gpx, _pin, _mode, _def_lv) { \
    .bsp_gpio_table = _name,                               \
    .gpio_periph = _gpx,                                   \
    .mode = GPIO_MODE_##_mode,                             \
    .speed = 0,                                            \
    .pin = GPIO_PIN_##_pin,                                \
    .rcu_periph = 0,                                       \
    .def_lv = _def_lv,                                     \
}

typedef enum
{
  PIN_CHG,
  PIN_DSG,
  PIN_TEMP3,
  PIN_ACCVS,
  PIN_ADDRS,
  PIN_ILV,
  PIN_FVS48,
  PIN_RVS12,
  PIN_TEMP1,
  PIN_TEMP2,
  PIN_ILA,
  PIN_ILB,
  PIN_IHV,
  PIN_VCC_8VD,
  PIN_INTC25K,
  PIN_RMT,
  PIN_I2_SCL,
  PIN_I2_SDA,
  PIN_BUCK_PWMH_A,
  PIN_BUCK_PWML_A,
  PIN_BOOST_PWML_A,
  PIN_BOOST_PWMH_A,
  PIN_LG,
  PIN_LR,
  PIN_BOOST_PWMH_B,
  PIN_BOOST_PWML_B, 
  PIN_BUCK_PWMH_B,
  PIN_BUCK_PWML_B,
  PIN_PG_EN,
  PIN_LED1,
  PIN_TX_BLE,
  PIN_RX_BLE,
  PIN_AUXOFF,
  PIN_POFF,
  PIN_LIST_RX,
  PIN_LIST_TX,
  PIN_RE,
  PIN_USART0_TX,
  PIN_USART0_RX,
  PIN_CAN0_RX,
  PIN_CAN0_TX,
  PIN_TEXT,
} bsp_gpio_table_e;

typedef struct
{
  bsp_gpio_table_e bsp_gpio_table;
  uint32_t gpio_periph;
  uint32_t mode;
  uint32_t speed;
  uint32_t pin;
  uint32_t rcu_periph;
  uint8_t def_lv;
} bsp_gpio_parm_t;

void bsp_gpio_init(void);
void bsp_gpio_set_bit(bsp_gpio_table_e num, uint8_t val);
void bsp_gpio_get_bit(bsp_gpio_table_e num, uint8_t *val);




#endif  

