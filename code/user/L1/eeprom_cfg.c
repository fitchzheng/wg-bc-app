#include "eeprom_cfg.h"
#include "section.h"
#include "wg_com_v2.h"
#include "string.h"
#include "ctrl_app.h"
#include "stdbool.h"
#include "bsp_gpio.h"
#include "get_com_data.h"
#include "fault.h"
#include "bsp_iic.h"
#include "flash.h"
#include "gpio.h"
#include "adc_check.h"
#include "adc.h"
#include "shell.h"
#include "bat_charge_pattern.h"
#include "get_com_data.h"
#include "charge_control.h"
eeprom_wg_com_v2_param_t eeprom_wg_com_v2_param;
eeprom_wg_com_v2_param_t eeprom_backup_wg_com_v2_param;
uint8_t eeprom_page_read_data[EE_24CXX_PAGE_SIZE];
//uint8_t flash_buffer_data[4096] = {0};
// 初始化           1word->2byte   起始0地址
// P00厂家数据区   42word->84byte  起始地址为2
// P02控制设置     12word->24byte  起始地址为86
// P03设置参数区   52word->104byte 起始地址为110
// 安全减少计数器（防止下溢）
void eeprom_cfg_init(void)
{
    IICx_Read_Byte(P00_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&eeprom_wg_com_v2_param, 
           (uint8_t *)&eeprom_page_read_data, 
           (sizeof(eeprom_wg_com_v2_param.is_writed)+sizeof(eeprom_wg_com_v2_param.wg_com_v2_product_info)));

    IICx_Read_Byte(P02_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl, 
           (uint8_t *)&eeprom_page_read_data, 
            sizeof(eeprom_wg_com_v2_param.wg_com_v2_ctrl));

    IICx_Read_Byte(P03_1_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
    memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param, 
           (uint8_t *)&eeprom_page_read_data, 
           (sizeof(eeprom_wg_com_v2_param.wg_com_v2_param)));
  
    if (eeprom_wg_com_v2_param.is_writed != EEROM_INIT_DATA)
    {
            eeprom_wg_com_v2_param.is_writed = EEROM_INIT_DATA;
            WG_COM_V2_SET_DATA_UINT((('V'<<8)+'1'),wg_com_v2_product_info.ProtocolVersion[0]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'1'),wg_com_v2_product_info.ProtocolVersion[1]);
            WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.ProductType[0]);
            WG_COM_V2_SET_DATA_UINT(5,wg_com_v2_product_info.ProductType[1]);
            WG_COM_V2_SET_DATA_UINT((('V'<<8)+'1'),wg_com_v2_product_info.HardverVerzi[0]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'3'),wg_com_v2_product_info.HardverVerzi[1]);
            WG_COM_V2_SET_DATA_UINT((('V'<<8)+'2'),wg_com_v2_product_info.SoftVersion[0]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'1'),wg_com_v2_product_info.SoftVersion[1]);
            WG_COM_V2_SET_DATA_UINT((('W'<<8)+'G'),wg_com_v2_product_info.SnSerial[0]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'5'),wg_com_v2_product_info.SnSerial[1]);
            WG_COM_V2_SET_DATA_UINT((('-'<<8)+'2'),wg_com_v2_product_info.SnSerial[2]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'2'),wg_com_v2_product_info.SnSerial[3]);
            WG_COM_V2_SET_DATA_UINT((('5'<<8)+'0'),wg_com_v2_product_info.SnSerial[4]);
            WG_COM_V2_SET_DATA_UINT((('7'<<8)+'2'),wg_com_v2_product_info.SnSerial[5]);
            WG_COM_V2_SET_DATA_UINT((('2'<<8)+'-'),wg_com_v2_product_info.SnSerial[6]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'),wg_com_v2_product_info.SnSerial[7]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'),wg_com_v2_product_info.SnSerial[8]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'),wg_com_v2_product_info.SnSerial[9]);
            WG_COM_V2_SET_DATA_UINT((('W'<<8)+'G'),wg_com_v2_product_info.ProductName[0]);
            WG_COM_V2_SET_DATA_UINT((('-'<<8)+'B'),wg_com_v2_product_info.ProductName[1]);
            WG_COM_V2_SET_DATA_UINT((('C'<<8)+'1'),wg_com_v2_product_info.ProductName[2]);
            WG_COM_V2_SET_DATA_UINT((('5'<<8)+'0'),wg_com_v2_product_info.ProductName[3]);
            WG_COM_V2_SET_DATA_UINT((('0'<<8)+'M'),wg_com_v2_product_info.ProductName[4]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[5]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[6]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[7]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[8]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[9]);
            WG_COM_V2_SET_DATA_UINT(1,wg_com_v2_product_info.Address);
            WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.ApplicationScenarios);
            WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.CustomizationVersion);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[0]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[1]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[2]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[3]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[4]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[5]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[6]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[7]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[8]);
            WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.MacAddress[9]);
            WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.BtName);
            
            memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_product_info,
                   (uint8_t *)&wg_com_v2_product_info,
                   sizeof(wg_com_v2_product_info));

            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.FactoryReset);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.PowerOnOff);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetPowerMode);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetChargMode);
            WG_COM_V2_SET_DATA_UINT(0x0407, wg_com_v2_ctrl.InpBatyType);
            WG_COM_V2_SET_DATA_UINT(0x0400, wg_com_v2_ctrl.OutBatyType);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetBootTimeA);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetBootTimeB);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetOnCurrStartTimeA);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.SetOnCurrStartTimeB);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ZeroCurrCalibration);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ResetFactoryData);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.BatModeFR);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.MpptSwitch);
            WG_COM_V2_SET_DATA_UINT(1, wg_com_v2_ctrl.SleepModeOnOff);

            memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
                   (uint8_t *)&wg_com_v2_ctrl,
                   sizeof(wg_com_v2_ctrl));
            
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpVoltCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpVoltCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpShowVoltCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpShowVoltCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.InpShowCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.InpShowCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutVoltCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutVoltCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutShowVoltCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutShowVoltCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.OutShowCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.OutShowCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.AOutShowCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.AOutShowCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.BOutShowCurrCalibrK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.BOutShowCurrCalibrB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.VoltCompensationAK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.VoltCompensationAB);
            WG_COM_V2_SET_DATA_UINT(1.0f, wg_com_v2_param.VoltCompensationBK);
            WG_COM_V2_SET_DATA_UINT(0.0f, wg_com_v2_param.VoltCompensationBB);
            for (uint8_t i = 0; i < 2; i++)
            {
                wg_com_v2_param.Retain10[i] = 0xFFFF;
            }
            WG_COM_V2_SET_DATA_UINT(12.0f, wg_com_v2_param.SetInpVolt);
            WG_COM_V2_SET_DATA_UINT(125.0f, wg_com_v2_param.SetInpCurr);
            WG_COM_V2_SET_DATA_UINT(1500.0f, wg_com_v2_param.SetInpCurrPower);
            WG_COM_V2_SET_DATA_UINT(12.0f, wg_com_v2_param.SetOutVolt);
            WG_COM_V2_SET_DATA_UINT(125.0f, wg_com_v2_param.SetOutCurr);
            WG_COM_V2_SET_DATA_UINT(1500.0f, wg_com_v2_param.SetOutCurrPower);
            WG_COM_V2_SET_DATA_UINT(10.0f, wg_com_v2_param.SetInpUvlo);
            WG_COM_V2_SET_DATA_UINT(11.0f, wg_com_v2_param.SetInpUvloRecover);
            WG_COM_V2_SET_DATA_UINT(61.5f, wg_com_v2_param.SetInpOVP);
            WG_COM_V2_SET_DATA_UINT(60.0f, wg_com_v2_param.SetInpOVPRecover);
            WG_COM_V2_SET_DATA_UINT(10.0f, wg_com_v2_param.SetOutUvlo);
            WG_COM_V2_SET_DATA_UINT(11.0f, wg_com_v2_param.SetOutUvloRecover);
            WG_COM_V2_SET_DATA_UINT(61.5f, wg_com_v2_param.SetOutOVP);
            WG_COM_V2_SET_DATA_UINT(60.0f, wg_com_v2_param.SetOutOVPRecover);
            WG_COM_V2_SET_DATA_INT(120.0f, wg_com_v2_param.SetInsideTemp);
            WG_COM_V2_SET_DATA_INT(120.0f, wg_com_v2_param.SetOutsideTemp);
            WG_COM_V2_SET_DATA_UINT(100.0f, wg_com_v2_param.SetInpChargLedCurr);
            WG_COM_V2_SET_DATA_UINT(99.0f, wg_com_v2_param.SetInpFullLedCurr);
            WG_COM_V2_SET_DATA_UINT(100.0f, wg_com_v2_param.SetOutChargLedCurr);
            WG_COM_V2_SET_DATA_UINT(99.0f, wg_com_v2_param.SetOutFullLedCurr);
            WG_COM_V2_SET_DATA_UINT(13.60f, wg_com_v2_param.AuotForwardOpenVoltA);
            WG_COM_V2_SET_DATA_UINT(12.00f, wg_com_v2_param.AuotForwardVeerVoltA);
            WG_COM_V2_SET_DATA_UINT(13.00f, wg_com_v2_param.AuotForwardShutVoltA);
            WG_COM_V2_SET_DATA_UINT(12.50f, wg_com_v2_param.AuotReverseOpenVoltB);
            WG_COM_V2_SET_DATA_UINT(12.00f, wg_com_v2_param.AuotReverseShutVoltB);
            WG_COM_V2_SET_DATA_INT(105.0f, wg_com_v2_param.SetTemp2);

            memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param,
                   (uint8_t *)&wg_com_v2_param,
                   sizeof(wg_com_v2_param));

            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_wg_com_v2_param, 
                   (sizeof(eeprom_wg_com_v2_param.is_writed)+sizeof(eeprom_wg_com_v2_param.wg_com_v2_product_info)));
            IICx_Write_Byte(P00_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
            
            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl, 
                    sizeof(eeprom_wg_com_v2_param.wg_com_v2_ctrl));
            IICx_Write_Byte(P02_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
            
            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param, 
                   (sizeof(eeprom_wg_com_v2_param.wg_com_v2_param)));
            IICx_Write_Byte(P03_1_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));

            memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param, 
                   (uint8_t *)&eeprom_wg_com_v2_param, 
                   (sizeof(eeprom_backup_wg_com_v2_param)));

            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_backup_wg_com_v2_param, 
                   (sizeof(eeprom_backup_wg_com_v2_param.is_writed)+sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_product_info)));
            IICx_Write_Byte(P00_BACKUP_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));

            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl, 
                    sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl));
            IICx_Write_Byte(P02_BACKUP_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));

            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_param, 
                   (sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_param)));
            IICx_Write_Byte(P03_BACKUP_1_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
    }
    else
    {
        memcpy((uint8_t *)&wg_com_v2_product_info,
               (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_product_info,
               sizeof(wg_com_v2_product_info));
        
        memcpy((uint8_t *)&wg_com_v2_ctrl,
               (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
               sizeof(wg_com_v2_ctrl));

        memcpy((uint8_t *)&wg_com_v2_param,
               (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param,
               sizeof(wg_com_v2_param));

        memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param, 
               (uint8_t *)&eeprom_wg_com_v2_param, 
               (sizeof(eeprom_backup_wg_com_v2_param)));

        IICx_Read_Byte(P00_BACKUP_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param, 
               (uint8_t *)&eeprom_page_read_data, 
               (sizeof(eeprom_backup_wg_com_v2_param.is_writed)+sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_product_info)));

        IICx_Read_Byte(P02_BACKUP_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl, 
               (uint8_t *)&eeprom_page_read_data, 
                sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl));

        IICx_Read_Byte(P03_BACKUP_1_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_param, 
               (uint8_t *)&eeprom_page_read_data, 
               (sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_param)));
    }
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.SetPowerMode, wg_com_v2_ctrl.SetPowerMode);
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.SetChargMode, wg_com_v2_ctrl.SetChargMode);
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.InpBatyType, wg_com_v2_ctrl.InpBatyType);
    WG_COM_V2_GET_DATA_UINT(State_Control_Data.OutBatyType, wg_com_v2_ctrl.OutBatyType);

    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.FactoryReset);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.PowerOnOff);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ZeroCurrCalibration);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.ResetFactoryData);
    WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.BatModeFR);
    WG_COM_V2_SET_DATA_UINT(7, wg_com_v2_realtime_data.StateCharge);
    get_wg_com_data_rum();
}

