#include "ctrl_app.h"
#include "buck_boost.h"
#include "adc.h"
#include "data_com.h"
#include "bsp_pwm.h"
#include "section.h"
#include "my_math.h"
#include "open_loop.h"
#include "shell.h"
#include "stdint.h"
#include "fault.h"
#include "adc_check.h"
#include "get_com_data.h"
#include "mppt.h"

#ifndef CAN_ON_OFF
#define CAN_ON_OFF 1
#endif

static bsp_pwm_t bsp_pwm[BUCK_BOOST_L_LOOP_NUM];

//static buck_boost_t buck_boost;
buck_boost_t buck_boost;

CTRL_MODE_E ctrl_mode = CTRL_IDLE;

float fvs48 = 0.0f;
float ihv = 0.0f;
float rvs12_lmt = 0.0f;
float ilv = 0.0f;
float rvs12 = 0.0f;
float ila = 0.0f;
float ilb = 0.0f;
float fvs48_lmt = 0.0f;
float ihv_lmt = 0.0f;
float ilv_lmt = 0.0f;
float fvs48_pwr_lmt = 0.0f;
float rvs12_pwr_lmt = 0.0f;
static mppt_cfg_para_t mppt;
static CTRL_MODE_E mppt_ctrl_mode = CTRL_IDLE;
static float mppt_vin_flt = 0.0f;
static float mppt_pin_flt = 0.0f;
//uint8_t mppt_mode = 1;

