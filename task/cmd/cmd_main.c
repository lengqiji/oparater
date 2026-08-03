#include "board_config.h"
#include "rn_typedef.h"
#include "alarm.h"
#include "bsp_can.h"
#include "cmd/cmd_addr.h"
#include "cmd/cmd_update.h"
#include "cmd_def.h"
#include "hc32_ddl.h"
#include "lock.h"
#include "para.h"
#include "qf.h"
#include "thread_check.h"
#include "bsp_qf.h"
#include "jump_motor.h"
#include "footLock_motor.h"
#include "rn_printf.h"
// 主控绣作



void cmd_main_emb(struct can_message *pFrame)
{
    uint8_t  scmd, len;

    //    if (cmd_scroll_id_judge(pFrame)==0)
    //        return;


    scmd = pFrame->data[1];
    len  = pFrame->len;
    if (len != 2)
    {
		 return;
    }
    switch (scmd) {
        case CMD_MAIN_NEW_NDL:  // 新一针
            if (len == 2) {
	            if(Para_head.jump_no_follow)
	            {
					Para_head.mainshaft_run = 0;
	            }
	            else
	            {
	               	 Para_head.mainshaft_run = 1;
	            }
                Fun_Thread_NewNdl();
                Fun_FootLock_NewNdlCnt();

                Mod_Qf_ErrJudge();
                Bsp_Qf_Clear();
            }

            break;

        case CMD_MAIN_FLAT:  // 平绣头
            if (Para_head.cur_type == HEAD_TYPE_FLAT) {
                break;
            }
            if (Para_head.type != HEAD_TYPE_FLAT) {
                Para_head.en       = 0;
                Para_head.type_err = 1;
            } else {
               // Para_head.en = Para_head.set_en;
                 Para_head.en = 0;
                Para_head.type_err = 0;
            }
            Para_head.cur_type = HEAD_TYPE_FLAT;
            break;

        case CMD_MAIN_TAPE:  // 盘带头
            if (Para_head.cur_type == HEAD_TYPE_TAPE) {
                break;
            }
            if (Para_head.type != HEAD_TYPE_TAPE) {
                Para_head.en       = 0;
                Para_head.type_err = 1;
            } else {
                Para_head.en = Para_head.set_en;
                // Para_head.en = 0;
                Para_head.type_err = 0;
            }
            Para_head.cur_type = HEAD_TYPE_TAPE;

            break;

        case CMD_MAIN_TOWEL:  // 毛巾绣头
            if (Para_head.cur_type == HEAD_TYPE_TOWEL) {
                break;
            }
            if (Para_head.type != HEAD_TYPE_TOWEL) {
                Para_head.en       = 0;
                Para_head.type_err = 1;
            } else {
                Para_head.en = Para_head.set_en;
                // Para_head.en = 0;
                Para_head.type_err = 0;
            }
            Para_head.cur_type = HEAD_TYPE_TOWEL;
            break;

        case CMD_MAIN_NORMAL:  // 正常绣
//            Para_emb.stat = STATUS_NORMAL;
//            if(Para_emb.jump==1)
//            {
//				Para_emb.jump = 0;
//				if (Para_head.fac_id != CLIENT_TYPE_YUEMEI)
//				Fun_JumpMotor_JumpSet(1);
//            }
            break;

        case CMD_MAIN_PATCH:  // 补绣
            Para_emb.stat = STATUS_PATCH;
            Para_emb.jump = 1;
            break;

        case CMD_MAIN_STOP:  // 主轴停止
            Fun_JumpMotor_JumpSet(0);
            if(Fun_Thread_GetBrk())//如果还在断线的话
            {
				Para_head.stat = STATUS_PATCH;
            }
//            Fun_JumpMotor_Small_Cur_Set(0);
            Para_emb.stat                    = STATUS_STOP;
            Para_head.mainshaft_run          = 0;
            
            Fun_Thread_MainStop();
			Fun_FootLock_ClrNdlCnt();
            break;

        case CMD_MAIN_START:  // 主轴启动
            Fun_Thread_Rst();
            Fun_JumpMotor_Small_Cur_Set(0);
            Para_head.iskey_patch   = 0;
            if(Para_head.jump_no_follow)
            {
				Para_head.mainshaft_run = 0;
            }
            else
            {
               	Para_head.mainshaft_run = 1;
            }
            Mod_Qf_ErrClear();
            Alarm_Clr(ALARM_BIT_NO_QF);
            Alarm_Clr(ALARM_BIT_QF_ERR);

            Fun_Lock_SetPatchNo(0);
            break;

        case CMD_MAIN_BRK_SELFCHECK_START:  // 断线部件自检开始,未下发
                                            // Para_emb.stat = STATUS_TSTBRK;
                                            // shell_printf("selfbrk open0\n");
            break;

        case CMD_MAIN_BRK_SELFCHECK_END:  // 断线部件自检结束,未下发
                                          // Para_emb.stat = STATUS_STOP;
                                          // shell_printf("selfbrk stop0\n");
            break;

        case CMD_MAIN_ZERO:  // 主轴在零位
            break;

        case CMD_MAIN_NO_ZERO:  // 主轴不在零位
            break;
        case CMD_MAIN_LOCK_NO:  // 不补修的机头锁住面线夹
            Fun_Lock_SetPatchNo(1);
            break;
        default:
            break;
    }
}



void Task_Cmd_Main(struct can_message *pFrame)
{
    uint8_t scmd;

    //cmd = pFrame->data[0];//0xe0
    scmd = pFrame->data[1];

	if (((scmd>=CMD_MAIN_UP_START_CODE)&&(scmd<=CMD_MAIN_UP_SEQ2))
		|| (scmd==CMD_MAIN_UP_START))
    {
        Task_Cmd_Update(pFrame);
    }
	else if (((scmd>=CMD_MAIN_ADDR_START)&&(scmd<=CMD_MAIN_ADDR_END))
		|| ((scmd>=CMD_MAIN_ADDR_LOOK_START)&&(scmd<=CMD_MAIN_ADDR_LOOK_END))
		|| (scmd==CMD_MAIN_ADDR_SET))
    {
        Task_Cmd_Addr(pFrame);
    }
	//else if (scmd == CMD_MAIN_RST_END)//机头复位结束
	//{
		//cmd_rst(pFrame);
	//}
	else
	{
		cmd_main_emb(pFrame);
	}
}
