#pragma once
#include "rn_typedef.h"
extern void Fun_Thread_Init(void);

extern void Fun_Thread_Rst(void);

extern void Fun_Thread_BrkCheckMs(void);

extern void Fun_Thread_NewNdlLen(uint8_t param);

extern void Fun_Thread_NewNdl(void);

extern void Fun_Thread_MainStop(void);

extern void Fun_Thread_SetNdlNum(uint8_t num);

extern void Fun_Thread_SetCurNdlNum(uint8_t num);

extern void Fun_Thread_SetStartFilter(uint8_t filter);

extern void Fun_Thread_SetType(uint8_t type);

extern void Fun_Thread_SetTopSense(uint8_t param);

extern void Fun_Thread_SetBotSense(uint8_t param);

extern void Fun_Thread_SetTopFilter(uint8_t param);

extern void Fun_Thread_SetBotFilter(uint8_t param);

extern void Fun_Thread_SetIndepCheck(uint8_t param);

extern void Fun_Thread_SetEn(int en);

extern void Fun_Thread_SetSelfCheck(int en);

extern void Fun_Thread_CheckStart(void);

extern void Fun_Thread_ClrBrk();
extern void Fun_Thread_SetBrk();
extern uint32_t Fun_Thread_GetBrk();

extern void Fun_Thread_SetNdlNumReverse(uint8_t rev);
extern void Fun_Thread_SetNdlNum_Mode_Type(uint8_t _mode,uint8_t _type);

extern void     Fun_thread_ExtInfoBrk(uint32_t brk);
extern uint32_t Fun_Thread_GetIndepChk(void);
extern uint32_t Fun_Thread_Splice_no_judge_flag(void);
extern void Fun_thread_Splice_head_ndl_num(uint32_t splice_head_ndl_num);
extern void Fun_thread_splice_head_group_calc(void);
extern uint8_t Fun_thread_Is_splice_samegroup(uint8_t idno);
