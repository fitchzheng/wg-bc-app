#include "mppt.h"
#include "math.h"

#define MPPT_PWR_EPSILON 0.5f
#define MPPT_DPWR_EPSILON 0.2f
#define MPPT_REL_IMPROVE_TINY 0.005f
#define MPPT_REL_IMPROVE_SMALL 0.02f

static float mppt_clampf(float val, float low, float high)
{
    if (val < low)
    {
        return low;
    }
    if (val > high)
    {
        return high;
    }
    return val;
}

static float mppt_safe_divf(float numerator, float denominator)
{
    if (fabsf(denominator) <= 1e-6f)
    {
        return 0.0f;
    }
    return numerator / denominator;
}

static float mppt_get_next_step_delta(const mppt_cfg_para_t *self,
                                      float center_pwr,
                                      float delta_pwr_step1,
                                      float delta_pwr_step2,
                                      float volt_err)
{
    float best_improve = 0.0f;
    float step_span;
    float pwr_ratio;
    float volt_ratio;
    float mix_ratio;
    float improve_ratio = 0.0f;
    if (delta_pwr_step1 > best_improve)
    {
        best_improve = delta_pwr_step1;
    }
    if (delta_pwr_step2 > best_improve)
    {
        best_improve = delta_pwr_step2;
    }

    if (self->fastStepDeltaVolt <= self->slowStepDeltaVolt)
    {
        return self->slowStepDeltaVolt;
    }

    if ((delta_pwr_step1 < 0.0f) && (delta_pwr_step2 < 0.0f))
    {
        return self->slowStepDeltaVolt;
    }

    if (fabsf(center_pwr) > MPPT_PWR_EPSILON)
    {
        improve_ratio = mppt_safe_divf(best_improve, fabsf(center_pwr));
    }

    if ((best_improve <= self->slowStepPwrThr) || (improve_ratio <= MPPT_REL_IMPROVE_TINY))
    {
        if (fabsf(volt_err) <= self->mpptSettleVoltThres)
        {
            return self->slowStepDeltaVolt;
        }
        return self->midStepDeltaVolt;
    }

    if ((improve_ratio <= MPPT_REL_IMPROVE_SMALL) &&
        (fabsf(volt_err) <= (self->mpptSettleVoltThres * 2.0f)))
    {
        return self->midStepDeltaVolt;
    }

    if ((best_improve <= (self->fastStepPwrThr * 0.5f)) ||
        (improve_ratio <= (MPPT_REL_IMPROVE_SMALL * 2.0f)))
    {
        return self->midStepDeltaVolt;
    }

    step_span = self->fastStepDeltaVolt - self->slowStepDeltaVolt;
    pwr_ratio = mppt_safe_divf(best_improve, self->fastStepPwrThr);
    volt_ratio = mppt_safe_divf(fabsf(volt_err), self->mpptSettleVoltThres * 4.0f);
    pwr_ratio = mppt_clampf(pwr_ratio, 0.0f, 1.0f);
    volt_ratio = mppt_clampf(volt_ratio, 0.0f, 1.0f);

    mix_ratio = pwr_ratio;
    if (volt_ratio > mix_ratio)
    {
        mix_ratio = volt_ratio;
    }

    return self->slowStepDeltaVolt + step_span * mix_ratio;
}

static float mppt_get_grow_step(const mppt_cfg_para_t *self, float base_step)
{
    if (self->mpptGrowCnt >= 6)
    {
        return self->fastStepDeltaVolt;
    }
    if (self->mpptGrowCnt >= 3)
    {
        return self->midStepDeltaVolt;
    }
    return base_step;
}

static float mppt_slew_step(const mppt_cfg_para_t *self, float current_step, float target_step)
{
    float step_delta;
    float max_change;

    if (target_step < self->slowStepDeltaVolt)
    {
        target_step = self->slowStepDeltaVolt;
    }
    if (target_step > self->fastStepDeltaVolt)
    {
        target_step = self->fastStepDeltaVolt;
    }

    step_delta = target_step - current_step;
    max_change = self->midStepDeltaVolt - self->slowStepDeltaVolt;
    if (max_change < 0.02f)
    {
        max_change = 0.02f;
    }
    max_change *= 0.35f;

    if (step_delta > max_change)
    {
        step_delta = max_change;
    }
    else if (step_delta < -max_change)
    {
        step_delta = -max_change;
    }

    return mppt_clampf(current_step + step_delta,
                       self->slowStepDeltaVolt,
                       self->fastStepDeltaVolt);
}

