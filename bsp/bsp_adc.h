#pragma once

#include <stdint.h>

#define DET_CHAN_5V  15
#define DET_CHAN_24V 8

#define ADC_5V_VALUE  2354  // 5V,5/5.8*2.2/3.3*4096
#define ADC_24V_VALUE 2952  // 24V,24/22.2*2.2/3.3*4096

extern uint16_t Bsp_Adc_GetVar(uint16_t ch);
extern void     Bsp_Adc_Init(void);
