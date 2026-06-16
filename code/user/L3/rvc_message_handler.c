/**
 * @file rvc_message_handler.c
 * @brief RV-C 消息处理实现
 */
#include "rvc_message_handler.h"
#include "rvc_address.h"
#include "bsp_can.h"
#include "get_com_data.h"
#include "wg_com_v2.h"

static uint8_t g_my_instance = 1;

/* ========== 内部变量 ========== */

static rvc_charger_command_callback_t g_charger_command_cb = NULL;
static rvc_request_for_dgn_callback_t g_request_for_dgn_cb = NULL;


static uint8_t g_dm_dsn = 0;  // 诊断序列号
static uint16_t g_current_spn = 0;  // 当前故障码
static uint8_t g_current_fmi = 0;
static uint8_t g_occurrence_count = 0;

/* ========== 内部函数声明 ========== */

static void handle_charger_command(uint32_t can_id, uint8_t *data, uint8_t len);
static void handle_request_for_dgn(uint32_t can_id, uint8_t *data, uint8_t len);

static uint32_t build_can_id(uint8_t priority, uint32_t dgn, uint8_t sa);

/* ========== 公共 API 实现 ========== */

uint8_t rvc_send_dm_rv(uint8_t instance, uint16_t spn, uint8_t fmi)
{
    uint32_t can_id = build_can_id(6, RVC_DGN_DM_RV, 0xFF);

    // 更新故障状态
    if (spn != g_current_spn || fmi != g_current_fmi) {
        g_current_spn = spn;
        g_current_fmi = fmi;
        g_occurrence_count = (spn != 0) ? 1 : 0;
        g_dm_dsn++;  // 状态改变，序列号递增
    }

    uint8_t data[8];
    data[0] = g_dm_dsn;
    data[1] = spn & 0xFF;
    data[2] = (spn >> 8) & 0xFF;
    data[3] = fmi;
    data[4] = g_occurrence_count;
    data[5] = rvc_address_get_current();  // DSA = 自己的地址
    data[6] = instance;
    data[7] = 0xFF;  // Reserved

    return bsp_rvc_can_tx(can_id, data, 8);
}

void rvc_message_handler_init(void)
{
    g_charger_command_cb = NULL;
    g_request_for_dgn_cb = NULL;

    // 读取本机 Instance
    g_my_instance = 1;  // 或直接用 RVC_CHARGER_INSTANCE
}

void rvc_register_charger_command_callback(rvc_charger_command_callback_t callback)
{
    g_charger_command_cb = callback;
}

void rvc_register_request_for_dgn_callback(rvc_request_for_dgn_callback_t callback)
{
    g_request_for_dgn_cb = callback;
}

static void handle_manufacturer_data(uint32_t gdn, uint8_t *data, uint8_t len)
{
    if (len < 7) {
        return;  // 数据长度不足
    }

    // 检查实例号
    if ((data[0] != 0xFF) && (data[0] != g_my_instance)) {
        return;  // 不是请求我的
    }

    // 提取发送者地址
    uint32_t tx_can_id = 0;//build_can_id(6, RVC_DGN_ACKNOWLEDGEMENT, rvc_address_get_current());
    uint8_t tx_data[8];
    uint16_t data_u16 = 0;
    switch(gdn){
        case RVC_DGN_PROPRIETARY_PROTOCOL_VERSION_R:
            memset(tx_data, 0xFF, sizeof(tx_data));
            tx_data[0] = data[0];
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_PROTOCOL_VERSION_R, rvc_address_get_current());
            for(uint16_t i = 0;i < 2;i++){
                data_u16 = wg_com_v2_product_info.ProtocolVersion[i];
                tx_data[i*2+1] = (data_u16&0xff);
                tx_data[i*2+2] = ((data_u16>>8)&0xff);
            }
            bsp_rvc_can_tx(tx_can_id, tx_data, 8);
            break;
        case RVC_DGN_PROPRIETARY_PRODUCT_TYPE_R:
            memset(tx_data, 0xFF, sizeof(tx_data));
            tx_data[0] = data[0];
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_PRODUCT_TYPE_R, rvc_address_get_current());
            for(uint16_t i = 0;i < 2;i++){
                data_u16 = wg_com_v2_product_info.ProductType[i];
                tx_data[i*2+1] = (data_u16&0xff);
                tx_data[i*2+2] = ((data_u16>>8)&0xff);
            }
            bsp_rvc_can_tx(tx_can_id, tx_data, 8);
            break;
        case RVC_DGN_PROPRIETARY_HARDVER_VERSION_R:
            memset(tx_data, 0xFF, sizeof(tx_data));
            tx_data[0] = data[0];
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_HARDVER_VERSION_R, rvc_address_get_current());
            for(uint16_t i = 0;i < 2;i++){
                data_u16 = wg_com_v2_product_info.HardverVerzi[i];
                tx_data[i*2+1] = (data_u16&0xff);
                tx_data[i*2+2] = ((data_u16>>8)&0xff);
            }
            bsp_rvc_can_tx(tx_can_id, tx_data, 8);
            break;
        case RVC_DGN_PROPRIETARY_SOFT_VERSION_R:
            memset(tx_data, 0xFF, sizeof(tx_data));
            tx_data[0] = data[0];
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SOFT_VERSION_R, rvc_address_get_current());
            for(uint16_t i = 0;i < 2;i++){
                data_u16 = wg_com_v2_product_info.SoftVersion[i];
                tx_data[i*2+1] = (data_u16&0xff);
                tx_data[i*2+2] = ((data_u16>>8)&0xff);
            }
            bsp_rvc_can_tx(tx_can_id, tx_data, 8);
            break;
        case RVC_DGN_PROPRIETARY_SN_SERIAL_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SN_SERIAL_R, rvc_address_get_current());
            if(data[1] >= 4)
            {
                return;
            }
            memset(tx_data, 0xFF, sizeof(tx_data));
            tx_data[0] = data[0];
            tx_data[1] = data[1];
            for(uint16_t i = 0;i < 3;i++){
                data_u16 = wg_com_v2_product_info.SnSerial[(tx_data[1]*3)+i];
                tx_data[i*2+2] = (data_u16&0xff);
                tx_data[i*2+3] = ((data_u16>>8)&0xff);
            }
            bsp_rvc_can_tx(tx_can_id, tx_data, 8);
            break;
        case RVC_DGN_PROPRIETARY_PRODUCT_NAME_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_PRODUCT_NAME_R, rvc_address_get_current());
            if(data[1] >= 4)
            {
                return;
            }
            memset(tx_data, 0xFF, sizeof(tx_data));
            tx_data[0] = data[0];
            tx_data[1] = data[1];
            for(uint16_t i = 0;i < 3;i++){
                data_u16 = wg_com_v2_product_info.ProductName[(tx_data[1]*3)+i];
                tx_data[i*2+2] = (data_u16&0xff);
                tx_data[i*2+3] = ((data_u16>>8)&0xff);
            }
            bsp_rvc_can_tx(tx_can_id, tx_data, 8);
            break;
        case RVC_DGN_PROPRIETARY_ADDRE_APPLI_R:
            memset(tx_data, 0xFF, sizeof(tx_data));
            tx_data[0] = data[0];
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_ADDRE_APPLI_R, rvc_address_get_current());
            data_u16 = wg_com_v2_product_info.Address;
            tx_data[2] = (data_u16&0xff);
            tx_data[1] = ((data_u16>>8)&0xff);
            data_u16 = wg_com_v2_product_info.ApplicationScenarios;
            tx_data[4] = (data_u16&0xff);
            tx_data[3] = ((data_u16>>8)&0xff);
            bsp_rvc_can_tx(tx_can_id, tx_data, 8);
            break;
        case RVC_DGN_PROPRIETARY_CUST_BT_NAME_R:
            memset(tx_data, 0xFF, sizeof(tx_data));
            tx_data[0] = data[0];
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_CUST_BT_NAME_R, rvc_address_get_current());
            data_u16 = wg_com_v2_product_info.CustomizationVersion;
            tx_data[1] = (data_u16&0xff);
            tx_data[2] = ((data_u16>>8)&0xff);
            data_u16 = wg_com_v2_product_info.BtName;
            tx_data[3] = (data_u16&0xff);
            tx_data[4] = ((data_u16>>8)&0xff);
            bsp_rvc_can_tx(tx_can_id, tx_data, 8);
            break;
        case RVC_DGN_PROPRIETARY_MAC_ADDRESS_R:
            if(data[1] >= 4)
            {
                return;
            }
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_MAC_ADDRESS_R, rvc_address_get_current());
            memset(tx_data, 0xFF, sizeof(tx_data));
            tx_data[0] = data[0];
            tx_data[1] = data[1];
            for(uint16_t i = 0;i < 3;i++){
                data_u16 = wg_com_v2_product_info.MacAddress[(tx_data[1]*3)+i];
                tx_data[i*2+2] = ((data_u16>>8)&0xff);
                tx_data[i*2+3] = (data_u16&0xff);
            }
            bsp_rvc_can_tx(tx_can_id, tx_data, 8);
            break;
        default:
            return;
    }
}

