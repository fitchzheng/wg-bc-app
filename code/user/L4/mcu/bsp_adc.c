#include "bsp_adc.h"
#include "bsp_dma.h"
#include "section.h"

static uint16_t __attribute__((aligned(4))) adc0_1_value[ADC0_1_TABLE_MAX];
static uint16_t __attribute__((aligned(4))) adc2_value[ADC2_TABLE_MAX];

bsp_adc_linear_calib_t bsp_adc0_1_linear_calib[ADC0_1_TABLE_MAX] = {
    [ILA] = {
        .gain = BSP_ADC_ILA_GAIN,
        .bias = BSP_ADC_ILA_BIAS,
    },
    [ILB] = {
        .gain = BSP_ADC_ILB_GAIN,
        .bias = BSP_ADC_ILB_BIAS,
    },
    [ILV] = {
        .gain = BSP_ADC_ILV_GAIN,
        .bias = BSP_ADC_ILV_BIAS,
    },
    [IHV] = {
        .gain = BSP_ADC_IHV_GAIN,
        .bias = BSP_ADC_IHV_BIAS,
    },
    [FVS48] = {
        .gain = BSP_ADC_FVS48_GAIN,
        .bias = BSP_ADC_FVS48_BIAS,
    },
    [RVS12] = {
        .gain = BSP_ADC_RVS12_GAIN,
        .bias = BSP_ADC_RVS12_BIAS,
    },
    [VDD8V] = {
        .gain = BSP_ADC_VDD_8V_GAIN,
        .bias = BSP_ADC_VDD_8V_BIAS,
    },
    [RMTVS] = {
        .gain = BSP_ADC_RMTVS_GAIN,
        .bias = BSP_ADC_RMTVS_BIAS,
    },
};

bsp_adc_linear_calib_t bsp_adc2_linear_calib[ADC2_TABLE_MAX] = {
    [TEMP1] = {
        .gain = BSP_ADC_TEMP1_GAIN,
        .bias = BSP_ADC_TEMP1_BIAS,
    },
    [TEMP2] = {
        .gain = BSP_ADC_TEMP2_GAIN,
        .bias = BSP_ADC_TEMP2_BIAS,
    },
    [TEMP3] = {
        .gain = BSP_ADC_TEMP3_GAIN,
        .bias = BSP_ADC_TEMP3_BIAS,
    },
    [ACCVS] = {
        .gain = BSP_ADC_ACCVS_GAIN,
        .bias = BSP_ADC_ACCVS_BIAS,
    },
    [ADDRS] = {
        .gain = BSP_ADC_ADDRS_GAIN,
        .bias = BSP_ADC_ADDRS_BIAS,
    },

};

const bsp_adc_param_t bsp_adc0_1_param[ADC0_1_TABLE_MAX] = {
    {
        BSP_ADC0_1_REG_PARM(ILA, GPIOA, 4, 4),
    },
    {
        BSP_ADC0_1_REG_PARM(ILB, GPIOA, 5, 5),
    },
    {
        BSP_ADC0_1_REG_PARM(ILV, GPIOC, 3, 13),
    },
    {
        BSP_ADC0_1_REG_PARM(IHV, GPIOA, 6, 6),
    },
    {
        BSP_ADC0_1_REG_PARM(FVS48, GPIOA, 0, 0),
    },
    {
        BSP_ADC0_1_REG_PARM(RVS12, GPIOA, 1, 1),
    },
    {
        BSP_ADC0_1_REG_PARM(VDD8V, GPIOC, 4, 14),
    },
    {
        BSP_ADC0_1_REG_PARM(RMTVS, GPIOB, 1, 9),
    },
};

const bsp_adc_param_t bsp_adc2_param[ADC2_TABLE_MAX] = {
    {
        BSP_ADC2_REG_PARM(TEMP1, GPIOA, 2, 2),
    },
    {
        BSP_ADC2_REG_PARM(TEMP2, GPIOA, 3, 3),
    },
    {
        BSP_ADC2_REG_PARM(TEMP3, GPIOC, 0, 10),
    },
    {
        BSP_ADC2_REG_PARM(ACCVS, GPIOC, 1, 11),
    },
    {
        BSP_ADC2_REG_PARM(ADDRS, GPIOC, 2, 12),
    },
};

