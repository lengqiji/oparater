#pragma once

// 锁线任务
extern void Fun_Lock_TaskMs(void);  // 必须1ms一次

extern void Fun_Lock_Init(void);

extern void Fun_Lock_Hold(void);
extern void Fun_Lock_SetHold(void);

extern void Fun_Lock_Release(void);
extern void Fun_Lock_SetRelease(void);

extern void Fun_Lock_DelayOpenSet(void);

extern void Fun_Lock_DelayOpenCount(void);

extern void Fun_Lock_SetLockPower(uint8_t param);

extern void Fun_Lock_SetDir(uint8_t param);
extern void Fun_Lock_SetPatchNo(uint8_t param);
extern void Fun_Lock_SetType(uint8_t type);
extern void Fun_Lock_SetPwmDuty(uint8_t duty);
