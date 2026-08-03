#pragma once

#include "bsp_can.h"
extern void Mod_Cmd_Init(void);

extern void Mod_Cmd_Recv(void);

extern void Mod_Cmd_RspSetAddr(void);

extern void Mod_Cmd_RspAutoSetAddr(void);

extern void Mod_Cmd_RspLookAddr(void);

extern void Mod_Cmd_RspAlarm(uint16_t alarm_var);

extern void Mod_Cmd_RspKeyStat(uint8_t status);
extern void Mod_Cmd_RspHeadStat(uint8_t status);

extern void Mod_Cmd_RspBrk(void);
extern void Mod_Cmd_RspHookState(uint8_t status);
extern void Mod_Cmd_RspHeart(struct can_message *pFrame);
extern void Mod_Cmd_RspVerExt(struct can_message *pFrame);
extern void Mod_Cmd_RspResetProc(void);
extern void Mod_Cmd_RspFootStatus(uint8_t status, uint8_t cnt);
extern void Mod_Cmd_RspFootLoc(uint8_t foot_loc);

extern void Mod_Cmd_TxCallBack(void (*callback)(struct can_message *));
