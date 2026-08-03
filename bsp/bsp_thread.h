#pragma once

#include <stdint.h>

extern void Bsp_Thread_Init(void);
extern uint16_t Bsp_Thread_GetBotCount(void);

extern void Bsp_Thread_ClearBotCount(void);

extern void Bsp_Thread_SelectNdl(uint8_t ndl_num);

extern uint8_t Bsp_Thread_TopDetYes(void);

extern uint8_t Bsp_Thread_BotCnDetYes(void);

extern uint8_t Bsp_Thread_BotDetYes(void);
extern void Bsp_Thread_ERED_SelectNdl(uint8_t ndl_num,uint8_t _ndl_num_mode,uint8_t _ndl_num_type,uint8_t _a_opto_num,uint8_t _b_opto_num);

