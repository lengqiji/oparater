#include "bsp_gpio.h"
#include "board_config.h"
#include "hc32_ddl.h"

void bsp_gpio_init(void)
{
    // 调试口要关闭才能使用

    PORT_DebugPortSetting(TDI, Disable);
    PORT_DebugPortSetting(TRST, Disable);
    PORT_DebugPortSetting(TDO_SWO, Disable);

    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    PORT_Init(MCU_LED_PORT, MCU_LED_PIN, &stcPortInit);

    PORT_Init(ZPG_PORT, ZPG_PIN, &stcPortInit);
    PORT_Init(ZPR_PORT, ZPR_PIN, &stcPortInit);
    PORT_Init(LR_PORT, LR_PIN, &stcPortInit);
    PORT_Init(LG_PORT, LG_PIN, &stcPortInit);
    PORT_Init(ACK_O_PORT, ACK_O_PIN, &stcPortInit);

    ZPG_OFF;
    ZPR_OFF;

    stcPortInit.enPinMode = Pin_Mode_In;
    PORT_Init(ACK_I_PORT, ACK_I_PIN, &stcPortInit);
    PORT_Init(SW_UP_PORT, SW_UP_PIN, &stcPortInit);
    PORT_Init(SW_DOWN_PORT, SW_DOWN_PIN, &stcPortInit);

    PORT_AlwaysOn(PortA, Enable);
    PORT_AlwaysOn(PortB, Enable);
    PORT_AlwaysOn(PortC, Enable);
    PORT_AlwaysOn(PortH, Enable);
}
