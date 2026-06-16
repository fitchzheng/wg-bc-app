#include "can_wg.h"
#include "can_packet.h"
#include "wg_com_v2.h"
#include "can_comm.h"
#include "section.h"
#include "string.h"
#include "shell.h"


static float voltcomp_com = 0.0f;

#if (SHELL_ON_OFF == 1)
static float voltcomp = 0.0f;
REG_SHELL_VAR(voltcomp, voltcomp, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
#endif

void can_wg_set_volt_comp(float val)
{
    voltcomp_com = val;
}

void can_wg_get_volt_comp(float *val)
{
    *val = 0;
}

void can_wg_handle_set_mfg(const can_packet_t *packet)
{
    can_wg_mfg_t *mfg_data = (can_wg_mfg_t *)&packet->data.raw[2];
    switch (mfg_data->byte2_3.valuetype)
    {
    case CAN_WG_MFG_VALUETYPE_PROTOCOL:
        memcpy((uint8_t *)&wg_com_v2_product_info.ProtocolVersion[0], mfg_data->byte4_7.data, 4);
        break;
    case CAN_WG_MFG_VALUETYPE_PRODUCT_TYPE:
        memcpy((uint8_t *)&wg_com_v2_product_info.ProductType[0], mfg_data->byte4_7.data, 4);
        break;
    case CAN_WG_MFG_VALUETYPE_HARDWARE_VERSION:
        memcpy((uint8_t *)&wg_com_v2_product_info.HardverVerzi[0], mfg_data->byte4_7.data, 4);
        break;
    case CAN_WG_MFG_VALUETYPE_SOFTWARE_VERSION:
        memcpy((uint8_t *)&wg_com_v2_product_info.SoftVersion[0], mfg_data->byte4_7.data, 4);
        break;
    case CAN_WG_MFG_VALUETYPE_SN:
        if (mfg_data->byte2_3.offset >= 5)
        {
            return; // 错误的偏移量
        }
        memcpy((uint8_t *)&wg_com_v2_product_info.SnSerial[mfg_data->byte2_3.offset * 2], mfg_data->byte4_7.data, 4);
        break;
    case CAN_WG_MFG_VALUETYPE_PRODUCT_NAME:
        if (mfg_data->byte2_3.offset >= 5)
        {
            return; // 错误的偏移量
        }
        memcpy((uint8_t *)&wg_com_v2_product_info.ProductName[mfg_data->byte2_3.offset * 2], mfg_data->byte4_7.data, 4);
        break;
    case CAN_WG_MFG_VALUETYPE_ADDRESS:
        memcpy((uint8_t *)&wg_com_v2_product_info.Address, mfg_data->byte4_7.data, 2);
        break;
    case CAN_WG_MFG_VALUETYPE_APPLICATION_SCENARIO:
        memcpy((uint8_t *)&wg_com_v2_product_info.ApplicationScenarios, mfg_data->byte4_7.data, 2);
        break;
    case CAN_WG_MFG_VALUETYPE_PROTOCOL_CUSTOM:
        memcpy((uint8_t *)&wg_com_v2_product_info.CustomizationVersion, mfg_data->byte4_7.data, 2);
        break;
    default:
        break;
    }
}

void can_wg_handle_get_mfg(const can_packet_t *packet)
{
    can_packet_data_u can_packet_data = {0};
    can_packet_data.data.err = 0;
    can_packet_data.data.msgtype = CAN_WG_MSGTYPE_GET_MFG;
    can_packet_data.data.errtype = ERRTYPE_OK; // 无错误
    can_wg_mfg_t *mfg_data = (can_wg_mfg_t *)&can_packet_data.data.data_info.valueinfo;
    mfg_data->byte2_3.valuetype = packet->data.data.data_info.valueinfo.valuetype.byte2;
    mfg_data->byte2_3.offset = packet->data.data.data_info.valueinfo.valuetype.byte3;
    uint32_t pack_num = 0;
    uint8_t *p_data = 0;
    switch (packet->data.data.data_info.valueinfo.valuetype.byte2)
    {
    case CAN_WG_MFG_VALUETYPE_PROTOCOL:
        p_data = (uint8_t *)&wg_com_v2_product_info.ProtocolVersion[0];
        pack_num = 1;
        break;
    case CAN_WG_MFG_VALUETYPE_PRODUCT_TYPE:
        p_data = (uint8_t *)&wg_com_v2_product_info.ProductType[0];
        pack_num = 1;
        break;
    case CAN_WG_MFG_VALUETYPE_HARDWARE_VERSION:
        p_data = (uint8_t *)&wg_com_v2_product_info.HardverVerzi[0];
        pack_num = 1;
        break;
    case CAN_WG_MFG_VALUETYPE_SOFTWARE_VERSION:
        p_data = (uint8_t *)&wg_com_v2_product_info.SoftVersion[0];
        pack_num = 1;
        break;
    case CAN_WG_MFG_VALUETYPE_SN:
        if (mfg_data->byte2_3.offset >= 5)
        {
            return; // 错误的偏移量
        }
        p_data = (uint8_t *)&wg_com_v2_product_info.SnSerial[mfg_data->byte2_3.offset * 2];
        pack_num = 5;
        break;
    case CAN_WG_MFG_VALUETYPE_PRODUCT_NAME:
        if (mfg_data->byte2_3.offset >= 5)
        {
            return; // 错误的偏移量
        }
        p_data = (uint8_t *)&wg_com_v2_product_info.ProductName[mfg_data->byte2_3.offset * 2];
        pack_num = 5;
        break;
    case CAN_WG_MFG_VALUETYPE_ADDRESS:
        p_data = (uint8_t *)&wg_com_v2_product_info.Address;
        pack_num = 1;
        break;
    case CAN_WG_MFG_VALUETYPE_APPLICATION_SCENARIO:
        p_data = (uint8_t *)&wg_com_v2_product_info.ApplicationScenarios;
        pack_num = 1;
        break;
    case CAN_WG_MFG_VALUETYPE_PROTOCOL_CUSTOM:
        p_data = (uint8_t *)&wg_com_v2_product_info.CustomizationVersion;
        pack_num = 1;
        break;
    }
    for (mfg_data->byte2_3.offset = mfg_data->byte2_3.offset;
         mfg_data->byte2_3.offset < pack_num;
         mfg_data->byte2_3.offset++)
    {
        memcpy(mfg_data->byte4_7.data, p_data + mfg_data->byte2_3.offset * 4, 4);
        mfg_data->byte2_3.valuetype = CAN_WG_MFG_VALUETYPE_PROTOCOL;
        can_packet_transmit_data(
            PROTNO_WG,
            packet->id.id.ptp,     // ptp
            packet->id.id.srcaddr, // dstaddr
            packet->id.id.dstaddr, // srcaddr
            (mfg_data->byte2_3.offset >= (pack_num - 1))
                ? 0
                : 1, // cnt
            1,       // res1
            1,       // res2
            can_packet_data.raw);
    }
}

void can_wg_handle_get_rtdata(const can_packet_t *packet)
{
    can_packet_data_u can_packet_data = {0};
    can_packet_data.data.err = 0;
    can_packet_data.data.msgtype = CAN_WG_MSGTYPE_GET_RTDATA;
    can_packet_data.data.errtype = ERRTYPE_OK; // 无错误

    can_wg_get_rtdata_t *rtdata = (can_wg_get_rtdata_t *)&can_packet_data.data.data_info.valueinfo;
    uint8_t pack_num = (packet->data.data.data_info.valueinfo.valuetype.byte3 / 2) + (packet->data.data.data_info.valueinfo.valuetype.byte3 % 2);

    for (rtdata->byte2_3.data_offset = 0; rtdata->byte2_3.data_offset < pack_num; rtdata->byte2_3.data_offset++)
    {
        memcpy(rtdata->byte4_7.data, (uint8_t *)((uint16_t *)(&wg_com_v2_realtime_data) + (packet->data.data.data_info.valueinfo.valuetype.byte2) + rtdata->byte2_3.data_offset * 2), 4);
        can_packet_transmit_data(
            PROTNO_WG,
            packet->id.id.ptp,     // ptp
            packet->id.id.srcaddr, // dstaddr
            packet->id.id.dstaddr, // srcaddr
            (rtdata->byte2_3.data_offset >= (pack_num - 1))
                ? 0
                : 1, // cnt
            1,       // res1
            1,       // res2
            can_packet_data.raw);
    }
}

void can_wg_handle_set_param(const can_packet_t *packet)
{
    uint8_t *data = (uint8_t *)&wg_com_v2_param;
    can_wg_param_t *param_data = (can_wg_param_t *)&packet->data.raw[2];
    if (param_data->data_num >= 2)
    {
        return; // 错误的参数个数
    }
    memcpy(data + param_data->data_offset * 4, param_data->data, 2 + 2 * param_data->data_num);
}

void can_wg_handle_get_param(const can_packet_t *packet)
{
    can_packet_data_u can_packet_data = {0};
    can_packet_data.data.err = 0;
    can_packet_data.data.msgtype = CAN_WG_MSGTYPE_GET_PARAM;
    can_packet_data.data.errtype = ERRTYPE_OK; // 无错误
    can_packet_data.data.data_info.valueinfo = packet->data.data.data_info.valueinfo;

    can_wg_param_t *param_data = (can_wg_param_t *)&can_packet_data.data.data_info.valueinfo;
    can_wg_param_t *param_packet = (can_wg_param_t *)&packet->data.raw[2];

    uint8_t pack_num = (param_packet->data_num / 2) + (param_packet->data_num % 2);
    uint8_t *p_data = (uint8_t *)((uint16_t *)&wg_com_v2_param + param_data->data_offset);
    for (uint32_t i = 0; i < pack_num; i++)
    {
        memcpy(param_data->data, p_data + i * 4, 4);
        can_packet_transmit_data(
            PROTNO_WG,
            packet->id.id.ptp,     // ptp
            packet->id.id.srcaddr, // dstaddr
            packet->id.id.dstaddr, // srcaddr
            (param_data->data_offset >= (pack_num - 1))
                ? 0
                : 1, // cnt
            1,       // res1
            1,       // res2
            can_packet_data.raw);
    }
}

void can_wg_handle_set_ctrl(const can_packet_t *packet)
{
    uint8_t valuetype = packet->data.data.data_info.valueinfo.valuetype.byte2;
    uint8_t *p_data = (uint8_t *)&packet->data.data.data_info.valueinfo.value.bytes[0];
    can_wg_ctrl_common_t *common_ctrl = (can_wg_ctrl_common_t *)p_data;
    switch (valuetype)
    {
    case CAN_WG_CTRL_VALUETYPE_COMMON:
        wg_com_v2_ctrl.FactoryReset = common_ctrl->byte4.factory_reset ? 0x0100 : 0; // 恢复出厂设置
        wg_com_v2_ctrl.PowerOnOff = common_ctrl->byte4.power_onoff ? 0x0100 : 0;     // 开关机状态
        wg_com_v2_ctrl.SetPowerMode = common_ctrl->byte4.power_mode ? 0x0100 : 0;    // 电源模式
        wg_com_v2_ctrl.SetChargMode = common_ctrl->byte4.charg_mode;                 // 充电模式
        break;
    case CAN_WG_CTRL_VALUETYPE_BATTERY_TYPE:
        memcpy((uint8_t *)&wg_com_v2_ctrl.InpBatyType, p_data, 2);     // A端电池类型
        memcpy((uint8_t *)&wg_com_v2_ctrl.OutBatyType, p_data + 2, 2); // B端电池类型
        break;
    case CAN_WG_CTRL_VALUETYPE_BOOT_TIME:
        memcpy((uint8_t *)&wg_com_v2_ctrl.SetBootTimeA, p_data, 2);     // A端开机时间
        memcpy((uint8_t *)&wg_com_v2_ctrl.SetBootTimeB, p_data + 2, 2); // B端开机时间
        break;
    case CAN_WG_CTRL_VALUETYPE_SOFT_START:
        memcpy((uint8_t *)&wg_com_v2_ctrl.SetOnCurrStartTimeA, p_data, 2);     // A端软起动时间
        memcpy((uint8_t *)&wg_com_v2_ctrl.SetOnCurrStartTimeB, p_data + 2, 2); // B端软起动时间
        break;
    default:
        break;
    }
}

void can_wg_handle_get_ctrl(const can_packet_t *packet)
{
    can_packet_data_u can_packet_data = {0};
    can_packet_data.data.err = 0;
    can_packet_data.data.msgtype = CAN_WG_MSGTYPE_GET_CTRL;
    can_packet_data.data.errtype = ERRTYPE_OK; // 无错误

    uint8_t valuetype = packet->data.data.data_info.valueinfo.valuetype.byte2;
    can_wg_ctrl_common_t common_ctrl;
    switch (valuetype)
    {
    case CAN_WG_CTRL_VALUETYPE_COMMON:
        common_ctrl.byte4.factory_reset = wg_com_v2_ctrl.FactoryReset ? 1 : 0;
        common_ctrl.byte4.power_onoff = wg_com_v2_ctrl.PowerOnOff ? 1 : 0;
        common_ctrl.byte4.power_mode = wg_com_v2_ctrl.SetPowerMode ? 1 : 0;
        common_ctrl.byte4.charg_mode = wg_com_v2_ctrl.SetChargMode >> 8;
        memset(common_ctrl.rsvd, 0, sizeof(common_ctrl.rsvd));
        memcpy(can_packet_data.data.data_info.valueinfo.value.bytes, &common_ctrl, sizeof(common_ctrl));
        break;
    case CAN_WG_CTRL_VALUETYPE_BATTERY_TYPE:
        memcpy(can_packet_data.data.data_info.valueinfo.value.bytes, (uint8_t *)&wg_com_v2_ctrl.InpBatyType, 2);
        memcpy(can_packet_data.data.data_info.valueinfo.value.bytes + 2, (uint8_t *)&wg_com_v2_ctrl.OutBatyType, 2);
        break;
    case CAN_WG_CTRL_VALUETYPE_BOOT_TIME:
        memcpy(can_packet_data.data.data_info.valueinfo.value.bytes, (uint8_t *)&wg_com_v2_ctrl.SetBootTimeA, 2);
        memcpy(can_packet_data.data.data_info.valueinfo.value.bytes + 2, (uint8_t *)&wg_com_v2_ctrl.SetBootTimeB, 2);
        break;
    case CAN_WG_CTRL_VALUETYPE_SOFT_START:
        memcpy(can_packet_data.data.data_info.valueinfo.value.bytes, (uint8_t *)&wg_com_v2_ctrl.SetOnCurrStartTimeA, 2);
        memcpy(can_packet_data.data.data_info.valueinfo.value.bytes + 2, (uint8_t *)&wg_com_v2_ctrl.SetOnCurrStartTimeB, 2);
        break;
    default:
        return;
    }
    can_packet_data.data.data_info.valueinfo.valuetype.byte2 = valuetype;
    can_packet_data.data.data_info.valueinfo.valuetype.byte3 = 0;
    can_packet_transmit_data(
        PROTNO_WG,
        packet->id.id.ptp,
        packet->id.id.srcaddr,
        packet->id.id.dstaddr,
        0, 1, 1,
        can_packet_data.raw);
}

static uint32_t timeout = 0;

void can_wg_get_master_voltcomp(const can_packet_t *packet)
{
    timeout = TIME_CNT_2S_IN_1MS;
//    voltcomp = packet->data.data.data_info.valueinfo.value.f;
}

void can_timeout_func(void)
{
    if (timeout)
    {
        timeout--;
    }
//    else if (HOST_ADDR != 0x01)
//    {
//        voltcomp = 0.0f;
//    }
}

void can_comm_wg_dispatch(const can_packet_t *packet)
{
    uint8_t msgtype = packet->data.data.msgtype;
    switch (msgtype)
    {
    case CAN_WG_MSGTYPE_SET_MFG:
        can_wg_handle_set_mfg(packet);
        break;
    case CAN_WG_MSGTYPE_SET_CTRL:
        can_wg_handle_set_ctrl(packet);
        break;
    case CAN_WG_MSGTYPE_SET_PARAM:
        can_wg_handle_set_param(packet);
        break;
    case CAN_WG_MSGTYPE_GET_MFG:
        can_wg_handle_get_mfg(packet);
        break;
    case CAN_WG_MSGTYPE_GET_RTDATA:
        can_wg_handle_get_rtdata(packet);
        break;
    case CAN_WG_MSGTYPE_GET_CTRL:
        can_wg_handle_get_ctrl(packet);
        break;
    case CAN_WG_MSGTYPE_GET_PARAM:
        can_wg_handle_get_param(packet);
        break;
    case CAN_WG_MSGTYPE_VOLTCOMP:
        can_wg_get_master_voltcomp(packet);
        break;
    default:
        break;
    }
}

void can_comm_set_slave_voltcomp(void)
{
    can_packet_data_u can_packet_data = {0};
    // Implementation for setting slave voltage compensation
    if (HOST_ADDR == 0x01)
    {
        can_packet_data.data.err = 0;
        can_packet_data.data.msgtype = CAN_WG_MSGTYPE_VOLTCOMP;
        can_packet_data.data.errtype = ERRTYPE_OK; // 无错误

        memcpy(can_packet_data.data.data_info.valueinfo.value.bytes, (uint8_t *)&voltcomp_com, 4);
        can_packet_transmit_data(
            PROTNO_WG,
            0,    // ptp
            0x00, // dstaddr
            0x01, // srcaddr
            0,    // cnt
            1,    // res1
            1,    // res2
            can_packet_data.raw);
//        voltcomp = voltcomp_com;
    }
}

//REG_TASK(50, can_comm_set_slave_voltcomp);