static void handle_real_time_data(uint32_t gdn, uint8_t *data, uint8_t len)
{
    if (len < 7) {
        return;  // 数据长度不足
    }

    // 检查实例号
    if ((data[0] != 0xFF) && (data[0] != g_my_instance)) {
        return;  // 不是请求我的
    }

    // 提取发送者地址
    uint32_t tx_can_id = 0;//build_can_id(6, RVC_DGN_ACKNOWLEDGEMENT, rvc_address_get_current());
    uint8_t tx_data[8];
    memset(tx_data, 0xFF, sizeof(tx_data));
    tx_data[0] = data[0];

    switch(gdn){
        case RVC_DGN_PROPRIETARY_AVOLT_ACURR_APOWER_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_AVOLT_ACURR_APOWER_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_realtime_data.InpVolt>>8)&0xff);           // A端电压
            tx_data[2] = (wg_com_v2_realtime_data.InpVolt&0xff);                // A端电压
            tx_data[3] = ((wg_com_v2_realtime_data.InpCurr>>8)&0xff);           // A端电流
            tx_data[4] = (wg_com_v2_realtime_data.InpCurr&0xff);                // A端电流
            tx_data[5] = ((wg_com_v2_realtime_data.InpCurrPower>>8)&0xff);      // A端功率
            tx_data[6] = (wg_com_v2_realtime_data.InpCurrPower&0xff);           // A端功率
            break;
        case RVC_DGN_PROPRIETARY_BVOLT_BCURR_BPOWER_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_BVOLT_BCURR_BPOWER_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_realtime_data.OutVolt>>8)&0xff);           // B端电压
            tx_data[2] = (wg_com_v2_realtime_data.OutVolt&0xff);                // B端电压
            tx_data[3] = ((wg_com_v2_realtime_data.OutCurr>>8)&0xff);           // B端电流
            tx_data[4] = (wg_com_v2_realtime_data.OutCurr&0xff);                // B端电流
            tx_data[5] = ((wg_com_v2_realtime_data.OutCurrPower>>8)&0xff);      // B端功率
            tx_data[6] = (wg_com_v2_realtime_data.OutCurrPower&0xff);           // B端功率
            break;
        case RVC_DGN_PROPRIETARY_T1_T2_TA_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_T1_T2_TA_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_realtime_data.InsideTemp>>8)&0xff);        // T1温度
            tx_data[2] = (wg_com_v2_realtime_data.InsideTemp&0xff);             // T1温度
            tx_data[3] = ((wg_com_v2_realtime_data.OutsideTemp>>8)&0xff);       // T2温度
            tx_data[4] = (wg_com_v2_realtime_data.OutsideTemp&0xff);            // T2温度
            tx_data[5] = ((wg_com_v2_realtime_data.Temp2>>8)&0xff);             // Ta温度
            tx_data[6] = (wg_com_v2_realtime_data.Temp2&0xff);                  // Ta温度
            break;
        case RVC_DGN_PROPRIETARY_POWER_CHARG_STATE_CHARGEE_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_POWER_CHARG_STATE_CHARGEE_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_realtime_data.PowerMode>>8)&0xff);         // 电源模式
            tx_data[2] = (wg_com_v2_realtime_data.PowerMode&0xff);              // 电源模式
            tx_data[3] = ((wg_com_v2_realtime_data.ChargMode>>8)&0xff);         // 充电模式
            tx_data[4] = (wg_com_v2_realtime_data.ChargMode&0xff);              // 充电模式
            tx_data[5] = ((wg_com_v2_realtime_data.StateCharge>>8)&0xff);       // 充电状态
            tx_data[6] = (wg_com_v2_realtime_data.StateCharge&0xff);            // 充电状态
            break;
        case RVC_DGN_PROPRIETARY_FAULT_SIGN_ALARM_SIGN_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_FAULT_SIGN_ALARM_SIGN_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_realtime_data.FaultSign>>8)&0xff);         // 故障信号
            tx_data[2] = (wg_com_v2_realtime_data.FaultSign&0xff);              // 故障信号
            tx_data[3] = ((wg_com_v2_realtime_data.AlarmSign>>8)&0xff);         // 告警信号
            tx_data[4] = (wg_com_v2_realtime_data.AlarmSign&0xff);              // 告警信号
            break;
        case RVC_DGN_PROPRIETARY_VOLTA_VOLTB_ADDVOLT_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_VOLTA_VOLTB_ADDVOLT_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_realtime_data.CompensationVoltA>>8)&0xff); // A端补偿
            tx_data[2] = (wg_com_v2_realtime_data.CompensationVoltA&0xff);      // A端补偿
            tx_data[3] = ((wg_com_v2_realtime_data.CompensationVoltB>>8)&0xff); // B端补偿
            tx_data[4] = (wg_com_v2_realtime_data.CompensationVoltB&0xff);      // B端补偿
            tx_data[5] = ((wg_com_v2_realtime_data.ADDVolt>>8)&0xff);           // ADD辅源电压
            tx_data[6] = (wg_com_v2_realtime_data.ADDVolt&0xff);                // ADD辅源电压
            break;
        default:
            return;
    }
    bsp_rvc_can_tx(tx_can_id, tx_data, 8);
}


