#include "bsp_motor.h"
#include "bsp_motor_def.h"
#include "hc32_ddl.h"
#include "rn_typedef.h"

// 此中断 高于 ucos时钟中断的15优先级
#define MOTOR_STEP_IRQ_PRIORITY (5)
/**
 * @brief Selectable Mixed Decay Function
 * @details
 * DECAY2  DECAY1 Function
 * L L Mixed Decay
 * L H Slow Decay only
 * H L Fast Decay only
 * H H ADMD
 * @author lincq <lincq@raynen.cn>
 * @date 2024-05-21
 */
#define SMT_A_DECAY1_PORT (PortC)
#define SMT_A_DECAY1_PIN  (Pin02)
#define SMT_A_DECAY2_PORT (PortC)
#define SMT_A_DECAY2_PIN  (Pin03)

#define SMT_B_DECAY1_PORT (PortC)
#define SMT_B_DECAY1_PIN  (Pin04)
#define SMT_B_DECAY2_PORT (PortB)
#define SMT_B_DECAY2_PIN  (Pin02)

#define SMT_A_DECAY1_L() PORT_ResetBits(SMT_A_DECAY1_PORT, SMT_A_DECAY1_PIN)
#define SMT_A_DECAY1_H() PORT_SetBits(SMT_A_DECAY1_PORT, SMT_A_DECAY1_PIN)
#define SMT_A_DECAY2_L() PORT_ResetBits(SMT_A_DECAY2_PORT, SMT_A_DECAY2_PIN)
#define SMT_A_DECAY2_H() PORT_SetBits(SMT_A_DECAY2_PORT, SMT_A_DECAY2_PIN)

#define SMT_B_DECAY1_L() PORT_ResetBits(SMT_B_DECAY1_PORT, SMT_B_DECAY1_PIN)
#define SMT_B_DECAY1_H() PORT_SetBits(SMT_B_DECAY1_PORT, SMT_B_DECAY1_PIN)
#define SMT_B_DECAY2_L() PORT_ResetBits(SMT_B_DECAY2_PORT, SMT_B_DECAY2_PIN)
#define SMT_B_DECAY2_H() PORT_SetBits(SMT_B_DECAY2_PORT, SMT_B_DECAY2_PIN)

#define SMT_DECAY1_L(id) ((id) == A_MOTOR ? SMT_A_DECAY1_L() : SMT_B_DECAY1_L())
#define SMT_DECAY1_H(id) ((id) == A_MOTOR ? SMT_A_DECAY1_H() : SMT_B_DECAY1_H())
#define SMT_DECAY2_L(id) ((id) == A_MOTOR ? SMT_A_DECAY2_L() : SMT_B_DECAY2_L())
#define SMT_DECAY2_H(id) ((id) == A_MOTOR ? SMT_A_DECAY2_H() : SMT_B_DECAY2_H())

#define DECAY_MIXED(id)                                                        \
    do {                                                                       \
        SMT_DECAY1_L(id);                                                      \
        SMT_DECAY2_L(id);                                                      \
    } while (0)
#define DECAY_SLOW(id)                                                         \
    do {                                                                       \
        SMT_DECAY1_L(id);                                                      \
        SMT_DECAY2_H(id);                                                      \
    } while (0)

#define DECAY_FAST(id)                                                         \
    do {                                                                       \
        SMT_DECAY1_H(id);                                                      \
        SMT_DECAY2_L(id);                                                      \
    } while (0)

#define DECAY_ADMD(id)                                                         \
    do {                                                                       \
        SMT_DECAY1_H(id);                                                      \
        SMT_DECAY2_H(id);                                                      \
    } while (0)
/**
 * @brief Function of DMODE0, DMODE1,DMODE2
 * @details
 * DMODE0 DMODE1 DMODE2 Function
 * L L L Full step resolution setting
 * L L H Half step resolution (a) setting
 * L H L Half step resolution (b) setting
 * L H H Quarter step resolution setting
 * H L L 1/8 step resolution setting
 * H L H 1/16 step resolution setting
 * H H L 1/32 step resolution setting
 * H H H 1/32 step resolution setting
 * @author lincq <lincq@raynen.cn>
 * @date 2024-05-21
 */
#define SMT_A_DMODE0 1
#define SMT_B_DMODE0 1

#define SMT_A_DMODE1_PORT (PortC)
#define SMT_A_DMODE1_PIN  (Pin00)
#define SMT_A_DMODE2_PORT (PortC)
#define SMT_A_DMODE2_PIN  (Pin01)