#if (SHELL_ON_OFF == 1)
REG_SHELL_VAR(ADC_FVS48_OBS, fvs48, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ADC_IHV_OBS, ihv, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ADC_ILV_OBS, ilv, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ADC_RVS12_OBS, rvs12, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ADC_ILA_OBS, ila, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ADC_ILB_OBS, ilb, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ILA_ACT, buck_boost.inter.l_loop[0].input.i_act, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ILA_REF, buck_boost.inter.l_loop[0].input.i_ref, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ILA_VL, buck_boost.inter.l_loop[0].output.l_volt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ILB_ACT, buck_boost.inter.l_loop[1].input.i_act, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ILB_REF, buck_boost.inter.l_loop[1].input.i_ref, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
REG_SHELL_VAR(ILB_VL, buck_boost.inter.l_loop[1].output.l_volt, SHELL_FP32, 10000.0f, -10000.0f, NULL, SHELL_STA_NULL)
//REG_SHELL(mppt_mode, mppt_mode, SHELL_DATA_UINT8_T, 0, NULL)

#endif
static void set_mppt_ref(float val)
{
    SetMpptVoltRef(&mppt, val);
}

static float get_mppt_ref(void)
{
    return GetMpptVoltRef(&mppt);
}

static float mppt_get_volt_fdk(void)
{
    return mppt_vin_flt;
}

static float mppt_get_pwr_fdk(void)
{
    return mppt_pin_flt;
}

static float mppt_get_input_volt(CTRL_MODE_E mode)
{
    return (mode == CTRL_BACKWARD) ? rvs12 : fvs48;
}

static float mppt_get_input_pwr(CTRL_MODE_E mode)
{
    return (mode == CTRL_BACKWARD) ? (rvs12 * (-ilv)) : (fvs48 * ihv);
}

static float mppt_get_init_volt(CTRL_MODE_E mode)
{
    return (mode == CTRL_BACKWARD) ? adc_get_rvs12() : adc_get_fvs48();
}

static void mppt_analog_flt(void)
{
    const float vin = mppt_get_input_volt(mppt_ctrl_mode);
    const float pin = mppt_get_input_pwr(mppt_ctrl_mode);

    mppt_vin_flt = 0.02f * vin + 0.98f * mppt_vin_flt;
    mppt_pin_flt = 0.02f * pin + 0.98f * mppt_pin_flt;
}

static void mppt_init(CTRL_MODE_E mode)
{
    float init_volt = mppt_get_init_volt(mode);
    float start_volt = init_volt * 0.8f;

    DN_LMT(init_volt, 12.0f);
    DN_LMT(start_volt, 12.0f);

    mppt_ctrl_mode = mode;
    mppt_vin_flt = init_volt;
    mppt_pin_flt = 0.0f;

    mppt.setMpptRef = set_mppt_ref;
    mppt.getMpptRef = get_mppt_ref;
    mppt.getMpptVoltFdk = mppt_get_volt_fdk;
    mppt.getMpptPwrFdk = mppt_get_pwr_fdk;
    mppt.mpptStartVolt = start_volt;
    mppt.stepDeltaVolt = 1.2f;
    mppt.fastStepDeltaVolt = 1.2f;
    mppt.midStepDeltaVolt = 0.5f;
    mppt.slowStepDeltaVolt = 0.12f;
    mppt.fastStepPwrThr = 90.0f;
    mppt.slowStepPwrThr = 15.0f;
    mppt.fastStepVoltThr = 0.0f;
    mppt.slowStepVoltThr = 0.0f;
    mppt.mpptLoseCtrVoltThres = 1.0f;
    mppt.mpptSettleVoltThres = 0.25f;
    mppt.voltRef = start_volt;
    mppt.pwrStep1 = 0.0f;
    mppt.pwrStep2 = 0.0f;
    mppt.pwrStep3 = 0.0f;
    mppt.mpptVoc = init_volt;
    mppt.mpptVoltUplimit = init_volt;
    mppt.mpptVoltDnlimit = 12.0f;
    mppt.mpptLoseCtrTimeCnt = 0;
    mppt.mpptTimeThres = 500;
    mppt.mpptLoseCtrTimeThres = TIME_CNT_3S_IN_1MS / mppt.mpptTimeThres;
    mppt.mpptSettleWaitCnt = 0;
    mppt.mpptSettleWaitThres = 100;
    mppt.mpptEnableFlg = 0;
    mppt.mpptPauseFlg = 0;
    mppt.mpptTimeCnt = 0;
    mppt.mpptSubStep = MPPT_DISTURB;
    mppt.mpptGrowCnt = 0;
    mppt.mpptShrinkCnt = 0;
    mppt.mpptDir = MPPT_DIR_DECREASE;
}

void ctrl_app_init(CTRL_MODE_E mode)
{
    for (int i = 0; i < BUCK_BOOST_L_LOOP_NUM; i++)
    {
        buck_boost.cfg.l_val[i] = L_VAL;
			  buck_boost.inter.is_ccm[i]=1;
				buck_boost.inter.dcm_cnt[i]=0;		
    }
    buck_boost.cfg.c_val = V_CAP;
    buck_boost.cfg.ctrl_freq = CTRL_FREQ;
    buck_boost.cfg.ts = 1.0f / CTRL_FREQ;

    buck_boost.cfg.pwm_freq = PWM_FREQ;
    buck_boost.cfg.pwm_ts = 1.0f / PWM_FREQ;

    buck_boost_init(&buck_boost);
    
    ihv_lmt = 1.0f;/*起始限幅电流*/
    ilv_lmt = 1.0f;/*起始限幅电流*/
    fvs48_lmt = adc_get_fvs48();
    rvs12_lmt = adc_get_rvs12();
    mppt_init(mode);

    if(mode == CTRL_BACKWARD)
    {
        if(fvs48_lmt < rvs12_lmt)
        {
            buck_boost.inter.buck_boost_mode = BB_MODE_BUCK;
            buck_boost.inter.bb_mode_duty[0].inter.mode = BUCK_BOOST_MODE_BUCK;
            buck_boost.inter.bb_mode_duty[1].inter.mode = BUCK_BOOST_MODE_BUCK;
        }
        else
        {
            buck_boost.inter.buck_boost_mode = BB_MODE_BOOST;
            buck_boost.inter.bb_mode_duty[0].inter.mode = BUCK_BOOST_MODE_BOOST;
            buck_boost.inter.bb_mode_duty[1].inter.mode = BUCK_BOOST_MODE_BOOST;
        } 
    }
    else
    {
        if(fvs48_lmt > rvs12_lmt)
        {
            buck_boost.inter.buck_boost_mode = BB_MODE_BUCK;
            buck_boost.inter.bb_mode_duty[0].inter.mode = BUCK_BOOST_MODE_BUCK;
            buck_boost.inter.bb_mode_duty[1].inter.mode = BUCK_BOOST_MODE_BUCK;
        }
        else
        {
            buck_boost.inter.buck_boost_mode = BB_MODE_BOOST;
            buck_boost.inter.bb_mode_duty[0].inter.mode = BUCK_BOOST_MODE_BOOST;
            buck_boost.inter.bb_mode_duty[1].inter.mode = BUCK_BOOST_MODE_BOOST;
        }
    }


}

float get_ihv_lmt_soft_curr(void)
{
    return ihv_lmt;
}

void set_ihv_lmt_soft_curr(float val)
{
    ihv_lmt = val;
}

float get_ilv_lmt_soft_curr(void)
{
    return ilv_lmt;
}

void set_ilv_lmt_soft_curr(float val)
{
    ilv_lmt = val;
}
extern float ila_bias;
extern float ilb_bias;
extern float ihv_bias;
extern float ilv_bias;

extern float fvs48_k;
extern float rvs12_k;
extern float ihv_k;
extern float ilv_k;

extern bsp_adc_param_t bsp_adc0_param[ADC0_TABLE_MAX];
extern bsp_adc_linear_calib_t bsp_adc0_linear_calib[ADC0_TABLE_MAX];

extern bsp_adc_param_t bsp_adc1_param[ADC1_TABLE_MAX];
extern bsp_adc_linear_calib_t bsp_adc1_linear_calib[ADC1_TABLE_MAX] ;

extern uint8_t get_up_data_flag(void);
static uint32_t fault_delay = 0;
static uint8_t low_volt = 0;
uint8_t stop_soft = 0;
uint8_t stop_soft_up = 0;
#include "gpio.h"

#include "scope.h"
float ihv_lmt_act = 0;
float ihv_lmt_tag = 0;
float ilv_lmt_act = 0;
float ilv_lmt_tag = 0;
REG_SCOPE(SOFT_STOP,300,10,ihv_lmt_act,ihv_lmt_tag,ilv_lmt_act,ilv_lmt_tag)

void RAMFUNC ctrl_app_run(void)
{
     // gpio_set_db2(1);
    fvs48 = (ADC_GetValue((CM_ADC_TypeDef *)bsp_adc0_param[FVS48].adc_periph,   \
                           bsp_adc0_param[FVS48].adc_ch)        *               \
                           bsp_adc0_linear_calib[FVS48].gain    +               \
                           bsp_adc0_linear_calib[FVS48].bias)   *               \
                           fvs48_k;//adc_get_fvs48();
    ihv = (((ADC_GetValue((CM_ADC_TypeDef *)bsp_adc1_param[IHV].adc_periph,     \
                           bsp_adc1_param[IHV].adc_ch)          *               \
                           bsp_adc1_linear_calib[IHV].gain      +               \
                           bsp_adc1_linear_calib[IHV].bias)     *               \
                           ihv_k) - ihv_bias);//adc_get_ihv();
    ilv = (((ADC_GetValue((CM_ADC_TypeDef *)bsp_adc0_param[ILV].adc_periph,     \
                           bsp_adc0_param[ILV].adc_ch)          *               \
                           bsp_adc0_linear_calib[ILV].gain      +               \
                           bsp_adc0_linear_calib[ILV].bias)     *
                           ilv_k) - ilv_bias);//adc_get_ilv();
    rvs12 = (ADC_GetValue((CM_ADC_TypeDef *)bsp_adc0_param[RVS12].adc_periph,   \
                           bsp_adc0_param[RVS12].adc_ch)        *               \
                           bsp_adc0_linear_calib[RVS12].gain    +               \
                           bsp_adc0_linear_calib[RVS12].bias)   *               \
                           rvs12_k;//adc_get_rvs12();
    ila = (ADC_GetValue((CM_ADC_TypeDef *)bsp_adc0_param[ILA].adc_periph,       \
                           bsp_adc0_param[ILA].adc_ch)          *               \
                           bsp_adc0_linear_calib[ILA].gain      +               \
                           bsp_adc0_linear_calib[ILA].bias)     - ila_bias;//adc_get_ila();
    ilb = (ADC_GetValue((CM_ADC_TypeDef *)bsp_adc1_param[ILB].adc_periph,       \
                           bsp_adc1_param[ILB].adc_ch)          *               \
                           bsp_adc1_linear_calib[ILB].gain      +               \
                           bsp_adc1_linear_calib[ILB].bias) - ilb_bias;//adc_get_ilb();
    fvs48_pwr_lmt = data_com_get_fvs48_pwr_lmt();
    rvs12_pwr_lmt = data_com_get_rvs12_pwr_lmt();

	 // gpio_set_db2(0);
	 // gpio_set_db2(1);
	
    const float aux_volt = adc_get_vdd_8v();
    const uint8_t is_abnormal = (aux_volt > ADC_CHECK_AUX_ABNORMAL_UP) ||
                                (aux_volt < ADC_CHECK_AUX_ABNORMAL_DN);

    if (is_abnormal)
    {
        fault_set_fault(FAULT_AUX_IS_ERR);
        ctrl_app_disable();
        ctrl_mode = CTRL_IDLE;
    }
    
    if(get_up_data_flag() == 1)
    {
        ctrl_app_disable();
        ctrl_mode = CTRL_IDLE;
    }
    
    float inp_volt = 0;
    float out_volt = 0;
	float out_curr = 0;
//	float set_curr = 0;
	if(ctrl_mode == CTRL_BACKWARD)
	{
		inp_volt = rvs12;
		uint8_t in_abnormal = (inp_volt < 9.5f);
		if(in_abnormal)
		{
			fault_set_fault(FAULT_RVS12_UVP);
			ctrl_app_disable();
			ctrl_mode = CTRL_IDLE;
		}
		if(inp_volt < get_wg_com_v2_data.com_param.SetOutUvlo-0.3f){
			low_volt = 1;
		}else{
			low_volt = 0;
		}
		out_volt = fvs48;
		out_curr = ihv;
//		set_curr = ihv_lmt;
		stop_soft_up = (((out_volt <= 5.0f)||(out_curr >= -10.0f)/* || (set_curr <= 2.0f)*/)&&(charge_state_data.soft_close_flag == 1));
	}else if(ctrl_mode == CTRL_FORWARD)
	{
		inp_volt = fvs48;
		uint8_t in_abnormal = (inp_volt < 9.5f);
		if(in_abnormal)
		{
			fault_set_fault(FAULT_FVS48_UVP);
			ctrl_app_disable();
			ctrl_mode = CTRL_IDLE;
		}
		if(inp_volt < get_wg_com_v2_data.com_param.SetInpUvlo-0.3f){
			low_volt = 1;
		}else{
			low_volt = 0;
		}
		out_volt = rvs12;
		out_curr = ilv;
//		set_curr = ilv_lmt;
		stop_soft_up = (((out_volt <= 5.0f)||(out_curr <= 10.0f)/* || (set_curr <= 2.0f)*/)&&(charge_state_data.soft_close_flag == 1));
	}else{
		stop_soft_up = 0;
	}
	
	if(stop_soft == 0){
		if(charge_state_data.soft_close_flag == 1){
			stop_soft = stop_soft_up;
			if(stop_soft == 1){
//				fault_set_fault(FAULT_SOFT_STOP);
				SCOPE_TRIGGER(SOFT_STOP);
				ctrl_mode = CTRL_IDLE;
			}
			ihv_lmt_act = ihv;
			ihv_lmt_tag = ihv_lmt;
			ilv_lmt_act = ilv;
			ilv_lmt_tag = ilv_lmt;
		}
	}else{
//		fault_set_fault(FAULT_SOFT_STOP);
		ctrl_mode = CTRL_IDLE;
	}
	if(charge_state_data.soft_close_flag == 0){stop_soft = 0;}

    if (ctrl_mode == CTRL_IDLE)   //软关电压低于2.5V或电流小5A封波
    {
        if (mppt.mpptEnableFlg != 0)
        {
            MpptDisable(&mppt);
        }

        for (uint8_t i = 0; i < BUCK_BOOST_L_LOOP_NUM; i++)
        {
            bsp_pwm[i].left_duty = 0.0f;
            bsp_pwm[i].right_duty = 0.0f;
            bsp_pwm[i].left_up_en = 0;
            bsp_pwm[i].left_dn_en = 0;
            bsp_pwm[i].right_up_en = 0;
            bsp_pwm[i].right_dn_en = 0;
        }

        bsp_pwm_set_a_duty(&bsp_pwm[0]);
        bsp_pwm_set_b_duty(&bsp_pwm[1]);
        FLT(ihv_bias, adc_get_ihv_no_bias(), 0.01f);
        FLT(ilv_bias, adc_get_ilv_no_bias(), 0.01f);
        FLT(ila_bias, adc_get_ila_no_bias(), 0.01f);
        FLT(ilb_bias, adc_get_ilb_no_bias(), 0.01f);

        adc_set_ihv_bias(ihv_bias);
        adc_set_ilv_bias(ilv_bias);
        adc_set_ila_bias(ila_bias);
        adc_set_ilb_bias(ilb_bias);
        fault_delay=0;
        return;
    }

    if(ctrl_mode == ADDRS_FORWARD)
    {
        if(rvs12 > (charge_state_data.SetOutVolt+2.0f))
        {
            if(++fault_delay >= 72){
                    fault_delay = 0;
                    fault_set_fault(FAULT_RVS12_OVP);
                    ctrl_app_disable();
                    ctrl_mode = CTRL_IDLE;
            }
        }else{
            fault_delay = 0;
        }

        if(get_show_ilv_show() > (125*1.05f))//最大电流的0.5倍
        {
            fault_set_fault(FAULT_RVS12_SCP);
            ctrl_app_disable();
            ctrl_mode = CTRL_IDLE;
        }
    }
    else if(ctrl_mode == ADDRS_BACKWARD)
    {
        if(fvs48 > (charge_state_data.SetOutVolt+2.0f))
        {
            if(++fault_delay >= 72){
                fault_delay = 0;
                fault_set_fault(FAULT_FVS48_OVP);
                ctrl_app_disable();
                ctrl_mode = CTRL_IDLE;
            }
        }else{
            fault_delay = 0;
        }

        if(get_show_ihv_show() > (125*1.05f))//最大电流的0.5倍
        {
            fault_set_fault(FAULT_FVS48_SCP);
            ctrl_app_disable();
            ctrl_mode = CTRL_IDLE;
        }
    }

    RAMP(ihv_lmt/*当前限幅电流*/, data_com_get_ihv_lmt()/*目标限幅电流*/, 0.01f/*电流软起步进，单位A/40us*/);
    RAMP(ilv_lmt/*当前限幅电流*/, data_com_get_ilv_lmt()/*目标限幅电流*/, 0.01f/*电流软起步进，单位A/40us*/);

        SCOPE_RUN(SOFT_STOP);
        
    
    if (ctrl_mode == CTRL_BACKWARD)
    {
        buck_boost.input.v_in = rvs12;
        buck_boost.input.i_in = -ilv;
        buck_boost.input.v_out_ref = fvs48_lmt;
        buck_boost.input.i_out = -ihv;
        buck_boost.input.v_out = fvs48;
        buck_boost.input.i_l[0] = -ila;
        buck_boost.input.i_l[1] = -ilb;
        buck_boost.input.vin_lmt = (charge_state_data.mppt_mode_flag == 1) ? mppt.voltRef : rvs12_lmt;
        buck_boost.input.iin_lmt = ilv_lmt;
        buck_boost.input.iout_lmt = ihv_lmt;
        buck_boost.input.pwr_in_lmt = (low_volt == 0) ? rvs12_pwr_lmt : (50.0f);
        buck_boost.input.pwr_out_lmt = fvs48_pwr_lmt;
    }
    else
    {
        buck_boost.input.v_in = fvs48;
        buck_boost.input.i_in = ihv;
        buck_boost.input.v_out_ref = rvs12_lmt;
        buck_boost.input.i_out = ilv;
        buck_boost.input.v_out = rvs12;
        buck_boost.input.i_l[0] = ila;
        buck_boost.input.i_l[1] = ilb;
        buck_boost.input.vin_lmt = ((ctrl_mode == CTRL_FORWARD) && (charge_state_data.mppt_mode_flag == 1)) ? mppt.voltRef : fvs48_lmt;
        buck_boost.input.iin_lmt = ihv_lmt;
        buck_boost.input.iout_lmt = ilv_lmt;
        buck_boost.input.pwr_in_lmt = (low_volt == 0) ? fvs48_pwr_lmt : (50.0f);
        buck_boost.input.pwr_out_lmt = rvs12_pwr_lmt;
    }
	
    if ((ctrl_mode == CTRL_FORWARD) ||
        (ctrl_mode == CTRL_BACKWARD))
    {
        buck_boost_run(&buck_boost);
    }
    else if (ctrl_mode == CTRL_BIDIRECTIONAL)
    {
        buck_boost_bidirectional(&buck_boost);
    }
	
    if ((ctrl_mode == CTRL_FORWARD) ||
        (ctrl_mode == CTRL_BIDIRECTIONAL))
    {
        for (uint8_t i = 0; i < BUCK_BOOST_L_LOOP_NUM; i++)
        {
            bsp_pwm[i].left_duty = buck_boost.output.duty[i].buck_duty;
            bsp_pwm[i].right_duty = buck_boost.output.duty[i].boost_duty;
            bsp_pwm[i].left_up_en = buck_boost.output.duty[i].buck_up_en;
            bsp_pwm[i].left_dn_en = buck_boost.output.duty[i].buck_dn_en;
            bsp_pwm[i].right_up_en = buck_boost.output.duty[i].boost_up_en;
            bsp_pwm[i].right_dn_en = buck_boost.output.duty[i].boost_dn_en;
        }
    }
    else if (ctrl_mode == CTRL_BACKWARD)
    {
        for (uint8_t i = 0; i < BUCK_BOOST_L_LOOP_NUM; i++)
        {
            bsp_pwm[i].left_duty = buck_boost.output.duty[i].boost_duty;
            bsp_pwm[i].right_duty = buck_boost.output.duty[i].buck_duty;
            bsp_pwm[i].left_up_en = buck_boost.output.duty[i].boost_up_en;
            bsp_pwm[i].left_dn_en = buck_boost.output.duty[i].boost_dn_en;
            bsp_pwm[i].right_up_en = buck_boost.output.duty[i].buck_up_en;
            bsp_pwm[i].right_dn_en = buck_boost.output.duty[i].buck_dn_en;
        }
    }
    else
    {
        for (uint8_t i = 0; i < BUCK_BOOST_L_LOOP_NUM; i++)
        {
            bsp_pwm[i].left_duty = 0.0f;
            bsp_pwm[i].right_duty = 0.0f;
            bsp_pwm[i].left_up_en = 0;
            bsp_pwm[i].left_dn_en = 0;
            bsp_pwm[i].right_up_en = 0;
            bsp_pwm[i].right_dn_en = 0;
        }
    }
	
    if ((buck_boost.inter.bb_mode_duty[0].output.is_half_freq == 1) &&
        (buck_boost.inter.bb_mode_duty[1].output.is_half_freq == 1))
    {        
        //HRPWM_SetPeriodValue_Buf(PWM_UNIT,((CTRL_PERIOD + 1) << 1)-64);
				bsp_pwm_change_half_freq_pwma();
				bsp_pwm_change_half_freq_pwmb();
    }
    else 
    {
        //HRPWM_SetPeriodValue_Buf(PWM_UNIT,CTRL_PERIOD);
				bsp_pwm_change_full_freq_pwma();
				bsp_pwm_change_full_freq_pwmb();
    }

//    buck_boost.inter.bb_mode_duty[0].output.is_half_freq
//        ? bsp_pwm_change_half_freq_pwma()
//        : bsp_pwm_change_full_freq_pwma();

//    buck_boost.inter.bb_mode_duty[1].output.is_half_freq
//        ? bsp_pwm_change_half_freq_pwmb()
//        : bsp_pwm_change_full_freq_pwmb();

    bsp_pwm_set_a_duty(&bsp_pwm[0]);
    bsp_pwm_set_b_duty(&bsp_pwm[1]);

//    bsp_pwm_update_chclt2();  // 更新通道配置
//    bsp_hrpwm1_update_trig(); // 触发hrpwm1单次缓存
}

REG_INTERRUPT(1, ctrl_app_run)

static void mppt_task(void)
{
    if ((ctrl_mode != CTRL_FORWARD) &&
        (ctrl_mode != CTRL_BACKWARD))
    {
        if (mppt.mpptEnableFlg != 0)
        {
            MpptDisable(&mppt);
        }
        return;
    }

    if (charge_state_data.mppt_mode_flag != 1)
    {
        if (mppt.mpptEnableFlg != 0)
        {
            MpptDisable(&mppt);
        }
        return;
    }

    if ((mppt_ctrl_mode != ctrl_mode) ||
        (mppt.mpptEnableFlg == 0))
    {
        mppt_init(ctrl_mode);
        MpptEnable(&mppt);
    }

    mppt_analog_flt();
    MpptProcess(&mppt);
}

REG_TASK(1, mppt_task)

void ctrl_app_buck_boost_dc_init(CTRL_MODE_E mode)
{
    ctrl_app_init(mode);
    ctrl_mode = mode;
    if (((mode == CTRL_FORWARD) ||
         (mode == CTRL_BACKWARD)) &&
         (charge_state_data.mppt_mode_flag == 1))
    {
        MpptEnable(&mppt);
    }
    else
    {
        MpptDisable(&mppt);
    }
    if (mode == CTRL_BIDIRECTIONAL)
    {
        buck_boost.inter.volt_loop.inter.pid.cfg.i_err_lmt_min = 0.0f;
        buck_boost.inter.volt_loop.inter.pid.cfg.output_lmt_min = 0.0f;
        buck_boost.inter.vin_lmt.inter.pid.cfg.i_err_lmt_min = 0.0f;
        buck_boost.inter.vin_lmt.inter.pid.cfg.output_lmt_min = 0.0f;
    }
}

void RAMFUNC ctrl_app_disable(void)
{
    MpptDisable(&mppt);
    ctrl_mode = CTRL_IDLE;
}

void ctrl_app_set_rvs12_lmt(float val)
{
    rvs12_lmt = val;
}

void ctrl_app_set_fvs48_lmt(float val)
{
    fvs48_lmt = val;
}

float ctrl_app_get_rvs12_lmt(void)
{
    return rvs12_lmt;
}

float ctrl_app_get_fvs48_lmt(void)
{
    return fvs48_lmt;
}

CTRL_MODE_E RAMFUNC ctrl_app_get_ctrl_mode(void)
{
    return ctrl_mode;
}

uint8_t RAMFUNC ctrl_app_get_is_run(void)
{
    return (ctrl_mode != CTRL_IDLE) ? 1 : 0;
}

#ifdef IS_PLECS

#include "plecs.h"

void RAMFUNC buck_boost_plecs(void)
{
    plecs_set_output(23, buck_boost.inter.l_loop[0].input.i_act);
    plecs_set_output(24, buck_boost.inter.l_loop[0].input.i_ref);
    plecs_set_output(25, ihv_bias);
    plecs_set_output(26, ilv_bias);
    plecs_set_output(27, ila_bias);
    plecs_set_output(28, ilb_bias);
}

REG_INTERRUPT(10, buck_boost_plecs);

#endif
