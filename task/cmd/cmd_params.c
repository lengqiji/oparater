#include "cmd_params.h"
#include "cmd_def.h"
#include "jump_motor.h"
#include "lock.h"
#include "thread_check.h"
#include "para.h"

void Task_Cmd_Params(struct can_message *pFrame)
{
    uint8_t        cmd, scmd;
    uint8_t        param;

    cmd   = pFrame->data[0];
    scmd  = pFrame->data[1];
    param = scmd & CMD_HEAD_PARAM__MASK;  // 参数
    scmd &= (~CMD_HEAD_PARAM__MASK);

    if ((pFrame->id >= 0x401) && (pFrame->id <= 0x4FF) && (pFrame->len == 2)
        && (pFrame->data[0] == 0x1C)) {
        if ((Fun_Thread_GetIndepChk() && (pFrame->data[1] == Para_head.addr))||
           (Para_head.is_splice && (Fun_Thread_Splice_no_judge_flag()==1)&&(Fun_thread_Is_splice_samegroup(pFrame->data[1])==1))){
            Para_head.stat = STATUS_PATCH;
            Fun_thread_ExtInfoBrk(1);
        }
        return;
    }
    switch (cmd)
    {
        case CMD_NDL_NUM:  // 机型针数
            Fun_Thread_SetNdlNum(pFrame->data[1]);
            break;

        case CMD_NDL_CUR:  // 当前针号
            Fun_Thread_SetCurNdlNum(pFrame->data[1]);
            break;

        case CMD_HEAD_NDL_LEN:  // 当前针长
            Fun_Thread_NewNdlLen(pFrame->data[1]);
            break;
        case CMD_HEAD_MOTOR_ADJUST:  // 步进角度调节
            if(pFrame->len == 2)
            {
            	Fun_JumpMotor_StepsAdj((int8_t)pFrame->data[1]);
            }
            break;
        case CMD_HEAD_PARAM:  // 机头参数
            switch (scmd)
            {
                case CMD_HEAD_PARAM_MAG_POWER:  // 跳跃电磁铁电压调节
                    //
                    break;
                case CMD_HEAD_PARAM_START_FILTER_NDL:  // 起绣不检测断线针数
                    Fun_Thread_SetStartFilter(param);
                    break;
                case CMD_HEAD_PARAM_BRK_CHECK_TYPE:  // 断检装置类型
                    Fun_Thread_SetType(param);
                    break;
                case CMD_HEAD_PARAM_TOP_SENSE:  // 面线检测灵敏度
                    Fun_Thread_SetTopSense(param);
                    break;

                case CMD_HEAD_PARAM_BOT_SENSE:  // 底线检测灵敏度
                    Fun_Thread_SetBotSense(param);
                    break;

                case CMD_HEAD_PARAM_TOP_FILTER_NDL:  // 面线断线去抖动针数
                    Fun_Thread_SetTopFilter(param);
                    break;

                case CMD_HEAD_PARAM_BOT_FILTER_NDL:  // 底线断线去抖动针数
                    Fun_Thread_SetBotFilter(param);
                    break;

                case CMD_HEAD_PARAM_LOCK_POWER:  // 面线夹持电磁铁电压
                    Fun_Lock_SetLockPower(param);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}
