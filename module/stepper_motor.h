#pragma once

#include "bsp_motor.h"
#include "rn_typedef.h"
extern void Mod_Motor_Init(void);

//返回值是false代表上个脉冲未完成，不执行新的脉冲
extern bool Mod_Motor_Act(Enum_MotorID id, uint freq, int step);

extern bool Mod_Motor_ActAbs(Enum_MotorID id, uint freq, int pos);

extern void Mod_Motor_Stop(Enum_MotorID id);

extern void Mod_Motor_ForceSpeedDown(Enum_MotorID id);

extern void Mod_Motor_SetSpeedTable(Enum_MotorID id, int *p_table, int size);

extern bool Mod_Motor_GetRunState(Enum_MotorID id);

extern void Mod_Motor_SetLogicDir(Enum_MotorID id, bool dir);
extern void Mod_Motor_SetHoldI(Enum_MotorID id, uint var);
extern void Mod_Motor_SetWorkI(Enum_MotorID id, uint var);
extern int Mod_Motor_IsRun(Enum_MotorID id);

extern uint Mod_Motor_GetWorkI(Enum_MotorID id);
extern void Mod_Motor_SetFinishCallBack(void (*finish_cb)(Enum_MotorID));
