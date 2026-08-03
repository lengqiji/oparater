#include "bsp_motor_current.h"
#include "hc32_ddl.h"
#include "rn_typedef.h"
#include "bsp_motor_def.h"
/* TIMA_1_PWM2 */
#define PWM_A_VCR_TIMERA_UNIT              (M4_TMRA1)
#define PWM_A_VCR_TIMERA_UNIT_CLOCK        (PWC_FCG2_PERIPH_TIMA1)
#define PWM_A_VCR_TIMERA_UNIT_OVERFLOW_INT (INT_TMRA1_OVF)
#define PWM_A_VCR_TIMERA_CH                (TimeraCh2)
#define PWM_A_VCR_TIMERA_CH_PORT           (PortA)
#define PWM_A_VCR_TIMERA_CH_PIN            (Pin09)
#define PWM_A_VCR_TIMERA_CH_FUNC           (Func_Tima0)
#define PWM_A_VCR_TIMERA_COUNT_OVERFLOW    (5000)
/* TIMA_1_PWM3 */
#define PWM_B_VCR_TIMERA_UNIT              (M4_TMRA1)
#define PWM_B_VCR_TIMERA_UNIT_CLOCK        (PWC_FCG2_PERIPH_TIMA1)
#define PWM_B_VCR_TIMERA_UNIT_OVERFLOW_INT (INT_TMRA1_OVF)
#define PWM_B_VCR_TIMERA_CH                (TimeraCh3)
#define PWM_B_VCR_TIMERA_CH_PORT           (PortA)
#define PWM_B_VCR_TIMERA_CH_PIN            (Pin10)
#define PWM_B_VCR_TIMERA_CH_FUNC           (Func_Tima0)
#define PWM_B_VCR_TIMERA_COUNT_OVERFLOW    (5000)

// PWM_VCR(步进电机电流值通过PWM占空比来控制)定时器初始化
void motor_a_pwm_vcr_init(void)
{
    stc_timera_compare_init_t      stcTimerCompareInit;
    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcTimerCompareInit);

        /* Configuration TIMERA compare pin */
    PORT_SetFunc(PWM_A_VCR_TIMERA_CH_PORT,
                 PWM_A_VCR_TIMERA_CH_PIN,
                 PWM_A_VCR_TIMERA_CH_FUNC,
                 Disable);
     PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);

    stc_timera_base_init_t         stcTimeraInit;
    MEM_ZERO_STRUCT(stcTimeraInit);

    // Configuration peripheral clock
    PWC_Fcg2PeriphClockCmd(PWM_A_VCR_TIMERA_UNIT_CLOCK, Enable);


    // Configuration timera unit 1 base structure
    stcTimeraInit.enClkDiv        = TimeraPclkDiv1;
    stcTimeraInit.enCntMode       = TimeraCountModeSawtoothWave;
    stcTimeraInit.enCntDir        = TimeraCountDirUp;
    stcTimeraInit.enSyncStartupEn = Disable;
    stcTimeraInit.u16PeriodVal    = PWM_A_VCR_TIMERA_COUNT_OVERFLOW;
    TIMERA_BaseInit(PWM_A_VCR_TIMERA_UNIT, &stcTimeraInit);

    /* Configuration timera unit 1 compare structure */
    stcTimerCompareInit.u16CompareVal = PWM_A_VCR_TIMERA_COUNT_OVERFLOW /5;

    stcTimerCompareInit.enStartCountOutput = TimeraCountStartOutputLow;
    stcTimerCompareInit.enStopCountOutput  = TimeraCountStopOutputLow;
    stcTimerCompareInit.enCompareMatchOutput =
        TimeraCompareMatchOutputHigh;                                      ////
    stcTimerCompareInit.enPeriodMatchOutput = TimeraPeriodMatchOutputLow;  //
    stcTimerCompareInit.enSpecifyOutput     = TimeraSpecifyOutputInvalid;
    // stcTimerCompareInit.enCacheEn = Enable;
    // stcTimerCompareInit.enTriangularTroughTransEn = Enable;
    // stcTimerCompareInit.enTriangularCrestTransEn = Disable;
    // stcTimerCompareInit.u16CompareCacheVal =
    // stcTimerCompareInit.u16CompareVal;
    /* Configure Channel 3 */
    TIMERA_CompareInit(PWM_A_VCR_TIMERA_UNIT,
                       PWM_A_VCR_TIMERA_CH,
                       &stcTimerCompareInit);
    TIMERA_CompareCmd(PWM_A_VCR_TIMERA_UNIT, PWM_A_VCR_TIMERA_CH, Enable);
    TIMERA_Cmd(PWM_A_VCR_TIMERA_UNIT, Enable);
}
void motor_b_pwm_vcr_init(void)
{
    stc_timera_compare_init_t      stcTimerCompareInit;
    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcTimerCompareInit);
    /* Configuration TIMERA compare pin */
    PORT_SetFunc(PWM_B_VCR_TIMERA_CH_PORT,
                 PWM_B_VCR_TIMERA_CH_PIN,
                 PWM_B_VCR_TIMERA_CH_FUNC,
                 Disable);

    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);

    // Configuration peripheral clock
    stc_timera_base_init_t         stcTimeraInit;
    MEM_ZERO_STRUCT(stcTimeraInit);
    PWC_Fcg2PeriphClockCmd(PWM_B_VCR_TIMERA_UNIT_CLOCK, Enable);

    //Configuration timera unit 1 base structure
    stcTimeraInit.enClkDiv        = TimeraPclkDiv1;
    stcTimeraInit.enCntMode       = TimeraCountModeSawtoothWave;
    stcTimeraInit.enCntDir        = TimeraCountDirUp;
    stcTimeraInit.enSyncStartupEn = Disable;
    stcTimeraInit.u16PeriodVal    = PWM_B_VCR_TIMERA_COUNT_OVERFLOW;
    TIMERA_BaseInit(PWM_B_VCR_TIMERA_UNIT, &stcTimeraInit);

    /* Configuration timera unit 1 compare structure */
    stcTimerCompareInit.u16CompareVal = PWM_B_VCR_TIMERA_COUNT_OVERFLOW/5;
    stcTimerCompareInit.enStartCountOutput = TimeraCountStartOutputLow;
    stcTimerCompareInit.enStopCountOutput  = TimeraCountStopOutputLow;
    stcTimerCompareInit.enCompareMatchOutput =
        TimeraCompareMatchOutputHigh;                                      ////
    stcTimerCompareInit.enPeriodMatchOutput = TimeraPeriodMatchOutputLow;  //
    stcTimerCompareInit.enSpecifyOutput     = TimeraSpecifyOutputInvalid;
    // stcTimerCompareInit.enCacheEn = Enable;
    // stcTimerCompareInit.enTriangularTroughTransEn = Enable;
    // stcTimerCompareInit.enTriangularCrestTransEn = Disable;
    // stcTimerCompareInit.u16CompareCacheVal =
    // stcTimerCompareInit.u16CompareVal;
    /* Configure Channel 3 */
    TIMERA_CompareInit(PWM_B_VCR_TIMERA_UNIT,
                       PWM_B_VCR_TIMERA_CH,
                       &stcTimerCompareInit);
    TIMERA_CompareCmd(PWM_B_VCR_TIMERA_UNIT, PWM_B_VCR_TIMERA_CH, Enable);
    TIMERA_Cmd(PWM_B_VCR_TIMERA_UNIT, Enable);
}