/*****************************************************************************
 * @函数名   : PvInputPowerCalculation
 * @功能     : 双扰动MPPT追踪主流程
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
void MpptProcess(mppt_cfg_para_t *self)
{
    if ((self->setMpptRef == NULL) ||
        (self->getMpptRef == NULL) ||
        (self->getMpptVoltFdk == NULL) ||
        (self->getMpptPwrFdk == NULL))
        return;
    float currStepDeltaVolt;
    float currStepVolt;
    float centerVolt;
    float sampleStepVolt;
    int16_t curStepDir;
    float deltaPwrStep1;
    float deltaPwrStep2;
    float bestDeltaPwr;
    float curVoltFdb;
    if (self->mpptEnableFlg == 0 || self->mpptPauseFlg == 0)
    {
        return;
    }
    if (++(self->mpptTimeCnt) < self->mpptTimeThres)
    { // mpptTimeThres * 任务时间，进入一次MPPT
        return;
    }
    curVoltFdb = self->getMpptVoltFdk();
    if (fabsf(curVoltFdb - self->getMpptRef()) > self->mpptSettleVoltThres)
    {
        if (self->mpptSettleWaitCnt < self->mpptSettleWaitThres)
        {
            self->mpptSettleWaitCnt++;
            return;
        }
    }
    self->mpptTimeCnt = 0;
    self->mpptSettleWaitCnt = 0;
    switch (self->mpptSubStep)
    {
    case MPPT_DISTURB:
        currStepVolt = self->getMpptRef();
        if ((currStepVolt <= (self->mpptVoltDnlimit + self->slowStepDeltaVolt)) &&
            (self->mpptDir == MPPT_DIR_DECREASE))
        {
            self->setMpptRef(self->mpptVoltDnlimit);
            self->stepDeltaVolt = self->slowStepDeltaVolt;
            self->mpptGrowCnt = 0;
            self->mpptShrinkCnt++;
            self->mpptSubStep = MPPT_DISTURB;
            break;
        }
        /* 电压跟踪不上对应的参考和反馈差值阈值，需要考虑采样误差，各项目自定，默认值可为2.0f */
        if (fabsf(curVoltFdb - currStepVolt) > self->mpptLoseCtrVoltThres)
        {
            /* 电压不受控超过一定时间给定当前电压作为参考 */
            if ((self->mpptLoseCtrTimeCnt)++ > self->mpptLoseCtrTimeThres)
            {
                self->setMpptRef(curVoltFdb);
                self->mpptDir = MPPT_DIR_DECREASE;
                self->stepDeltaVolt = self->slowStepDeltaVolt;
                self->mpptSettleWaitCnt = 0;
            }
            self->mpptSubStep = MPPT_DISTURB;
            break;
        }
        else
        {
            self->mpptLoseCtrTimeCnt = 0;
        }
        self->pwrStep1 = self->getMpptPwrFdk();
        if ((fabsf(self->pwrStep1) <= MPPT_PWR_EPSILON) &&
            (currStepVolt >= (curVoltFdb - self->slowStepDeltaVolt)))
        {
            self->mpptDir = MPPT_DIR_DECREASE;
            currStepVolt = curVoltFdb - self->midStepDeltaVolt;
            self->setMpptRef(currStepVolt);
            self->stepDeltaVolt = self->midStepDeltaVolt;
            self->mpptSettleWaitCnt = 0;
            self->mpptSubStep = MPPT_DISTURB;
            break;
        }
        curStepDir = self->mpptDir;
        currStepDeltaVolt = curStepDir * self->stepDeltaVolt;
        currStepVolt += currStepDeltaVolt;
        self->setMpptRef(currStepVolt);
        self->mpptSettleWaitCnt = 0;
        self->mpptSubStep = MPPT_REV_DISTURB;
        break;

    case MPPT_REV_DISTURB:
        self->pwrStep2 = self->getMpptPwrFdk();
        curStepDir = -self->mpptDir;
        currStepVolt = self->getMpptRef();
        currStepDeltaVolt = 2.0f * curStepDir * self->stepDeltaVolt;
        currStepVolt += currStepDeltaVolt;
        self->setMpptRef(currStepVolt);
        self->mpptSettleWaitCnt = 0;
        self->mpptSubStep = MPPT_DIR_DECIDE;
        break;
    case MPPT_DIR_DECIDE:
        self->pwrStep3 = self->getMpptPwrFdk();
        deltaPwrStep1 = self->pwrStep2 - self->pwrStep1;
        deltaPwrStep2 = self->pwrStep3 - self->pwrStep1;
        bestDeltaPwr = deltaPwrStep1;
        if (fabsf(deltaPwrStep2) > fabsf(bestDeltaPwr))
        {
            bestDeltaPwr = deltaPwrStep2;
        }
        sampleStepVolt = self->stepDeltaVolt;
        centerVolt = self->getMpptRef() + self->mpptDir * sampleStepVolt;
        {
            float target_step = mppt_get_next_step_delta(self,
                                                         self->pwrStep1,
                                                         deltaPwrStep1,
                                                         deltaPwrStep2,
                                                         curVoltFdb - centerVolt);
            self->stepDeltaVolt = mppt_slew_step(self, self->stepDeltaVolt, target_step);
        }

        if ((centerVolt <= (self->mpptVoltDnlimit + self->slowStepDeltaVolt)) &&
            (self->mpptDir == MPPT_DIR_DECREASE))
        {
            self->setMpptRef(self->mpptVoltDnlimit);
            self->stepDeltaVolt = self->slowStepDeltaVolt;
            self->mpptGrowCnt = 0;
            self->mpptShrinkCnt++;
            self->mpptSettleWaitCnt = 0;
            self->mpptSubStep = MPPT_DISTURB;
            break;
        }

        if ((fabsf(self->pwrStep1) <= MPPT_PWR_EPSILON) &&
            (centerVolt >= (curVoltFdb - self->slowStepDeltaVolt)))
        {
            self->mpptDir = MPPT_DIR_DECREASE;
            self->stepDeltaVolt = self->midStepDeltaVolt;
            self->setMpptRef(curVoltFdb - self->midStepDeltaVolt);
            self->mpptSettleWaitCnt = 0;
            self->mpptSubStep = MPPT_DISTURB;
            break;
        }

        if ((deltaPwrStep1 < 0.0f) && (deltaPwrStep2 < 0.0f))
        {
            self->mpptGrowCnt = 0;
            self->mpptShrinkCnt++;
            self->setMpptRef(centerVolt);
            self->stepDeltaVolt = mppt_slew_step(self, self->stepDeltaVolt, self->slowStepDeltaVolt);
        }
        else if ((fabsf(deltaPwrStep1) <= MPPT_DPWR_EPSILON) &&
                 (fabsf(deltaPwrStep2) <= MPPT_DPWR_EPSILON))
        {
            self->mpptGrowCnt = 0;
            self->mpptShrinkCnt++;
            self->setMpptRef(centerVolt);
            self->stepDeltaVolt = mppt_slew_step(self, self->stepDeltaVolt, self->slowStepDeltaVolt);
        }
        else if ((deltaPwrStep1 >= 0.0f) &&
                 (deltaPwrStep1 >= deltaPwrStep2))
        {
            self->mpptGrowCnt++;
            self->mpptShrinkCnt = 0;
            self->stepDeltaVolt = mppt_slew_step(self,
                                                 self->stepDeltaVolt,
                                                 mppt_get_grow_step(self, self->stepDeltaVolt));
            self->setMpptRef(centerVolt + self->mpptDir * self->stepDeltaVolt);
        }
        else if ((deltaPwrStep2 >= 0.0f) &&
                 (deltaPwrStep2 > deltaPwrStep1))
        {
            self->mpptDir = -self->mpptDir;
            self->mpptGrowCnt = 1;
            self->mpptShrinkCnt = 0;
            self->stepDeltaVolt = mppt_slew_step(self, self->stepDeltaVolt, self->midStepDeltaVolt);
            self->setMpptRef(centerVolt + self->mpptDir * self->stepDeltaVolt);
        }
        else
        {
            self->mpptGrowCnt = 0;
            self->mpptShrinkCnt++;
            self->setMpptRef(centerVolt);
            if (fabsf(bestDeltaPwr) <= self->slowStepPwrThr)
            {
                self->stepDeltaVolt = mppt_slew_step(self, self->stepDeltaVolt, self->slowStepDeltaVolt);
            }
        }
        self->mpptSettleWaitCnt = 0;
        self->mpptSubStep = MPPT_DISTURB;
        break;
    default:
        self->mpptSubStep = MPPT_DISTURB;
        break;
    }
}

