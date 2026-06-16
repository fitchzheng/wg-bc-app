#include "flash_cfg.h"
#include "flash.h"
#include "section.h"
#include "wg_com_v2.h"
#include "string.h"
#include "ctrl_app.h"
#include "stdbool.h"

//flash_wg_com_v2_param_t flash_wg_com_v2_param;

//uint8_t flash_buffer[2048] = {0};

//void flash_cfg_init(void)
//{
//    flash_read(FLASH_CFG,
//               0,
//               (uint16_t *)&flash_buffer[0]);

//    memcpy((uint8_t *)&flash_wg_com_v2_param,
//           (uint8_t *)&flash_buffer[0],
//           sizeof(flash_wg_com_v2_param));

//    if (flash_wg_com_v2_param.is_writed != 0x5A5A)
//    {
//        flash_wg_com_v2_param.is_writed = 0x5A5A;
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.InpVoltCalibrK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.InpVoltCalibrB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.InpCurrCalibrK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.InpCurrCalibrB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.InpShowVoltCalibrK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.InpShowVoltCalibrB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.InpShowCurrCalibrK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.InpShowCurrCalibrB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.OutVoltCalibrK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.OutVoltCalibrB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.OutCurrCalibrK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.OutCurrCalibrB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.OutShowVoltCalibrK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.OutShowVoltCalibrB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.OutShowCurrCalibrK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.OutShowCurrCalibrB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.AOutShowCurrCalibrK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.AOutShowCurrCalibrB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.BOutShowCurrCalibrK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.BOutShowCurrCalibrB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.VoltCompensationAK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.VoltCompensationAB);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.VoltCompensationBK);
//        WG_COM_V2_SET_DATA(0.0f, wg_com_v2_param.VoltCompensationBB);
//        for (uint8_t i = 0; i < 2; i++)
//        {
//            wg_com_v2_param.Retain10[i] = 0xFFFF;
//        }
//        WG_COM_V2_SET_DATA(10.0f, wg_com_v2_param.SetInpVolt);
//        WG_COM_V2_SET_DATA(100.0f, wg_com_v2_param.SetInpCurr);
//        WG_COM_V2_SET_DATA(1600.0f, wg_com_v2_param.SetInpCurrPower);
//        WG_COM_V2_SET_DATA(14.6f, wg_com_v2_param.SetOutVolt);
//        WG_COM_V2_SET_DATA(100.0f, wg_com_v2_param.SetOutCurr);
//        WG_COM_V2_SET_DATA(1500.0f, wg_com_v2_param.SetOutCurrPower);
//        WG_COM_V2_SET_DATA(9.5f, wg_com_v2_param.SetInpUvlo);
//        WG_COM_V2_SET_DATA(10.5f, wg_com_v2_param.SetInpUvloRecover);
//        WG_COM_V2_SET_DATA(37.0f, wg_com_v2_param.SetInpOVP);
//        WG_COM_V2_SET_DATA(36.0f, wg_com_v2_param.SetInpOVPRecover);
//        WG_COM_V2_SET_DATA(9.5f, wg_com_v2_param.SetOutUvlo);
//        WG_COM_V2_SET_DATA(10.5f, wg_com_v2_param.SetOutUvloRecover);
//        WG_COM_V2_SET_DATA(37.0f, wg_com_v2_param.SetOutOVP);
//        WG_COM_V2_SET_DATA(36.0f, wg_com_v2_param.SetOutOVPRecover);
//        WG_COM_V2_SET_DATA(100.0f, wg_com_v2_param.SetInsideTemp);
//        WG_COM_V2_SET_DATA(100.0f, wg_com_v2_param.SetOutsideTemp);
//        WG_COM_V2_SET_DATA(1.5f, wg_com_v2_param.SetInpChargLedCurr);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.SetInpFullLedCurr);
//        WG_COM_V2_SET_DATA(1.5f, wg_com_v2_param.SetOutChargLedCurr);
//        WG_COM_V2_SET_DATA(1.0f, wg_com_v2_param.SetOutFullLedCurr);
//        WG_COM_V2_SET_DATA(13.60f, wg_com_v2_param.AuotForwardOpenVoltA);
//        WG_COM_V2_SET_DATA(13.00f, wg_com_v2_param.AuotForwardVeerVoltA);
//        WG_COM_V2_SET_DATA(12.00f, wg_com_v2_param.AuotForwardShutVoltA);
//        WG_COM_V2_SET_DATA(12.50f, wg_com_v2_param.AuotReverseOpenVoltB);
//        WG_COM_V2_SET_DATA(12.00f, wg_com_v2_param.AuotReverseShutVoltB);
//        WG_COM_V2_SET_DATA(100.0f, wg_com_v2_param.SetTemp2);

