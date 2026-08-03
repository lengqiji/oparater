#include "bsp_adc.h"
#include "hc32_ddl.h"

static uint16_t adc_value_dma[ADC1_CH_COUNT] = {0};

/* ADC1 channel definition for this example. */
#define ADC1_VPOS1_AVG_CHANNEL (1 << DET_CHAN_5V)
#define ADC1_VPOS2_AVG_CHANNEL (1 << DET_CHAN_24V)

#define ADC1_SA_NORMAL_CHANNEL (ADC1_VPOS1_AVG_CHANNEL | ADC1_VPOS2_AVG_CHANNEL)
#define ADC1_SA_CHANNEL        (ADC1_SA_NORMAL_CHANNEL)
#define ADC1_SA_CHANNEL_COUNT  (2)
#define ADC1_CHANNEL           (ADC1_SA_CHANNEL)

/* ADC1 channel sampling time. */
#define ADC1_SA_CHANNEL_SAMPLE_TIME                                            \
    {                                                                          \
        0x30, 0x40                                                             \
    }

/* DMA definition for ADC1. */
#define ADC1_SA_DMA_UNIT   (M4_DMA2)
#define ADC1_SA_DMA_PWC    (PWC_FCG0_PERIPH_DMA2)
#define ADC1_SA_DMA_CH     (DmaCh3)
#define ADC1_SA_DMA_TRGSRC (EVT_ADC1_EOCA)

void AdcSetPinMode(uint8_t u8AdcPin, en_pin_mode_t enMode)
{
    en_port_t       enPort = PortA;
    en_pin_t        enPin  = Pin00;
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = enMode;
    stcPortInit.enPullUp  = Disable;

    switch (u8AdcPin) {
        case ADC1_IN0:
            enPort = PortA;
            enPin  = Pin00;
            break;

        case ADC1_IN1:
            enPort = PortA;
            enPin  = Pin01;
            break;

        case ADC1_IN2:
            enPort = PortA;
            enPin  = Pin02;
            break;

        case ADC1_IN3:
            enPort = PortA;
            enPin  = Pin03;
            break;

        case ADC12_IN4:
            enPort = PortA;
            enPin  = Pin04;
            break;

        case ADC12_IN5:
            enPort = PortA;
            enPin  = Pin05;
            break;

        case ADC12_IN6:
            enPort = PortA;
            enPin  = Pin06;
            break;

        case ADC12_IN7:
            enPort = PortA;
            enPin  = Pin07;
            break;

        case ADC12_IN8:
            enPort = PortB;
            enPin  = Pin00;
            break;

        case ADC12_IN9:
            enPort = PortB;
            enPin  = Pin01;
            break;

        case ADC12_IN10:
            enPort = PortC;
            enPin  = Pin00;
            break;

        case ADC12_IN11:
            enPort = PortC;
            enPin  = Pin01;
            break;

        case ADC1_IN12:
            enPort = PortC;
            enPin  = Pin02;
            break;

        case ADC1_IN13:
            enPort = PortC;
            enPin  = Pin03;
            break;

        case ADC1_IN14:
            enPort = PortC;
            enPin  = Pin04;
            break;

        case ADC1_IN15:
            enPort = PortC;
            enPin  = Pin05;
            break;

        default:
            return;
    }

    PORT_Init(enPort, enPin, &stcPortInit);
}

/**
 * @description:
 * @return {*}
 * @Author: yizh
 * @Date: 2022-12-03 20:12:17
 * @LastEditTime: Do not edit
 * @autoAdd: false
 */
void AdcSetChannelPinMode(const M4_ADC_TypeDef *ADCx,
                          uint32_t              u32Channel,
                          en_pin_mode_t         enMode)
{
    uint8_t u8ChIndex;
    uint8_t u8ChOffset = 0u;
    if (M4_ADC1 == ADCx) {
        u32Channel &= ADC1_PIN_MASK_ALL;
    } else {
        u32Channel &= ADC2_PIN_MASK_ALL;
        u8ChOffset = 4u;
    }

    u8ChIndex = 0u;
    while (0u != u32Channel) {
        if (u32Channel & 0x1ul) {
            AdcSetPinMode((u8ChIndex + u8ChOffset), enMode);
        }
        u32Channel >>= 1u;
        u8ChIndex++;
    }
}

/**
 * @description:
 * @return {*}
 * @Author: yizh
 * @Date: 2022-12-02 23:09:14
 * @LastEditTime: Do not edit
 * @autoAdd: false
 */

static void AdcInitConfig(void)
{
    stc_adc_init_t stcAdcInit;

    MEM_ZERO_STRUCT(stcAdcInit);

    stcAdcInit.enResolution = AdcResolution_12Bit;
    stcAdcInit.enDataAlign  = AdcDataAlign_Right;
    stcAdcInit.enAutoClear  = AdcClren_Enable;
    stcAdcInit.enScanMode   = AdcMode_SAContinuous;
    stcAdcInit.enRschsel    = AdcRschsel_Restart;
    /* 1. Enable ADC1. */
    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_ADC1, Enable);
    /* 2. Initialize ADC1. */
    ADC_Init(M4_ADC1, &stcAdcInit);
}

/**
 * @description: AD通道配置
 * @return {*}
 * @Author: yizh
 * @Date: 2022-12-03 19:15:37
 * @LastEditTime: Do not edit
 * @autoAdd: false
 */
