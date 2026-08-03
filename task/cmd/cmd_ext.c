#include "rn_typedef.h"

#include "bsp_can.h"
#include "bsp_flash.h"
#include "cmd_def.h"
#include "hook_motor.h"
#include "footLock_motor.h"
#include "jump_motor.h"
#include "lock.h"
#include "para.h"
#include "thread_check.h"
enum _force_jump_type {
	eFORCE_JUMP_TYPE_FLAT = 0,
	eFORCE_JUMP_TYPE_TAPE1,
	eFORCE_JUMP_TYPE_TAPE2,
	eFORCE_JUMP_TYPE_MAX
};
void Task_Cmd_Ext_Param(struct can_message *pFrame)
{
    uint8_t cmd;
    uint8_t param;
	uint8_t param2;
    switch (pFrame->id) {
        case CANID_CMD_HEAD_PARAM_PWR:  // 设置电压报警比例
            break;
        case CANID_CMD_HEAD_PARAM_JUMP_TYPE:  // 设置跳跃类型
            break;
        case CANID_CMD_HEAD_PARAM_LOCK_PARA:  // 设置面线夹参数C12
            cmd   = pFrame->data[0];
            param = pFrame->data[1];
            switch (cmd) {
                case CMD_HEAD_PARAM_LOCK_TYPE:  // 面线夹类型
                    Fun_Lock_SetType(param);
                    break;

                case CMD_HEAD_PARAM_LOCK_DUTY:  // 非自保持面线夹占空比
                    Fun_Lock_SetPwmDuty(param);
                    break;
                case CMD_HEAD_PARAM_LOCK_DIR:  // 面线夹方向--03
                    Fun_Lock_SetDir(param);
                    break;
                default:
                    break;
            }
            break;
        case CANID_CMD_TAPE_MOTOR_LOGIC_DIR:
            break;
        case CANID_CMD_HEAD_PARAM_SMT_PARA:  // 设置电机参数C13
            cmd   = pFrame->data[0];
            param = pFrame->data[1];
            switch (cmd) {
                case CMD_HEAD_PARAM_MOTOR_CUR_JUMP:  // 跳跃电机电流
                    Fun_JumpMotor_SetWorkI(param);   // 200=2A
                    break;
                case CMD_HEAD_PARAM_MOTOR_CUR_HOOK:  // 勾线电机电流
                    if (Para_head.mach_type != MACH_TYPE_FOOT_COM_JD) {
                        Fun_HookMotor_SetWorkI(param);
                    }
                    break;
                case CMD_HEAD_PARAM_MOTOR_LOGIC_DIR:  // 电机逻辑方向

                    if (param == 1)
                        Fun_JumpMotor_SetDir(pFrame->data[2]);
                    else if (param == 2) {
                        if (Para_head.mach_type != MACH_TYPE_FOOT_COM_JD) {
                            Fun_HookMotor_SetDir(pFrame->data[2]);
                        }
                    }
                    break;
                case CMD_HEAD_PARAM_FOOT_COM_CFG:
                    Fun_FootLock_SetCfg(&pFrame->data[1]);
                    break;
                default:
                    break;
            }
            break;
        case CANID_CMD_HEAD_PARAM_COMBINE_PARA:  // 设置组合头参数C14
            cmd    = pFrame->data[0];
            param  = pFrame->data[1];
            param2 = pFrame->data[2];
            switch (cmd) {
                case C14_INDEP_CHECK:
                    Fun_Thread_SetIndepCheck(param);
                    break;
                case C14_NDL_NUM_REVERSE:
                    Fun_Thread_SetNdlNumReverse(param);
                    break;
                case C14_JUMP_MOTOR_DEFAULT_STEP:
                    Fun_JumpMotor_SetDefaultStep(param);
                    break;
                case C14_NDL_NUM_MODE:
                    Fun_Thread_SetNdlNum_Mode_Type(param, param2);
                    break;
				case C14_SPLICE_HEAD:
                    if (param >= 2)
					{
						Para_head.is_splice = 1;
						Para_head.splice_head_num = param;
						Fun_thread_splice_head_group_calc();
					}
					else
					{
						Para_head.is_splice = 0;
					}
                    break;

			 case C14_SPLICE_CH_NUM:
					Fun_thread_Splice_head_ndl_num(param);
                    break;   
                default:
                    break;
            }
            break;
		case CANID_CMD_FACTORY_ID:
		    switch (pFrame->data[0]) {
				case 0x01:
                    Bsp_Flash_SetFacId(*((uint32_t *) &pFrame->data[1]));
					break;
				case 0x02:
                    Bsp_Flash_SetMachType(pFrame->data[1]);   
                    Fun_JumpMotor_Force_JumpSet(0);
					break;
				case 0x04:
				//收到命令直接设置停机并跳跃
				   Fun_JumpMotor_Force_JumpSet(1);
//			       Fun_JumpMotor_Small_Cur_Set(1);
                   Para_head.small_cur_mode = 1;//参数晚发强制小电流 

				
//					/* 因为C29导致没动作，这个为了强制动作 */
//					{
//						uint32_t  head_type;
//						const int cvt_tab[]
//							= {[1 << eFORCE_JUMP_TYPE_FLAT]
//								= HEAD_TYPE_FLAT,
//								[1 << eFORCE_JUMP_TYPE_TAPE1]
//								= HEAD_TYPE_TAPE,
//								[1 << eFORCE_JUMP_TYPE_TAPE2]
//								= HEAD_TYPE_TAPE};
//						for (int type = 0;
//								type < eFORCE_JUMP_TYPE_MAX;
//								type++) {
//							int bit = 1 << type;
//							if (pFrame->data[2] & bit) {
//								head_type = cvt_tab[bit];
//							}
//							if (head_type == Para_head.type) {
//								if (pFrame->data[1] == 0x01) {
//									Fun_JumpMotor_forceJumpSet(1);
//								} else if (pFrame->data[1] == 0x02) {
//									Fun_JumpMotor_forceJumpSet(1);
//								}
////								head_param.jump_param.force_jump = 1;
//								break;
//							}
//						}
//					}
					break;
				default:
					break;
			}
			break;
        default:
            break;
    }
}