static void handle_control_settings(uint32_t gdn, uint8_t *data, uint8_t len)
{
    if (len < 7) {
        return;  // 数据长度不足
    }

    // 检查实例号
    if ((data[0] != 0xFF) && (data[0] != g_my_instance)) {
        return;  // 不是请求我的
    }

    // 提取发送者地址
    uint32_t tx_can_id = 0;//build_can_id(6, RVC_DGN_ACKNOWLEDGEMENT, rvc_address_get_current());
    uint8_t tx_data[8];
    memset(tx_data, 0xFF, sizeof(tx_data));
    tx_data[0] = data[0];
    switch(gdn){
        case RVC_DGN_PROPRIETARY_ON_Off_SETPOWER_SET_CHARG_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_ON_Off_SETPOWER_SET_CHARG_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_ctrl.PowerOnOff>>8)&0xff);                 // 开关机状态
            tx_data[2] = (wg_com_v2_ctrl.PowerOnOff&0xff);                      // 开关机状态
            tx_data[3] = ((wg_com_v2_ctrl.SetPowerMode>>8)&0xff);               // 电源模式
            tx_data[4] = (wg_com_v2_ctrl.SetPowerMode&0xff);                    // 电源模式
            tx_data[5] = ((wg_com_v2_ctrl.SetChargMode>>8)&0xff);               // 充电模式
            tx_data[6] = (wg_com_v2_ctrl.SetChargMode&0xff);                    // 充电模式
            break;
        case RVC_DGN_PROPRIETARY_FACTORY_RESET_RESET_FACTORY_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_FACTORY_RESET_RESET_FACTORY_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_ctrl.FactoryReset>>8)&0xff);               // 恢复出厂设置
            tx_data[2] = (wg_com_v2_ctrl.FactoryReset&0xff);                    // 恢复出厂设置
            tx_data[3] = ((wg_com_v2_ctrl.ResetFactoryData>>8)&0xff);           // 恢复厂家数据
            tx_data[4] = (wg_com_v2_ctrl.ResetFactoryData&0xff);                // 恢复厂家数据
            break;
        case RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTA_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTA_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_ctrl.InpBatyType>>8)&0xff);                // A端电池类型(高8位类型,低8位电压)
            tx_data[2] = (wg_com_v2_ctrl.InpBatyType&0xff);                     // A端电池类型(高8位类型,低8位电压)
            tx_data[3] = ((wg_com_v2_ctrl.SetBootTimeA>>8)&0xff);               // A端开机时间
            tx_data[4] = (wg_com_v2_ctrl.SetBootTimeA&0xff);                    // A端开机时间
            tx_data[5] = ((wg_com_v2_ctrl.SetOnCurrStartTimeA>>8)&0xff);        // A端开机电流软起动时间
            tx_data[6] = (wg_com_v2_ctrl.SetOnCurrStartTimeA&0xff);             // A端开机电流软起动时间
            break;
        case RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTB_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTB_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_ctrl.OutBatyType>>8)&0xff);                // B端电池类型(高8位类型,低8位电压)
            tx_data[2] = (wg_com_v2_ctrl.OutBatyType&0xff);                     // B端电池类型(高8位类型,低8位电压)
            tx_data[3] = ((wg_com_v2_ctrl.SetBootTimeB>>8)&0xff);               // B端开机时间
            tx_data[4] = (wg_com_v2_ctrl.SetBootTimeB&0xff);                    // B端开机时间
            tx_data[5] = ((wg_com_v2_ctrl.SetOnCurrStartTimeB>>8)&0xff);        // B端开机电流软起动时间
            tx_data[6] = (wg_com_v2_ctrl.SetOnCurrStartTimeB&0xff);             // B端开机电流软起动时间
            break;
        case RVC_DGN_PROPRIETARY_ZERO_CURR_BAT_MODE_FR_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_ZERO_CURR_BAT_MODE_FR_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_ctrl.ZeroCurrCalibration>>8)&0xff);        // 端零电流校准
            tx_data[2] = (wg_com_v2_ctrl.ZeroCurrCalibration&0xff);             // 端零电流校准
            tx_data[3] = ((wg_com_v2_ctrl.BatModeFR>>8)&0xff);                  // 电池模式正反向切换
            tx_data[4] = (wg_com_v2_ctrl.BatModeFR&0xff);                       // 电池模式正反向切换
            break;
        default:
            return;
    }
    bsp_rvc_can_tx(tx_can_id, tx_data, 8);
}

