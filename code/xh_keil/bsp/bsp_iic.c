#include "bsp_iic.h"
#include "hc32_ll_aos.h"
#include "ev_hc32f334_lqfp64.h"
#include "section.h"

static uint32_t u32PageSize;
static uint32_t u32Capacity;

static int32_t BSP_24CXX_I2C_Init(void);
static void BSP_24CXX_I2C_DeInit(void);
static int32_t BSP_24CXX_I2C_WritePage(uint16_t u16Addr, const uint8_t *pu8Buf, uint32_t u32Len);
static int32_t BSP_24CXX_I2C_Read(uint16_t u16Addr, uint8_t *pu8Buf, uint32_t u32Len);
static int32_t BSP_24CXX_I2C_GetStatus(void);

static stc_24cxx_ll_t m_stc24cxxLL = {
    .u32PageSize = EE_24CXX_PAGE_SIZE,      // 页大小
    .u32Capacity = EE_24CXX_CAPACITY,       // 容量
    .Delay = DDL_DelayUS,
    .Init = BSP_24CXX_I2C_Init,             //初始化
    .DeInit = BSP_24CXX_I2C_DeInit,
    .WritePage = BSP_24CXX_I2C_WritePage,   // 写数据
    .Read = BSP_24CXX_I2C_Read,             // 读数据
    .GetStatus = BSP_24CXX_I2C_GetStatus,
};


int32_t EE_24CXX_Init(const stc_24cxx_ll_t *pstc24cxxLL)
{
    int32_t i32Ret;
		pstc24cxxLL->DeInit();
		GPIO_ResetPins(BSP_24CXX_I2C_SCL_PORT, BSP_24CXX_I2C_SCL_PIN);
		GPIO_ResetPins(BSP_24CXX_I2C_SDA_PORT, BSP_24CXX_I2C_SDA_PIN);
		pstc24cxxLL->Delay(50000UL);
        pstc24cxxLL->Delay(50000UL);
    if ((pstc24cxxLL == NULL) || (pstc24cxxLL->u32PageSize == 0U) || (pstc24cxxLL->u32Capacity == 0U) ||
        (pstc24cxxLL->Init == NULL)) {
        i32Ret = LL_ERR_INVD_PARAM;
    } else {
        u32PageSize = pstc24cxxLL->u32PageSize;
        u32Capacity = pstc24cxxLL->u32Capacity;
        i32Ret = pstc24cxxLL->Init();
    }
    return i32Ret;
}

static void BSP_24CXX_I2C_DeInit(void)
{
    /* Initialize I2C port*/
//    GPIO_REG_Unlock();
    GPIO_SetFunc(BSP_24CXX_I2C_SCL_PORT, BSP_24CXX_I2C_SCL_PIN, GPIO_FUNC_0);
    GPIO_SetFunc(BSP_24CXX_I2C_SDA_PORT, BSP_24CXX_I2C_SDA_PIN, GPIO_FUNC_0);
//    GPIO_REG_Lock();
    (void)I2C_DeInit(BSP_24CXX_I2C_UNIT);
}

static int32_t BSP_24CXX_I2C_WritePage(uint16_t u16Addr, const uint8_t *pu8Buf, uint32_t u32Len)
{
    uint16_t u16MemAddrTemp;
#if (EE_24CXX_MEM_ADDR_LEN == 1U)
    u16MemAddrTemp = u16Addr;
#else
    u16MemAddrTemp = (uint16_t)((((uint32_t)u16Addr >> 8) & 0xFFUL) + (((uint32_t)u16Addr << 8) & 0xFF00UL));
#endif
    return BSP_I2C_Write(BSP_24CXX_I2C_UNIT, EE_24CXX_DEV_ADDR, (const uint8_t *)&u16MemAddrTemp, EE_24CXX_MEM_ADDR_LEN, pu8Buf, u32Len);
}