static uint8_t calibration(uint32_t adc_periph)
{
    ADC_CTL1(adc_periph) |= ADC_CTL1_ADCON;
    uint32_t i = 14;
    while (i--)
        ;
    ADC_CTL1(adc_periph) |= ADC_CTL1_RSTCLB;
    i = 120000000;
    while (ADC_CTL1(adc_periph) & ADC_CTL1_RSTCLB)
    {
        i--;
        if (i == 0)
        {
            return 0;
        }
    };
    ADC_CTL1(adc_periph) |= ADC_CTL1_CLB;
    i = 120000000;
    while (ADC_CTL1(adc_periph) & ADC_CTL1_CLB)
    {
        i--;
        if (i == 0)
        {
            return 0;
        }
    }
    return 1;
}

void bsp_adc_init(void)
{
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(RCU_ADC1);
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);

    for (uint8_t i = 0; i < ADC0_1_TABLE_MAX; i++)
    {
        gpio_init(bsp_adc0_1_param[i].gpio_periph, GPIO_MODE_AIN, GPIO_OSPEED_MAX, bsp_adc0_1_param[i].pin);
        if (bsp_adc0_1_param[i].gpio_periph != NULL)
        {
            gpio_init(bsp_adc0_1_param[i].gpio_periph, GPIO_MODE_AIN, GPIO_OSPEED_MAX, bsp_adc0_1_param[i].pin);
        }
    }

    adc_deinit(ADC0);
    adc_deinit(ADC1);

    ADC_CTL0(ADC0) |= ADC_CTL0_SM;
    ADC_CTL1(ADC0) |= ADC_CTL1_DMA;
    ADC_RSQ0(ADC0) &= ~ADC_RSQ0_RL;
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_TABLE_MAX / 2);
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_T1_CH1);
    adc_tempsensor_vrefint_enable();
    adc_mode_config(ADC_DAUL_REGULAL_PARALLEL);
    ADC_CTL0(ADC1) |= ADC_CTL0_SM;
    ADC_CTL1(ADC1) |= ADC_CTL1_DMA;
    ADC_RSQ0(ADC1) &= ~ADC_RSQ0_RL;
    adc_channel_length_config(ADC1, ADC_REGULAR_CHANNEL, ADC0_1_TABLE_MAX / 2);
    adc_external_trigger_config(ADC1, ADC_REGULAR_CHANNEL, ENABLE);
    adc_external_trigger_source_config(ADC1, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_T1_CH1);
    adc_tempsensor_vrefint_enable();
    
    for (uint8_t i = 0; i < ADC0_1_TABLE_MAX; i++)
    {
        adc_regular_channel_config(bsp_adc0_1_param[i].adc_periph,
                                   bsp_adc0_1_param[i].rank,
                                   bsp_adc0_1_param[i].adc_ch,
                                   bsp_adc0_1_param[i].sample_time);
    }

    dma_parameter_struct dma0_parameter;
    rcu_periph_clock_enable(RCU_DMA0);

    dma_deinit(DMA0, DMA_CH0);
    dma0_parameter.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma0_parameter.number = ADC0_1_TABLE_MAX / 2;
    dma0_parameter.priority = DMA_PRIORITY_ULTRA_HIGH;

    dma0_parameter.periph_addr = (uint32_t)(&ADC_RDATA(ADC0));
    dma0_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma0_parameter.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;

    dma0_parameter.memory_addr = (uint32_t)((uint32_t *)&adc0_1_value[0]);
    dma0_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma0_parameter.memory_width = DMA_MEMORY_WIDTH_32BIT;

    dma_init(DMA0, DMA_CH0, &dma0_parameter);

    dma_circulation_enable(DMA0, DMA_CH0);
    dma_channel_enable(DMA0, DMA_CH0);
    calibration(ADC0);
    calibration(ADC1);

    rcu_periph_clock_enable(RCU_ADC2);

    for (uint8_t i = 0; i < ADC2_TABLE_MAX; i++)
    {
        gpio_init(bsp_adc2_param[i].gpio_periph, GPIO_MODE_AIN, GPIO_OSPEED_MAX, bsp_adc2_param[i].pin);
    }

    ADC_CTL1(ADC2) |= ADC_CTL1_CTN;
    ADC_CTL0(ADC2) |= ADC_CTL0_SM;
    ADC_CTL1(ADC2) |= ADC_CTL1_DMA;
    ADC_RSQ0(ADC2) &= ~ADC_RSQ0_RL;
    adc_channel_length_config(ADC2, ADC_REGULAR_CHANNEL, ADC2_TABLE_MAX);
    adc_external_trigger_config(ADC2, ADC_REGULAR_CHANNEL, ENABLE);
    adc_external_trigger_source_config(ADC2, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);

    for (uint8_t i = 0; i < ADC2_TABLE_MAX; i++)
    {
        adc_regular_channel_config(bsp_adc2_param[i].adc_periph,
                                   bsp_adc2_param[i].rank,
                                   bsp_adc2_param[i].adc_ch,
                                   bsp_adc2_param[i].sample_time);
    }

    dma_parameter_struct dma1_parameter;
    rcu_periph_clock_enable(RCU_DMA1);
    dma_deinit(DMA1, DMA_CH4);

    dma1_parameter.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma1_parameter.number = ADC2_TABLE_MAX;
    dma1_parameter.priority = DMA_PRIORITY_LOW;
    dma1_parameter.periph_addr = (uint32_t)(&ADC_RDATA(ADC2));
    dma1_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma1_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma1_parameter.memory_addr = (uint32_t)((uint16_t *)&adc2_value[0]);
    dma1_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma1_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init(DMA1, DMA_CH4, &dma1_parameter);
    dma_circulation_enable(DMA1, DMA_CH4);
    dma_channel_enable(DMA1, DMA_CH4);
    calibration(ADC2);
    adc_software_trigger_enable(ADC2, ADC_REGULAR_CHANNEL);
}

