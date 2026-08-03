#ifndef __CMD_ADDR_H__
#define __CMD_ADDR_H__

#include "bsp_can.h"

extern void Task_Cmd_Addr(struct can_message *pFrame);
extern void Task_Cmd_Addr_Ext(struct can_message *pFrame);

#endif