void Task_Cmd_Ext_HookSet(struct can_message *pFrame)
{
    int en    = pFrame->data[0] & 0x01;
    int spd   = (pFrame->data[2] << 8) + pFrame->data[1];
    int pulse = (pFrame->data[6] << 24) + (pFrame->data[5] << 16)
              + (pFrame->data[4] << 8) + pFrame->data[3];
    Fun_HookMotor_ActFlag(en, spd, pulse);
}
void Task_Cmd_Ext_HeadEn(struct can_message *pFrame)
{
    uint8_t head_byte = 0;
    uint8_t head_bit  = 0;
    uint8_t data      = 0;
    uint8_t group     = ((pFrame->id) & 0xF) + 1;

    // 判断机头地址是否在当前组
    if ((Para_head.addr > ((group - 1) * 64))
        && (Para_head.addr <= (group * 64))) {
        head_byte = ((Para_head.addr - 1) % 64) / 8;
        head_bit  = (Para_head.addr - 1) % 8;
        data      = pFrame->data[head_byte];

        if ((data >> head_bit) & 0x1)  // 打开机头
        {
            Para_head.en     = 1;
            Para_head.set_en = 1;
        } else  // 关闭机头
        {
            Para_head.en     = 0;
            Para_head.set_en = 0;
        }
    }
}
void Task_Cmd_Ext_ManualTrim(struct can_message *pFrame)
{
    if (pFrame->data[0] == 0x01) {
        Para_head.manual_trim = 1;
         Fun_JumpMotor_Small_Cur_Set(1);
    } else if (pFrame->data[0] == 0x02) {
        Para_head.manual_trim = 0;
        Fun_JumpMotor_Small_Cur_Set(0);
    }
}

void Task_Cmd_Ext_FootTestMode(struct can_message *pFrame)
{
    if (pFrame->data[0] == 0x01) {
        Para_head.pressFoot_test = 1;
    } else if (pFrame->data[0] == 0x02) {
        Para_head.pressFoot_test = 0;
    }
}

void Task_Cmd_Ext_SetLock(struct can_message *pFrame)
{
    if (pFrame->data[0] == FOOT_MAG_ON) {
        Fun_FootLock_SetLock(true);
    } else if (pFrame->data[0] == FOOT_MAG_OFF) {
        Fun_FootLock_SetLock(false);
    }
}
/*****************************************************************************
 函 数 名  : cmd_head_type_ext
 功能描述  : 机头扩展帧命令
 输入参数  : 无

0:平绣 1:大毛巾绣 2:激光绣 3:烫砖绣 4:热切割 5:雕孔 6:盘带1 7:盘带2 8:植绒绣
*****************************************************************************/
void Task_Cmd_Ext_Head_Type(struct can_message *pFrame)
{
    uint8_t scmd, len;
    scmd = pFrame->data[1];
    len = pFrame->len;
    if ((len == 2) && (pFrame->data[0] == 0xE0))
    {
        switch (scmd)
        {
            case HDTYPE_PHOLE:
						case HDTYPE_INDHOLE:
            case HDTYPE_FLAT:
            case    HDTYPE_HOTCUT:
                Para_head.cur_type = HEAD_TYPE_FLAT;
                break;
            case HDTYPE_TAPE1:
            case HDTYPE_TAPE2:
                Para_head.cur_type = HEAD_TYPE_TAPE;
                break;
            case HDTYPE_TOWEL:
                Para_head.cur_type = HEAD_TYPE_TOWEL;
                break;
            case HDTYPE_TUFTING:
                Para_head.cur_type = HEAD_TYPE_TUFTING;
                break;
						case HDTYPE_LASER:
							Para_head.cur_type = HEAD_TYPE_LASER;
							break;
            default:
                Para_head.cur_type = HEAD_TYPE_NONE;
                break;
        }
        if (Para_head.type != Para_head.cur_type)
        {
            Para_head.en = 0;
            Para_head.type_err = 1;
        }
        else
        {
            Para_head.en = Para_head.set_en; 
            Para_head.type_err = 0;      
        }
     }
}

void Task_Cmd_Ext_HeadStatus(struct can_message *pFrame)
{
    uint8_t cmd  = 0;
    uint8_t addr = ((pFrame->id) & (0xFF));
    if(Para_head.cur_type != HEAD_TYPE_FLAT)//防止其他类型的断线报警导致的，机头开下针
    {
     return;
    }
    if ((pFrame->len == 2) && (addr == Para_head.addr)) {
        cmd = pFrame->data[1];

        switch (cmd) {
            case STATUS_NORMAL:
                Para_head.stat = STATUS_NORMAL;
                Fun_thread_ExtInfoBrk(0);
                break;
            case STATUS_PATCH:              
                Para_head.stat = STATUS_PATCH;
                break;
            default:
                break;
        }
    }
}
