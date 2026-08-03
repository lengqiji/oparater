#include "signal_led.h"
#include "bsp_gpio.h"
#include "cmd.h"
#include "led.h"
#include "para.h"
// 状态
typedef enum {
    LED_STATUS_OFF,             // 关闭---机头关
    LED_STATUS_RED,             // 红常亮---按键补绣
    LED_STATUS_GRN,             // 绿常亮
    LED_STATUS_RED_FLASH,       // 红闪---编址
    LED_STATUS_GRN_FLASH,       // 绿闪---正在补绣
    LED_STATUS_RED_GRN_FLASH,   // 红绿闪---机头未初始化
    LED_STATUS_RED_FAST_FLASH,  // 红快闪
    LED_STATUS_RED3_GRN1,       // 红快闪3次，绿慢闪1次---断线
} LED_STATUS;

// 参数结构体
typedef struct {
    LED_STATUS sta;      // 当前状态
    LED_STATUS pre_sta;  // 上次状态
    uint32_t   cnt;      // 计时，10ms
} led_param_s;

static led_param_s led_param;  // led参数

// led状态设置 //2ms
void led_update_state(void)
{
    if (Para_head.stat == STATUS_INITIAL) {
        led_param.sta = LED_STATUS_RED_GRN_FLASH;
    } else if (Para_head.stat == STATUS_OFF) {
        led_param.sta = LED_STATUS_OFF;
    }

    else if (Para_head.addr_ok_flag || Para_head.addr_cnt || Para_emb.IsBrk) {
        led_param.sta = LED_STATUS_RED_FLASH;
        if (Para_head.addr_cnt) {
            Para_head.addr_cnt--;
            if (!Para_head.addr_cnt) {
                Mod_Cmd_RspLookAddr();
            }
        }

    } else if (Para_head.iskey_patch == 1)  //
    {
        led_param.sta = LED_STATUS_RED;
    } else if ((Para_head.stat == STATUS_PATCH)
               || (Para_head.pressFoot_test))  // 改为绿闪
    {
        led_param.sta = LED_STATUS_GRN_FLASH;
    } else {
        led_param.sta = LED_STATUS_GRN;
    }
}

// 红快闪3次（500ms），绿慢闪1次(750ms)
void led_redf3_grn1(void)
{
    static uint8_t cnt = 0, n = 0;

    cnt++;
    if (cnt >= 20) {
        cnt = 0;
        n++;
        if (n <= 6) {
            LR_TRIG;
        } else if (n == 7) {
            LR_OFF;
            LG_ON;
        } else if (n == 8) {
            LR_OFF;
            LG_OFF;
            n = 0;
        }
    }
}

// 10ms
void Fun_Led_Task(void)
{
    static uint8_t _ucMs = 0;
    if (++_ucMs < 10) {
        return;
    }
    _ucMs = 0;
    led_update_state();

    if (led_param.sta != led_param.pre_sta) {
        led_param.pre_sta = led_param.sta;
        led_param.cnt     = 0;

        switch (led_param.sta) {
            case LED_STATUS_GRN:
                LG_ON;
                LR_OFF;
                Mod_Led_Out(LED_GRN);
                break;
            case LED_STATUS_RED:
                LR_ON;
                LG_OFF;
                Mod_Led_Out(LED_RED);
                break;
            case LED_STATUS_OFF:
                LG_OFF;
                LR_OFF;
                Mod_Led_Out(LED_OFF);
                break;

            case LED_STATUS_GRN_FLASH:
            case LED_STATUS_RED_GRN_FLASH:
                LG_ON;
                LR_OFF;
                if (led_param.sta == LED_STATUS_GRN_FLASH)
                    Mod_Led_Out(LED_RED);
                else
                    Mod_Led_Out(LED_OFF);
                break;

            case LED_STATUS_RED_FLASH:
            case LED_STATUS_RED3_GRN1:
                LR_ON;
                LG_OFF;
                if (Para_emb.IsBrk)
                    Mod_Led_Out(LED_RED);
                break;

            default:
                break;
        }
    } else {
        led_param.cnt++;

        if (led_param.sta == LED_STATUS_RED3_GRN1) {
            led_redf3_grn1();
        } else if ((led_param.cnt % 25) == 0) {
            if (led_param.sta == LED_STATUS_RED_GRN_FLASH) {
                LG_TRIG;
                LR_TRIG;
            } else if (led_param.sta == LED_STATUS_GRN_FLASH) {
                LG_TRIG;
            } else if (led_param.sta == LED_STATUS_RED_FLASH) {
                LR_TRIG;
            }
        }
    }
}
