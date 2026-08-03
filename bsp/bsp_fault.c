#include "bsp_fault.h"
#include "hc32_ddl.h"

#define FLT_LOCK_PORT       (PortC)
#define FLT_LOCK_PIN        (Pin12)
#define FLT_LOCK_EXT_CH     ExtiCh12
#define FLT_LOCK_EIQ        INT_PORT_EIRQ12
#define FLT_LOCK_IRQHandler Int012_IRQn
#define FLT_LOCK_PRIORITY   (DDL_IRQ_PRIORITY_10)
#define FLT_LOCK_YES        ((PORT_GetBit(FLT_LOCK_PORT, FLT_LOCK_PIN) == 0) ? 1 : 0)

static int FaultFlg = 0;

static void _vFaultCallback(void)
{
    FaultFlg = 1;
}

void Bsp_Fault_Init(void)
{
    stc_exint_config_t  stcExtiConfig;
    stc_port_init_t     stcPortInit;
    stc_irq_regi_conf_t stcIrqRegiConf;

    MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcPortInit);
    MEM_ZERO_STRUCT(stcIrqRegiConf);

    stcExtiConfig.enExitCh = FLT_LOCK_EXT_CH;
    /* Filter setting */
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk   = Pclk3Div8;
    stcExtiConfig.enExtiLvl  = ExIntFallingEdge;
    EXINT_Init(&stcExtiConfig);
    /* Set External Int */
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enExInt   = Enable;
    stcPortInit.enPullUp  = Enable;
    PORT_Init(FLT_LOCK_PORT, FLT_LOCK_PIN, &stcPortInit);
    /* Select External Int Ch.13 */
    stcIrqRegiConf.enIntSrc = FLT_LOCK_EIQ;
    /* Register External Int to Vect.No.000 */
    stcIrqRegiConf.enIRQn = FLT_LOCK_IRQHandler;
    /* Callback function */
    stcIrqRegiConf.pfnCallback = _vFaultCallback;
    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);
    /* Clear pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, FLT_LOCK_PRIORITY);
    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
}

int Bsp_Lock_IsFault(void)
{
    return FaultFlg;
}

void Bsp_ClrLockFault(void)
{
    FaultFlg = 0;
}