#define SMT_B_DMODE1_PORT (PortA)
#define SMT_B_DMODE1_PIN  (Pin04)
#define SMT_B_DMODE2_PORT (PortA)
#define SMT_B_DMODE2_PIN  (Pin05)

#define SMT_A_DMODE1_L() PORT_ResetBits(SMT_A_DMODE1_PORT, SMT_A_DMODE1_PIN)
#define SMT_A_DMODE1_H() PORT_SetBits(SMT_A_DMODE1_PORT, SMT_A_DMODE1_PIN)
#define SMT_A_DMODE2_L() PORT_ResetBits(SMT_A_DMODE2_PORT, SMT_A_DMODE2_PIN)
#define SMT_A_DMODE2_H() PORT_SetBits(SMT_A_DMODE2_PORT, SMT_A_DMODE2_PIN)

#define SMT_B_DMODE1_L() PORT_ResetBits(SMT_B_DMODE1_PORT, SMT_B_DMODE1_PIN)
#define SMT_B_DMODE1_H() PORT_SetBits(SMT_B_DMODE1_PORT, SMT_B_DMODE1_PIN)
#define SMT_B_DMODE2_L() PORT_ResetBits(SMT_B_DMODE2_PORT, SMT_B_DMODE2_PIN)
#define SMT_B_DMODE2_H() PORT_SetBits(SMT_B_DMODE2_PORT, SMT_B_DMODE2_PIN)

#define SMT_DMODE1_L(id) ((id) == A_MOTOR ? SMT_A_DMODE1_L() : SMT_B_DMODE1_L())
#define SMT_DMODE1_H(id) ((id) == A_MOTOR ? SMT_A_DMODE1_H() : SMT_B_DMODE1_H())
#define SMT_DMODE2_L(id) ((id) == A_MOTOR ? SMT_A_DMODE2_L() : SMT_B_DMODE2_L())
#define SMT_DMODE2_H(id) ((id) == A_MOTOR ? SMT_A_DMODE2_H() : SMT_B_DMODE2_H())

#if (SMT_A_DMODE0 == 1) && (SMT_B_DMODE0 == 1)
    #define STEP_RESOLUTION_1IN8(id)                                           \
        do {                                                                   \
            SMT_DMODE1_L(id);                                                  \
            SMT_DMODE2_L(id);                                                  \
        } while (0)
    #define STEP_RESOLUTION_1IN16(id)                                          \
        do {                                                                   \
            SMT_DMODE1_L(id);                                                  \
            SMT_DMODE2_H(id);                                                  \
        } while (0)
    #define STEP_RESOLUTION_1IN32(id)                                          \
        do {                                                                   \
            SMT_DMODE1_H(id);                                                  \
            SMT_DMODE2_L(id);                                                  \
        } while (0)
#endif
/**
 * @brief Torque Function
 * @details
 * TRQ1 pin input TRQ0 pin input Function
 * L L Torque setting: 100 %
 * L H Torque setting: 75 %
 * H L Torque setting: 50 %
 * H H Torque setting: 25 %
 * @author lincq <lincq@raynen.cn>
 * @date 2024-05-21
 */
#define SMT_A_TRQ0_PORT (PortC)
#define SMT_A_TRQ0_PIN  (Pin15)
#define SMT_A_TRQ1_PORT (PortA)
#define SMT_A_TRQ1_PIN  (Pin03)

#define SMT_B_TRQ0_PORT (PortC)
#define SMT_B_TRQ0_PIN  (Pin13)
#define SMT_B_TRQ1_PORT (PortC)
#define SMT_B_TRQ1_PIN  (Pin14)

#define SMT_A_TRQ0_L() PORT_ResetBits(SMT_A_TRQ0_PORT, SMT_A_TRQ0_PIN)
#define SMT_A_TRQ0_H() PORT_SetBits(SMT_A_TRQ0_PORT, SMT_A_TRQ0_PIN)
#define SMT_A_TRQ1_L() PORT_ResetBits(SMT_A_TRQ1_PORT, SMT_A_TRQ1_PIN)
#define SMT_A_TRQ1_H() PORT_SetBits(SMT_A_TRQ1_PORT, SMT_A_TRQ1_PIN)

#define SMT_B_TRQ0_L() PORT_ResetBits(SMT_B_TRQ0_PORT, SMT_B_TRQ0_PIN)
#define SMT_B_TRQ0_H() PORT_SetBits(SMT_B_TRQ0_PORT, SMT_B_TRQ0_PIN)
#define SMT_B_TRQ1_L() PORT_ResetBits(SMT_B_TRQ1_PORT, SMT_B_TRQ1_PIN)
#define SMT_B_TRQ1_H() PORT_SetBits(SMT_B_TRQ1_PORT, SMT_B_TRQ1_PIN)