static int32_t BSP_24CXX_I2C_Read(uint16_t u16Addr, uint8_t *pu8Buf, uint32_t u32Len)
{
    uint16_t u16MemAddrTemp;
#if (EE_24CXX_MEM_ADDR_LEN == 1U)
    u16MemAddrTemp = u16Addr;
#else
    u16MemAddrTemp = (uint16_t)((((uint32_t)u16Addr >> 8) & 0xFFUL) + (((uint32_t)u16Addr << 8) & 0xFF00UL));
#endif
    return BSP_I2C_Read(BSP_24CXX_I2C_UNIT, EE_24CXX_DEV_ADDR, (const uint8_t *)&u16MemAddrTemp, EE_24CXX_MEM_ADDR_LEN, pu8Buf, u32Len);
}

static int32_t BSP_24CXX_I2C_GetStatus(void)
{
    return BSP_I2C_GetDevStatus(BSP_24CXX_I2C_UNIT, EE_24CXX_DEV_ADDR);
}

static int32_t BSP_24CXX_I2C_Init(void)
{
    stc_gpio_init_t stcGpioInit;
    /* Configuration I2C GPIO */
//    GPIO_REG_Unlock();
    (void)GPIO_StructInit(&stcGpioInit);
    (void)GPIO_Init(BSP_24CXX_I2C_SCL_PORT, BSP_24CXX_I2C_SCL_PIN, &stcGpioInit);
    (void)GPIO_Init(BSP_24CXX_I2C_SDA_PORT, BSP_24CXX_I2C_SDA_PIN, &stcGpioInit);
    GPIO_SetFunc(BSP_24CXX_I2C_SCL_PORT, BSP_24CXX_I2C_SCL_PIN, BSP_24CXX_I2C_SCL_FUNC);
    GPIO_SetFunc(BSP_24CXX_I2C_SDA_PORT, BSP_24CXX_I2C_SDA_PIN, BSP_24CXX_I2C_SDA_FUNC);
//    GPIO_REG_Lock();
    /* Enable I2C Peripheral*/
    FCG_Fcg1PeriphClockCmd(BSP_24CXX_I2C_FCG, ENABLE);
    return BSP_I2C_Init(BSP_24CXX_I2C_UNIT);
}


void bsp_iic_init(void)
{
    (void)EE_24CXX_Init(&m_stc24cxxLL);
}

int32_t EE_24CXX_Read(const stc_24cxx_ll_t *pstc24cxxLL, uint16_t u16Addr, uint8_t *pu8Buf, uint32_t u32Len)
{
    int32_t i32Ret;

    if (((u16Addr + u32Len) > u32Capacity) || (pstc24cxxLL == NULL) || (pstc24cxxLL->Read == NULL) ||
        (pu8Buf == NULL)) {
        i32Ret = LL_ERR_INVD_PARAM;
    } else {
        i32Ret = pstc24cxxLL->Read(u16Addr, pu8Buf, u32Len);
    }
    return i32Ret;
}


