#include "rn_typedef.h"
#include "Task_Cmd.h"
#include "Task_Main.h"
#include "Task_Motor.h"
#include "Task_Status.h"
#include "Task_Test.h"
#include "board_config.h"
#include "bsp.h"
#include "bsp_gpio.h"
#include "hc32_ddl.h"
#include "module.h"
#include "ucos_ii.h"
#include "rn_printf.h"

//  数值越小优先级越高
#define TASK_PRIO_TEST   (5)

#define TASK_PRIO_MOTOR  1
#define TASK_PRIO_MAIN   2
#define TASK_PRIO_CMD    3
#define TASK_PRIO_STATUS 11

#define TASK_STK_SIZE_TEST   4096
#define TASK_STK_SIZE_STATUS 1024
#define TASK_STK_SIZE_MAIN   1024
#define TASK_STK_SIZE_MOTOR  1024
#define TASK_STK_SIZE_CMD    1024

static OS_STK TaskStkTest[TASK_STK_SIZE_TEST];
static OS_STK TaskStkStatus[TASK_STK_SIZE_STATUS];
static OS_STK TaskStkMain[TASK_STK_SIZE_MAIN];
static OS_STK TaskStkCmd[TASK_STK_SIZE_CMD];
static OS_STK TaskStkMotor[TASK_STK_SIZE_CMD];

#define OS_TASK_CREATER(task, task_stk, task_stk_size, prio)                   \
    OSTaskCreateExt(task,                                                      \
                    (void *) 0,                                                \
                    (OS_STK *) &task_stk[task_stk_size - 1],                   \
                    prio,                                                      \
                    prio,                                                      \
                    (OS_STK *) &task_stk[0],                                   \
                    task_stk_size,                                             \
                    (void *) 0,                                                \
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

extern void flash_get_board_param(void);
int main(void)
{

#if (__FPU_USED == 1)
    SCB->CPACR |= (0xFu << 20);
    __DSB();
    __ISB();
#else
    #error fpu no use
#endif
    __disable_irq();
    Bsp_Init();
    flash_get_board_param();//先获取一次

    OS_CPU_SysTickInitFreq(200000000);  // cpu 200M
    OSInit(); /* Initialize "uC/OS-II, The Real-Time Kernel"*/


    __enable_irq();

    Module_Init();
#if (DEBUG_TOOL == PRINT_USE_JLINK_RTT)
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
#endif
    /* 上拨机头上电，进入测试模式 */
    if (SW_UP_YES) {
        INFO_PRINT("BOARD_TEST: "__DATE__" "__TIME__ " --\r\n\r\n");
        OS_TASK_CREATER(vTaskTest,TaskStkTest,TASK_STK_SIZE_TEST,TASK_PRIO_TEST);
    }
    else
    {
        INFO_PRINT("APP: "__DATE__" "__TIME__" --\r\n\r\n");
        OS_TASK_CREATER(TaskCmd, TaskStkCmd, TASK_STK_SIZE_CMD, TASK_PRIO_CMD);
        OS_TASK_CREATER(TaskMotorQf, TaskStkMotor, TASK_STK_SIZE_MOTOR, TASK_PRIO_MOTOR);
        OS_TASK_CREATER(TaskStatus,TaskStkStatus,TASK_STK_SIZE_STATUS,TASK_PRIO_STATUS);
        OS_TASK_CREATER(TaskMain,TaskStkMain,TASK_STK_SIZE_MAIN,TASK_PRIO_MAIN);
    }

    OSStart(); /* Start multitasking (i.e. give control to uC/OS-II)*/

    return (0);
}