#define SMT_TRQ0_L(id) ((id) == A_MOTOR ? SMT_A_TRQ0_L() : SMT_B_TRQ0_L())
#define SMT_TRQ0_H(id) ((id) == A_MOTOR ? SMT_A_TRQ0_H() : SMT_B_TRQ0_H())
#define SMT_TRQ1_L(id) ((id) == A_MOTOR ? SMT_A_TRQ1_L() : SMT_B_TRQ1_L())
#define SMT_TRQ1_H(id) ((id) == A_MOTOR ? SMT_A_TRQ1_H() : SMT_B_TRQ1_H())

#define TORQUE_RATE_100(id)                                                    \
    do {                                                                       \
        SMT_TRQ0_L(id);                                                        \
        SMT_TRQ1_L(id);                                                        \
    } while (0)
#define TORQUE_RATE_75(id)                                                     \
    do {                                                                       \
        SMT_TRQ0_L(id);                                                        \
        SMT_TRQ1_H(id);                                                        \
    } while (0)
#define TORQUE_RATE_50(id)                                                     \
    do {                                                                       \
        SMT_TRQ0_H(id);                                                        \
        SMT_TRQ1_L(id);                                                        \
    } while (0)
#define TORQUE_RATE_25(id)                                                     \
    do {                                                                       \
        SMT_TRQ0_H(id);                                                        \
        SMT_TRQ1_H(id);                                                        \
    } while (0)


typedef enum _decayMode {
    eDECAY_MODE_MIXED = 0,
    eDECAY_MODE_SLOW,
    eDECAY_MODE_FAST,
    eDECAY_MODE_ADMD, /* Advanced Dynamic Mixed Decay */
    eDECAY_MODE_MAX
} DecayMode_e;

typedef enum _stepResolution {
    eSTEP_RESOLUTION_1IN8 = 0,
    eSTEP_RESOLUTION_1IN16,
    eSTEP_RESOLUTION_1IN32
} StepResolution_e;

typedef enum _torqueRate {
    eTORQUE_RATE_100 = 0,
    eTORQUE_RATE_75,
    eTORQUE_RATE_50,
    eTORQUE_RATE_25
} TorqueRate_e;

static int MotorPos[MAX_MOTOR];



void Bsp_Motor_SetDecayMode(Enum_MotorID id, DecayMode_e mode);
void Bsp_Motor_SetStepResolution(Enum_MotorID id, StepResolution_e res);
void Bsp_Motor_SetTorque(Enum_MotorID id, TorqueRate_e rate);

void (*FreqChange_Callback)(Enum_MotorID);
void FreqChange(Enum_MotorID id){}


static void TimeOverA(void)
{
    SMT_A_STEP_TOG();
    TIMERA_ClearFlag(SPWM_A_TIMERA_UNIT, TimeraFlagOverflow);
    if (SMT_A_STEP_STATE() == 0)
    {  // 一高一低算一次
        FreqChange_Callback(A_MOTOR);  // 脉冲计数
    }
    else
    {//高电平计数
        if(SMT_A_DIR_STATE())
            MotorPos[A_MOTOR]++;
        else
            MotorPos[A_MOTOR]--;
    }
}
static void TimeOverB(void)
{
    SMT_B_STEP_TOG();
    TIMERA_ClearFlag(SPWM_B_TIMERA_UNIT, TimeraFlagOverflow);
    if (SMT_B_STEP_STATE() == 0)
    {// 一高一低算一次
        FreqChange_Callback(B_MOTOR);  // 脉冲计数
    }
    else
    {//高电平计数
        if(SMT_B_DIR_STATE())
            MotorPos[B_MOTOR]++;
        else
            MotorPos[B_MOTOR]--;
    }
}

