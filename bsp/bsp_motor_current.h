#pragma once

typedef enum {
    A_MOTOR = 0,
    B_MOTOR,
    MAX_MOTOR,
} Enum_MotorID;

extern void Bsp_Motor_Current_Init(void);
extern void Bsp_Motor_Current_Set(Enum_MotorID id, int current_x10);
extern int Bsp_Motor_Current_Get(Enum_MotorID id);