static void handle_set_parameter_area(uint32_t gdn, uint8_t *data, uint8_t len)
{
    if (len < 7) {
        return;  // 数据长度不足
    }

    // 检查实例号
    if ((data[0] != 0xFF) && (data[0] != g_my_instance)) {
        return;  // 不是请求我的
    }

    // 提取发送者地址
    uint32_t tx_can_id = 0;//build_can_id(6, RVC_DGN_ACKNOWLEDGEMENT, rvc_address_get_current());
    uint8_t tx_data[8];
    memset(tx_data, 0xFF, sizeof(tx_data));
    tx_data[0] = data[0];
    switch(gdn){
        case RVC_DGN_PROPRIETARY_SET_AVOLT_ACURR_APOWER_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_AVOLT_ACURR_APOWER_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.SetInpVolt>>8)&0xff);                // A端电压
            tx_data[2] = (wg_com_v2_param.SetInpVolt&0xff);                     // A端电压
            tx_data[3] = ((wg_com_v2_param.SetInpCurr>>8)&0xff);                // A端电流
            tx_data[4] = (wg_com_v2_param.SetInpCurr&0xff);                     // A端电流
            tx_data[5] = ((wg_com_v2_param.SetInpCurrPower>>8)&0xff);           // A端功率
            tx_data[6] = (wg_com_v2_param.SetInpCurrPower&0xff);                // A端功率
            break;
        case RVC_DGN_PROPRIETARY_SET_BVOLT_BCURR_BPOWER_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_BVOLT_BCURR_BPOWER_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.SetOutVolt>>8)&0xff);                // B端电压
            tx_data[2] = (wg_com_v2_param.SetOutVolt&0xff);                     // B端电压
            tx_data[3] = ((wg_com_v2_param.SetOutCurr>>8)&0xff);                // B端电流
            tx_data[4] = (wg_com_v2_param.SetOutCurr&0xff);                     // B端电流
            tx_data[5] = ((wg_com_v2_param.SetOutCurrPower>>8)&0xff);           // B端功率
            tx_data[6] = (wg_com_v2_param.SetOutCurrPower&0xff);                // B端功率
            break;
        case RVC_DGN_PROPRIETARY_SET_AUVLO_AUVLO_RECOVER_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_AUVLO_AUVLO_RECOVER_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.SetInpUvlo>>8)&0xff);                // A端欠压保护
            tx_data[2] = (wg_com_v2_param.SetInpUvlo&0xff);                     // A端欠压保护
            tx_data[3] = ((wg_com_v2_param.SetInpUvloRecover>>8)&0xff);         // A端欠压保护恢复
            tx_data[4] = (wg_com_v2_param.SetInpUvloRecover&0xff);              // A端欠压保护恢复
            break;
        case RVC_DGN_PROPRIETARY_SET_AOVP_AUVLO_OVPRECOVER_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_AOVP_AUVLO_OVPRECOVER_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.SetInpOVP>>8)&0xff);                 // A端过压保护
            tx_data[2] = (wg_com_v2_param.SetInpOVP&0xff);                      // A端过压保护
            tx_data[3] = ((wg_com_v2_param.SetInpOVPRecover>>8)&0xff);          // A端过压保护恢复
            tx_data[4] = (wg_com_v2_param.SetInpOVPRecover&0xff);               // A端过压保护恢复
            break;
        case RVC_DGN_PROPRIETARY_SET_BUVLO_BUVLO_RECOVER_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_BUVLO_BUVLO_RECOVER_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.SetOutUvlo>>8)&0xff);                 // B端欠压保护
            tx_data[2] = (wg_com_v2_param.SetOutUvlo&0xff);                      // B端欠压保护
            tx_data[3] = ((wg_com_v2_param.SetOutUvloRecover>>8)&0xff);          // B端欠压保护恢复
            tx_data[4] = (wg_com_v2_param.SetOutUvloRecover&0xff);               // B端欠压保护恢复
            break;
        case RVC_DGN_PROPRIETARY_SET_BOVP_BUVLO_OVPRECOVER_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_BOVP_BUVLO_OVPRECOVER_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.SetOutOVP>>8)&0xff);                  // B端过压保护
            tx_data[2] = (wg_com_v2_param.SetOutOVP&0xff);                       // B端过压保护
            tx_data[3] = ((wg_com_v2_param.SetOutOVPRecover>>8)&0xff);           // B端过压保护恢复
            tx_data[4] = (wg_com_v2_param.SetOutOVPRecover&0xff);                // B端过压保护恢复
            break;
        case RVC_DGN_PROPRIETARY_SET_T1_T2_TA_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_T1_T2_TA_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.SetInsideTemp>>8)&0xff);              // T1温度
            tx_data[2] = (wg_com_v2_param.SetInsideTemp&0xff);                   // T1温度
            tx_data[3] = ((wg_com_v2_param.SetOutsideTemp>>8)&0xff);             // T2温度
            tx_data[4] = (wg_com_v2_param.SetOutsideTemp&0xff);                  // T2温度
            tx_data[5] = ((wg_com_v2_param.SetTemp2>>8)&0xff);                   // Ta温度
            tx_data[6] = (wg_com_v2_param.SetTemp2&0xff);                        // Ta温度
            break;
        case RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.SetInpChargLedCurr>>8)&0xff);         // A端充电指示灯电流
            tx_data[2] = (wg_com_v2_param.SetInpChargLedCurr&0xff);              // A端充电指示灯电流
            tx_data[3] = ((wg_com_v2_param.SetInpFullLedCurr>>8)&0xff);          // A端充满指示灯电流
            tx_data[4] = (wg_com_v2_param.SetInpFullLedCurr&0xff);               // A端充满指示灯电流
            break;
        case RVC_DGN_PROPRIETARY_SET_BCHARG_BFULL_LED_CURR_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.SetOutChargLedCurr>>8)&0xff);         // B端充电指示灯电流
            tx_data[2] = (wg_com_v2_param.SetOutChargLedCurr&0xff);              // B端充电指示灯电流
            tx_data[3] = ((wg_com_v2_param.SetOutFullLedCurr>>8)&0xff);          // B端充满指示灯电流
            tx_data[4] = (wg_com_v2_param.SetOutFullLedCurr&0xff);               // B端充满指示灯电流
            break;
        case RVC_DGN_PROPRIETARY_AUOT_OPEN_VEER_SHUT_VOLT_A_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.AuotForwardOpenVoltA>>8)&0xff);       // 自动模式正向A端开启电压
            tx_data[2] = (wg_com_v2_param.AuotForwardOpenVoltA&0xff);            // 自动模式正向A端开启电压
            tx_data[3] = ((wg_com_v2_param.AuotForwardVeerVoltA>>8)&0xff);       // 自动模式正向转向A电压
            tx_data[4] = (wg_com_v2_param.AuotForwardVeerVoltA&0xff);            // 自动模式正向转向A电压
            tx_data[5] = ((wg_com_v2_param.AuotForwardShutVoltA>>8)&0xff);       // 自动模式正向A端关闭电压
            tx_data[6] = (wg_com_v2_param.AuotForwardShutVoltA&0xff);            // 自动模式正向A端关闭电压
            break;
        case RVC_DGN_PROPRIETARY_AUOT_OPEN_SHUT_VOLT_B_R:
            tx_can_id = build_can_id(6, RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_R, rvc_address_get_current());
            tx_data[1] = ((wg_com_v2_param.AuotReverseOpenVoltB>>8)&0xff);       // 自动模式反向B端开启电压
            tx_data[2] = (wg_com_v2_param.AuotReverseOpenVoltB&0xff);            // 自动模式反向B端开启电压
            tx_data[3] = ((wg_com_v2_param.AuotReverseShutVoltB>>8)&0xff);       // 自动模式反向B端关闭电压
            tx_data[4] = (wg_com_v2_param.AuotReverseShutVoltB&0xff);            // 自动模式反向B端关闭电压
            break;
        default:
            return;
    }
    bsp_rvc_can_tx(tx_can_id, tx_data, 8);
}

