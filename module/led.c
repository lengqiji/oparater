#include "led.h"
#include "bsp_gpio.h"

// 对外输出同步状态，传给特种绣
void Mod_Led_Out(LED_CTRL sta)
{
    switch (sta) {
        case LED_GRN:
            ZPG_ON;
            ZPR_OFF;
            break;

        case LED_RED:
            ZPR_ON;
            ZPG_OFF;
            break;

        case LED_OFF:
            ZPG_OFF;
            ZPR_OFF;
            break;

        default:
            break;
    }
}