int32_t EE_24CXX_Write(const stc_24cxx_ll_t *pstc24cxxLL, uint16_t u16Addr, const uint8_t *pu8Buf, uint32_t u32Len)
{
    uint32_t u32PageNum;
    uint8_t u8SingleNumStart;
    uint8_t u8SingleNumEnd;
    uint32_t u32NumRemainTemp = u32Len;
    uint32_t u32WriteOffset = 0UL;
    uint16_t u16WriteAddrTemp = u16Addr;
    int32_t i32Ret = LL_OK;
    uint32_t i;

    if (((u16Addr + u32Len) > u32Capacity) || (u32PageSize == 0U) || (pstc24cxxLL == NULL) ||
        (pstc24cxxLL->WritePage == NULL) || (pstc24cxxLL->Delay == NULL) || (pu8Buf == NULL)) {
        return LL_ERR_INVD_PARAM;
    }

    /* If start write address is align with page size */
    if (0U == (u16WriteAddrTemp % u32PageSize)) {
        /* If Write number is less than page size */
        if (u32Len < u32PageSize) {
            u8SingleNumStart = (uint8_t)u32Len;
        } else {
            /* If Write number is more than page size */
            u8SingleNumStart = 0U;
        }
        u32NumRemainTemp -= (uint32_t)u8SingleNumStart;
    } else {
        /* If start write address is not align with page size */
        u8SingleNumStart = (uint8_t)(u32PageSize - (u16WriteAddrTemp % u32PageSize));
        if ((uint32_t)u8SingleNumStart > u32Len) {
            u8SingleNumStart = (uint8_t)u32Len;
        }
        u32NumRemainTemp -= (uint32_t)u8SingleNumStart;
    }

    u32PageNum = u32NumRemainTemp / u32PageSize;
    u8SingleNumEnd = (uint8_t)(u32NumRemainTemp % u32PageSize);

    if (0UL != u8SingleNumStart) {
        i32Ret = pstc24cxxLL->WritePage(u16WriteAddrTemp, &pu8Buf[u32WriteOffset], (uint32_t)u8SingleNumStart);
        /* Delay about 5ms for EEPROM */
        pstc24cxxLL->Delay(5000U);
        u16WriteAddrTemp += u8SingleNumStart;
        u32WriteOffset += (uint32_t)u8SingleNumStart;
    }

    if (LL_OK == i32Ret) {
        if (0UL != u32PageNum) {
            for (i = 0UL; i < u32PageNum; i++) {
                i32Ret = pstc24cxxLL->WritePage(u16WriteAddrTemp, &pu8Buf[u32WriteOffset], u32PageSize);
                /* Delay about 5ms for EEPROM */
                pstc24cxxLL->Delay(5000U);
                u16WriteAddrTemp += (uint16_t)u32PageSize;
                u32WriteOffset += u32PageSize;
                if (LL_OK != i32Ret) {
                    break;
                }
            }
        }

        if (LL_OK == i32Ret) {
            if (0UL != u8SingleNumEnd) {
                i32Ret = pstc24cxxLL->WritePage(u16WriteAddrTemp, &pu8Buf[u32WriteOffset], (uint32_t)u8SingleNumEnd);
                /* Delay about 5ms for EEPROM */
                pstc24cxxLL->Delay(5000U);
            }
        }
    }
    return i32Ret;
}

/**
 * @brief  24CXX wait idle.
 * @param  [in] pstc24cxxLL         Pointer to a @ref stc_24cxx_ll_t structure.
 * @retval int32_t:
 *         - LL_OK:                 Success
 *         - LL_ERR_TIMEOUT:        Failed
 *         - LL_ERR_INVD_PARAM:     Invalid parameter
 */
int32_t EE_24CXX_WaitIdle(const stc_24cxx_ll_t *pstc24cxxLL)
{
    int32_t i32Ret = LL_OK;
    volatile uint32_t u32Tmp = 0UL;

    if ((pstc24cxxLL == NULL) || (pstc24cxxLL->GetStatus == NULL)) {
        return LL_ERR_INVD_PARAM;
    }
    while (LL_OK != pstc24cxxLL->GetStatus()) {
        if (EE_24CXX_WAIT_TIMEOUT == u32Tmp++) {
            i32Ret = LL_ERR_TIMEOUT;
            break;
        }
    }
    return i32Ret;
}


void IICx_Write_Byte(uint32_t Address,uint8_t* ndata,uint32_t size)
{
    EE_24CXX_Write(&m_stc24cxxLL,Address,ndata,size);
}

void IICx_Read_Byte(uint32_t Address,uint8_t* ndata,uint32_t size)
{
    EE_24CXX_Read(&m_stc24cxxLL,Address,ndata,size);
}

void bsp_iic_deinit(void)
{
    m_stc24cxxLL.DeInit();
}


