#include "data_com.h"
#include "section.h"
#include "shell.h"
#include "stddef.h"
#include "my_math.h"
#include "adc.h"
#include "wg_com.h"
#include "wg_com_v2.h"
#include "adc_check.h"
#include "ctrl_app.h"
#include "power_sw.h"
#include "fault.h"
#include "temp_derate.h"
#include "get_com_data.h"
filter_show_data_t filter_show_data_value;
static float rvs12_lmt = 0.0f;
static float ihv_lmt = 0.0f;
static float ilv_lmt = 0.0f;
static float fvs48_lmt = 0.0f;
static float fvs48_pwr_lmt = 0.0f;
static float rvs12_pwr_lmt = 0.0f;
static float open_loop_gain = 0.0f;
static uint8_t open_loop_mode = 0;
#ifndef IS_PLECS
#if (SHELL_ON_OFF == 1)
REG_SHELL_VAR(rvs12_lmt, rvs12_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ihv_lmt, ihv_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ilv_lmt, ilv_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(fvs48_lmt, fvs48_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(fvs48_pwr_lmt, fvs48_pwr_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(rvs12_pwr_lmt, rvs12_pwr_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(open_loop_gain, open_loop_gain, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(open_loop_mode, open_loop_mode, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)
#endif
#endif

// 核心归一化函数
static void normalize_to_big_endian_bcd(float value, float gain, uint8_t *p_output)
{
    uint16_t raw = (uint16_t)(value * gain); // 放大
    uint16_t bcd = DEC_TO_BCD_16(raw);       // 转BCD编码

    // 写入大端格式，高字节在前
    p_output[0] = (bcd >> 8) & 0xFF; // 高8位
    p_output[1] = bcd & 0xFF;        // 低8位
}

// 核心解码函数
static float decode_from_big_endian_bcd(uint8_t *p_input, float gain)
{
    uint16_t bcd = ((uint16_t)p_input[0] << 8) | (uint16_t)p_input[1]; // 大端拼成16位
    uint16_t dec = BCD_TO_DEC_16(bcd);                                 // BCD转成十进制
    return ((float)dec) * gain;                                        // 还原浮点数
}

float data_com_get_rvs12_lmt(void)
{
    rvs12_lmt = charge_state_data.rvs12_lmt;
    return rvs12_lmt;
}

float RAMFUNC data_com_get_ihv_lmt(void)
{
	if(charge_state_data.check_state == eADDRS_BACKWARD){
		ihv_lmt = (charge_state_data.soft_close_flag == 1) ? (1) : (charge_state_data.ihv_lmt);
	}else{
		ihv_lmt = charge_state_data.ihv_lmt;
	}
    return ihv_lmt;
}

float RAMFUNC data_com_get_ilv_lmt(void)
{
	if(charge_state_data.check_state == eADDRS_BACKWARD){
		ilv_lmt = charge_state_data.ilv_lmt;
	}else{
		ilv_lmt = (charge_state_data.soft_close_flag == 1) ? (1) : (charge_state_data.ilv_lmt);
	}
    return ilv_lmt;
}

float data_com_get_fvs48_lmt(void)
{
    fvs48_lmt = charge_state_data.fvs48_lmt;
    return fvs48_lmt;
}

float RAMFUNC data_com_get_fvs48_pwr_lmt(void)
{
    fvs48_pwr_lmt = charge_state_data.fvs48_pwr_lmt;
    return fvs48_pwr_lmt;
}

float RAMFUNC data_com_get_rvs12_pwr_lmt(void)
{
    rvs12_pwr_lmt = charge_state_data.rvs12_pwr_lmt;
    return rvs12_pwr_lmt;
}

float data_com_get_open_loop_gain(void)
{
    return open_loop_gain;
}

uint8_t data_com_get_open_loop_mode(void)
{
    return open_loop_mode;
}

#ifdef IS_PLECS
#include "plecs.h"
#include "my_math.h"

REG_SHELL_VAR(rvs12_lmt, rvs12_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ihv_lmt, ihv_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ilv_lmt, ilv_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(fvs48_lmt, fvs48_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(fvs48_pwr_lmt, fvs48_pwr_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(rvs12_pwr_lmt, rvs12_pwr_lmt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(open_loop_gain, open_loop_gain, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(open_loop_mode, open_loop_mode, SHELL_UINT8, 0xFFu, 0u, NULL, SHELL_STA_NULL)

void data_com_run(void)
{
    rvs12_lmt = plecs_get_input(PLECS_INPUT_V_OUT_REF);
    ihv_lmt = plecs_get_input(PLECS_INPUT_I_IN_LMT);
    ilv_lmt = plecs_get_input(PLECS_INPUT_I_OUT_LMT);
    fvs48_lmt = plecs_get_input(PLECS_INPUT_V_IN_LMT);
    fvs48_pwr_lmt = plecs_get_input(PLECS_INPUT_PWR_IN_LMT);
    rvs12_pwr_lmt = plecs_get_input(PLECS_INPUT_PWR_OUT_LMT);
    open_loop_mode = plecs_get_input(PLECS_INPUT_OPEN_LOOP_MODE);
}
#else

volatile volt_protect_t volt_protect = {0};

static void set_volt_protect_func(wg_com_frame_t *p_frame)
{
    if (p_frame->length == 9)
    {
        volt_protect.uvp = decode_from_big_endian_bcd((uint8_t *)&p_frame->info[0], 0.01f);
        volt_protect.uvp_hys = decode_from_big_endian_bcd((uint8_t *)&p_frame->info[2], 0.01f);
        volt_protect.ovp = decode_from_big_endian_bcd((uint8_t *)&p_frame->info[4], 0.01f);
        volt_protect.ovp_hys = decode_from_big_endian_bcd((uint8_t *)&p_frame->info[6], 0.01f);
    }
}

REG_WG_COM(WG_COM_CMD_SET_UNDERVOLT_PROTECT, set_volt_protect_func);

static void get_volt_protect_func(wg_com_frame_t *p_frame)
{
    if (p_frame->length == 1)
    {
        volt_protect_ret_t volt_protect_ret = {0};
        normalize_to_big_endian_bcd(volt_protect.ovp, 100.0f, (uint8_t *)&volt_protect_ret.ovp);
        normalize_to_big_endian_bcd(volt_protect.ovp_hys, 100.0f, (uint8_t *)&volt_protect_ret.ovp_hys);
        normalize_to_big_endian_bcd(volt_protect.uvp, 100.0f, (uint8_t *)&volt_protect_ret.uvp);
        normalize_to_big_endian_bcd(volt_protect.uvp_hys, 100.0f, (uint8_t *)&volt_protect_ret.uvp_hys);
//        wg_com_send(WG_COM_CMD_RET_GET_UNDERVOLT,
//                    (uint8_t *)&volt_protect_ret,
//                    sizeof(volt_protect_ret));
    }
}

REG_WG_COM(WG_COM_CMD_GET_UNDERVOLT_PROTECT, get_volt_protect_func);

static void set_output_params_func(wg_com_frame_t *p_frame)
{
    if (p_frame->length == 7)
    {
        rvs12_lmt = decode_from_big_endian_bcd((uint8_t *)&p_frame->info[0], 0.01f);
        ilv_lmt = decode_from_big_endian_bcd((uint8_t *)&p_frame->info[2], 0.01f);
        rvs12_pwr_lmt = decode_from_big_endian_bcd((uint8_t *)&p_frame->info[4], 1.0f);

        output_params_ack_t output_params_ack = {0};
        output_params_ack.ack = 0;
        normalize_to_big_endian_bcd(rvs12_lmt, 100.0f, (uint8_t *)&output_params_ack.rvs12_lmt);
        normalize_to_big_endian_bcd(ilv_lmt, 100.0f, (uint8_t *)&output_params_ack.ilv_lmt);
        normalize_to_big_endian_bcd(rvs12_pwr_lmt, 1.0f, (uint8_t *)&output_params_ack.rvs12_pwr_lmt);

//        wg_com_send(WG_COM_CMD_RET_ACK_OUTPUT_PARAMS,
//                    (uint8_t *)&output_params_ack,
//                    sizeof(output_params_ack_t));
    }
}

REG_WG_COM(WG_COM_CMD_SET_OUTPUT_PARAMS, set_output_params_func);

static void get_output_params_func(wg_com_frame_t *p_frame)
{
    if (p_frame->length == 1)
    {
        output_params_ack_t output_params_ack = {0};
        output_params_ack.ack = 0;

        normalize_to_big_endian_bcd(rvs12_lmt, 100.0f, (uint8_t *)&output_params_ack.rvs12_lmt);
        normalize_to_big_endian_bcd(ilv_lmt, 100.0f, (uint8_t *)&output_params_ack.ilv_lmt);
        normalize_to_big_endian_bcd(rvs12_pwr_lmt, 1.0f, (uint8_t *)&output_params_ack.rvs12_pwr_lmt);

//        wg_com_send(WG_COM_CMD_RET_GET_SET_PARAMS,
//                    (uint8_t *)&output_params_ack,
//                    sizeof(output_params_ack_t));
    }
}

REG_WG_COM(WG_COM_CMD_GET_SET_PARAMS, get_output_params_func);

static void get_output_info_func(wg_com_frame_t *p_frame)
{
    if (p_frame->length == 1)
    {
        output_info_t output_info = {0};

        output_info.ack = 0;

        normalize_to_big_endian_bcd(get_show_rvs12_show(), 100.0f, (uint8_t *)&output_info.output_voltage);
        normalize_to_big_endian_bcd(fabsf(get_show_ilv_show()), 100.0f, (uint8_t *)&output_info.output_current);
        normalize_to_big_endian_bcd(get_show_fvs48_show(), 100.0f, (uint8_t *)&output_info.input_voltage);
        normalize_to_big_endian_bcd(fabsf(get_show_ihv_show()), 10.0f, (uint8_t *)&output_info.input_current);
        normalize_to_big_endian_bcd(25.0f, 1.0f, (uint8_t *)&output_info.temperature1);
        normalize_to_big_endian_bcd(25.0f, 1.0f, (uint8_t *)&output_info.temperature2);
        normalize_to_big_endian_bcd(25.0f, 1.0f, (uint8_t *)&output_info.temperature3);
        normalize_to_big_endian_bcd(25.0f, 1.0f, (uint8_t *)&output_info.primary_temperature);

        output_info.warning_flags = 0;
        output_info.protection_type = 0;

//        wg_com_send(WG_COM_CMD_RET_OUTPUT_INFO,
//                    (uint8_t *)&output_info,
//                    sizeof(output_info_t));
    }
}

REG_WG_COM(WG_COM_CMD_GET_OUTPUT_INFO, get_output_info_func)

void data_com_init(void)
{
    WG_COM_V2_SET_DATA_UINT((('V'<<8)+'1'),wg_com_v2_product_info.ProtocolVersion[0]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'),wg_com_v2_product_info.ProtocolVersion[1]);
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
    WG_COM_V2_SET_DATA_UINT((('-'<<8)+'C'),wg_com_v2_product_info.ProductName[1]);
    WG_COM_V2_SET_DATA_UINT((('1'<<8)+'5'),wg_com_v2_product_info.ProductName[2]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'0'),wg_com_v2_product_info.ProductName[3]);
    WG_COM_V2_SET_DATA_UINT((('B'<<8)+'M'),wg_com_v2_product_info.ProductName[4]);
    WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[5]);
    WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[6]);
    WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[7]);
    WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[8]);
    WG_COM_V2_SET_DATA_UINT(((' '<<8)+' '),wg_com_v2_product_info.ProductName[9]);
    WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.ApplicationScenarios);
    WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.CustomizationVersion);

    WG_COM_V2_SET_DATA_UINT(fvs48_lmt, wg_com_v2_param.SetInpVolt);
    WG_COM_V2_SET_DATA_UINT(ihv_lmt, wg_com_v2_param.SetInpCurr);
    WG_COM_V2_SET_DATA_UINT(fvs48_pwr_lmt, wg_com_v2_param.SetInpCurrPower);
    WG_COM_V2_SET_DATA_UINT(rvs12_lmt, wg_com_v2_param.SetOutVolt);
    WG_COM_V2_SET_DATA_UINT(ilv_lmt, wg_com_v2_param.SetOutCurr);
    WG_COM_V2_SET_DATA_UINT(rvs12_pwr_lmt, wg_com_v2_param.SetOutCurrPower);
    
    WG_COM_V2_SET_DATA_UINT(13.60f,wg_com_v2_param.AuotForwardOpenVoltA);
    WG_COM_V2_SET_DATA_UINT(12.00f,wg_com_v2_param.AuotForwardVeerVoltA);
    WG_COM_V2_SET_DATA_UINT(13.00f,wg_com_v2_param.AuotForwardShutVoltA);
    WG_COM_V2_SET_DATA_UINT(12.50f,wg_com_v2_param.AuotReverseOpenVoltB);
    WG_COM_V2_SET_DATA_UINT(12.00f,wg_com_v2_param.AuotReverseShutVoltB);
}

//REG_INIT(data_com_init)
void data_com_read_mode(void)
{
    WG_COM_V2_SET_DATA_UINT((('V'<<8)+'1'),wg_com_v2_product_info.ProtocolVersion[0]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'1'),wg_com_v2_product_info.ProtocolVersion[1]);
    WG_COM_V2_SET_DATA_UINT(0,wg_com_v2_product_info.ProductType[0]);
    WG_COM_V2_SET_DATA_UINT(5,wg_com_v2_product_info.ProductType[1]);
    WG_COM_V2_SET_DATA_UINT((('V'<<8)+'1'),wg_com_v2_product_info.HardverVerzi[0]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'3'),wg_com_v2_product_info.HardverVerzi[1]);
    WG_COM_V2_SET_DATA_UINT((('V'<<8)+'2'),wg_com_v2_product_info.SoftVersion[0]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'1'),wg_com_v2_product_info.SoftVersion[1]);
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

    WG_COM_V2_SET_DATA_UINT((('W'<<8)+'G'),wg_com_v2_product_info.SnSerial[0]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'5'),wg_com_v2_product_info.SnSerial[1]);
    WG_COM_V2_SET_DATA_UINT((('0'<<8)+'1'),wg_com_v2_product_info.SnSerial[7]);
}
REG_TASK(20, data_com_read_mode)

