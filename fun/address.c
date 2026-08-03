#include "rn_typedef.h"
#include "para.h"
#include "led.h"
#include "cmd.h"
#include "bsp_gpio.h"
#include "bsp_flash.h"

#define SET_ADDR_TZX_DELAY  (100)
#define SW_ADDR_ACK_O_DELAY (SET_ADDR_TZX_DELAY + 20)


void led_addr_to_tzx(void)
{
    static uint8_t flag = 0;//给特种绣输出灯标志
    static uint8_t n = 0;
    
	//if ((Para_head.addr_ok_flag) && (Para_head.set_addr == 0))//同一机头多次编址也都要输出
    if (Para_head.addr_ok_flag)//同一机头多次编址只输出一次
    {
        n++;
        if (flag == 0)
        {
            n = 0;
            flag = 1;
            Mod_Led_Out(LED_RED);
            //shell_printf("led addr red\r\n");
        }
        else if (flag == 1)
        {
            //if (n >= 50)
            if (n >= SET_ADDR_TZX_DELAY)
            {
                flag = 2;
                Mod_Led_Out(LED_GRN);
                if (Para_head.addr_auto_flag)
                {
                    Mod_Cmd_RspAutoSetAddr();
                }
                else
                {
                    Mod_Cmd_RspSetAddr();
                }
                //shell_printf("led addr grn\r\n");
            }
        }
    }
    else
    {
        flag = 0;
    }
}


// 状态
typedef enum {
    SW_STATUS_OFF,     // 无
    SW_STATUS_ON,      // 打开
    SW_STATUS_CHANGE,  // 改变
} SW_STATUS;



SW_STATUS ack_i_get_state(void)
{
    static uint8_t cnt         = 0;
    static uint8_t pin_pre_sta = 0;
    uint8_t        pin_sta     = 0;
    uint8_t        sta         = 0;
    cnt++;

    pin_sta = ACK_I_YES;
    if (pin_pre_sta != pin_sta) {
        pin_pre_sta = pin_sta;
        cnt         = 0;
        sta         = SW_STATUS_CHANGE;
        // shell_printf("ack i=%d\n", pin_sta);
    } else {
        if (cnt >= 10) {
            sta = pin_sta;
        }
    }

    return (SW_STATUS)(sta);
}

void flash_set_addr(uint32_t id)
{

    //shell_printf("set addr=%d ok\r\n", id);

    if (id == Para_head.addr)
    {
        //return;
    }

    Para_head.addr = id;
    flash_board_param.addr = id;
    flash_board_param.type = (uint8_t)Para_head.type; 
    flash_board_param.type <<= 8;
    flash_board_param.type += (uint8_t)Para_head.jump_type; 
    flash_set_board_param((uint8_t*)&flash_board_param);
}



void Fun_Addr_Task(int key_state)
{
    uint8_t ack = 0;
    static uint8_t flag = 0;
    static uint8_t n = 0;//计时
    led_addr_to_tzx();
    
    if (Para_head.addr_auto_flag == 0)
    {
        return;
    }

    ack = key_state;
    if (ack == SW_STATUS_ON)
    {
        if (Para_head.type == Para_head.addr_type)//类型相同
        {
            if ((Para_head.set_addr != 0) && (Para_head.addr_no_set_flag))
            {
                Para_head.addr_no_set_flag = 0;
                //shell_printf("set addr enter\r\n");
                flash_set_addr(Para_head.set_addr);
                //shell_printf("set addr level\r\n");
                Para_emb.stat = STATUS_STOP;
                Para_head.set_addr = 0;
                Para_head.addr_ok_flag = 1;
				//cmd_rsp_auto_set_addr();
                flag = 1;
                n = 0;
            } 
        }
        else
        {
            if (Para_head.addr_no_set_flag)
            {
                Para_head.addr_no_set_flag = 0;
                Para_emb.stat = STATUS_STOP;
                Para_head.set_addr = 0;
                flag = 1;
                n = 0;
            }
        }
    } 

    //等待发送完成后，触发下一机头编址
    if (flag)
    {
        //if (n++ >= 80)
        if (n++ >= SW_ADDR_ACK_O_DELAY)
        {
            flag = 0;
            ACK_O_ON();
            //shell_printf("trig next auto addr\n");
        }
    }
    
}

void Fun_Addr_SetAddrDeal()
{
    if(Para_head.set_addr != 0 && Para_head.addr_no_set_flag)
    {//设置地址
        Para_head.addr_no_set_flag = 0;
        Para_head.type = Para_head.addr_type;
        __disable_irq();
        flash_set_addr(Para_head.set_addr);
        __enable_irq();
        Para_emb.stat = STATUS_STOP;
        Para_head.set_addr = 0;
        Para_head.addr_ok_flag = 1;
    }
}






