#include "can_wg.h"
#include "can_packet.h"
#include "wg_com_v2.h"
#include "can_comm.h"
#include "section.h"
#include "string.h"
#include "shell.h"


static float voltcomp_com = 0.0f;

static uint16_t can_wg_get_u16_le(const uint8_t *p_data)
{
    return (uint16_t)(p_data[0] | ((uint16_t)p_data[1] << 8));
}

static void can_wg_put_u16_le(uint8_t *p_data, uint16_t data)
{
    p_data[0] = (uint8_t)(data & 0x00FFU);
    p_data[1] = (uint8_t)((data >> 8) & 0x00FFU);
}

static void can_wg_put_reg_u16_le(uint8_t *p_data, const void *reg)
{
    can_wg_put_u16_le(p_data, get_uint16((uint8_t *)reg));
}

static uint8_t can_wg_write_registers_from_le(uint16_t addr, uint16_t count, const uint8_t *le_data)
{
    uint8_t reg_data[8] = {0};

    if ((count == 0U) || (count > 4U))
    {
        return 0U;
    }

    for (uint16_t i = 0; i < count; i++)
    {
        set_uint16(&reg_data[i * 2U], can_wg_get_u16_le(&le_data[i * 2U]));
    }

    return wg_com_v2_write_registers(addr, count, reg_data);
}

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
    can_wg_param_t *param_data = (can_wg_param_t *)&packet->data.raw[2];
    uint16_t count = (uint16_t)param_data->data_num + 1U;
    if (param_data->data_num >= 2)
    {
        return; // 错误的参数个数
    }
    (void)can_wg_write_registers_from_le((uint16_t)(WG_COM_V2_PARAM_ADDR + param_data->data_offset),
                                         count,
                                         param_data->data);
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
    uint16_t *p_data = (uint16_t *)&wg_com_v2_param + param_data->data_offset;
    for (uint32_t i = 0; i < pack_num; i++)
    {
        can_wg_put_reg_u16_le(&param_data->data[0], &p_data[i * 2U]);
        can_wg_put_reg_u16_le(&param_data->data[2], &p_data[(i * 2U) + 1U]);
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
    uint8_t reg_data[8] = {0};

    switch (valuetype)
    {
    case CAN_WG_CTRL_VALUETYPE_COMMON:
        set_uint16(&reg_data[0], common_ctrl->byte4.factory_reset ? 1U : 0U);
        set_uint16(&reg_data[2], common_ctrl->byte4.power_onoff ? 1U : 0U);
        set_uint16(&reg_data[4], common_ctrl->byte4.power_mode ? 1U : 0U);
        set_uint16(&reg_data[6], common_ctrl->byte4.charg_mode);
        (void)wg_com_v2_write_registers(WG_COM_V2_CTRL_ADDR, 4U, reg_data);
        break;
    case CAN_WG_CTRL_VALUETYPE_BATTERY_TYPE:
        (void)can_wg_write_registers_from_le((uint16_t)(WG_COM_V2_CTRL_ADDR + 4U), 2U, p_data);
        break;
    case CAN_WG_CTRL_VALUETYPE_BOOT_TIME:
        (void)can_wg_write_registers_from_le((uint16_t)(WG_COM_V2_CTRL_ADDR + 6U), 2U, p_data);
        break;
    case CAN_WG_CTRL_VALUETYPE_SOFT_START:
        (void)can_wg_write_registers_from_le((uint16_t)(WG_COM_V2_CTRL_ADDR + 8U), 2U, p_data);
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
    can_packet_data.data.errtype = ERRTYPE_OK;

    uint8_t valuetype = packet->data.data.data_info.valueinfo.valuetype.byte2;
    can_wg_ctrl_common_t common_ctrl;
    switch (valuetype)
    {
    case CAN_WG_CTRL_VALUETYPE_COMMON:
        common_ctrl.byte4.factory_reset = get_uint16((uint8_t *)&wg_com_v2_ctrl.FactoryReset) ? 1U : 0U;
        common_ctrl.byte4.power_onoff = get_uint16((uint8_t *)&wg_com_v2_ctrl.PowerOnOff) ? 1U : 0U;
        common_ctrl.byte4.power_mode = get_uint16((uint8_t *)&wg_com_v2_ctrl.SetPowerMode) ? 1U : 0U;
        common_ctrl.byte4.charg_mode = (uint8_t)get_uint16((uint8_t *)&wg_com_v2_ctrl.SetChargMode);
        memset(common_ctrl.rsvd, 0, sizeof(common_ctrl.rsvd));
        memcpy(can_packet_data.data.data_info.valueinfo.value.bytes, &common_ctrl, sizeof(common_ctrl));
        break;
    case CAN_WG_CTRL_VALUETYPE_BATTERY_TYPE:
        can_wg_put_reg_u16_le(can_packet_data.data.data_info.valueinfo.value.bytes, &wg_com_v2_ctrl.InpBatyType);
        can_wg_put_reg_u16_le(can_packet_data.data.data_info.valueinfo.value.bytes + 2, &wg_com_v2_ctrl.OutBatyType);
        break;
    case CAN_WG_CTRL_VALUETYPE_BOOT_TIME:
        can_wg_put_reg_u16_le(can_packet_data.data.data_info.valueinfo.value.bytes, &wg_com_v2_ctrl.SetBootTimeA);
        can_wg_put_reg_u16_le(can_packet_data.data.data_info.valueinfo.value.bytes + 2, &wg_com_v2_ctrl.SetBootTimeB);
        break;
    case CAN_WG_CTRL_VALUETYPE_SOFT_START:
        can_wg_put_reg_u16_le(can_packet_data.data.data_info.valueinfo.value.bytes, &wg_com_v2_ctrl.SetOnCurrStartTimeA);
        can_wg_put_reg_u16_le(can_packet_data.data.data_info.valueinfo.value.bytes + 2, &wg_com_v2_ctrl.SetOnCurrStartTimeB);
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