static void handle_manufacturer_data_w(uint32_t gdn, uint8_t *data, uint8_t len)
{
    if (len < 7) {
        return;  // 数据长度不足
    }

    // 检查实例号
    if ((data[0] != 0xFF) && (data[0] != g_my_instance)) {
        return;  // 不是请求我的
    }

    // 提取发送者地址
    //uint32_t tx_can_id = build_can_id(6, RVC_DGN_ACKNOWLEDGEMENT, rvc_address_get_current());
    //uint8_t tx_data[8];
    uint16_t data_u16 = 0;
    switch(gdn){
        case RVC_DGN_PROPRIETARY_SN_SERIAL_W:
            switch(data[1])
            {
                case 0:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.SnSerial[0]);
                    data_u16 = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.SnSerial[1]);
                    data_u16 = (data[6]<<8)+data[7];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.SnSerial[2]);
                    break;
                case 1:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.SnSerial[3]);
                    data_u16 = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.SnSerial[4]);
                    data_u16 = (data[6]<<8)+data[7];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.SnSerial[5]);
                    break;
                case 2:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.SnSerial[6]);
                    data_u16 = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.SnSerial[7]);
                    data_u16 = (data[6]<<8)+data[7];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.SnSerial[8]);
                    break;
                case 3:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.SnSerial[9]);
                    break;
                default:
                    // 未知 DGN，忽略
                    return;
            }
            handle_manufacturer_data(RVC_DGN_PROPRIETARY_SN_SERIAL_R, data, 8);
            break;
        case RVC_DGN_PROPRIETARY_PRODUCT_NAME_W:
            switch(data[1])
            {
                case 0:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ProductName[0]);
                    data_u16 = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ProductName[1]);
                    data_u16 = (data[6]<<8)+data[7];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ProductName[2]);
                    break;
                case 1:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ProductName[3]);
                    data_u16 = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ProductName[4]);
                    data_u16 = (data[6]<<8)+data[7];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ProductName[5]);
                    break;
                case 2:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ProductName[6]);
                    data_u16 = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ProductName[7]);
                    data_u16 = (data[6]<<8)+data[7];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ProductName[8]);
                    break;
                case 3:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ProductName[9]);
                    break;
                default:
                    // 未知 DGN，忽略
                    return;
            }
            handle_manufacturer_data(RVC_DGN_PROPRIETARY_PRODUCT_NAME_R, data, 8);
            break;
        case RVC_DGN_PROPRIETARY_ADDRE_APPLI_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.Address);
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.ApplicationScenarios);
            handle_manufacturer_data(RVC_DGN_PROPRIETARY_ADDRE_APPLI_R, data, 8);
            break;
        case RVC_DGN_PROPRIETARY_CUST_BT_NAME_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.CustomizationVersion);
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.BtName);
            handle_manufacturer_data(RVC_DGN_PROPRIETARY_CUST_BT_NAME_R, data, 8);
        case RVC_DGN_PROPRIETARY_MAC_ADDRESS_W:
            switch(data[1])
            {
                case 0:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.MacAddress[0]);
                    data_u16 = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.MacAddress[1]);
                    data_u16 = (data[6]<<8)+data[7];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.MacAddress[2]);
                    break;
                case 1:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.MacAddress[3]);
                    data_u16 = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.MacAddress[4]);
                    data_u16 = (data[6]<<8)+data[7];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.MacAddress[5]);
                    break;
                case 2:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.MacAddress[6]);
                    data_u16 = (data[4]<<8)+data[5];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.MacAddress[7]);
                    data_u16 = (data[6]<<8)+data[7];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.MacAddress[8]);
                    break;
                case 3:
                    data_u16 = (data[2]<<8)+data[3];
                    WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_product_info.MacAddress[9]);
                    break;
                default:
                    // 未知 DGN，忽略
                    return;
            }
            handle_manufacturer_data(RVC_DGN_PROPRIETARY_MAC_ADDRESS_R, data, 8);
            break;
        default:
            // 未知 DGN，忽略
            break;
    }
}

static void handle_control_settings_w(uint32_t gdn, uint8_t *data, uint8_t len)
{
    if (len < 7) {
        return;  // 数据长度不足
    }

    // 检查实例号
    if ((data[0] != 0xFF) && (data[0] != g_my_instance)) {
        return;  // 不是请求我的
    }

    // 提取发送者地址
    //uint32_t tx_can_id = 0;//build_can_id(6, RVC_DGN_ACKNOWLEDGEMENT, rvc_address_get_current());
    //uint8_t tx_data[8];
    uint32_t send_gnd = 0;
    uint16_t data_u16 = 0;
    switch(gdn){
        case RVC_DGN_PROPRIETARY_ON_Off_SETPOWER_SET_CHARG_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.PowerOnOff);
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.SetPowerMode);
            data_u16 = (data[6]<<8)+data[5];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.SetChargMode);
            send_gnd = RVC_DGN_PROPRIETARY_ON_Off_SETPOWER_SET_CHARG_R;
            break;
        case RVC_DGN_PROPRIETARY_FACTORY_RESET_RESET_FACTORY_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.FactoryReset);
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.ResetFactoryData);
            send_gnd = RVC_DGN_PROPRIETARY_FACTORY_RESET_RESET_FACTORY_R;
            break;
        case RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTA_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.InpBatyType);
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.SetBootTimeA);
            data_u16 = (data[6]<<8)+data[5];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.SetOnCurrStartTimeA);
            send_gnd = RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTA_R;
            break;
        case RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTB_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.OutBatyType);
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.SetBootTimeB);
            data_u16 = (data[6]<<8)+data[5];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.SetOnCurrStartTimeB);
            send_gnd = RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTB_R;
            break;
        case RVC_DGN_PROPRIETARY_ZERO_CURR_BAT_MODE_FR_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.ZeroCurrCalibration);
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_ctrl.BatModeFR);
            send_gnd = RVC_DGN_PROPRIETARY_ZERO_CURR_BAT_MODE_FR_R;
            break;
        default:
            // 未知 DGN，忽略
            break;
    }
    handle_control_settings(send_gnd, data, 8);
}

