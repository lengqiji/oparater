#include "rn_typedef.h"
#include "para.h"
#include "hook_motor.h"
#include "bsp_motor.h"
#include "stepper_motor.h"
#include "cmd.h"

#define MOTOR_HOOK_ID       B_MOTOR
#define MOTOR_HOOK_WORKI      25//2.5
#define MOTOR_HOOK_HOLDI      8//

#define MOTOR_HOOK_DIV           32
#define MOTOR_HOOK_STEP_RES_COEF (32 / 8) //操作盒（0~550），这里是为了跟旧板兼容

typedef enum
{
    MOTOR_STA_RUN_ERR,		//电机运行出错
    MOTOR_STA_RUN_OK,		//电机运行完成
    MOTOR_STA_RUN_ING,		//电机运行中
}MOTOR_STA_E;


typedef struct _motorHookCfg {
    int  bActFlg;  // 勾线是否动作标志
    int   freq;    /* 勾线电机动作速度 */
    int   pulse;     /* 勾线电机动作脉冲数，负数表示反方向 */
    MOTOR_STA_E   state;      /* 勾线状态 */
} MotorHookCfg_s;


MotorHookCfg_s HookState;


static int speed_table[] = {
    400,401,406,415,426,442,460,482,507,536,
    568,603,641,683,729,778,830,885,944,1006,
    1072,1140,1213,1288,1367,1450,1532,1611,1686,1759,
    1828,1893,1955,2014,2040,2069,2090,2122,2150,2170,
    2195,2216,2236,2258,2278,2296,2315,2332,2348,2363,
    2378,2392,2404,2417,2427,2439,2449,2458,2465,2473,
    2478,2484,2489,2493,2500,3200,3400,3600,3800,4000,
    4200,4400,4600,4800,5000,5200,5400,5600,5800,6000,
    6200,6400,6600,6800,7000,7200,7400,7600,7800,8000,
    8200,8400,8600,8800,9000,9200,9400,9600,9800,10000
};


void Fun_HookMotor_Init(void)
{
    Mod_Motor_SetHoldI(MOTOR_HOOK_ID, MOTOR_HOOK_HOLDI);
    Mod_Motor_SetWorkI(MOTOR_HOOK_ID, MOTOR_HOOK_WORKI);
    //Bsp_Motor_Current_Set(MOTOR_HOOK_ID, MOTOR_HOOK_HOLDI);
    Bsp_Motor_Current_Set(MOTOR_HOOK_ID, 0);//初始给0
   // Bsp_Motor_Current_Set(MOTOR_HOOK_ID, 0);//常规不锁轴
    Mod_Motor_SetSpeedTable(MOTOR_HOOK_ID,&speed_table[0],sizeof(speed_table)/sizeof(int));
}

void HookMotor_Act(uint freq, int step)
{
    Mod_Motor_Act(MOTOR_HOOK_ID,freq,step);
}


void Fun_HookMotor_Task()
{

    if (Para_head.stat == STATUS_OFF || HookState.pulse == 0)
    {
        if (HookState.bActFlg != 0)
        {
            HookState.bActFlg = 0;
            Mod_Cmd_RspHookState(MOTOR_STA_RUN_OK);//需回复完成
            HookState.state = MOTOR_STA_RUN_OK;
            return;
        }
    }

    if (HookState.bActFlg != 0)
    {
        HookMotor_Act(HookState.freq,HookState.pulse);
        HookState.bActFlg = 0;
        HookState.state = MOTOR_STA_RUN_ING;
    }

    if(HookState.state == MOTOR_STA_RUN_ING)
    {
       if(Mod_Motor_IsRun(MOTOR_HOOK_ID) == 0)//这次完成
       {
           Mod_Cmd_RspHookState(MOTOR_STA_RUN_OK);//需回复完成
           HookState.state = MOTOR_STA_RUN_OK;
       }

    }
}

void Fun_HookMotor_ActFlag(int en,uint freq, int step)
{
    if(en == 0)
        Mod_Motor_SetHoldI(MOTOR_HOOK_ID, 0);
    else
        Mod_Motor_SetHoldI(MOTOR_HOOK_ID, MOTOR_HOOK_HOLDI);

    HookState.bActFlg = 1;
    HookState.pulse = step * MOTOR_HOOK_STEP_RES_COEF;//
    HookState.freq = freq * MOTOR_HOOK_STEP_RES_COEF;//
}

void Fun_HookMotor_SetWorkI(uint8_t         para)
{
    Mod_Motor_SetWorkI(MOTOR_HOOK_ID, para/10);
}

void Fun_HookMotor_SetDir(uint8_t para)
{
    if (para <= 1 && Para_head.type == HEAD_TYPE_FLAT)
        Mod_Motor_SetLogicDir(MOTOR_HOOK_ID, para);
}
