#include <stdint.h>

#include "bsp_qf.h"
#include "hc32_ddl.h"
#include "rn_typedef.h"

// 四倍频
#define QF_IN_PORT         (PortA)
#define QF_IN_PIN          (Pin11)
#define QF_TIMER4_UNIT     (M4_TMR41)
#define QF_TIMER4_UNIT_CLK (PWC_FCG2_PERIPH_TIM41)

void Bsp_Qf_Clear(void)
{
    TIMER4_CNT_SetCountVal(QF_TIMER4_UNIT, 0);
}

// 获取四倍频计数
uint32_t Bsp_Qf_Read(void)
{
    return (TIMER4_CNT_GetCountVal(QF_TIMER4_UNIT) / 2);
}

// 初始化
void Bsp_Qf_init(void)
{
    stc_timer4_cnt_init_t stcCntInit;
    stc_irq_regi_conf_t   stcIrqRegiCfg;

    /* Clear structures */
    MEM_ZERO_STRUCT(stcCntInit);
    MEM_ZERO_STRUCT(stcIrqRegiCfg);

    /* Enable peripheral clock */
    PWC_Fcg2PeriphClockCmd(QF_TIMER4_UNIT_CLK, Enable);

    /* Initialize CLK I/O */
    PORT_SetFunc(QF_IN_PORT, QF_IN_PIN, Func_Tim4, Disable);

    /* Timer4-CNT : Initialize CNT configuration structure */
    stcCntInit.enBufferCmd  = Disable;
    stcCntInit.enClk        = Timer4CntExtclk;
    stcCntInit.enClkDiv     = Timer4CntPclkDiv1; /* Timer4-CNT clock divide */
    stcCntInit.u16Cycle     = 0xFFFF;            /* Timer4-QF-CNT cycle */
    stcCntInit.enCntMode    = Timer4CntSawtoothWave;
    stcCntInit.enZeroIntCmd = Disable; /* Disable zero match interrupt */
    stcCntInit.enPeakIntCmd = Enable;  /* Enable peak match interrupt */
    stcCntInit.enZeroIntMsk = Timer4CntIntMask0;
    stcCntInit.enPeakIntMsk = Timer4CntIntMask0;
    TIMER4_CNT_Init(QF_TIMER4_UNIT, &stcCntInit); /* Initialize Timer4-CNT */

    TIMER4_CNT_ClearCountVal(QF_TIMER4_UNIT);
    TIMER4_CNT_Start(QF_TIMER4_UNIT);
}

uint8_t Bsp_Qf_GetPortLevel(void)
{
    return PORT_GetBit(QF_IN_PORT, QF_IN_PIN);
}
