#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include "hc32_ddl.h"
#include "para.h"
// LED
#define MCU_LED_PORT     (PortB)
#define MCU_LED_PIN      (Pin01)
#define MCU_LED_ON()     (PORT_SetBits(MCU_LED_PORT, MCU_LED_PIN))
#define MCU_LED_OFF()    (PORT_ResetBits(MCU_LED_PORT, MCU_LED_PIN))
#define MCU_LED_TOGGLE() (PORT_Toggle(MCU_LED_PORT, MCU_LED_PIN))

// 状态灯
#define LR_PORT (PortB)
#define LR_PIN  (Pin14)
#define LR_ON   PORT_ResetBits(LR_PORT, LR_PIN)
#define LR_OFF  PORT_SetBits(LR_PORT, LR_PIN)
#define LR_TRIG PORT_Toggle(LR_PORT, LR_PIN)

#define LG_PORT (PortB)
#define LG_PIN  (Pin10)
#define LG_ON   PORT_ResetBits(LG_PORT, LG_PIN)
#define LG_OFF  PORT_SetBits(LG_PORT, LG_PIN)
#define LG_TRIG PORT_Toggle(LG_PORT, LG_PIN)

#define ZPR_PORT   (PortA)
#define ZPR_PIN    (Pin06)
#define ZPR_ON     PORT_ResetBits(ZPR_PORT, ZPR_PIN)
#define ZPR_OFF    PORT_SetBits(ZPR_PORT, ZPR_PIN)
#define ZPR_TOGGLE PORT_Toggle(ZPR_PORT, ZPR_PIN)

#define ZPG_PORT   (PortA)
#define ZPG_PIN    (Pin07)
#define ZPG_ON     PORT_ResetBits(ZPG_PORT, ZPG_PIN)
#define ZPG_OFF    PORT_SetBits(ZPG_PORT, ZPG_PIN)
#define ZPG_TOGGLE PORT_Toggle(ZPG_PORT, ZPG_PIN)

// can编址
#define ACK_I_PORT (PortB)
#define ACK_I_PIN  (Pin08)
#define ACK_I_YES  ((PORT_GetBit(ACK_I_PORT, ACK_I_PIN) == 0) ? 1 : 0)

#define ACK_O_PORT  (PortA)
#define ACK_O_PIN   (Pin00)
#define ACK_O_ON()  PORT_SetBits(ACK_O_PORT, ACK_O_PIN)
#define ACK_O_OFF() PORT_ResetBits(ACK_O_PORT, ACK_O_PIN)

// 按键
#define SW_UP_PORT (PortA)
#define SW_UP_PIN  (Pin01)
#define SW_UP_YES  ((PORT_GetBit(SW_UP_PORT, SW_UP_PIN) == 0) ? 1 : 0)

#define SW_DOWN_PORT (PortA)
#define SW_DOWN_PIN  (Pin02)
#define SW_DOWN_YES  ((PORT_GetBit(SW_DOWN_PORT, SW_DOWN_PIN) == 0) ? 1 : 0)

extern void bsp_gpio_init(void);

#endif /* __BSP_GPIO_H__ */
