//ADC
//GD功能：
//1、ADC0与ADC1使用规则组并行模式
//2、ADC0负责 ILA,ILV,FVS48,TEMP_INT,
//3、ADC1负责 ILB,IHV,RVS12,RMTVS,	
//4、AADC0,1被TIMER1同步触发
//5、转换结果通过DMA搬运
//6、ADC2使用规则组，负责TEMP1,TEMP2,TEMP3,ACCVS,ADDRS,
//7、ADC2使用软件触发，结果也通过DMA搬运。
//8、最终获取采样值接口 bsp_adc_get_xxx

//小华F334差异:
//1、三个独立采样单元，可被同步触发
//2、每个采样通道都有独立的结果寄存器，可以不使用DMA搬运

//计划修改如下：
//1、ADC0负责 ILA,ILV,FVS48,
//2、ADC1负责 ILB,IHV,RVS12,RMTVS,
//3、ADC2负责TEMP1,TEMP2,TEMP3,ACCVS,ADDRS,
//4、三个单元同步触发，转换结果不需要DMA搬运
//5、使用bsp_adc_get_xxx接口返回采样值 

#include "bsp_adc.h"
#include "hc32_ll_aos.h"
#include "section.h"

static uint16_t __attribute__((aligned(4))) adc0_value[ADC0_TABLE_MAX];
static uint16_t __attribute__((aligned(4))) adc1_value[ADC1_TABLE_MAX];



bsp_adc_linear_calib_t bsp_adc0_linear_calib[ADC0_TABLE_MAX] = {
    [ILA] = {
        .gain = BSP_ADC_ILA_GAIN,
        .bias = BSP_ADC_ILA_BIAS,
    },
    [ILV] = {
        .gain = BSP_ADC_ILV_GAIN,
        .bias = BSP_ADC_ILV_BIAS,
    },
    [FVS48] = {
        .gain = BSP_ADC_FVS48_GAIN,
        .bias = BSP_ADC_FVS48_BIAS,
    },
    [RVS12] = {
        .gain = BSP_ADC_RVS12_GAIN,
        .bias = BSP_ADC_RVS12_BIAS,
    },
    [TEMP1] = {
        .gain = BSP_ADC_TEMP1_GAIN,
        .bias = BSP_ADC_TEMP1_BIAS,
    },
    [TEMP2] = {
        .gain = BSP_ADC_TEMP2_GAIN,
        .bias = BSP_ADC_TEMP2_BIAS,
    },
    
};

bsp_adc_linear_calib_t bsp_adc1_linear_calib[ADC1_TABLE_MAX] = {
    
    [ILB] = {
        .gain = BSP_ADC_ILB_GAIN,
        .bias = BSP_ADC_ILB_BIAS,
    },
    [IHV] = {
        .gain = BSP_ADC_IHV_GAIN,
        .bias = BSP_ADC_IHV_BIAS,
    },
    [VDD8V] = {
        .gain = BSP_ADC_VDD_8V_GAIN,
        .bias = BSP_ADC_VDD_8V_BIAS,
    },
    [ACCVS] = {
        .gain = BSP_ADC_ACCVS_GAIN,
        .bias = BSP_ADC_ACCVS_BIAS,
    },
//    [ADDRS] = {
//        .gain = BSP_ADC_ADDRS_GAIN,
//        .bias = BSP_ADC_ADDRS_BIAS,
//    },
    [TEMP3] = {
        .gain = BSP_ADC_TEMP3_GAIN,
        .bias = BSP_ADC_TEMP3_BIAS,
    },
    [RMTVS] = {
        .gain = BSP_ADC_RMTVS_GAIN,
        .bias = BSP_ADC_RMTVS_BIAS,
    },
};

const bsp_adc_param_t bsp_adc0_param[ADC0_TABLE_MAX] = {
    {
        BSP_ADC0_REG_PARM(ILA, GPIO_PORT_A, 04, 4),
    },
    {
        BSP_ADC0_REG_PARM(ILV, GPIO_PORT_C, 03, 13),
    },
    {
        BSP_ADC0_REG_PARM(FVS48, GPIO_PORT_A, 00, 0),
    },
    {
        BSP_ADC0_REG_PARM(RVS12, GPIO_PORT_A, 01, 1),
    },
    {
        BSP_ADC0_REG_PARM(TEMP1, GPIO_PORT_A, 02, 2),
    },
    {
        BSP_ADC0_REG_PARM(TEMP2, GPIO_PORT_A, 03, 3),
    },
   
};

const bsp_adc_param_t bsp_adc1_param[ADC1_TABLE_MAX] = {
    
    {
        BSP_ADC1_REG_PARM(ILB, GPIO_PORT_A, 05, 5),
    },
    {
        BSP_ADC1_REG_PARM(IHV, GPIO_PORT_A, 06, 6),
    },
    {
        BSP_ADC1_REG_PARM(VDD8V, GPIO_PORT_C, 04, 14),
    },
    {
        BSP_ADC1_REG_PARM(ACCVS, GPIO_PORT_C, 01, 11),
    },
//    {
//        BSP_ADC1_REG_PARM(ADDRS, GPIO_PORT_C, 02, 12),  //此通道只能放在ADC0,1
//    },
    {
        BSP_ADC1_REG_PARM(TEMP3, GPIO_PORT_C, 00, 10),
    },
    {
        BSP_ADC1_REG_PARM(RMTVS, GPIO_PORT_B, 01, 9),
    },
   
};


