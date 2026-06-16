#ifndef __BSP_IIC_H
#define __BSP_IIC_H

#include "gd32f30x.h"

#define I2C_SPEED               400000
#define I2CX_SLAVE_ADDRESS7     0xA0

#define I2CX                    I2C1
#define RCU_GPIO_I2C            RCU_GPIOB
#define RCU_I2C                 RCU_I2C1
#define I2C_SCL_PORT            GPIOB
#define I2C_SDA_PORT            GPIOB
#define I2C_SCL_PIN             GPIO_PIN_10
#define I2C_SDA_PIN             GPIO_PIN_11

#define Write_SUCCEED           0
#define Write_TIMEOUT_FAULT     1

#define Read_SUCCEED            0
#define Read_TIMEOUT_FAULT      1


void bsp_iic_init(void);
uint8_t IICx_Write_Byte(uint32_t i2c_periph,uint32_t Address,uint8_t* ndata,uint32_t size,uint32_t Timeout);
uint8_t IICx_Read_Byte(uint32_t i2c_periph,uint32_t Address,uint8_t* ndata,uint32_t size,uint32_t Timeout);
#endif