REG_INIT(eeprom_cfg_init)

static uint8_t update_trig = 0;
static bool eeprom_memcmp(uint8_t zone,void *ptr1,void *ptr2, size_t size)
{
    uint8_t *p1 = (uint8_t *)ptr1;
    uint8_t *p2 = (uint8_t *)ptr2;
    if (p1 == NULL || p2 == NULL)
    {
        return false;
    }

    for (size_t i = 0; i < size; i++)
    {
        if (p1[i] != p2[i])
        {
            switch(zone)
            {
                case eEEPROM_WRITE_P00_ZONE:
                    if((i >= (0x0008*2))&&(i<(0x29*2)))
                    {
                        return false;
                    }
                    break;
                case eEEPROM_WRITE_P02_ZONE:
                    if((i >= (2*2))&&(i < (11*2)))
                    {
                        return false;
                    }
                    break;
                case eEEPROM_WRITE_P03_1_ZONE:
                        return false;
                default:
                    return true;
            }

        }
    }

    return true;
}

void eeprom_cfg_update(void)
{
    Save_Backups_Data();
    if (update_trig == 0)
    {
        if (eeprom_memcmp(eEEPROM_WRITE_P00_ZONE,
                         (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_product_info,
                         (uint8_t *)&wg_com_v2_product_info,
                         sizeof(wg_com_v2_product_info)) == false)
        {
            update_trig |= 1;
        }
        
        if (eeprom_memcmp(eEEPROM_WRITE_P02_ZONE,
                         (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
                         (uint8_t *)&wg_com_v2_ctrl,
                         sizeof(wg_com_v2_ctrl)) == false)
        {
            update_trig |= 2;
        }

        if (eeprom_memcmp(eEEPROM_WRITE_P03_1_ZONE,
                         (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param,
                         (uint8_t *)&wg_com_v2_param,
                         (sizeof(wg_com_v2_param))) == false)
        {
            update_trig |= 4;
        }
    }

    if (update_trig != 0)
    {
        if((update_trig&0x01) == 0x01)
        {
            update_trig &= 0xFE;
            memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_product_info,
                  (uint8_t *)&wg_com_v2_product_info,
                   sizeof(wg_com_v2_product_info));
            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            eeprom_wg_com_v2_param.wg_com_v2_product_info.BtName = 0;
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_wg_com_v2_param, 
                   (sizeof(eeprom_wg_com_v2_param.is_writed)+sizeof(eeprom_wg_com_v2_param.wg_com_v2_product_info)));
            IICx_Write_Byte(P00_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        }
        else if((update_trig&0x02) == 0x02)
        {
            update_trig &= 0xFD;
            memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
                  (uint8_t *)&wg_com_v2_ctrl,
                   sizeof(wg_com_v2_ctrl));
            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            eeprom_wg_com_v2_param.wg_com_v2_ctrl.FactoryReset = 0;
            eeprom_wg_com_v2_param.wg_com_v2_ctrl.PowerOnOff = 0;
            eeprom_wg_com_v2_param.wg_com_v2_ctrl.ZeroCurrCalibration = 0;
            eeprom_wg_com_v2_param.wg_com_v2_ctrl.ResetFactoryData = 0;
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl, 
                    sizeof(eeprom_wg_com_v2_param.wg_com_v2_ctrl));
            IICx_Write_Byte(P02_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        }
        else if((update_trig&0x04) == 0x04)
        {
            update_trig &= 0xFB;
            memcpy((uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param,
                   (uint8_t *)&wg_com_v2_param,
                   (sizeof(wg_com_v2_param)));
            memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
            memcpy((uint8_t *)&eeprom_page_read_data, 
                   (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param, 
                   (sizeof(eeprom_wg_com_v2_param.wg_com_v2_param)));
            IICx_Write_Byte(P03_1_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        }
        else
        {
            update_trig = 0;
        }
    }
}


void Save_Backups_Data(void)
{
    uint16_t GetDataFlag = 0;

    WG_COM_V2_GET_DATA_UINT(GetDataFlag, wg_com_v2_ctrl.ResetFactoryData);

    if(GetDataFlag)
    {
        GetDataFlag = 0;
        charge_control_run();
        memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
        if ((eeprom_memcmp(eEEPROM_WRITE_P00_ZONE,
                         (uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_product_info,
                         (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_product_info,
                         sizeof(eeprom_wg_com_v2_param.wg_com_v2_product_info)) == false)
        || (eeprom_page_read_data[1] != ((eeprom_wg_com_v2_param.is_writed&0xff00) >> 8)) 
        || (eeprom_page_read_data[0] !=  (eeprom_wg_com_v2_param.is_writed&0x00ff)))
        {
			memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param, 
				   (uint8_t *)&eeprom_wg_com_v2_param, 
				   (sizeof(eeprom_wg_com_v2_param.is_writed) + sizeof(eeprom_wg_com_v2_param.wg_com_v2_product_info)));
			memcpy((uint8_t *)&eeprom_page_read_data, 
				   (uint8_t *)&eeprom_backup_wg_com_v2_param, 
				   (sizeof(eeprom_backup_wg_com_v2_param.is_writed) + sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_product_info)));
			IICx_Write_Byte(P00_BACKUP_ADDR,
                           (uint8_t *)&eeprom_page_read_data,
                           (sizeof(eeprom_backup_wg_com_v2_param.is_writed) + sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_product_info)));
        }
		
        memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
        if (eeprom_memcmp(eEEPROM_WRITE_P02_ZONE,
                         (uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl,
                         (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl,
                         sizeof(eeprom_wg_com_v2_param.wg_com_v2_ctrl)) == false)
        {
			memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl, 
				   (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_ctrl, 
				   (sizeof(eeprom_wg_com_v2_param.wg_com_v2_ctrl)));
			memcpy((uint8_t *)&eeprom_page_read_data, 
				   (uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl, 
					sizeof(eeprom_backup_wg_com_v2_param.wg_com_v2_ctrl));
			IICx_Write_Byte(P02_BACKUP_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        }

        memset(&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
        if (eeprom_memcmp(eEEPROM_WRITE_P03_1_ZONE,
                         (uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_param,
                         (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param,
                         sizeof(eeprom_wg_com_v2_param.wg_com_v2_param)) == false)
        {
			memcpy((uint8_t *)&eeprom_backup_wg_com_v2_param.wg_com_v2_param, 
				   (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param, 
				   (sizeof(eeprom_wg_com_v2_param.wg_com_v2_param)));
			memcpy(&eeprom_page_read_data, 
				  (uint8_t *)&eeprom_wg_com_v2_param.wg_com_v2_param, 
				  (sizeof(eeprom_wg_com_v2_param.wg_com_v2_param)));
			IICx_Write_Byte(P03_BACKUP_1_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
        }
        WG_COM_V2_SET_DATA_UINT(GetDataFlag, wg_com_v2_ctrl.ResetFactoryData);
        WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.BatModeFR);
    }
    
    WG_COM_V2_GET_DATA_UINT(GetDataFlag, wg_com_v2_ctrl.FactoryReset);
    if(GetDataFlag)
    {
        GetDataFlag = 0;
        charge_control_run();
        IICx_Read_Byte(P00_BACKUP_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));

		if((eeprom_page_read_data[1] == ((eeprom_wg_com_v2_param.is_writed&0xff00) >> 8)) &&
		   (eeprom_page_read_data[0] ==  (eeprom_wg_com_v2_param.is_writed&0x00ff)))
		{
			memcpy((uint8_t *)&wg_com_v2_product_info, 
				   (uint8_t *)&eeprom_page_read_data[sizeof(eeprom_wg_com_v2_param.is_writed)], 
				   sizeof(wg_com_v2_product_info));
			
			IICx_Read_Byte(P02_BACKUP_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
			memcpy((uint8_t *)&wg_com_v2_ctrl, 
				   (uint8_t *)&eeprom_page_read_data, 
					sizeof(wg_com_v2_ctrl));

			IICx_Read_Byte(P03_BACKUP_1_ADDR,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
			memcpy((uint8_t *)&wg_com_v2_param, 
				   (uint8_t *)&eeprom_page_read_data, 
				   (sizeof(wg_com_v2_param)));

            WG_COM_V2_SET_DATA_UINT(GetDataFlag, wg_com_v2_ctrl.FactoryReset); 
            WG_COM_V2_GET_DATA_UINT(State_Control_Data.SetPowerMode, wg_com_v2_ctrl.SetPowerMode);
            WG_COM_V2_GET_DATA_UINT(State_Control_Data.SetChargMode, wg_com_v2_ctrl.SetChargMode);
            WG_COM_V2_GET_DATA_UINT(State_Control_Data.InpBatyType, wg_com_v2_ctrl.InpBatyType);
            WG_COM_V2_GET_DATA_UINT(State_Control_Data.OutBatyType, wg_com_v2_ctrl.OutBatyType);
            WG_COM_V2_SET_DATA_UINT(0, wg_com_v2_ctrl.BatModeFR);
            WG_COM_V2_SET_DATA_UINT(1, wg_com_v2_ctrl.PowerOnOff);
		}
    }
}

REG_TASK(100, eeprom_cfg_update)


void erasure_eeprom_data(void)
{
    static uint32_t ErasureDataDelay = 0;
    static uint8_t  ErasureFlag = 0;
    if(bsp_get_addrs() == 0)
    {
        if(ErasureDataDelay > 10)
        {
            ErasureDataDelay = 0;
            if(ErasureFlag == 0)
            {
                ErasureFlag = 1;
                memset((uint8_t *)&eeprom_page_read_data, 0xFF, sizeof(eeprom_page_read_data));
                for(uint32_t i = 0;i < EE_24CXX_CAPACITY;)
                {
                    IICx_Write_Byte(i,(uint8_t *)&eeprom_page_read_data,sizeof(eeprom_page_read_data));
                    i += EE_24CXX_PAGE_SIZE;
                }
            }
        }
        else
        {
            ++ErasureDataDelay;
        }
    }
    else
    {
        ErasureFlag = 0;
        ErasureDataDelay = 0;
    }
}

REG_TASK(100, erasure_eeprom_data)