static void handle_set_parameter_area_w(uint32_t gdn, uint8_t *data, uint8_t len)
{
    if (len < 7) {
        return;  // 数据长度不足
    }

    // 检查实例号
    if ((data[0] != 0xFF) && (data[0] != g_my_instance)) {
        return;  // 不是请求我的
    }

    // 提取发送者地址
    //uint32_t tx_can_id = 0;//build_can_id(6, RVC_DGN_ACKNOWLEDGEMENT, rvc_address_get_current());
    //uint8_t tx_data[8];
    uint32_t send_gnd = 0;
    uint16_t data_u16 = 0;
    switch(gdn){
        case RVC_DGN_PROPRIETARY_SET_AVOLT_ACURR_APOWER_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetInpVolt);          // A端电压
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetInpCurr);          // A端电流
            data_u16 = (data[6]<<8)+data[5];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetInpCurrPower);     // A端功率
            send_gnd = RVC_DGN_PROPRIETARY_SET_AVOLT_ACURR_APOWER_R;
            break;
        case RVC_DGN_PROPRIETARY_SET_BVOLT_BCURR_BPOWER_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetOutVolt);          // B端电压
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetOutCurr);          // B端电流
            data_u16 = (data[6]<<8)+data[5];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetOutCurrPower);     // B端功率
            send_gnd = RVC_DGN_PROPRIETARY_SET_BVOLT_BCURR_BPOWER_R;
            break;
        case RVC_DGN_PROPRIETARY_SET_AUVLO_AUVLO_RECOVER_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetInpUvlo);          // A端欠压保护
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetInpUvloRecover);   // A端欠压保护恢复
            send_gnd = RVC_DGN_PROPRIETARY_SET_AUVLO_AUVLO_RECOVER_R;
            break;
        case RVC_DGN_PROPRIETARY_SET_AOVP_AUVLO_OVPRECOVER_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetInpOVP);           // A端过压保护
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetInpOVPRecover);    // A端过压保护恢复
            send_gnd = RVC_DGN_PROPRIETARY_SET_AOVP_AUVLO_OVPRECOVER_R;
            break;
        case RVC_DGN_PROPRIETARY_SET_BUVLO_BUVLO_RECOVER_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetOutUvlo);           // B端欠压保护
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetOutUvloRecover);    // B端欠压保护恢复
            send_gnd = RVC_DGN_PROPRIETARY_SET_BUVLO_BUVLO_RECOVER_R;
            break;
        case RVC_DGN_PROPRIETARY_SET_BOVP_BUVLO_OVPRECOVER_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetOutOVP);            // B端过压保护
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetOutOVPRecover);     // B端过压保护恢复
            send_gnd = RVC_DGN_PROPRIETARY_SET_BOVP_BUVLO_OVPRECOVER_R;
            break;
        case RVC_DGN_PROPRIETARY_SET_T1_T2_TA_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetInsideTemp);         // T1温度
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetOutsideTemp);        // T2温度
            data_u16 = (data[6]<<8)+data[5];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetTemp2);              // Ta温度
            send_gnd = RVC_DGN_PROPRIETARY_SET_T1_T2_TA_R;
            break;
        case RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetInpChargLedCurr);    // A端充电指示灯电流
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetInpFullLedCurr);     // A端充满指示灯电流
            send_gnd = RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_R;
            break;
        case RVC_DGN_PROPRIETARY_SET_BCHARG_BFULL_LED_CURR_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetOutChargLedCurr);    // B端充电指示灯电流
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.SetOutFullLedCurr);     // B端充满指示灯电流  
            send_gnd = RVC_DGN_PROPRIETARY_SET_BCHARG_BFULL_LED_CURR_R;
            break;
        case RVC_DGN_PROPRIETARY_AUOT_OPEN_VEER_SHUT_VOLT_A_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.AuotForwardOpenVoltA);  // 自动模式正向A端开启电压
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.AuotForwardVeerVoltA);  // 自动模式正向转向A电压
            data_u16 = (data[6]<<8)+data[5];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.AuotForwardShutVoltA);  // 自动模式正向A端关闭电压
            send_gnd = RVC_DGN_PROPRIETARY_AUOT_OPEN_VEER_SHUT_VOLT_A_R;
            break;
        case RVC_DGN_PROPRIETARY_AUOT_OPEN_SHUT_VOLT_B_W:
            data_u16 = (data[2]<<8)+data[1];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.AuotReverseOpenVoltB);  // 自动模式反向B端开启电压
            data_u16 = (data[4]<<8)+data[3];
            WG_COM_V2_SET_DATA_UINT(data_u16, wg_com_v2_param.AuotReverseShutVoltB);  // 自动模式反向B端关闭电压
            send_gnd = RVC_DGN_PROPRIETARY_AUOT_OPEN_SHUT_VOLT_B_R;
            break;
        default:
            return;

    }
    handle_set_parameter_area(send_gnd, data, 8);
}