/*****************************************************************************
 * @函数名   : MpptEnable
 * @功能     : 使能MPPT追踪
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
void MpptEnable(mppt_cfg_para_t *self)
{
    self->mpptDir = MPPT_DIR_DECREASE;
    self->mpptEnableFlg = 1;
    self->mpptPauseFlg = 1;
    self->mpptSubStep = MPPT_DISTURB;
    self->stepDeltaVolt = self->midStepDeltaVolt;
    self->mpptSettleWaitCnt = 0;
    /* 刚开机时从0.8Voc追踪 */
    self->setMpptRef(self->mpptStartVolt);
}

/*****************************************************************************
 * @函数名   : MpptDisable
 * @功能     : 禁能MPPT追踪，用于故障处理
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
void MpptDisable(mppt_cfg_para_t *self)
{
    self->mpptStartVolt = 0.0f;
    self->voltRef = 0.0f;
    self->pwrStep1 = 0.0f;
    self->pwrStep2 = 0.0f;
    self->pwrStep3 = 0.0f;
    self->mpptVoc = 0.0f;
    self->mpptVoltUplimit = 0.0f;
    self->mpptVoltDnlimit = 0.0f;
    self->mpptEnableFlg = 0;
    self->mpptPauseFlg = 0;
    self->mpptLoseCtrTimeCnt = 0;
    self->mpptSettleWaitCnt = 0;
    self->mpptTimeCnt = 0;
    self->mpptGrowCnt = 0;
    self->mpptShrinkCnt = 0;
}

/*****************************************************************************
 * @函数名   : MpptPause
 * @功能     : 暂停MPPT追踪，用于短时处理
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
void MpptPause(mppt_cfg_para_t *self)
{
    self->mpptPauseFlg = 0;
}

/*****************************************************************************
 * @函数名   : MpptResume
 * @功能     : 用于暂停MPPT追踪时的唤醒
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
void MpptResume(mppt_cfg_para_t *self)
{
    self->mpptPauseFlg = 1;
    self->mpptSubStep = MPPT_DISTURB;
    self->mpptSettleWaitCnt = 0;
    self->mpptGrowCnt = 0;
    self->mpptShrinkCnt = 0;
}

/*****************************************************************************
 * @函数名   : SetMpptVoltRef
 * @功能     : 设置MPPT电压参考
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
void SetMpptVoltRef(mppt_cfg_para_t *self, float voltRef)
{
    self->voltRef = voltRef;
    CLAMP(self->voltRef, self->mpptVoltDnlimit, self->mpptVoltUplimit);
}

/*****************************************************************************
 * @函数名   : GetMpptVoltRef
 * @功能     : 获取MPPT电压参考
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
float GetMpptVoltRef(mppt_cfg_para_t *self)
{
    return self->voltRef;
}

/*****************************************************************************
 * @函数名   : SetMpptVoc
 * @功能     : 设置MPPT开路电压
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
void SetMpptVoc(mppt_cfg_para_t *self, float voltRef)
{
    self->mpptVoc = voltRef;
    self->mpptVoltUplimit = voltRef;
}

/*****************************************************************************
 * @函数名   : SetMpptUpLimitVolt
 * @功能     : 设置MPPT电压上限
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
void SetMpptUpLimitVolt(mppt_cfg_para_t *self, float volt)
{
    self->mpptVoltUplimit = volt;
}

/*****************************************************************************
 * @函数名   : SetMpptDnLimitVolt
 * @功能     : 设置MPPT电压下限
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
void SetMpptDnLimitVolt(mppt_cfg_para_t *self, float volt)
{
    self->mpptVoltDnlimit = volt;
}

/*****************************************************************************
 * @函数名   : SetMpptStartVolt
 * @功能     : 设置MPPT追踪起始电压
 * @参数     : self - MPPT配置参数结构体指针
 * @返回值   : 无
 * @说明     :
 *****************************************************************************/
void SetMpptStartVolt(mppt_cfg_para_t *self, float voltRef)
{
    self->mpptStartVolt = voltRef;
    CLAMP(self->mpptStartVolt, MPPT_VOLT_MIN_COFF * self->mpptVoc, MPPT_VOLT_MAX_COFF * self->mpptVoc);
    CLAMP(self->mpptStartVolt, self->mpptVoltDnlimit, self->mpptVoltUplimit);
}

#include "get_com_data.h"
#include "wg_com_v2.h"
#include "bat_mode.h"
void mppt_mode_run(void)
{
    if(updated_parameter())
    {
        init_mppt_mode_parameter();
    }

    if((get_wg_com_v2_data.com_ctrl.SetChargMode != eSET_FORWARD))
    {
        WG_COM_V2_SET_DATA_UINT(eSET_FORWARD, wg_com_v2_ctrl.SetChargMode);
    }

    bat_b_arguments_limi();
    BattChargingCurve(&charge_state_data,0);
}