void AdcChannelConfig(void)
{
    stc_adc_ch_cfg_t stcChCfg;
    uint8_t          au8Adc1SaSampTime[ADC1_SA_CHANNEL_COUNT]
        = ADC1_SA_CHANNEL_SAMPLE_TIME;

    MEM_ZERO_STRUCT(stcChCfg);

    /**************************** Add ADC1 channels
     * ****************************/
    /* 1. Set the ADC pin to analog mode. */
    AdcSetChannelPinMode(M4_ADC1, ADC1_CHANNEL, Pin_Mode_Ana);

    stcChCfg.u32Channel  = ADC1_SA_CHANNEL;
    stcChCfg.u8Sequence  = ADC_SEQ_A;
    stcChCfg.pu8SampTime = au8Adc1SaSampTime;
    /* 2. Add ADC channel. */
    ADC_AddAdcChannel(M4_ADC1, &stcChCfg);

/* 3. Configure the average channel if you need. */
#if ADC_FILM_MODE
    ADC_ConfigAvg(M4_ADC1, AdcAvcnt_4);

#else
    ADC_ConfigAvg(M4_ADC1, AdcAvcnt_16);
#endif
    /* 4. Add average channel if you need. */
    ADC_AddAvgChannel(M4_ADC1, ADC1_VPOS1_AVG_CHANNEL);
    ADC_AddAvgChannel(M4_ADC1, ADC1_VPOS2_AVG_CHANNEL);
}
/**
 * @description:
 * @return {*}
 * @Author: yizh
 * @Date: 2022-12-03 19:15:57
 * @LastEditTime: Do not edit
 * @autoAdd: false
 */
void AdcTriggerConfig(void)
{
    stc_adc_trg_cfg_t stcTrgCfg;

    MEM_ZERO_STRUCT(stcTrgCfg);
    /*
     * If select an event(@ref en_event_src_t) to trigger ADC,
     * AOS must be enabled first.
     */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);
    /* ADC1 sequence A will be started by software. */
    ADC_TriggerSrcCmd(M4_ADC1, ADC_SEQ_A, Disable);
    /* ADC1 sequence B scan ends to trigger ADC1 sequence A. */
    stcTrgCfg.u8Sequence = ADC_SEQ_A;
    stcTrgCfg.enTrgSel   = AdcTrgsel_TRGX0;
    stcTrgCfg.enInTrg0   = EVT_ADC1_EOCA;
    ADC_ConfigTriggerSrc(M4_ADC1, &stcTrgCfg);
    ADC_TriggerSrcCmd(M4_ADC1, ADC_SEQ_A, Disable);
}
/**
 * @description: AD的DMA配置
 * @return {*}
 * @Author: yizh
 * @Date: 2022-12-03 19:16:08
 * @LastEditTime: Do not edit
 * @autoAdd: false
 */
void AdcDmaConfig(void)
{
    stc_dma_config_t stcDmaCfg;
    MEM_ZERO_STRUCT(stcDmaCfg);
    stcDmaCfg.u16BlockSize            = ADC1_CH_COUNT;
    stcDmaCfg.u16TransferCnt          = 0u;
    stcDmaCfg.u32SrcAddr              = (uint32_t) (&M4_ADC1->DR0);
    stcDmaCfg.u32DesAddr              = (uint32_t) (&adc_value_dma[0]);
    stcDmaCfg.u16DesRptSize           = ADC1_CH_COUNT;
    stcDmaCfg.u16SrcRptSize           = ADC1_CH_COUNT;
    stcDmaCfg.u32DmaLlp               = 0u;
    stcDmaCfg.stcSrcNseqCfg.u16Cnt    = 0u;
    stcDmaCfg.stcSrcNseqCfg.u32Offset = 0u;
    stcDmaCfg.stcDesNseqCfg.u16Cnt    = 0u;
    stcDmaCfg.stcDesNseqCfg.u32Offset = 0u;
    stcDmaCfg.stcDmaChCfg.enSrcInc    = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enDesInc    = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enSrcRptEn  = Enable;
    stcDmaCfg.stcDmaChCfg.enDesRptEn  = Enable;
    stcDmaCfg.stcDmaChCfg.enSrcNseqEn = Disable;
    stcDmaCfg.stcDmaChCfg.enDesNseqEn = Disable;
    stcDmaCfg.stcDmaChCfg.enTrnWidth  = Dma16Bit;
    stcDmaCfg.stcDmaChCfg.enLlpEn     = Disable;
    stcDmaCfg.stcDmaChCfg.enIntEn     = Disable;
    PWC_Fcg0PeriphClockCmd(ADC1_SA_DMA_PWC, Enable);
    DMA_InitChannel(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, &stcDmaCfg);
    DMA_Cmd(ADC1_SA_DMA_UNIT, Enable);
    DMA_ChannelCmd(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, Enable);
    DMA_ClearIrqFlag(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, TrnCpltIrq);
    DMA_SetTriggerSrc(ADC1_SA_DMA_UNIT, ADC1_SA_DMA_CH, ADC1_SA_DMA_TRGSRC);
}

void Bsp_Adc_Init(void)
{
    AdcInitConfig();
    AdcChannelConfig();
    AdcTriggerConfig();
    AdcDmaConfig();
    ADC_StartConvert(M4_ADC1);
}

uint16_t Bsp_Adc_GetVar(uint16_t ch)
{
    return adc_value_dma[ch];
}