void rvc_message_handler_process(uint32_t can_id, uint8_t *data, uint8_t len)
{
    // 提取 DGN
    uint32_t dgn = (can_id >> 8) & 0x1FFFF;

    // 根据 DGN 分发处理
    switch (dgn) {
        case RVC_DGN_CHARGER_COMMAND:
            handle_charger_command(can_id, data, len);
            break;

        case RVC_DGN_REQUEST_FOR_DGN:
            handle_request_for_dgn(can_id, data, len);
            break;

        case RVC_DGN_CHARGER_CONFIGURATION_COMMAND:
            // TODO: 实现配置命令处理
            break;

        case RVC_DGN_PROPRIETARY_PROTOCOL_VERSION_R:
        case RVC_DGN_PROPRIETARY_PRODUCT_TYPE_R:
        case RVC_DGN_PROPRIETARY_HARDVER_VERSION_R:
        case RVC_DGN_PROPRIETARY_SOFT_VERSION_R:
        case RVC_DGN_PROPRIETARY_SN_SERIAL_R:
        case RVC_DGN_PROPRIETARY_PRODUCT_NAME_R:
        case RVC_DGN_PROPRIETARY_ADDRE_APPLI_R:
        case RVC_DGN_PROPRIETARY_CUST_BT_NAME_R:
        case RVC_DGN_PROPRIETARY_MAC_ADDRESS_R:
            handle_manufacturer_data(dgn, data, len);
            break;

        case RVC_DGN_PROPRIETARY_AVOLT_ACURR_APOWER_R:
        case RVC_DGN_PROPRIETARY_BVOLT_BCURR_BPOWER_R:
        case RVC_DGN_PROPRIETARY_T1_T2_TA_R:
        case RVC_DGN_PROPRIETARY_POWER_CHARG_STATE_CHARGEE_R:
        case RVC_DGN_PROPRIETARY_FAULT_SIGN_ALARM_SIGN_R:
        case RVC_DGN_PROPRIETARY_VOLTA_VOLTB_ADDVOLT_R:
            handle_real_time_data(dgn, data, len);
            break;

        case RVC_DGN_PROPRIETARY_ON_Off_SETPOWER_SET_CHARG_R:
        case RVC_DGN_PROPRIETARY_FACTORY_RESET_RESET_FACTORY_R:
        case RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTA_R:
        case RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTB_R:
        case RVC_DGN_PROPRIETARY_ZERO_CURR_BAT_MODE_FR_R:
            handle_control_settings(dgn, data, len);
            break;

        case RVC_DGN_PROPRIETARY_SET_AVOLT_ACURR_APOWER_R:
        case RVC_DGN_PROPRIETARY_SET_BVOLT_BCURR_BPOWER_R:
        case RVC_DGN_PROPRIETARY_SET_AUVLO_AUVLO_RECOVER_R:
        case RVC_DGN_PROPRIETARY_SET_AOVP_AUVLO_OVPRECOVER_R:
        case RVC_DGN_PROPRIETARY_SET_BUVLO_BUVLO_RECOVER_R:
        case RVC_DGN_PROPRIETARY_SET_BOVP_BUVLO_OVPRECOVER_R:
        case RVC_DGN_PROPRIETARY_SET_T1_T2_TA_R:
        case RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_R:
        case RVC_DGN_PROPRIETARY_SET_BCHARG_BFULL_LED_CURR_R:
        case RVC_DGN_PROPRIETARY_AUOT_OPEN_VEER_SHUT_VOLT_A_R:
        case RVC_DGN_PROPRIETARY_AUOT_OPEN_SHUT_VOLT_B_R:
            handle_set_parameter_area(dgn, data, len);
            break;

        case RVC_DGN_PROPRIETARY_SN_SERIAL_W:
        case RVC_DGN_PROPRIETARY_PRODUCT_NAME_W:
        case RVC_DGN_PROPRIETARY_ADDRE_APPLI_W:
        case RVC_DGN_PROPRIETARY_CUST_BT_NAME_W:
        case RVC_DGN_PROPRIETARY_MAC_ADDRESS_W:
            handle_manufacturer_data_w(dgn, data, len);
            break;

        case RVC_DGN_PROPRIETARY_ON_Off_SETPOWER_SET_CHARG_W:
        case RVC_DGN_PROPRIETARY_FACTORY_RESET_RESET_FACTORY_W:
        case RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTA_W:
        case RVC_DGN_PROPRIETARY_BATY_TYPE_BOOT_CURR_STARTB_W:
        case RVC_DGN_PROPRIETARY_ZERO_CURR_BAT_MODE_FR_W:
            handle_control_settings_w(dgn, data, len);
            break;

        case RVC_DGN_PROPRIETARY_SET_AVOLT_ACURR_APOWER_W:
        case RVC_DGN_PROPRIETARY_SET_BVOLT_BCURR_BPOWER_W:
        case RVC_DGN_PROPRIETARY_SET_AUVLO_AUVLO_RECOVER_W:
        case RVC_DGN_PROPRIETARY_SET_AOVP_AUVLO_OVPRECOVER_W:
        case RVC_DGN_PROPRIETARY_SET_BUVLO_BUVLO_RECOVER_W:
        case RVC_DGN_PROPRIETARY_SET_BOVP_BUVLO_OVPRECOVER_W:
        case RVC_DGN_PROPRIETARY_SET_T1_T2_TA_W:
        case RVC_DGN_PROPRIETARY_SET_ACHARG_AFULL_LED_CURR_W:
        case RVC_DGN_PROPRIETARY_SET_BCHARG_BFULL_LED_CURR_W:
        case RVC_DGN_PROPRIETARY_AUOT_OPEN_VEER_SHUT_VOLT_A_W:
        case RVC_DGN_PROPRIETARY_AUOT_OPEN_SHUT_VOLT_B_W:
            handle_set_parameter_area_w(dgn, data, len);
            break;

        default:
            // 未知 DGN，忽略
            break;
    }
}

uint8_t rvc_send_ack(uint8_t ack_code, uint8_t instance, uint32_t dgn_acked, uint8_t target_sa)
{
    uint32_t can_id = build_can_id(6, RVC_DGN_ACKNOWLEDGEMENT, target_sa);

    uint8_t data[8];
    data[0] = ack_code;
    data[1] = instance;
    data[2] = 0xFF;  // Instance Bank
    data[3] = 0xFF;  // Reserved
    data[4] = rvc_address_get_current();  // Source Address
    data[5] = dgn_acked & 0xFF;
    data[6] = (dgn_acked >> 8) & 0xFF;
    data[7] = (dgn_acked >> 16) & 0x01;

    return bsp_rvc_can_tx(can_id, data, 8);
}

uint8_t rvc_send_charger_status(uint8_t instance, float voltage, float current,
                              uint8_t current_percent, uint8_t state)
{
    uint32_t can_id = build_can_id(6, RVC_DGN_CHARGER_STATUS, 0xFF);

    uint16_t voltage_raw = (uint16_t)(voltage / 0.05f);
    uint16_t current_raw = (uint16_t)(current / 0.05f);

    uint8_t data[8];
    data[0] = instance;
    data[1] = voltage_raw & 0xFF;
    data[2] = (voltage_raw >> 8) & 0xFF;
    data[3] = current_raw & 0xFF;
    data[4] = (current_raw >> 8) & 0xFF;
    data[5] = current_percent;
    data[6] = state;
    data[7] = 0x01;  // Default state on power-up = Enabled

    return bsp_rvc_can_tx(can_id, data, 8);
}

uint8_t rvc_send_charger_status_2(uint8_t instance, float voltage, float current, int8_t temperature)
{
    uint32_t can_id = build_can_id(6, RVC_DGN_CHARGER_STATUS_2, 0xFF);

    uint16_t voltage_raw = (uint16_t)(voltage / 0.05f);
    uint16_t current_raw = (uint16_t)(current / 0.05f);
    uint8_t temp_raw = (temperature >= -40) ? (temperature + 40) : 0xFF;

    uint8_t data[8];
    data[0] = instance;
    data[1] = 0xFF;  // DC Source Instance (deprecated)
    data[2] = 0x00;  // Charger Priority (0=unassigned)
    data[3] = voltage_raw & 0xFF;
    data[4] = (voltage_raw >> 8) & 0xFF;
    data[5] = current_raw & 0xFF;
    data[6] = (current_raw >> 8) & 0xFF;
    data[7] = temp_raw;

    return bsp_rvc_can_tx(can_id, data, 8);
}

/* ========== 内部函数实现 ========== */

static uint32_t build_can_id(uint8_t priority, uint32_t dgn, uint8_t sa)
{
    uint32_t dgn_high = (dgn >> 8) & 0x1FF;
    uint32_t dgn_low = dgn & 0xFF;

    return ((uint32_t)priority << 26) |
           (dgn_high << 16) |
           (dgn_low << 8) |
           sa;
}