void spwm_a_motor_init(void)
{
    stc_timera_base_init_t stcTimeraInit;
    stc_irq_regi_conf_t    stcIrqRegiConf;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcTimeraInit);
    MEM_ZERO_STRUCT(stcIrqRegiConf);

    /* Configuration peripheral clock */
    PWC_Fcg2PeriphClockCmd(SPWM_A_TIMERA_UNIT_CLOCK, Enable);

    /* Configuration timera unit 1 structure */
    stcTimeraInit.enClkDiv        = TimeraPclkDiv8;
    stcTimeraInit.enCntMode       = TimeraCountModeSawtoothWave;
    stcTimeraInit.enCntDir        = TimeraCountDirUp;
    stcTimeraInit.enSyncStartupEn = Disable;
    stcTimeraInit.u16PeriodVal    = SPWM_A_TIMERA_COUNT_OVERFLOW;
    TIMERA_BaseInit(SPWM_A_TIMERA_UNIT, &stcTimeraInit);
    TIMERA_IrqCmd(SPWM_A_TIMERA_UNIT, TimeraIrqOverflow, Enable);

    /* Configure interrupt of timera unit 1 */
    stcIrqRegiConf.enIntSrc    = SPWM_A_TIMERA_UNIT_OVERFLOW_INT;
    stcIrqRegiConf.enIRQn      = Int006_IRQn;
    stcIrqRegiConf.pfnCallback = &TimeOverA;
    enIrqRegistration(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, MOTOR_STEP_IRQ_PRIORITY);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
    /* Sync startup timera unit 2 when timera unit 1 startup */
    TIMERA_Cmd(SPWM_A_TIMERA_UNIT, Disable);
}

// 初始化
void spwm_b_motor_init(void)
{
    stc_timera_base_init_t stcTimeraInit;
    stc_irq_regi_conf_t    stcIrqRegiConf;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcTimeraInit);
    MEM_ZERO_STRUCT(stcIrqRegiConf);

    /* Configuration peripheral clock */
    PWC_Fcg2PeriphClockCmd(SPWM_B_TIMERA_UNIT_CLOCK, Enable);

    /* Configuration timera unit 1 structure */
    stcTimeraInit.enClkDiv        = TimeraPclkDiv8;
    stcTimeraInit.enCntMode       = TimeraCountModeSawtoothWave;
    stcTimeraInit.enCntDir        = TimeraCountDirUp;
    stcTimeraInit.enSyncStartupEn = Disable;
    stcTimeraInit.u16PeriodVal    = SPWM_B_TIMERA_COUNT_OVERFLOW;
    TIMERA_BaseInit(SPWM_B_TIMERA_UNIT, &stcTimeraInit);
    TIMERA_IrqCmd(SPWM_B_TIMERA_UNIT, TimeraIrqOverflow, Enable);

    /* Configure interrupt of timera unit 1 */
    stcIrqRegiConf.enIntSrc    = SPWM_B_TIMERA_UNIT_OVERFLOW_INT;
    stcIrqRegiConf.enIRQn      = Int007_IRQn;
    stcIrqRegiConf.pfnCallback = &TimeOverB;
    enIrqRegistration(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, MOTOR_STEP_IRQ_PRIORITY);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
    /* Sync startup timera unit 2 when timera unit 1 startup */
    TIMERA_Cmd(SPWM_B_TIMERA_UNIT, Disable);
}

void Bsp_Motor_Init(void)
{
    Bsp_Motor_Current_Init();
    FreqChange_Callback = &FreqChange;
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    PORT_Init(SMT_A_EN_PORT, SMT_A_EN_PIN, &stcPortInit);
    PORT_Init(SMT_A_DIR_PORT, SMT_A_DIR_PIN, &stcPortInit);
    PORT_Init(SMT_A_STEP_PORT, SMT_A_STEP_PIN, &stcPortInit);

    PORT_Init(SMT_B_EN_PORT, SMT_B_EN_PIN, &stcPortInit);
    PORT_Init(SMT_B_DIR_PORT, SMT_B_DIR_PIN, &stcPortInit);
    PORT_Init(SMT_B_STEP_PORT, SMT_B_STEP_PIN, &stcPortInit);

    PORT_Init(SMT_A_DECAY1_PORT, SMT_A_DECAY1_PIN, &stcPortInit);
    PORT_Init(SMT_A_DECAY2_PORT, SMT_A_DECAY2_PIN, &stcPortInit);
    PORT_Init(SMT_B_DECAY1_PORT, SMT_B_DECAY1_PIN, &stcPortInit);
    PORT_Init(SMT_B_DECAY2_PORT, SMT_B_DECAY2_PIN, &stcPortInit);

    PORT_Init(SMT_A_DMODE1_PORT, SMT_A_DMODE1_PIN, &stcPortInit);
    PORT_Init(SMT_A_DMODE2_PORT, SMT_A_DMODE2_PIN, &stcPortInit);
    PORT_Init(SMT_B_DMODE1_PORT, SMT_B_DMODE1_PIN, &stcPortInit);
    PORT_Init(SMT_B_DMODE2_PORT, SMT_B_DMODE2_PIN, &stcPortInit);

    PORT_Init(SMT_A_TRQ0_PORT, SMT_A_TRQ0_PIN, &stcPortInit);
    PORT_Init(SMT_A_TRQ1_PORT, SMT_A_TRQ1_PIN, &stcPortInit);
    PORT_Init(SMT_B_TRQ0_PORT, SMT_B_TRQ0_PIN, &stcPortInit);
    PORT_Init(SMT_B_TRQ1_PORT, SMT_B_TRQ1_PIN, &stcPortInit);

    SMT_A_STEP_OFF();
    SMT_B_STEP_OFF();
    SMT_A_EN_OFF();
    SMT_B_EN_OFF();

    spwm_a_motor_init();
    spwm_b_motor_init();

    Bsp_Motor_SetTorque(A_MOTOR, eTORQUE_RATE_100);
    Bsp_Motor_SetTorque(B_MOTOR, eTORQUE_RATE_100);
    Bsp_Motor_SetStepResolution(A_MOTOR, eSTEP_RESOLUTION_1IN32);
    Bsp_Motor_SetStepResolution(B_MOTOR, eSTEP_RESOLUTION_1IN32);
    Bsp_Motor_SetDecayMode(A_MOTOR, eDECAY_MODE_SLOW);
    Bsp_Motor_SetDecayMode(B_MOTOR, eDECAY_MODE_SLOW);
}