//        memcpy((uint8_t *)&flash_wg_com_v2_param.wg_com_v2_param,
//               (uint8_t *)&wg_com_v2_param,
//               sizeof(wg_com_v2_param));

//        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.FactoryReset);
//        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.PowerOnOff);
//        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.SetPowerMode);
//        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.SetChargMode);
//        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.InpBatyType);
//        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.OutBatyType);
//        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.SetBootTimeA);
//        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.SetBootTimeB);
//        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.SetOnCurrStartTimeA);
//        WG_COM_V2_SET_DATA(0, wg_com_v2_ctrl.SetOnCurrStartTimeB);

//        memcpy((uint8_t *)&flash_wg_com_v2_param.wg_com_v2_ctrl,
//               (uint8_t *)&wg_com_v2_ctrl,
//               sizeof(wg_com_v2_ctrl));

//        memset(&flash_buffer[0], 0xFF, sizeof(flash_buffer));

//        memcpy((uint8_t *)&flash_buffer[0],
//               (uint8_t *)&flash_wg_com_v2_param,
//               sizeof(flash_wg_com_v2_param));

//        flash_write(FLASH_CFG,
//                    0,
//                    (uint16_t *)&flash_buffer[0]);
//    }
//    else
//    {
//        memcpy((uint8_t *)&wg_com_v2_param,
//               (uint8_t *)&flash_wg_com_v2_param.wg_com_v2_param,
//               sizeof(wg_com_v2_param));

//        memcpy((uint8_t *)&wg_com_v2_ctrl,
//               (uint8_t *)&flash_wg_com_v2_param.wg_com_v2_ctrl,
//               sizeof(wg_com_v2_ctrl));
//    }
//}

////REG_INIT(flash_cfg_init)

//static uint8_t update_trig = 0;

//static bool flash_memcmp(const void *ptr1, const void *ptr2, size_t size)
//{
//    const uint8_t *p1 = (const uint8_t *)ptr1;
//    const uint8_t *p2 = (const uint8_t *)ptr2;

//    if (p1 == NULL || p2 == NULL)
//    {
//        return false;
//    }

//    for (size_t i = 0; i < size; i++)
//    {
//        if (p1[i] != p2[i])
//        {
//            return false;
//        }
//    }

//    return true;
//}

//void flash_cfg_update(void)
//{
//    if (update_trig == 0)
//    {
//        if (flash_memcmp((uint8_t *)&flash_wg_com_v2_param.wg_com_v2_param,
//                         (uint8_t *)&wg_com_v2_param,
//                         sizeof(wg_com_v2_param_t)) == false)
//        {
//            update_trig = 1;
//        }
//        if (flash_memcmp((uint8_t *)&flash_wg_com_v2_param.wg_com_v2_ctrl,
//                         (uint8_t *)&wg_com_v2_ctrl,
//                         sizeof(wg_com_v2_ctrl_t)) == false)
//        {
//            update_trig = 1;
//        }
//    }

//    if ((ctrl_app_get_is_run() == 0) &&
//        (update_trig == 1))
//    {
//        update_trig = 0;
//        memcpy((uint8_t *)&flash_wg_com_v2_param.wg_com_v2_param,
//               (uint8_t *)&wg_com_v2_param,
//               sizeof(wg_com_v2_param));

//        memcpy((uint8_t *)&flash_wg_com_v2_param.wg_com_v2_ctrl,
//               (uint8_t *)&wg_com_v2_ctrl,
//               sizeof(wg_com_v2_ctrl));

//        memset(&flash_buffer[0], 0xFF, sizeof(flash_buffer));
//        memcpy((uint8_t *)&flash_buffer[0],
//               (uint8_t *)&flash_wg_com_v2_param,
//               sizeof(flash_wg_com_v2_param));
//        flash_write(FLASH_CFG,
//                    0,
//                    (uint16_t *)&flash_buffer[0]);
//    }
//}

//REG_TASK(10, flash_cfg_update)
