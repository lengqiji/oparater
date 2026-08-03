#ifndef __CMD_PARAMS_H__
#define __CMD_PARAMS_H__

#include "bsp_can.h"

extern void Task_Cmd_Params(struct can_message *pFrame);
extern void Task_Cmd_Params_Ext(struct can_message *pFrame);

#endif
