#pragma once

extern void Task_Cmd_Work(struct can_message *pFrame);
extern void Task_Cmd_Work_Ext(struct can_message *pFrame);
extern void Task_Cmd_Ext_Param(struct can_message *pFrame);
extern void Task_Cmd_Ext_HookSet(struct can_message *pFrame);

extern void Task_Cmd_Ext_HeadEn(struct can_message *pFrame);

extern void Task_Cmd_Ext_ManualTrim(struct can_message *pFrame);
extern void Task_Cmd_Ext_FootTestMode(struct can_message *pFrame);
extern void Task_Cmd_Ext_SetLock(struct can_message *pFrame);
extern void Task_Cmd_Ext_HeadStatus(struct can_message *pFrame);
extern void Task_Cmd_Ext_Head_Type(struct can_message *pFrame);
