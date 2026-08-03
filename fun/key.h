#ifndef __KEY_H__
#define __KEY_H__

// 状态
typedef enum {
    KEY_STATUS_NO,      // 无
    KEY_STATUS_UP,      // 上拨
    KEY_STATUS_DOWN,    // 下拨
    KEY_STATUS_CHANGE,  // 改变
} KEY_STATUS;


extern KEY_STATUS Fun_Key_GetState(void);
extern void Fun_Key_UpdateState(void);
extern void Fun_Key_SetState(uint8_t _key_state);
#endif
