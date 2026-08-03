#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "board_config.h"
#include "bsp_adc.h"
#include "bsp_can.h"
#include "bsp_gpio.h"
#include "bsp_lock.h"
#include "bsp_qf.h"
#include "bsp_thread.h"
#include "cmd_def.h"
#include "rn_typedef.h"
#include "factory_test.h"
#include "motor_spdTab_calc.h"
#include "stepper_motor.h"
#include "bsp_thread.h"
extern void Bsp_Lock_SetDuty(uint32_t duty);
#define A0_DRV_PORT (PortD)
#define A0_DRV_PIN  (Pin02)
#define A0_DRV_ON   (PORT_SetBits(A0_DRV_PORT, A0_DRV_PIN))
#define A0_DRV_OFF  (PORT_ResetBits(A0_DRV_PORT, A0_DRV_PIN))

#define A1_DRV_PORT (PortB)
#define A1_DRV_PIN  (Pin03)
#define A1_DRV_ON   (PORT_SetBits(A1_DRV_PORT, A1_DRV_PIN))
#define A1_DRV_OFF  (PORT_ResetBits(A1_DRV_PORT, A1_DRV_PIN))

#define A2_DRV_PORT (PortB)
#define A2_DRV_PIN  (Pin04)
#define A2_DRV_ON   (PORT_SetBits(A2_DRV_PORT, A2_DRV_PIN))
#define A2_DRV_OFF  (PORT_ResetBits(A2_DRV_PORT, A2_DRV_PIN))

#define OEL_PORT (PortB)
#define OEL_PIN  (Pin06)
#define OEL_ON   (PORT_ResetBits(OEL_PORT, OEL_PIN))
#define OEL_OFF  (PORT_SetBits(OEL_PORT, OEL_PIN))

#define OEH_PORT (PortB)
#define OEH_PIN  (Pin05)
#define OEH_ON   (PORT_ResetBits(OEH_PORT, OEH_PIN))
#define OEH_OFF  (PORT_SetBits(OEH_PORT, OEH_PIN))

// 断检
#define TOP_DET_PORT (PortB)
#define TOP_DET_PIN  (Pin07)
#define TOP_DET_YES  ((PORT_GetBit(TOP_DET_PORT, TOP_DET_PIN) == 0) ? 1 : 0)

#define CN_DET_PORT (PortB)
#define CN_DET_PIN  (Pin09)
#define CN_DET_YES  ((PORT_GetBit(CN_DET_PORT, CN_DET_PIN) == 0) ? 1 : 0)

#define BOT_DET_PORT (PortH)
#define BOT_DET_PIN  (Pin02)
#define BOT_DET_YES  ((PORT_GetBit(BOT_DET_PORT, BOT_DET_PIN) == 1) ? 1 : 0)

#define BOT_DET_EXT_CH     ExtiCh02
#define BOT_DET_EIQ        INT_PORT_EIRQ2
#define BOT_DET_IRQHandler Int015_IRQn
#define BOT_DET_PRIORITY   (DDL_IRQ_PRIORITY_DEFAULT - 1)  // 高于系统中断15
#define SLAVE_NUM (0)  // 一块板只有一个MCU就是0

#define SINGLE_TYPE (0x00)
#define DIFF_TYPE   (0x01)

enum _cn1 {
    eLOUT_1 = 0,
    eLOUT_2,
    eLOUT_3,
    eLOUT_4,
    eLOUT_5,
    eLOUT_6,
    eLOUT_7,
    eLOUT_8,
    eLOUT_9,
    eLOUT_10,
    eLOUT_11,
    eLOUT_12,
    eLOUT_13,
    eLOUT_14,
    eLOUT_15,
    eLOUT_16,
    eLOUT_1_2_17,
    eLOUT_2_2_18,
    eLOUT_3_2_19,
    eLOUT_4_2_20,
    eLOUT_5_2_21,
    eLOUT_6_2_22,
    eLOUT_7_2_23,
    eLOUT_8_2_24,
};