//adc 初始化
void bsp_adc_init(void)
{
    stc_gpio_init_t stcGpioInit;
    stc_adc_init_t stcAdcInit;
    
    (void)GPIO_StructInit(&stcGpioInit);
    (void)ADC_StructInit(&stcAdcInit);
    
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC1, ENABLE);
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC2, ENABLE);
//    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC3, ENABLE);
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS,  ENABLE);
    
    stcAdcInit.u16ScanMode = ADC_MD_SEQA_SINGLESHOT; //序列A单次触发模式，触发一次采集一次
    
    (void)ADC_Init(CM_ADC1, &stcAdcInit);
    (void)ADC_Init(CM_ADC2, &stcAdcInit);
 //   (void)ADC_Init(CM_ADC3, &stcAdcInit);

    stcGpioInit.u16PinAttr   = PIN_ATTR_ANALOG; 
    for (uint8_t i = 0; i < ADC0_TABLE_MAX; i++)
    {
        /*if (bsp_adc0_param[i].gpio_periph != NULL)
        {   //IO配置*/
            GPIO_Init(bsp_adc0_param[i].gpio_periph, bsp_adc0_param[i].pin,&stcGpioInit);
        /*}*/
        //ADC通道配置
        ADC_ChCmd((CM_ADC_TypeDef *)bsp_adc0_param[i].adc_periph, ADC_SEQ_A, bsp_adc0_param[i].adc_ch, ENABLE);
        //ADC采样时间配置
        ADC_SetSampleTime((CM_ADC_TypeDef *)bsp_adc0_param[i].adc_periph,\
                           bsp_adc0_param[i].adc_ch, SAMPLE_TIME);
    }
    
    //ADC1触发配置
    ADC_TriggerConfig(CM_ADC1, ADC_SEQ_A, ADC_HARDTRIG_EVT0);
    AOS_SetTriggerEventSrc(AOS_ADC1_0, ADC_TRIG_EVT);
    ADC_TriggerCmd(CM_ADC1, ADC_SEQ_A, ENABLE);
    


    
    ////////////////////////////////////////
    //ADC3配置
    for (uint8_t i = 0; i < ADC1_TABLE_MAX; i++)
    {
        /*if (bsp_adc1_param[i].gpio_periph != NULL)
        {   //IO配置*/
            GPIO_Init(bsp_adc1_param[i].gpio_periph, bsp_adc1_param[i].pin,&stcGpioInit);
        /*}*/
        //ADC通道配置
        ADC_ChCmd((CM_ADC_TypeDef *)bsp_adc1_param[i].adc_periph, ADC_SEQ_A, bsp_adc1_param[i].adc_ch, ENABLE);
        //ADC采样时间配置
        ADC_SetSampleTime((CM_ADC_TypeDef *)bsp_adc1_param[i].adc_periph,\
                           bsp_adc1_param[i].adc_ch, SAMPLE_TIME);
    }
        //ADC2触发配置
    ADC_TriggerConfig(CM_ADC2, ADC_SEQ_A, ADC_HARDTRIG_EVT0);
    AOS_SetTriggerEventSrc(AOS_ADC2_0, ADC_TRIG_EVT);
    ADC_TriggerCmd(CM_ADC2, ADC_SEQ_A, ENABLE);
}

