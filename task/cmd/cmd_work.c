#include "rn_typedef.h"
#include "cmd_def.h"
#include "cmd_work.h"
#include "alarm.h"
#include "ucos_ii.h"
#include "para.h"
#include "lock.h"
#include "thread_check.h"
#include "jump_motor.h"

void Task_Cmd_Work(struct can_message *pFrame)
{
    uint8_t cmd, scmd, len;
    uint8_t head = 0;

    cmd = pFrame->data[0];
    scmd = pFrame->data[1];
    len = pFrame->len;
    if (len != 2)
    {
		 return;
    }
    if (cmd == CMD_HEAD_EN)//机头使能
    {
        if (len == 2)
        {
            if (Para_head.type_err)
            {
                Para_head.en = 0;
            } 
            else
            {
                if (Para_head.is_splice)
                {
                    head = Para_head.addr / Para_head.splice_head_num;
                    if (Para_head.addr % Para_head.splice_head_num)
                    {
                        head++;
                    }
                } else
                {
                    head = Para_head.addr;
                }
                //if (Para_head.addr == (scmd & 0x7F))
                if (head == (scmd & 0x7F))
                {
                    if (scmd & 0x80)
                    {
                        Para_head.en = 0;
                        Para_head.set_en = 0;
                    }
                    else
                    {
                        Para_head.en = 1;
                        Para_head.set_en = 1;
                    }
                }
            }
        }
        return;
    }

    //cmd=0xfc
    switch (scmd) //cmd=0xfc
    {
        case CMD_HEAD_JUMP_LOCK://打开跳跃电磁铁
            Fun_JumpMotor_Small_Cur_Set(1);
            Fun_JumpMotor_JumpSet(1);
            break;
        case CMD_HEAD_JUMP_UNLOCK://关闭跳跃电磁铁
            Fun_JumpMotor_Small_Cur_Set(0);
            Fun_JumpMotor_JumpSet(0);            
            Para_emb.stat = STATUS_STOP;
            break;
        case CMD_HEAD_THREAD_LOCK://锁面线fc 04
            Fun_Lock_Hold();
            break;
        case CMD_HEAD_THREAD_UNLOCK://松面线FC 03
            Fun_Lock_Release();
            break;
        case CMD_HEAD_BRK_CHECK_START://断线检测打开
            Fun_Thread_SetEn(1);
            break;
        case CMD_HEAD_BRK_CHECK_END://断线检测关闭
            Fun_Thread_SetEn(0);
            break;
        case CMD_HEAD_BRK_SELFCHECK_START://断线自检开始
            Fun_Thread_SetSelfCheck(1);
            Para_emb.stat = STATUS_TSTBRK;
            break;
        case CMD_HEAD_BRK_SELFCHECK_END://断线自检结束
            Fun_Thread_SetSelfCheck(0);
            Para_head.stat = STATUS_NORMAL;
            Para_emb.stat = STATUS_STOP;
            break;
        case CMD_HEAD_NORMAL_FLAT://正常绣平针
            Fun_JumpMotor_JumpSet(0);
            Fun_JumpMotor_JumpAngleStart();
            Para_emb.stat = STATUS_NORMAL;
            break;
        case CMD_HEAD_NORMAL_JUMP://正常绣跳针
            Fun_JumpMotor_JumpSet(1);
            Fun_JumpMotor_JumpAngleStart();
            Para_emb.stat = STATUS_NORMAL;
            break;
        case CMD_HEAD_PATCH_FLAT://补绣平针
            Fun_JumpMotor_JumpSet(0);
            Fun_JumpMotor_JumpAngleStart();
            Para_emb.stat = STATUS_PATCH;
            break;
        case CMD_HEAD_PATCH_JUMP://补绣跳针
            Fun_JumpMotor_JumpSet(1);
            Fun_JumpMotor_JumpAngleStart();
            Para_emb.stat = STATUS_PATCH;
            break;
        case CMD_HEAD_JUMP://停车锁针
            Fun_JumpMotor_JumpSet(1);
                        Fun_JumpMotor_JumpAngleStart();

            break;
        case CMD_HEAD_BRK_CHECK://断线检测开始
            Fun_Thread_CheckStart();
            break;
        case CMD_HEAD_NDL_DOWN://针停下位
            Fun_JumpMotor_SetNdlDownFlag();
            break;
        default:
            break;
    }
}

void Task_Cmd_Work_Ext(struct can_message *pFrame)
{
    uint8_t scmd;
 
    scmd = pFrame->data[1];

    switch (scmd) {
        case CMD_HEAD_CLEAR_ALARM:  // 清除报警
            break;

        default:
            break;
    }
}
