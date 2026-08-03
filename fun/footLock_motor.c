#include "hc32_ddl.h"
#include "rn_typedef.h"
#include "ucos_ii.h"

#include "para.h"
#include "stepper_motor.h"

#define MOTOR_FOOT_LOCK_ID           B_MOTOR
#define MOTOR_FOOT_DIV               32  // 细分数
#define FOOT_ANGLE_2_STEP(angle)     ((angle) * MOTOR_FOOT_DIV * 200 / 360)
#define MOTOR_FOOT_LOCK_DEFAULT_STEP FOOT_ANGLE_2_STEP(80)
#define MOTOR_FOOT_LOCK_FREQ         32000
#define FOOT_LOCK_CUR_WORK           12
#define FOOT_LOCK_CUR_HOLD           8

typedef struct _footLock {
    bool     bCmdFootLock;
    bool     bLock;
    uint32_t ulNewNdlCnt;
    uint32_t freq;
    int      iPulse;
} FootLock_s;

static void FootLock_ChangeHoldI();
static void FootLock_HeadOffJudge(uint8_t *lock);
static void FootLock_Lock(void);
static void FootLock_Unlock(void);

static int speed_table[] = {
    800,   815,   914,   1019,  1130,  1247,  1372,  1502,  1639,  1782,  1931,
    2087,  2168,  2249,  2333,  2418,  2505,  2593,  2683,  2774,  2868,  2962,
    3000,  3100,  3200,  3300,  3400,  3550,  3600,  3650,  3700,  3800,  3900,
    4000,  4300,  4500,  4800,  5000,  5100,  5250,  5300,  5400,  5450,  5500,
    5600,  5650,  5700,  5850,  5900,  6000,  6100,  6200,  6300,  6350,  6400,
    6500,  6600,  6650,  6700,  6750,  7000,  7200,  7300,  7400,  7500,  7600,
    7700,  7800,  8100,  8200,  8300,  8400,  8500,  8600,  8700,  8800,  8900,
    9200,  9300,  9400,  9500,  9600,  9700,  9800,  9900,  10000, 10500, 11000,
    11500, 12000, 12500, 13000, 13500, 14000, 14500, 15000, 15500, 16000, 16500,
    17000, 17500, 18000, 20000, 22000, 24000, 26000, 28000, 30000, 32000, 34000,
    36000, 38000, 40000, 42000, 44000, 46000, 48000, 50000, 52000, 54000, 56000,
    58000, 60000, 62000, 64000, 66000, 68000, 70000, 72000, 74000, 76000, 78000,
    80000, 82000, 84000, 86000, 88000, 90000, 92000, 94000, 96000, 98000, 100000};

static FootLock_s FootLock = {.bCmdFootLock = false,
                              .bLock        = false,
                              .ulNewNdlCnt  = 0,
                              .freq         = MOTOR_FOOT_LOCK_FREQ,
                              .iPulse       = MOTOR_FOOT_LOCK_DEFAULT_STEP};

void Fun_FootLock_Init(void)
{
    Mod_Motor_SetHoldI(MOTOR_FOOT_LOCK_ID, FOOT_LOCK_CUR_HOLD);
    Mod_Motor_SetWorkI(MOTOR_FOOT_LOCK_ID, FOOT_LOCK_CUR_WORK);
    Bsp_Motor_Current_Set(MOTOR_FOOT_LOCK_ID, FOOT_LOCK_CUR_HOLD);
    Mod_Motor_SetSpeedTable(MOTOR_FOOT_LOCK_ID,
                            &speed_table[0],
                            sizeof(speed_table) / sizeof(speed_table[0]));
}

void Fun_FootLock_Task(void)
{
    uint8_t foot_lock = 0;

    static uint8_t run_ndl_num_old = 0;
    static uint8_t unlock_cnt      = 2;

    if (Para_head.mainshaft_run)  // 主轴启动
    {
        if ((Para_head.stat == STATUS_OFF)
            || (Para_emb.stat == STATUS_PATCH)
                   && (Para_head.stat == STATUS_NORMAL)) {
            if (unlock_cnt) {
                foot_lock = 1;
            }
            if (run_ndl_num_old != FootLock.ulNewNdlCnt) {
                run_ndl_num_old = FootLock.ulNewNdlCnt;
                if (unlock_cnt) {
                    unlock_cnt--;
                }
            }
        }
    } else {
        run_ndl_num_old = 0;
        unlock_cnt      = 2;
        FootLock_HeadOffJudge(&foot_lock);
    }

    if (foot_lock || FootLock.bCmdFootLock) {
        FootLock_Lock();
    } else {
        FootLock_Unlock();
    }
    FootLock_ChangeHoldI();
}

void Fun_FootLock_NewNdlCnt(void)
{
    FootLock.ulNewNdlCnt++;
}

void Fun_FootLock_ClrNdlCnt(void)
{
    FootLock.ulNewNdlCnt = 0;
}

void Fun_FootLock_SetLock(bool bLock)
{
    FootLock.bCmdFootLock = bLock;
}

void Fun_FootLock_SetCfg(uint8_t *p_cfg)
{
    Mod_Motor_SetWorkI(MOTOR_FOOT_LOCK_ID, p_cfg[0]);
    FootLock.freq   = MOTOR_FOOT_LOCK_FREQ * p_cfg[1];
    FootLock.iPulse = FOOT_ANGLE_2_STEP(p_cfg[2]);
}

static void FootLock_ChangeHoldI()
{  // 跟随动作时保持原先和工作电流一样，这里要还原保持电流

    static uint workTime = 0;

    if (Mod_Motor_IsRun(MOTOR_FOOT_LOCK_ID))
        workTime = OSTimeGet();
    else if (workTime != 0) {
        int offset = OSTimeGet() - workTime;  // 1个100us
        if (abs(offset) > 1000)               // 100ms切换保持电流
        {
            workTime = 0;

            Mod_Motor_SetHoldI(MOTOR_FOOT_LOCK_ID, FOOT_LOCK_CUR_HOLD);

            if (FootLock.bLock == false)  // 保持电流直接0，有弹簧
                Bsp_Motor_Current_Set(MOTOR_FOOT_LOCK_ID, 0);
            else
                Bsp_Motor_Current_Set(MOTOR_FOOT_LOCK_ID, FOOT_LOCK_CUR_HOLD);
        }
    }
}

static void FootLock_HeadOffJudge(uint8_t *lock)
{
    static status_e _xOldState   = STATUS_INITIAL;
    static uint32_t _ulTimeBegin = 0;

    if (Para_head.stat == STATUS_INITIAL) {
        return;
    }

    if (OSTimeGet() < (_ulTimeBegin + 3000)) {
        *lock = 1;
    }

    if (Para_head.stat == _xOldState) {
        return;
    }
    _xOldState = Para_head.stat;

    if (Para_head.stat == STATUS_OFF) {
        _ulTimeBegin = OSTimeGet();
    }
}

static void FootLock_Lock(void)
{
    if (FootLock.bLock) {
        return;
    }

    if (true == Mod_Motor_ActAbs(
                    MOTOR_FOOT_LOCK_ID, FootLock.freq, FootLock.iPulse)) {
        FootLock.bLock = true;
    }
}

static void FootLock_Unlock(void)
{
    if (!FootLock.bLock) {
        return;
    }

    if (true == Mod_Motor_ActAbs(MOTOR_FOOT_LOCK_ID, FootLock.freq, 0)) {
        FootLock.bLock = false;
    }
}

