#ifndef __CMD_WORK_H__
#define __CMD_WORK_H__

#include "bsp_can.h"

extern void Task_Cmd_Work(struct can_message *pFrame);
extern void Task_Cmd_Work_Ext(struct can_message *pFrame);

#endif