void Bsp_Motor_Start(Enum_MotorID id, uint freq)
{
    if (freq <= 10)
        freq = 10;

    uint16_t period = 0;
    period          = 6250000 / freq;
    if (id == A_MOTOR) {
        SMT_A_EN_ON();
        TIMERA_SetPeriodValue(SPWM_A_TIMERA_UNIT, period);
        TIMERA_Cmd(SPWM_A_TIMERA_UNIT, Enable);
    } else {
        SMT_B_EN_ON();
        TIMERA_SetPeriodValue(SPWM_B_TIMERA_UNIT, period);
        TIMERA_Cmd(SPWM_B_TIMERA_UNIT, Enable);
    }
}

void Bsp_Motor_Stop(Enum_MotorID id)
{
    if (id == A_MOTOR) {
        TIMERA_Cmd(SPWM_A_TIMERA_UNIT, Disable);
        SMT_A_STEP_OFF();
    } else {
        TIMERA_Cmd(SPWM_B_TIMERA_UNIT, Disable);
        SMT_B_STEP_OFF();
    }
}

void Bsp_Motor_SetDir(Enum_MotorID id, uint dir)
{
    if (id == A_MOTOR) {
        if(dir == 1)
            SMT_A_DIR_ON();
        else
            SMT_A_DIR_OFF();
    } else {
        if(dir == 1)
            SMT_B_DIR_ON();
        else
            SMT_B_DIR_OFF();
    }

}

void Bsp_Motor_SetFreqCallBack(void (*fun)(Enum_MotorID))
{
    FreqChange_Callback = fun;
}
void Bsp_Motor_SetPos(Enum_MotorID id,int pos)
{
    MotorPos[id] = pos;
}
int Bsp_Motor_GetPos(Enum_MotorID id)
{
    return MotorPos[id];
}

void Bsp_Motor_SetDecayMode(Enum_MotorID id, DecayMode_e mode)
{
    switch (mode) {
        case eDECAY_MODE_MIXED:
            DECAY_MIXED(id);
            break;
        case eDECAY_MODE_SLOW:
            DECAY_SLOW(id);
            break;
        case eDECAY_MODE_FAST:
            DECAY_FAST(id);
            break;
        case eDECAY_MODE_ADMD:
            DECAY_ADMD(id);
            break;
        default:
            DECAY_MIXED(id);
            break;
    }
}

void Bsp_Motor_SetStepResolution(Enum_MotorID id, StepResolution_e res)
{
    switch (res) {
        case eSTEP_RESOLUTION_1IN8:
            STEP_RESOLUTION_1IN8(id);
            break;
        case eSTEP_RESOLUTION_1IN16:
            STEP_RESOLUTION_1IN16(id);
            break;
        case eSTEP_RESOLUTION_1IN32:
            STEP_RESOLUTION_1IN32(id);
            break;
        default:
            STEP_RESOLUTION_1IN8(id);
            break;
    }
}

 void Bsp_Motor_SetTorque(Enum_MotorID id, TorqueRate_e rate)
{
     switch (rate) {
         case eTORQUE_RATE_100:
             TORQUE_RATE_100(id);
             break;
         case eTORQUE_RATE_75:
             TORQUE_RATE_75(id);
             break;
         case eTORQUE_RATE_50:
             TORQUE_RATE_50(id);
             break;
         case eTORQUE_RATE_25:
             TORQUE_RATE_25(id);
             break;
         default:
             TORQUE_RATE_100(id);
             break;
     }
 }
