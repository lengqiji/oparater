#pragma once

#include <stdint.h>

extern void Fun_JumpMotor_Init(void);
extern void Fun_JumpMotor_TaskMs(void);
extern void Fun_JumpMotor_JumpSet(int para);
extern void Fun_JumpMotor_JumpAngleStart(void);
extern void Fun_JumpMotor_SetWorkI(uint8_t para);
extern void Fun_JumpMotor_SetDir(uint8_t para);
extern void Fun_JumpMotor_Follow();
extern void Fun_JumpMotor_SetNdlDownFlag(void);
extern void Fun_JumpMotor_StepsAdj(int8_t para);
extern void Fun_JumpMotor_SetDefaultStep(uint8_t para);
extern void Fun_JumpMotor_Force_JumpSet(int para);
extern int Fun_JumpMotor_Force_JumpGet(void);
extern void Fun_JumpMotor_Small_Cur_Set(uint32_t para);
extern uint32_t Fun_JumpMotor_Small_Cur_Get(void);
