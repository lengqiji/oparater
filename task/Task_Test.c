/**
 * @copyright Raynen 2023
 *
 * @file Task_Test.c
 * @date 2023-10-18
 * @author lincq <lincq@raynen.cn>
 * @brief   测试线程
 * @details 用于测试各个功能模块
 *
 * Change Logs:
 * Date           Author         Notes
 * 2023-10-18     lincq          创建文件
 */
#include "rn_typedef.h"
#include <stdbool.h>
#include <stdint.h>

#include "board_config.h"
#include "Task_Test.h"
#include "bsp_adc.h"
#include "bsp_can.h"
#include "bsp_fault.h"
#include "bsp_gpio.h"
#include "bsp_lock.h"
#include "bsp_motor.h"
#include "bsp_qf.h"
#include "bsp_thread.h"
#include "bsp_wdt.h"
#include "cmd.h"
#include "cmd/cmd_process.h"
#include "lock.h"
#include "motor_spdTab_calc.h"
#include "para.h"
#include "qf.h"
#include "rn_printf.h"
#include "stepper_motor.h"
#include "ucos_ii.h"

#include "rn_test_byModbus.h"


#define SW_LED_TEST 1
#define LOCK_TEST   1
#define CAN_TEST    1
#if (CAN_TEST == 1)
    #define CAN_SEND_TEST 1
    #define CAN_RECV_TEST 1
#endif
#define ADC_TEST   1
#define MOTOR_TEST 1
#if (MOTOR_TEST == 1)
    #define JUMP_MOTOR_TEST 1
    #define HOOK_MOTOR_TEST 1
#endif
#define QF_TEST           1
#define THREAD_CHECK_TEST 1

#if (CAN_SEND_TEST == 1)
static inline void TestCAN_Send(void)
{
    uint8_t _can_data[8];
    _can_data[0] = 0xff;
    _can_data[1] = 0xaa;
    Bsp_Can_Send(0x10, &_can_data[0], 2);
}
#endif
#if (CAN_RECV_TEST == 1)
void RecvCAN_TestCallback(struct can_message *pFrame)
{
    static uint16_t _usRecvCnt = 0;
    _usRecvCnt++;
    Test_ByModbus_Update(&_usRecvCnt, eIDX_CAN, 1);
    Bsp_Can_Send(pFrame->id, pFrame->data, pFrame->len);
}
#endif
#if (LOCK_TEST == 1)
static void _vLockTest(void);
#endif
#if (MOTOR_TEST == 1)
static void _vMotorTest(void);
#endif
#if (VALVE_TEST == 1)
static void _vValveTest(void);
#endif
#if (ADC_TEST == 1)
static inline void ADC_DetPrint(void)
{
    UINT16 _ulTestBuf[3] = {0};
    _ulTestBuf[0]        = Bsp_Adc_GetVar(DET_CHAN_5V);
    _ulTestBuf[1]        = 0;
    _ulTestBuf[2]        = Bsp_Adc_GetVar(DET_CHAN_24V);
    Test_ByModbus_Update(&_ulTestBuf[0], eIDX_DET_VOLT_5V, 3);
}
#endif
static int s_iTestSpdTab[100];

const char ProgName[16] = "hd_hc32f460_2o";

