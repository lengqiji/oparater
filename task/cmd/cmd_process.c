#include "hc32_ddl.h"
#include "rn_typedef.h"
#include "bsp_can.h"
#include "cmd/cmd_process.h"
#include "cmd_def.h"
#include "cmd/cmd_main.h"
#include "cmd/cmd_params.h"
#include "cmd/cmd_work.h"
#include "cmd/cmd_rst.h"
#include "cmd/cmd_ext.h"
#include "cmd.h"
#include "para.h"
#include "cmd/cmd_factoryTest.h"
#include "thread_check.h"
#include "key.h"

// 通信协议处理
void cmd_process(struct can_message *pFrame)
{
    uint8_t cmd;


    cmd = pFrame->data[0];
	


    switch (cmd) {
        case CMD_MAIN:  // 主控绣作命令
            Task_Cmd_Main(pFrame);
            break;

        case CMD_HEAD_PARAM:         // 机头参数
        case CMD_HEAD_MOTOR_ADJUST:  // 电机参数
        case CMD_NDL_NUM:            // 机型针数
        case CMD_NDL_CUR:            // 当前针数
        case CMD_HEAD_NDL_LEN:       // 当前针长
            Task_Cmd_Params(pFrame);
            break;

        case CMD_HEAD:     // 机头刺绣动作命令
        case CMD_HEAD_EN:  // 机头工作使能命令
            Task_Cmd_Work(pFrame);
            break;

        //case CMD_RST_STRRT://机头复位开始
        //case CMD_RST_END2://机头复位结束
        case CMD_RST_END3:  // 机头复位结束
            Task_Cmd_Rst(pFrame);
            break;

        default:
        
            break;
    }
}

// 扩展帧 通信协议处理
void cmd_process_ext(struct can_message *pFrame)
{
    Task_Cmd_Ext_Param(pFrame);

    if ((pFrame->id >= CANID_CMD_HEAD_AUTO_ADDR_START) && (pFrame->id <= CANID_CMD_HEAD_AUTO_ADDR_RSP))
    {
        //Task_Cmd_Addr_Ext(pFrame);
    }
    else if (pFrame->id == CANID_CMD_HEAD_CLEAR_ALARM)
    {

    }
    else if (pFrame->id == CANID_CMD_HEAD_TYPE)
    {
      
		Task_Cmd_Ext_Head_Type(pFrame);
    }
    else if (pFrame->id == CAN_ID_CMD_HEAD_PARAM_FOOT_CTRL)
    {
		Task_Cmd_Ext_SetLock(pFrame);
    }
    else if ((pFrame->id >= CANID_CMD_HEAD_SEND_ALARM) && (pFrame->id <= (CANID_CMD_HEAD_SEND_ALARM+0xff)))
    {
        switch (pFrame->data[0]) {
            case eSTATUS_HEAD:
                Task_Cmd_Ext_HeadStatus(pFrame);
                break;
            default:
                break;
        }
         if ((Para_head.is_splice) && (pFrame->data[0] == 0x07))
        {
            if (Fun_thread_Is_splice_samegroup(pFrame->id&0xFF))
            {
                Fun_Key_SetState(pFrame->data[1]);
            }
        }
    }
    else if (pFrame->id == CANID_CMD_HEAD_HOOK_SET)
    {
        Task_Cmd_Ext_HookSet(pFrame);
    }
    else if (pFrame->id == CAN_CMD_HEAD_READ_VERSION)
    {
        Mod_Cmd_RspVerExt(pFrame);
    }
    else if (pFrame->id == CANID_CMD_HEAD_HEART)
    {
        Mod_Cmd_RspHeart(pFrame);
    }
    else if ((pFrame->id >= CAN_CMD_HEAD_EN_G1) && (pFrame->id <= CAN_CMD_HEAD_EN_G5))
    {
        Task_Cmd_Ext_HeadEn(pFrame);
    }
    else if (pFrame->id == CANID_CMD_HEAD_MANUAL_TRIM)
    {
        Task_Cmd_Ext_ManualTrim(pFrame);
    }
	else if(pFrame->id == CAN_CMD_MAINSHAFT_SPEED)
	{//主轴速度
	    Para_emb.mainshaft_speed = pFrame->data[1] | ((uint16_t)pFrame->data[2] <<8);
	}
    else if(pFrame->id == CANID_FACTORY_TEST)
    { 	
        Task_Cmd_FactoryTest(pFrame);      
    }



}

void Task_Cmd_ProcessCallback(struct can_message *pFrame)
{
    if (pFrame->fmt == CAN_FMT_STDID) {
        cmd_process(pFrame);
    } else if (pFrame->fmt == CAN_FMT_EXTID) {
        cmd_process_ext(pFrame);
    }
}
