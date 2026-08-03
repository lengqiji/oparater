#include "ucos_ii.h"
#include "rn_typedef.h"
#include "Task_Main.h"
#include "lock.h"
#include "thread_check.h"
#include "hook_motor.h"
#include "jump_motor.h"
#include "footLock_motor.h"
#include "para.h"
#include "board_config.h"
#include "bsp_flash.h"
#include "cmd.h"
#include "qf.h"

void head_parm_init(void)
{
    Para_head.stat = STATUS_INITIAL;
    Para_head.jump_no_follow = 1;
    Para_head.en = 1;
    Para_head.set_en = 1;

    //---start--下面一起屏蔽也可以，main里面已经有flash_get_board_param
    Para_head.type = HDAPP_CFG_TYPE;
    Para_head.addr = 1;
    flash_get_board_param();
    //---end---
    
    //Para_head.jump_param.type = JUMP_TYPE_MAG;
    //Para_head.jump_param.type = JUMP_TYPE_MOTOR;

    Fun_Thread_SetCurNdlNum(1);
    Fun_Thread_Rst();
}

__attribute__((section("FastRam"))) void TaskMainInit()
{
    Fun_Lock_Hold();
    head_parm_init();
    //时间会影响，挪走Mod_Cmd_RspResetProc();
}



__attribute__((section("FastRam"))) void TaskMain(void *pvParams)
{//1ms一次
    Fun_Lock_Init();
    if (Para_head.mach_type == MACH_TYPE_FOOT_COM_JD) {
        Fun_FootLock_Init();
    } else {
        Fun_HookMotor_Init();
    }
    Fun_Thread_Init();
    TaskMainInit();
    OSTimeDlyHMSM(0, 0, 0, 10);//
    Mod_Cmd_RspResetProc();//发送can数据需等taskcmd 建立
    while(1)
    {
        //锁线任务
        Fun_Lock_TaskMs();
        //断线检测
        Fun_Thread_BrkCheckMs();
        if(Para_head.mach_type == MACH_TYPE_FOOT_COM_JD) {
            Fun_FootLock_Task();
        }else {
            //勾线电机动作
            Fun_HookMotor_Task();
        }

        OSTimeDlyHMSM(0, 0, 0, 1);
        Mod_Qf_RpmUpdateMs();//计数主轴速度
    }
}