float bsp_adc0_1_get_value(ADC0_1_TABLE_E name)
{

    return adc0_1_value[name] * bsp_adc0_1_linear_calib[name].gain + bsp_adc0_1_linear_calib[name].bias;
}

float bsp_adc2_get_value(ADC2_TABLE_E name)
{
    return adc2_value[name] * bsp_adc2_linear_calib[name].gain + bsp_adc2_linear_calib[name].bias;
}

float bsp_adc_get_ila(void)
{
    return bsp_adc0_1_get_value(ILA);
}

float bsp_adc_get_ilb(void)
{
    return bsp_adc0_1_get_value(ILB);
}

float bsp_adc_get_ihv(void)
{
    return bsp_adc0_1_get_value(IHV);
}

float bsp_adc_get_ilv(void)
{
    return bsp_adc0_1_get_value(ILV);
}

float bsp_adc_get_fvs48(void)
{
    return bsp_adc0_1_get_value(FVS48);
}

float bsp_adc_get_rvs12(void)
{
    return bsp_adc0_1_get_value(RVS12);
}

float bsp_adc_get_vdd_8V(void)
{
    return bsp_adc0_1_get_value(VDD8V);
}

float bsp_adc_get_rmtvs(void)
{
    return bsp_adc0_1_get_value(RMTVS);
}

float bsp_adc_get_accvs(void)
{
    return bsp_adc2_get_value(ACCVS);
}

float bsp_adc_get_addrs(void)
{
    return bsp_adc2_get_value(ADDRS);
}

float bsp_adc_get_temp1(void)
{
    return bsp_adc2_get_value(TEMP1);
}

float bsp_adc_get_temp2(void)
{
    return bsp_adc2_get_value(TEMP2);
}

float bsp_adc_get_temp3(void)
{
    return bsp_adc2_get_value(TEMP3);
}
