#pragma once


typedef enum {
    LED_OFF,             // 关闭---机头关
    LED_RED,             // 红常亮---按键补绣
    LED_GRN,             // 绿常亮
} LED_CTRL;

extern void Mod_Led_Out(LED_CTRL sta);