static float filter_show_value_adaptive(float old_value,
                                        float new_value,
                                        float snap_delta,
                                        float fast_delta)
{
    float delta = fabsf(new_value - old_value);

    if (delta >= snap_delta)
    {
        return new_value;
    }

    if (delta >= fast_delta)
    {
        return (0.5f * old_value) + (0.5f * new_value);
    }

    return (0.85f * old_value) + (0.15f * new_value);
}

void data_com_run(void)
{
    filter_show_data_value.fvs48_lmt = filter_show_value_adaptive(filter_show_data_value.fvs48_lmt, get_show_fvs48_show(), 1.0f, 0.2f);
    filter_show_data_value.ihv_lmt   = filter_show_value_adaptive(filter_show_data_value.ihv_lmt, fabsf(get_show_ihv_show()), 1.0f, 0.1f);
    filter_show_data_value.rvs12_lmt = filter_show_value_adaptive(filter_show_data_value.rvs12_lmt, get_show_rvs12_show(), 1.0f, 0.2f);
    filter_show_data_value.ilv_lmt   = filter_show_value_adaptive(filter_show_data_value.ilv_lmt, fabsf(get_show_ilv_show()), 1.0f, 0.1f);

    //WG_COM_V2_GET_DATA(fvs48_lmt, wg_com_v2_param.SetInpVolt);
    //WG_COM_V2_GET_DATA(ihv_lmt, wg_com_v2_param.SetInpCurr);
    WG_COM_V2_GET_DATA_UINT(fvs48_pwr_lmt, wg_com_v2_param.SetInpCurrPower);
    //WG_COM_V2_GET_DATA(rvs12_lmt, wg_com_v2_param.SetOutVolt);
    //WG_COM_V2_GET_DATA(ilv_lmt, wg_com_v2_param.SetOutCurr);
    WG_COM_V2_GET_DATA_UINT(rvs12_pwr_lmt, wg_com_v2_param.SetOutCurrPower);   

//    WG_COM_V2_SET_DATA_UINT(fabsf(get_show_ihv_show()), wg_com_v2_realtime_data.InpCurr);
//    WG_COM_V2_SET_DATA_UINT(fabsf(get_show_ilv_show()), wg_com_v2_realtime_data.OutCurr);
//    WG_COM_V2_SET_DATA_UINT(get_show_fvs48_show(), wg_com_v2_realtime_data.InpVolt);
//    WG_COM_V2_SET_DATA_UINT(get_show_rvs12_show(), wg_com_v2_realtime_data.OutVolt);
//    WG_COM_V2_SET_DATA_UINT(get_show_fvs48_show() * fabsf(get_show_ihv_show()), wg_com_v2_realtime_data.InpCurrPower);
//    WG_COM_V2_SET_DATA_UINT(get_show_rvs12_show() * fabsf(get_show_ilv_show()), wg_com_v2_realtime_data.OutCurrPower);
    WG_COM_V2_SET_DATA_UINT(filter_show_data_value.ihv_lmt, wg_com_v2_realtime_data.InpCurr);
    WG_COM_V2_SET_DATA_UINT(filter_show_data_value.ilv_lmt, wg_com_v2_realtime_data.OutCurr);
    WG_COM_V2_SET_DATA_UINT(filter_show_data_value.fvs48_lmt, wg_com_v2_realtime_data.InpVolt);
    WG_COM_V2_SET_DATA_UINT(filter_show_data_value.rvs12_lmt, wg_com_v2_realtime_data.OutVolt);
    WG_COM_V2_SET_DATA_UINT((filter_show_data_value.fvs48_lmt * filter_show_data_value.ihv_lmt), wg_com_v2_realtime_data.InpCurrPower);
    WG_COM_V2_SET_DATA_UINT((filter_show_data_value.rvs12_lmt * filter_show_data_value.ilv_lmt), wg_com_v2_realtime_data.OutCurrPower);

    WG_COM_V2_SET_DATA_INT(adc_check_get_ntc1_temp(), wg_com_v2_realtime_data.InsideTemp);
    WG_COM_V2_SET_DATA_INT(adc_check_get_ntc2_temp(), wg_com_v2_realtime_data.OutsideTemp);
    WG_COM_V2_SET_DATA_UINT(adc_get_accvs(), wg_com_v2_realtime_data.CompensationVoltA);
    WG_COM_V2_SET_DATA_UINT(adc_get_rmtvs(), wg_com_v2_realtime_data.CompensationVoltB);
    WG_COM_V2_SET_DATA_INT(adc_check_get_ntc3_temp(), wg_com_v2_realtime_data.Temp2);
    WG_COM_V2_SET_DATA_UINT(ADC_VDD_8V, wg_com_v2_realtime_data.ADDVolt);
}

#endif
 
REG_TASK(30, data_com_run)


