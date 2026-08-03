#include "hc32_ddl.h"

/* TIMA_2_PWM1 */
#define LOCK_PWM_TIMER_UNIT              (M4_TMRA2)
#define LOCK_PWM_TIMER_UNIT_CLOCK        (PWC_FCG2_PERIPH_TIMA2)
#define LOCK_PWM_TIMER_UNIT_OVERFLOW_INT (INT_TMRA2_OVF)
#define LOCK_PWM_TIMER_CH                (TimeraCh1)
#define LOCK_PWM_TIMER_CH_PORT           (PortA)
#define LOCK_PWM_TIMER_CH_PIN            (Pin15)
#define LOCK_PWM_TIMER_CH_FUNC           (Func_Tima0)
#define LOCK_PERIOD                      3200

// 锁线电磁铁
#define OE_PORT (PortC)
#define OE_PIN  (Pin10)
#define OE_ON   PORT_SetBits(OE_PORT, OE_PIN)
#define OE_OFF  PORT_ResetBits(OE_PORT, OE_PIN)

#define PHASE_PORT (PortC)
#define PHASE_PIN  (Pin11)
#define PHASE_ON   PORT_SetBits(PHASE_PORT, PHASE_PIN)
#define PHASE_OFF  PORT_ResetBits(PHASE_PORT, PHASE_PIN)
#define PHASE_TRIG PORT_Toggle(PHASE_PORT, PHASE_PIN)

// 设置占空比
void Bsp_Lock_SetDuty(uint32_t duty)
{
    uint32_t dt = duty * (LOCK_PERIOD / 100);

    TIMERA_SetCompareValue(LOCK_PWM_TIMER_UNIT, LOCK_PWM_TIMER_CH, dt);
}

// 锁线动作方向
void Bsp_Lock_SetDir(uint8_t dir)
{
    if (dir)
        PHASE_ON;
    else
        PHASE_OFF;
}

// 锁线使能
void Bsp_Lock_SetEn(uint8_t en)
{
    if (en) {
        Bsp_Lock_SetDuty(100);
    } else {
        Bsp_Lock_SetDuty(0);
    }
}

void lock_pwm_init(void)
{  // 暂未使用
    stc_timera_base_init_t    stcTimeraInit;
    stc_timera_compare_init_t stcTimerCompareInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcTimeraInit);
    MEM_ZERO_STRUCT(stcTimerCompareInit);

    /* Configuration peripheral clock */
    PWC_Fcg2PeriphClockCmd(LOCK_PWM_TIMER_UNIT_CLOCK, Enable);

    /* Configuration TIMERA compare pin */
    PORT_SetFunc(LOCK_PWM_TIMER_CH_PORT,
                 LOCK_PWM_TIMER_CH_PIN,
                 LOCK_PWM_TIMER_CH_FUNC,
                 Disable);

    /* Configuration timera unit 1 base structure */
    stcTimeraInit.enClkDiv        = TimeraPclkDiv1;
    stcTimeraInit.enCntMode       = TimeraCountModeSawtoothWave;
    stcTimeraInit.enCntDir        = TimeraCountDirUp;
    stcTimeraInit.enSyncStartupEn = Disable;
    stcTimeraInit.u16PeriodVal    = LOCK_PERIOD;
    TIMERA_BaseInit(LOCK_PWM_TIMER_UNIT, &stcTimeraInit);

    /* Configuration timera unit 1 compare structure */
    stcTimerCompareInit.u16CompareVal = 0;

    stcTimerCompareInit.enStartCountOutput   = TimeraCountStartOutputLow;
    stcTimerCompareInit.enStopCountOutput    = TimeraCountStopOutputLow;
    stcTimerCompareInit.enCompareMatchOutput = TimeraCompareMatchOutputLow;
    stcTimerCompareInit.enPeriodMatchOutput  = TimeraPeriodMatchOutputHigh;
    stcTimerCompareInit.enSpecifyOutput      = TimeraSpecifyOutputInvalid;

    TIMERA_CompareInit(LOCK_PWM_TIMER_UNIT,
                       LOCK_PWM_TIMER_CH,
                       &stcTimerCompareInit);
    TIMERA_CompareCmd(LOCK_PWM_TIMER_UNIT, LOCK_PWM_TIMER_CH, Enable);
    TIMERA_Cmd(LOCK_PWM_TIMER_UNIT, Enable);
}

// 锁线初始化
void Bsp_Lock_Init(void)
{
    stc_port_init_t stcPortInit;
    // io初始化
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;

    PORT_Init(PHASE_PORT, PHASE_PIN, &stcPortInit);
    PORT_Init(OE_PORT, OE_PIN, &stcPortInit);

    Bsp_Lock_SetDir(1);
    OE_ON;
    lock_pwm_init();
}
