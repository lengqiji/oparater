#include "rn_typedef.h"

#include "bsp_fault.h"
#include "bsp.h"
#include "bsp_adc.h"
#include "bsp_can.h"
#include "bsp_clock.h"
#include "bsp_gpio.h"
#include "bsp_lock.h"
#include "bsp_motor.h"
#include "bsp_qf.h"
#include "bsp_thread.h"
#include "hc32_ddl.h"
#include "bsp_uart.h"
#include "bsp_wdt.h"


void Bsp_Init(void)
{
    Bsp_Clock_Init();
    bsp_gpio_init();
    Bsp_Adc_Init();

    Bsp_Lock_Init();
    Bsp_Motor_Init();
    Bsp_Can_Init();
    Bsp_Qf_init();
    Bsp_Fault_Init();
    Bsp_Thread_Init();
    BSP_UartInit();
    wdt_init();
}
