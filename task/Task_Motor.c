#include "rn_typedef.h"
#include "Task_Motor.h"
#include "jump_motor.h"
#include "ucos_ii.h"

__attribute__((section("FastRam"))) void TaskMotorQf(void *pvParams)
{
    Fun_JumpMotor_Init();
    OSTimeDlyHMSM(0, 0, 0, 10);//等待其他认为初始化
	
    
    int count = 0;
    while(1)
    {
        count ++;
        if(count %10 == 0)
            Fun_JumpMotor_TaskMs();//跳跃电机动作
            
        Fun_JumpMotor_Follow();
        OSTimeDly(1);
    }
}
