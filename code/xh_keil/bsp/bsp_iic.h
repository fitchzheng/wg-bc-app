#ifndef __BSP_IIC_H
#define __BSP_IIC_H

#include "hc32_ll.h"
#include "hc32_ll_i2c.h"

//#define I2C_SPEED               400000
//#define I2CX_SLAVE_ADDRESS7     0xA0

//#define I2C_UNIT                CM_I2C
//#define RCU_I2C_FCG             FCG1_PERIPH_I2C

//#define I2C_SCL_PORT            GPIO_PORT_B
//#define I2C_SDA_PORT            GPIO_PORT_B
//#define I2C_SCL_PIN             GPIO_PIN_10
//#define I2C_SDA_PIN             GPIO_PIN_11
//#define I2C_SCL_SCL_FUNC       (GPIO_FUNC_53)
//#define I2C_SDA_SDA_FUNC       (GPIO_FUNC_52)


////////////////////////////////////////////////////////

#define Write_SUCCEED           0
#define Write_TIMEOUT_FAULT     1 

#define Read_SUCCEED            0
#define Read_TIMEOUT_FAULT      1


#define EE_24CXX_DEV_ADDR               (0x50U)
#define EE_24CXX_MEM_ADDR_LEN           (2U)
#define EE_24CXX_PAGE_SIZE              (128U)
#define EE_24CXX_CAPACITY               (64 * 1024UL)

/* I2C unit define */
#define BSP_24CXX_I2C_UNIT              (CM_I2C)
#define BSP_24CXX_I2C_FCG               (FCG1_PERIPH_I2C)

/* Define port and pin for SDA and SCL */
#define BSP_24CXX_I2C_SCL_PORT          (GPIO_PORT_B)
#define BSP_24CXX_I2C_SCL_PIN           (GPIO_PIN_10)
#define BSP_24CXX_I2C_SDA_PORT          (GPIO_PORT_B)
#define BSP_24CXX_I2C_SDA_PIN           (GPIO_PIN_11)
#define BSP_24CXX_I2C_SCL_FUNC          (GPIO_FUNC_53)
#define BSP_24CXX_I2C_SDA_FUNC          (GPIO_FUNC_52)

#define EE_24CXX_WAIT_TIMEOUT           (0x20000UL)
/////////////////////////////////////////////

typedef struct {
    /* Properties */
    uint32_t u32PageSize;
    uint32_t u32Capacity;
    /* Methods */
    void (*Delay)(uint32_t);
    int32_t (*Init)(void);
    void (*DeInit)(void);
    int32_t (*WritePage)(uint16_t u16Addr, const uint8_t *pu8Buf, uint32_t u32Len);
    int32_t (*Read)(uint16_t u16Addr, uint8_t *pu8Buf, uint32_t u32Len);
    int32_t (*GetStatus)(void);
} stc_24cxx_ll_t;


void bsp_iic_init(void);
void IICx_Write_Byte(uint32_t Address,uint8_t* ndata,uint32_t size);
void IICx_Read_Byte(uint32_t Address,uint8_t* ndata,uint32_t size);
void bsp_iic_deinit(void);
#endif