void Bsp_Motor_Current_Init()
{
    motor_a_pwm_vcr_init();
    motor_b_pwm_vcr_init();
}

static inline uint Current2Duty(uint current_x10)
{  // 3.0V对应2.23A 线性

#define MAX_CURRENT 22
    if (current_x10 > MAX_CURRENT)
        current_x10 = MAX_CURRENT;

    uint volt = current_x10 * 10 * 300 / 223;
    uint duty = volt * PWM_A_VCR_TIMERA_COUNT_OVERFLOW / 330;
    duty = PWM_A_VCR_TIMERA_COUNT_OVERFLOW - duty;  // 目前AB都是用定时器，值一样
    // 123
    if (duty <= 0)
        duty = 1;

    if (duty >= PWM_A_VCR_TIMERA_COUNT_OVERFLOW)
        duty = PWM_A_VCR_TIMERA_COUNT_OVERFLOW - 1;


    return duty;
}

static inline void Bsp_Motor_En_Off(Enum_MotorID id)
{
    if(id == A_MOTOR){
        SMT_A_EN_OFF();
    }else{
        SMT_B_EN_OFF();
    }
}
static int CurrentSave[MAX_MOTOR] = {0};

void Bsp_Motor_Current_Set(Enum_MotorID id, int current_x10)
{
    CurrentSave[id] = current_x10;
    INT16U duty = Current2Duty(current_x10);

    if (current_x10 == 0) {
        Bsp_Motor_En_Off(id);
    }

    if (id == A_MOTOR) {
        TIMERA_SetCompareValue(PWM_A_VCR_TIMERA_UNIT,
                               PWM_A_VCR_TIMERA_CH,
                               duty);
    } else {
        TIMERA_SetCompareValue(PWM_B_VCR_TIMERA_UNIT,
                               PWM_B_VCR_TIMERA_CH,
                               duty);
    }
}

int Bsp_Motor_Current_Get(Enum_MotorID id)
{
    return CurrentSave[id];
}

