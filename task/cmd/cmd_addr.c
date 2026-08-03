#include "rn_typedef.h"
#include "bsp_can.h"
#include "cmd_addr.h"
#include "bsp_gpio.h"
#include "cmd_def.h"

void Task_Cmd_Addr(struct can_message *pFrame)
{
    uint8_t cmd, scmd, len;


    cmd  = pFrame->data[0];  // 0xe0
    scmd = pFrame->data[1];
    len  = pFrame->len;

    switch (scmd) {
        case CMD_MAIN_ADDR_START:  // 机头编址开始
            if (len == 2)//这里限制下，防止升级包数据进来编址，
            {//-----------CMD_MAIN_ADDR_SET刚好标准帧且8位
                Para_head.en               = 1;
                Para_head.stat             = STATUS_NORMAL;
                Para_head.addr_no_set_flag = 1;
                Para_head.addr_ing         = 1;
                //Para_head.set_addr = 0;
                //shell_printf("addr start, set_en=%d\n", Para_head.set_en);
            }
            break;

        case CMD_MAIN_ADDR_END:  // 机头编址结束
           if (len == 2)//这里限制下，防止升级包数据造成误触发
           { 
	            Para_emb.stat              = STATUS_STOP;
	            Para_head.stat             = STATUS_NORMAL;
	            Para_head.en               = Para_head.set_en;
	            Para_head.addr_ok_flag     = 0;
	            Para_head.set_addr         = 0;
	            Para_head.addr_no_set_flag = 0;
	            Para_head.addr_ing         = 0;
	            //shell_printf("addr end, set_en=%d\n", Para_head.set_en);
            }
            break;

        case CMD_MAIN_ADDR_LOOK_START:  // 机头寻址开始
           if (len == 2)//这里限制下，防止升级包数据造成误触发
           {
	            Para_head.en   = 0;
	            Para_head.stat = STATUS_NORMAL;
	            //shell_printf("addr look start\n");
           }
            break;

        case CMD_MAIN_ADDR_LOOK_END:  // 机头寻址结束
          if (len == 2)//这里限制下，防止升级包数据造成误触发
           {
	            Para_emb.stat          = STATUS_STOP;
	            Para_head.stat         = STATUS_NORMAL;
	            Para_head.en           = Para_head.set_en;
	            Para_head.addr_ok_flag = 0;
	            //shell_printf("addr look end\n");
            }
            break;

        case CMD_MAIN_ADDR_SET:
            if ((len == 8) /*&& (pFrame->data[5] == 0)*/ &&
                (pFrame->data[6] == 0) && (pFrame->data[7] == 0)) 
            {
                // pFrame->data[3] =1平绣头     =2盘带头
                Para_head.addr_type = pFrame->data[3];
                if (pFrame->data[2] == 1)  // 机头编址
                {
                    Para_head.set_addr = pFrame->data[4];
                    //shell_printf("addr set id=%d\r\n", Para_head.set_addr);
                } 
                else if (pFrame->data[2] == 2)  // 机头寻址
                {
                    if ((Para_head.addr == (pFrame->data[4] | (pFrame->data[5]<<8))) && (Para_head.type == Para_head.addr_type))
                    {
                        Para_head.en = 1;
                        Para_head.stat = STATUS_NORMAL;
                        Para_head.addr_cnt = 500;  // 闪1s
                        //shell_printf("addr look\n");
                    }
                }
            }
            break;

        default:
            break;
    }
}

void Task_Cmd_Addr_Ext(struct can_message *pFrame)
{
    uint8_t cmd, scmd;

    // 机头应答部分
    if (Para_head.addr_auto_flag == 1) {
        if ((pFrame->id == 0xc02) && (pFrame->len == 8) &&
            (pFrame->data[0] == 0x15) && (pFrame->data[1] == 0x01)) {
            Para_head.set_addr = pFrame->data[3] + 1;
            //shell_printf("*ack addr auto set id=%d\r\n", Para_head.set_addr);
            return;
        }
    }

    // 操作盒下发部分
    cmd  = pFrame->data[0];
    scmd = pFrame->data[1];

    switch (scmd) {
        case CMD_MAIN_ADDR_START:  // 机头编址开始
            if ((pFrame->id == 0xc00) && (pFrame->len == 2) && (cmd == 0xe0)) {
                Para_head.en = 1;
                //        Para_head.set_addr = 0;
                Para_head.addr_auto_flag   = 1;
                Para_head.addr_no_set_flag = 1;
                Para_head.addr_ing         = 1;
                Para_head.stat             = STATUS_NORMAL;
                ACK_O_OFF();
                //shell_printf("addr auto start\n");
            }
            break;

        case CMD_MAIN_ADDR_END:  // 机头编址结束
            if ((pFrame->id == 0xc01) && (pFrame->len == 2) && (cmd == 0xe0)) {
                Para_emb.stat              = STATUS_STOP;
                Para_head.stat             = STATUS_NORMAL;
                Para_head.en               = Para_head.set_en;
                Para_head.addr_ok_flag     = 0;
                Para_head.addr_auto_flag   = 0;
                Para_head.addr_no_set_flag = 0;
                Para_head.set_addr         = 0;
                Para_head.addr_ing         = 0;
                ACK_O_OFF();
                //1闭两开暂不拼接 splice_head_group_calc();
                //shell_printf("addr auto end\n");
            }
            break;

        case CMD_MAIN_ADDR_SET:
            if ((pFrame->id == 0xc02) && (pFrame->len == 5) && (cmd == 0xe0)) {
                //pFrame->data[3] =1平绣头     =2盘带头
                Para_head.addr_type = pFrame->data[3] - 1;
                //if (Para_head.type == Para_head.addr_type)//类型相同
                {
                    if (pFrame->data[2] == 1)  // 机头编址
                    {
                        Para_head.set_addr = pFrame->data[4];
                        //shell_printf("addr auto set id=%d\r\n",Para_head.set_addr);
                    }
                }
            }
            break;

        default:
            break;
    }
}
