#pragma once

#include "bsp_motor_current.h"

extern void Bsp_Motor_Init(void);
extern void Bsp_Motor_SetFreqCallBack(void (*fun)(Enum_MotorID));
extern void Bsp_Motor_Stop(Enum_MotorID id);
extern void Bsp_Motor_Start(Enum_MotorID id, unsigned int freq);
extern void Bsp_Motor_SetDir(Enum_MotorID id, unsigned int dir);
extern int Bsp_Motor_GetPos(Enum_MotorID id);