//adc 初始化
void bsp_adc_init_pwc(void)
{
    stc_gpio_init_t stcGpioInit;
    stc_adc_init_t stcAdcInit;
    
    (void)GPIO_StructInit(&stcGpioInit);
    (void)ADC_StructInit(&stcAdcInit);
    
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC1, ENABLE);
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC2, ENABLE);
//    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC3, ENABLE);
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS,  ENABLE);
    
    stcAdcInit.u16ScanMode = ADC_MD_SEQA_SINGLESHOT; //序列A单次触发模式，触发一次采集一次
    
    (void)ADC_Init(CM_ADC1, &stcAdcInit);
    (void)ADC_Init(CM_ADC2, &stcAdcInit);
 //   (void)ADC_Init(CM_ADC3, &stcAdcInit);

    stcGpioInit.u16PinAttr   = PIN_ATTR_ANALOG; 
    for (uint8_t i = 0; i < ADC0_TABLE_MAX; i++)
    {
        /*if (bsp_adc0_param[i].gpio_periph != NULL)
        {   //IO配置*/
            GPIO_Init(bsp_adc0_param[i].gpio_periph, bsp_adc0_param[i].pin,&stcGpioInit);
        /*}*/
        //ADC通道配置
        ADC_ChCmd((CM_ADC_TypeDef *)bsp_adc0_param[i].adc_periph, ADC_SEQ_A, bsp_adc0_param[i].adc_ch, ENABLE);
        //ADC采样时间配置
        ADC_SetSampleTime((CM_ADC_TypeDef *)bsp_adc0_param[i].adc_periph,\
                           bsp_adc0_param[i].adc_ch, SAMPLE_TIME);
    }
    
    //ADC1触发配置
    ADC_TriggerConfig(CM_ADC1, ADC_SEQ_A, ADC_HARDTRIG_EVT0);
    AOS_SetTriggerEventSrc(AOS_ADC1_0, ADC_TRIG_EVT);
    ADC_TriggerCmd(CM_ADC1, ADC_SEQ_A, ENABLE);
    


    
    ////////////////////////////////////////
    //ADC3配置
    for (uint8_t i = 0; i < ADC1_TABLE_MAX; i++)
    {
        /*if (bsp_adc1_param[i].gpio_periph != NULL)
        {   //IO配置*/
            GPIO_Init(bsp_adc1_param[i].gpio_periph, bsp_adc1_param[i].pin,&stcGpioInit);
        /*}*/
        //ADC通道配置
        ADC_ChCmd((CM_ADC_TypeDef *)bsp_adc1_param[i].adc_periph, ADC_SEQ_A, bsp_adc1_param[i].adc_ch, ENABLE);
        //ADC采样时间配置
        ADC_SetSampleTime((CM_ADC_TypeDef *)bsp_adc1_param[i].adc_periph,\
                           bsp_adc1_param[i].adc_ch, SAMPLE_TIME);
    }
        //ADC2触发配置
    ADC_TriggerConfig(CM_ADC2, ADC_SEQ_A, ADC_HARDTRIG_EVT0);
    AOS_SetTriggerEventSrc(AOS_ADC2_0, ADC_TRIG_EVT);
    ADC_TriggerCmd(CM_ADC2, ADC_SEQ_A, ENABLE);
}

float RAMFUNC bsp_adc0_get_value(ADC0_TABLE_E name)
{

    return ADC_GetValue((CM_ADC_TypeDef *)bsp_adc0_param[name].adc_periph,\
                           bsp_adc0_param[name].adc_ch) * \
                           bsp_adc0_linear_calib[name].gain + \
                           bsp_adc0_linear_calib[name].bias;
}

float RAMFUNC bsp_adc1_get_value(ADC1_TABLE_E name)
{
    return ADC_GetValue((CM_ADC_TypeDef *)bsp_adc1_param[name].adc_periph,\
                           bsp_adc1_param[name].adc_ch) * \
                           bsp_adc1_linear_calib[name].gain + \
                           bsp_adc1_linear_calib[name].bias;
}


float RAMFUNC bsp_adc_get_ila(void)
{
    return bsp_adc0_get_value(ILA);
}

float RAMFUNC bsp_adc_get_ilb(void)
{
    return bsp_adc1_get_value(ILB);
}

float RAMFUNC bsp_adc_get_ihv(void)
{
    return bsp_adc1_get_value(IHV);
}

float RAMFUNC bsp_adc_get_ilv(void)
{
    return bsp_adc0_get_value(ILV);
}

float RAMFUNC bsp_adc_get_fvs48(void)
{
    return bsp_adc0_get_value(FVS48);
}

float RAMFUNC bsp_adc_get_rvs12(void)
{
    return bsp_adc0_get_value(RVS12);
}

float bsp_adc_get_rmtvs(void)
{
    return bsp_adc1_get_value(RMTVS);
}

float RAMFUNC bsp_adc_get_accvs(void)
{
    return bsp_adc1_get_value(ACCVS);
}

//float bsp_adc_get_addrs(void)
//{
//    return bsp_adc1_get_value(ADDRS);
//}

float bsp_adc_get_temp1(void)
{
    return bsp_adc0_get_value(TEMP1);
}

float bsp_adc_get_temp2(void)
{
    return bsp_adc0_get_value(TEMP2);
}

float bsp_adc_get_temp3(void)
{
    return bsp_adc1_get_value(TEMP3);
}

float bsp_adc_get_vdd8v(void)
{
    return bsp_adc1_get_value(VDD8V);
}

 void bsp_adc_deinit(void)
{
    for (uint8_t i = 0; i < ADC0_TABLE_MAX; i++)
    {
        //ADC通道配置
        ADC_ChCmd((CM_ADC_TypeDef *)bsp_adc0_param[i].adc_periph, ADC_SEQ_A, bsp_adc0_param[i].adc_ch, DISABLE);
    }

    ADC_TriggerCmd(CM_ADC1, ADC_SEQ_A, DISABLE);

    for (uint8_t i = 0; i < ADC1_TABLE_MAX; i++)
    {
        //ADC通道配置
        ADC_ChCmd((CM_ADC_TypeDef *)bsp_adc1_param[i].adc_periph, ADC_SEQ_A, bsp_adc1_param[i].adc_ch, DISABLE);
    }
    ADC_TriggerCmd(CM_ADC2, ADC_SEQ_A, DISABLE);
    
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC1, DISABLE);
    FCG_Fcg3PeriphClockCmd(FCG3_PERIPH_ADC2, DISABLE);
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS,  DISABLE);
}




