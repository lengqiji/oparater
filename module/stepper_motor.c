#include "rn_typedef.h"
#include "bsp_motor.h"
#include "stepper_motor.h"

#define BSP_SPEED_TABLE_SIZE 100

typedef struct {
    uint32_t workI;//工作电流
    uint32_t holdI;//保持电流

    uint32_t total_cnt;//总脉冲数
    uint32_t cur_cnt;  //当前脉冲数
    uint32_t cur_SpeedLevel;//单前速度挡
    uint32_t max_speed;//设定最高速度
    int speed_down_flag;//减速标志

    bool     bLogicDir;        // 1 方向取反
    bool     RunFlag;   /* 运行标志 */
    int     *p_SpeedTable;
    int   SpeedTableSize;
} step_conf_para;


static int     speed_table[BSP_SPEED_TABLE_SIZE] = {0};
static step_conf_para Step_Para[MAX_MOTOR];

static void Motor_Finish_Null(Enum_MotorID id) {}
static void (*Motor_Finish_Callback)(Enum_MotorID) = Motor_Finish_Null;

static inline int GetCurSpeed(Enum_MotorID id, uint level)
{
    if (level >= Step_Para[id].SpeedTableSize)
        level = Step_Para[id].SpeedTableSize - 1;
    return *(Step_Para[id].p_SpeedTable + level);
}

void Mod_Motor_FreqChange(Enum_MotorID id)
{  // 脉冲计数处理,
    step_conf_para *p_Para = &Step_Para[id];

    p_Para->cur_cnt++;
    if (p_Para->cur_cnt >= p_Para->total_cnt) {
        Mod_Motor_Stop(id);               // 停止
        Motor_Finish_Callback(id);
    } else {
        int left_cnt = p_Para->total_cnt - p_Para->cur_cnt;

        if (p_Para->cur_SpeedLevel > left_cnt) {  // 不够减速了
            p_Para->cur_SpeedLevel = left_cnt;
        } else if (p_Para->speed_down_flag) {  // 强制减速
            if (p_Para->cur_SpeedLevel > 0)
                p_Para->cur_SpeedLevel--;
        } else if (p_Para->cur_cnt <= p_Para->total_cnt / 2) {  // 可加速
            if (GetCurSpeed(id, p_Para->cur_SpeedLevel) <= p_Para->max_speed) {
                p_Para->cur_SpeedLevel++;
            }
        }

        int freq = GetCurSpeed(id, p_Para->cur_SpeedLevel);

        if (freq > p_Para->max_speed)
            freq = p_Para->max_speed;

        Bsp_Motor_Start(id, freq);
    }
}


void Mod_Motor_Init(void)
{
    for (int id = 0; id < MAX_MOTOR; id++) {
        Step_Para[id].p_SpeedTable        = &speed_table[0];
        Step_Para[id].SpeedTableSize      = sizeof(speed_table)/sizeof(int);
        Step_Para[id].RunFlag      = false;
        Step_Para[id].bLogicDir           = false;
        Step_Para[id].holdI = 8;//初始值
        Step_Para[id].workI = 20;
    }
    Bsp_Motor_SetFreqCallBack(&Mod_Motor_FreqChange);
}


bool Mod_Motor_ActAbs(Enum_MotorID id, uint freq, int pos)
{
    if(Step_Para[id].RunFlag == true)
        return false;//如果上一个未完成，曲线规划会有问题

    int cur_pos = Bsp_Motor_GetPos(id);
    if (Step_Para[id].bLogicDir) {
        cur_pos = -cur_pos;
    }
    int step = pos - cur_pos;

    return Mod_Motor_Act(id,freq,step);
}

bool Mod_Motor_Act(Enum_MotorID id, uint freq, int step)
{
    if(Step_Para[id].RunFlag == true)
        return false;//如果上一个未完成，曲线规划会有问题


    if(Step_Para[id].bLogicDir == true)
        step = -step;//方向是反的

    if (step == 0) {  // 脉冲为0直接停，回调
        return true;
    } else if (step < 0) {  // 先给方向，脉冲先低电平，驱动器上升沿计数
        step = - step;
        Bsp_Motor_SetDir(id,0);
    }
    else
        Bsp_Motor_SetDir(id,1);

    Step_Para[id].total_cnt       = step;  // 运行参数初始化
    Step_Para[id].cur_cnt         = 0;
    Step_Para[id].cur_SpeedLevel  = 0;
    Step_Para[id].speed_down_flag = 0;
    Step_Para[id].max_speed       = freq;
    Step_Para[id].RunFlag  = true;//启动标志给1

    Bsp_Motor_Current_Set(id, Step_Para[id].workI);
    int startFreq = Step_Para[id].p_SpeedTable[0];
    if(startFreq > freq)
        startFreq = freq;
    Bsp_Motor_Start(id, startFreq);  // 初始速度固定
    return true;
}
void Mod_Motor_Stop(Enum_MotorID id)
{
    Bsp_Motor_Stop(id);
    Bsp_Motor_Current_Set(id, Step_Para[id].holdI);
    Step_Para[id].RunFlag  = false;//启动标志给0
}

void Mod_Motor_SetSpeedTable(Enum_MotorID id, int *p_table, int size)
{
    Step_Para[id].p_SpeedTable = p_table;
    Step_Para[id].SpeedTableSize = size;
}

void Mod_Motor_SetLogicDir(Enum_MotorID id, bool dir)
{
    Step_Para[id].bLogicDir = (bool) dir;
}

void Mod_Motor_SetHoldI(Enum_MotorID id, uint var)
{
    Step_Para[id].holdI = var;
}
void Mod_Motor_SetWorkI(Enum_MotorID id, uint var)
{
    Step_Para[id].workI = var;
}
uint Mod_Motor_GetWorkI(Enum_MotorID id)
{
    return Step_Para[id].workI;
}

int Mod_Motor_IsRun(Enum_MotorID id)
{
    return Step_Para[id].RunFlag;
}

void Mod_Motor_SetFinishCallBack(void (*finish_cb)(Enum_MotorID))
{
    Motor_Finish_Callback = finish_cb;
}