enum _cn3 {
    eLIN_1 = 0,
    eLIN_2,
    eLIN_3,
    eLIN_4,
    eLIN_5,
	  eLIN_6,
	  eLIN_7,
	  eLIN_8, 
	  eLIN_9,
	  eLIN_10
};

enum _diffOut {
    eDIFF_OA1 = 0,
    eDIFF_OB1,
    eDIFF_OC1
};

static void _enter_test_mode(struct can_message *pFrame);
static void _exit_test_mode(struct can_message *pFrame);
static void _test_get_gpio_in(struct can_message *pFrame);
static void _test_get_gpio_out(struct can_message *pFrame);
static void _test_set_volt(struct can_message *pFrame);
static void _test_get_soft_ver(struct can_message *pFrame);
static void _test_get_adc(struct can_message *pFrame);
static void _test_motor_run(struct can_message *pFrame);

void (*Fun_Test_Op[])(struct can_message *pFrame)
    = {[eFACTORY_TEST_BEGIN]        = _enter_test_mode,
       [eFACTORY_TEST_END]          = _exit_test_mode,
       [eFACTORY_TEST_GET_INPUT]    = _test_get_gpio_in,
       [eFACTORY_TEST_SET_OUTPUT]   = _test_get_gpio_out,
       [eFACTORY_TEST_SET_VOLT]     = _test_set_volt,
       [eFACTORY_TEST_GET_SOFT_VER] = _test_get_soft_ver,
       [eFACTORY_TEST_GET_ADC_VAL]  = _test_get_adc,
       [eFACTORY_TEST_M_RUN_CW]     = _test_motor_run,
       [eFACTORY_TEST_M_RUN_CCW]    = _test_motor_run};

extern void Can_Buff_Post(uint32_t id, uint8_t *send_data, uint8_t len);

static int s_test_run = 0;

static void Test_Rsp_EnterTestMode(void)
{
    uint8_t buf[8] = {0};
    buf[0]         = eFACTORY_TEST_BEGIN;
    buf[1]         = SLAVE_NUM;
    buf[2]         = 0xFF;
    Can_Buff_Post(CANID_RSP_FACTORY_TEST, buf, 3);
}

static void _enter_test_mode(struct can_message *pFrame)
{
    if (pFrame->len == 4) {
        if ((pFrame->data[1] == SLAVE_NUM) && (pFrame->data[2] == 0xAB)
            && (pFrame->data[3] == 0xCD)) {
            s_test_run = 1;
            Test_Rsp_EnterTestMode();
            LR_OFF;
            LG_OFF;
        }
    }
}

static void _exit_test_mode(struct can_message *pFrame)
{
    if (pFrame->len == 2) {
        if (pFrame->data[1] == SLAVE_NUM) {
            s_test_run = 0;
            NVIC_SystemReset();
        }
    }
}

int Fun_Test_IsRun(void)
{
    return s_test_run;
}