static void handle_charger_command(uint32_t can_id, uint8_t *data, uint8_t len)
{
    if (len < 7) {
        return;  // 数据长度不足
    }

    // 解析命令
    rvc_charger_command_t cmd;
    cmd.instance = data[0];
    cmd.status = data[1];
    cmd.voltage = data[3] | (data[4] << 8);
    cmd.current = data[5] | (data[6] << 8);

    // 如果不是 0 也不是 1，就拒绝
    if ((cmd.instance != 0) && (cmd.instance != g_my_instance)) {
        return;
    }

    // 提取发送者地址
    uint8_t sender_sa = (can_id >> 8) & 0xFF;  // DGN-Low 就是发送者地址

    // 调用回调函数
    if (g_charger_command_cb != NULL) {
        g_charger_command_cb(&cmd);
    }

    // 发送 ACK
    rvc_send_ack(0x00, cmd.instance, RVC_DGN_CHARGER_COMMAND, sender_sa);
}

static void handle_request_for_dgn(uint32_t can_id, uint8_t *data, uint8_t len)
{
    if (len < 4) {
        return;  // 数据长度不足
    }

    // 解析请求
    rvc_request_for_dgn_t req;
    req.requested_dgn = data[0] | (data[1] << 8) | ((data[2] & 0x01) << 16);
    req.instance = data[3];
    req.sender_sa = (can_id >> 8) & 0xFF;  // DGN-Low

    // 检查实例号
    if ((req.instance != 0xFF) && (req.instance != g_my_instance)) {
        return;  // 不是请求我的
    }

    // 调用回调函数
    if (g_request_for_dgn_cb != NULL) {
        g_request_for_dgn_cb(&req);
    }

    // 如果请求 ADDRESS_CLAIM，重新发送
    if (req.requested_dgn == 0xEE00) {
        uint32_t claim_can_id = build_can_id(6, 0xEE00, 0xFF);
        rvc_name_t name;
        rvc_address_get_name(&name);
        bsp_rvc_can_tx(claim_can_id, (uint8_t *)&name, 8);
    }
}



/* ========== 充电器状态变量 ========== */

typedef struct {
    uint8_t enabled;
    float target_voltage;
    float target_current;
    float actual_voltage;
    float actual_current;
    uint8_t state;  // 0=禁用, 1=未充电, 2=Bulk, 7=CC/CV
} charger_state_t;

static charger_state_t g_charger_state = {
    .enabled = 0,
    .target_voltage = 24.0f,
    .target_current = 10.0f,
    .actual_voltage = 0.0f,
    .actual_current = 0.0f,
    .state = 0
};

/**
 * @brief 处理 CHARGER_COMMAND
 */
void on_charger_command(const rvc_charger_command_t *cmd)
{
    // 执行命令
    if (cmd->status == 1) {
        // 使能充电器
        g_charger_state.enabled = 1;
        g_charger_state.target_voltage = cmd->voltage * 0.05f;
        g_charger_state.target_current = cmd->current * 0.05f;
        g_charger_state.state = 7;  // CC/CV 模式

        // TODO: 启动实际的充电硬件
        // charger_hardware_enable();
        // charger_hardware_set_voltage(g_charger_state.target_voltage);
        // charger_hardware_set_current(g_charger_state.target_current);

    } else {
        // 禁用充电器
        g_charger_state.enabled = 0;
        g_charger_state.state = 0;

        // TODO: 关闭实际的充电硬件
        // charger_hardware_disable();
    }
}

/**
 * @brief 处理 REQUEST_FOR_DGN
 */
void on_request_for_dgn(const rvc_request_for_dgn_t *req)
{

}

void message_init(void)
{
    /* 初始化 RV-C 地址管理器 */
    rvc_address_init(RVC_ADDRESS_MODE_STATIC);
    /* 初始化消息处理器 */
    rvc_message_handler_init();

    /* 注册回调函数 */
    rvc_register_charger_command_callback(on_charger_command);
    rvc_register_request_for_dgn_callback(on_request_for_dgn);

    /* 启动地址声明流程 */
    if (!rvc_address_claim_start()) {
        //printf("Failed to start address claim!\r\n");
    }

}
#if(CAN_ON_OFF == 1)
REG_INIT_TP(message_init)
#endif

extern uint32_t systemtime;
void message_rum(void)
{
    static uint32_t last_status_time = 0;
    static uint32_t last_status2_time = 0;
    static uint32_t last_dm_rv_time = 0;
    uint32_t now = systemtime;

    /* 周期调用地址管理器 */
    rvc_address_process();

    /* 检查地址状态 */
    if (rvc_address_is_claimed()) {
        uint8_t my_addr = rvc_address_get_current();

        /* 周期发送 CHARGER_STATUS */
        uint32_t status_interval = g_charger_state.enabled ? 500 : 5000;
        if (now - last_status_time >= status_interval) {
            last_status_time = now;

            rvc_send_charger_status(g_my_instance,
                                   g_charger_state.target_voltage,
                                   g_charger_state.target_current,
                                   80,
                                   g_charger_state.state);
        }

        /* 周期发送 CHARGER_STATUS_2 (每 500ms) */
        if (now - last_status2_time >= 500) {
            last_status2_time = now;

            // TODO: 读取实际的电压/电流/温度
            // g_charger_state.actual_voltage = charger_hardware_read_voltage();
            // g_charger_state.actual_current = charger_hardware_read_current();

            // 模拟数据
            g_charger_state.actual_voltage = g_charger_state.target_voltage;
            g_charger_state.actual_current = g_charger_state.enabled ? 8.5f : 0.0f;

            rvc_send_charger_status_2(g_my_instance,
                                     g_charger_state.actual_voltage,
                                     g_charger_state.actual_current,
                                     25);
        }

        /* 周期发送 DM_RV（每 5000ms） */
        if (now - last_dm_rv_time >= 5000) {
            last_dm_rv_time = now;
            
            // 检查是否有故障
            uint16_t spn = 0;  // 0 = 无故障
            uint8_t fmi = 0;
            
            // TODO: 检查实际故障
            // if (over_temperature) {
            //     spn = 110;  // 温度过高
            //     fmi = 0;    // 数据有效但超出正常范围
            // }
            
            rvc_send_dm_rv(g_my_instance, spn, fmi);
        }
    } else if (rvc_address_get_state() == RVC_ADDR_STATE_CANNOT_CLAIM) {
        // 地址冲突，无法声明地址


        // LED 指示：错误状态（闪烁）


    } else {
        // 正在声明地址
        // LED 指示：快速闪烁
    }

}

#if(CAN_ON_OFF == 1)
REG_TASK(10, message_rum)
#endif