void vTaskTest(void *pvParams)
{
    uint16_t _ulMs        = 0;
    uint8_t  _ucNdlNum    = 0;
    UINT16   _ulTestTmp   = 0;
    UINT16   _ulDetTmp[2] = {0};

    char str[16] = {0};
    (void) pvParams;

#if (MOTOR_TEST == 1)

    Calc_S_Curve(&s_iTestSpdTab[0],
                 ARRAY_SZ(s_iTestSpdTab),
                 30,
                 200,
                 6400,
                 2,
                 2);
    Mod_Motor_SetSpeedTable(A_MOTOR,
                            &s_iTestSpdTab[0],
                            ARRAY_SZ(s_iTestSpdTab));
    Mod_Motor_SetSpeedTable(B_MOTOR,
                            &s_iTestSpdTab[0],
                            ARRAY_SZ(s_iTestSpdTab));
    Mod_Motor_SetWorkI(A_MOTOR, 15);
    Mod_Motor_SetWorkI(B_MOTOR, 15);
    Mod_Motor_SetHoldI(A_MOTOR, 5);
    Mod_Motor_SetHoldI(B_MOTOR, 5);
#endif
#if (CAN_RECV_TEST == 1)
    extern void Mod_Cmd_SetCallBack(void (*callback)(struct can_message *));
    Mod_Cmd_SetCallBack(RecvCAN_TestCallback);
#endif
    Test_ByModbus_Init();
    sprintf(str, "%lx", RN_SOFT_VER);
    Test_ByModbus_Update((UINT16 *) &str[0], 16, 16);
    Test_ByModbus_Update((UINT16 *) &ProgName[0], 24, 16);
    for (;;) {
        Test_ByModbus_Poll();
        wdt_feed();
        if (_ulMs % 10 == 0) {
#if (SW_LED_TEST == 1)
            if (SW_DOWN_YES) {
                LG_OFF;
                LR_OFF;
                _ulTestTmp = 1;
            } else if (SW_UP_YES) {
                LG_OFF;
                LR_ON;
                _ulTestTmp = 2;
            } else {
                LG_ON;
                LR_OFF;
                _ulTestTmp = 0;
            }
            Test_ByModbus_Update(&_ulTestTmp, eIDX_KEY_STATE, 1);
#endif
            if (ACK_I_YES) {
                ACK_O_OFF();
            } else {
                ACK_O_ON();
            }
        }
#if (LOCK_TEST == 1)
        _vLockTest();
#endif
        if (_ulMs % 500 == 0) {
            MCU_LED_TOGGLE();
        }

        if (_ulMs % 3000 == 0) {
            ZPR_TOGGLE;
            ZPG_TOGGLE;
#if (CAN_SEND_TEST == 1)
            TestCAN_Send();
#endif
#if (ADC_TEST == 1)
            ADC_DetPrint();
#endif
#if (QF_TEST == 1)
            _ulTestTmp = Bsp_Qf_Read();
            Test_ByModbus_Update(&_ulTestTmp, eIDX_QF, 1);
            Bsp_Qf_Clear();
#endif
#if (THREAD_CHECK_TEST == 1)
            _ulTestTmp = Bsp_Thread_GetBotCount();
            Test_ByModbus_Update(&_ulTestTmp, eIDX_BOT_CNT, 1);
            Bsp_Thread_ClearBotCount();
            if (Bsp_Thread_BotCnDetYes()) {
                _ulTestTmp = 1;
            } else {
                _ulTestTmp = 0;
            }
            Test_ByModbus_Update(&_ulTestTmp, eIDX_CN_IN, 1);
            for (uint8_t i = 0; i < 15; i++) {
                Bsp_Thread_SelectNdl(i);
                OSTimeDly(2);

                if (Bsp_Thread_BotDetYes()) {
                    _ulDetTmp[0] |= (1 << i);
                } else {
                    _ulDetTmp[0] &= ~(1 << i);
                }
                if (Bsp_Thread_TopDetYes()) {
                    _ulDetTmp[1] |= (1 << i);
                } else {
                    _ulDetTmp[1] &= ~(1 << i);
                }
            }
            _ulTestTmp = _ulDetTmp[0];
            Test_ByModbus_Update(&_ulTestTmp, eIDX_BOT_DET, 1);
            _ulTestTmp = _ulDetTmp[1];
            Test_ByModbus_Update(&_ulTestTmp, eIDX_TOP_DET, 1);
#endif
        }
#if (MOTOR_TEST == 1)
        _vMotorTest();
#endif
#if (CAN_RECV_TEST == 1)
        Mod_Cmd_Recv();
#endif
        OSTimeDlyHMSM(0, 0, 0, 1);
        _ulMs++;
    }
}
#if (LOCK_TEST == 1)
static void _vLockTest(void)
{
    static uint16_t _ulMs = 0;

    static UINT16 _ulFaultFlg = 0;
    /* 锁线电磁铁运行100ms，停1.9S，自保持 */
    if (_ulMs < 80) {
        Bsp_Lock_SetEn(1);
        Bsp_Lock_SetDir(1);
    } else if (_ulMs < 100) {
        Bsp_Lock_SetDir(0);
    } else if (_ulMs < 2000) {
        Bsp_Lock_SetEn(0);
        _ulFaultFlg = 0;
    } else {
        _ulMs = 0;
    }
    if (Bsp_Lock_IsFault()) {
        Bsp_ClrLockFault();
        _ulFaultFlg = 1;
    }
    Test_ByModbus_Update(&_ulFaultFlg, eIDX_FAULT, 1);
    _ulMs++;
}
#endif

#if (MOTOR_TEST == 1)
static void _vMotorTest(void)
{
    static uint16_t _ulMs = 0;
    /* 上电后步进电机运行1S，停1S */
    if (_ulMs < 1000) {
    #if (JUMP_MOTOR_TEST == 1)
        if (false == Mod_Motor_IsRun(A_MOTOR)) {
            Mod_Motor_Act(A_MOTOR, 0xffff, 6400);
        }
    #endif
    #if (HOOK_MOTOR_TEST == 1)
        if (false == Mod_Motor_IsRun(B_MOTOR)) {
            Mod_Motor_Act(B_MOTOR, 0xffff, 6400);
        }
    #endif
    } else if (_ulMs < 2000) {
    #if (JUMP_MOTOR_TEST == 1)
        if (false == Mod_Motor_IsRun(A_MOTOR)) {
            Mod_Motor_Act(A_MOTOR, 0xffff, -6400);
        }
    #endif
    #if (HOOK_MOTOR_TEST == 1)
        if (false == Mod_Motor_IsRun(B_MOTOR)) {
            Mod_Motor_Act(B_MOTOR, 0xffff, -6400);
        }
    #endif
    } else if (_ulMs < 3000) {
    #if (JUMP_MOTOR_TEST == 1)
        Mod_Motor_Stop(A_MOTOR);
    #endif
    #if (HOOK_MOTOR_TEST == 1)
        Mod_Motor_Stop(B_MOTOR);
    #endif
    } else {
        _ulMs = 0;
    }
    _ulMs++;
}
#endif