static void _test_get_gpio_in(struct can_message *pFrame)
{
    uint8_t buf[8]     = {0};
    uint8_t input_num  = pFrame->data[2];
    uint8_t input_type = pFrame->data[3];

    buf[0] = eFACTORY_TEST_GET_INPUT;
    buf[1] = SLAVE_NUM;
    buf[2] = input_num;

    if (input_type == SINGLE_TYPE) {
        switch (input_num) {
            /* 面检测试 */
            case eLOUT_1:
                // A0_OFF,A1_OFF,A2_OFF,OEL_ON,OEH_OFF
						    buf[3] = SW_UP_YES;

                break;
            case eLOUT_2:
                // A0_ON,A1_OFF,A2_OFF,OEL_ON,OEH_OFF
						    buf[3] = SW_DOWN_YES;

                break;
            case eLOUT_3:
                // A0_OFF,A1_ON,A2_OFF,OEL_ON,OEH_OFF
						    buf[3] = Bsp_Thread_BotCnDetYes();

                break;
            case eLOUT_4:
                // A0_OFF,A1_OFF,A2_ON,OEL_ON,OEH_OFF
						    buf[3] = Bsp_Thread_BotDetYes();

                break;
            case eLOUT_5:
                // A0_OFF,A1_OFF,A2_OFF,OEL_OFF,OEH_ON
						    buf[3] = ACK_I_YES;

                break;
            case eLOUT_6:
						    Bsp_Thread_SelectNdl(0);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_7:
                /* 底检脉冲输入 */
                Bsp_Thread_SelectNdl(1);
                buf[3] = Bsp_Thread_TopDetYes();

                break;
            case eLOUT_8:
                /* 底检板接入输入 */
                Bsp_Thread_SelectNdl(2);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_9:
                Bsp_Thread_SelectNdl(3);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_10:
                Bsp_Thread_SelectNdl(4);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_11:
                Bsp_Thread_SelectNdl(5);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_12:
                Bsp_Thread_SelectNdl(6);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_13:
                Bsp_Thread_SelectNdl(7);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_14:
                Bsp_Thread_SelectNdl(8);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_15:
                Bsp_Thread_SelectNdl(9);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_16:
                Bsp_Thread_SelectNdl(10);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_1_2_17:
                Bsp_Thread_SelectNdl(11);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_2_2_18:
                Bsp_Thread_SelectNdl(12);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_3_2_19:
                Bsp_Thread_SelectNdl(13);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_4_2_20:
                Bsp_Thread_SelectNdl(14);
                buf[3] = Bsp_Thread_TopDetYes();
                break;
            case eLOUT_5_2_21:
                Bsp_Thread_SelectNdl(15);
                buf[3] = Bsp_Thread_TopDetYes();
                break;						
            default:
                break;
        }
        buf[4] = SINGLE_TYPE;
    } else if (input_type == DIFF_TYPE) {
        switch (input_num) {
            case eDIFF_OA1:
                buf[3] = Bsp_Qf_GetPortLevel();
                break;
            default:
                break;
        }
        buf[4] = DIFF_TYPE;
    }
    Can_Buff_Post(CANID_RSP_FACTORY_TEST, buf, 5);
}

static void _test_get_gpio_out(struct can_message *pFrame)
{
    uint8_t output_num   = pFrame->data[2];
    uint8_t output_level = pFrame->data[3];
    uint8_t output_type  = pFrame->data[4];

    if (output_type == SINGLE_TYPE) {
        switch (output_num) {
            case eLIN_1:
                if (output_level == 1) {
                    LR_ON;
                } else {
                    LR_OFF;
                }
                break;
            case eLIN_2:
                if (output_level == 1) {
                    LG_ON;
                } else {
                    LG_OFF;
                }
                break;
            case eLIN_3:
                if (output_level == 1) {
                    A0_DRV_ON;
                } else {
                    A0_DRV_OFF;
                }
                break;
            case eLIN_4:
                if (output_level == 1) {
                    A1_DRV_ON;
                } else {
                    A1_DRV_OFF;
                }
                break;
            case eLIN_5:
                if (output_level == 1) {
                    A2_DRV_ON;
                } else {
                    A2_DRV_OFF;
                }
                break;
						case eLIN_6:
                if (output_level == 1) {
                    OEL_ON;
                } else {
                    OEL_OFF;
                }
                break;
						case eLIN_7:
                if (output_level == 1) {
                    OEH_ON;
                } else {
                    OEH_OFF;
                }
                break;
						case eLIN_8:
                if (output_level == 1) {
                    ACK_O_ON();
                } else {
                    ACK_O_OFF();
                }
                break;
						case eLIN_9:
                if (output_level == 1) {
                    ZPG_ON;
                } else {
                    ZPG_OFF;
                }
                break;
						case eLIN_10:
                if (output_level == 1) {
                    ZPR_ON;
                } else {
                    ZPR_OFF;
                }
                break;
            default:
                break;
        }
    }
}

static void _test_get_adc(struct can_message *pFrame)
{
    uint8_t adc_volt_chenel = 0;
    uint8_t buf[8]          = {0};

    adc_volt_chenel = pFrame->data[2];
    buf[0]          = eFACTORY_TEST_GET_ADC_VAL;
    buf[1]          = SLAVE_NUM;
    buf[2]          = adc_volt_chenel;
    int16_t adc_volt
        = (Bsp_Adc_GetVar(DET_CHAN_5V) + Bsp_Adc_GetVar(DET_CHAN_24V))
        - (ADC_5V_VALUE + ADC_24V_VALUE);
    buf[3] = 0;
    if (abs(adc_volt) < 100) {
        buf[4] = 1;
    } else {
        buf[4] = 0xFF;  // 回复 0 给测试工装那边流程会出问题
    }
    Can_Buff_Post(CANID_RSP_FACTORY_TEST, buf, 5);
}
#define MAX_RAW_VALUE  240u   // 对应100%
#define PERCENT_BASE   100u   
uint8_t get_percent_int(uint16_t raw_value) 
{
    // 限幅
    if (raw_value > MAX_RAW_VALUE) 
		{
        raw_value = MAX_RAW_VALUE;
    }
    // (原始值 * 100 + 120) / 240
    return (uint8_t)(((uint32_t)raw_value * PERCENT_BASE + (MAX_RAW_VALUE / 2)) / MAX_RAW_VALUE);
}

static void _test_set_volt(struct can_message *pFrame)
{
	uint16_t data = 0;
	uint8_t percent = 0;
    switch (pFrame->data[2]) 
		{
        case 5:
            LR_ON;          
            break;
        case 6:
            LG_ON;
            break;
				case 0x5c:	          
	          data = ((uint16_t)pFrame->data[3]<<8) | pFrame->data[4];
            percent =	get_percent_int(data);		
				    Bsp_Lock_SetDir(1);
            Bsp_Lock_SetDuty(percent);
        		break;
				case 0x5d:
	          data = ((uint16_t)pFrame->data[3]<<8) | pFrame->data[4];
            percent =	get_percent_int(data);
            Bsp_Lock_SetDir(0);				
            Bsp_Lock_SetDuty(percent);
        		break;	
        default:
					  break;
    }
}

static void _open_motor_rsp_cw(Enum_MotorID id)
{
    uint8_t buf[8] = {0};

    buf[0] = eFACTORY_TEST_M_RUN_CW;
    buf[1] = SLAVE_NUM;
    buf[2] = id;
    Can_Buff_Post(CANID_RSP_FACTORY_TEST, buf, 3);
}

static void _open_motor_rsp_ccw(Enum_MotorID id)
{
    uint8_t buf[8] = {0};

    buf[0] = eFACTORY_TEST_M_RUN_CCW;
    buf[1] = SLAVE_NUM;
    buf[2] = id;
    Can_Buff_Post(CANID_RSP_FACTORY_TEST, buf, 3);
}

static void _test_motor_run(struct can_message *pFrame)
{
    int32_t pls = pFrame->data[4] + (pFrame->data[3] << 8);

    if (pFrame->data[0] == eFACTORY_TEST_M_RUN_CCW) {
        pls = -pls;
        Mod_Motor_SetFinishCallBack(_open_motor_rsp_ccw);
    } else {
        Mod_Motor_SetFinishCallBack(_open_motor_rsp_cw);
    }

    switch (pFrame->data[2]) {
        case 0:
            Mod_Motor_Act(A_MOTOR, 8000, pls*4);
            break;
        case 1:
            Mod_Motor_Act(B_MOTOR, 8000, pls*4);
            break;
        default:
            break;
    }
}

static void _test_get_soft_ver(struct can_message *pFrame)
{
    uint8_t buf[8] = {0};

    if (pFrame->len == 3) {
        if ((pFrame->data[1] == SLAVE_NUM)
            && (pFrame->data[2] == 0xFF))  // 版本查询
        {
            buf[0] = eFACTORY_TEST_GET_SOFT_VER;
            buf[1] = HW_ID >> 20;               // 类型
            buf[2] = (DATE_INFO >> 16) & 0xff;  // 年
            buf[3] = (DATE_INFO) >> 8 & 0xff;   // 月
            buf[4] = DATE_INFO & 0xff;          // 日
            Can_Buff_Post(CANID_RSP_FACTORY_TEST, buf, 5);
        }
    }
}
