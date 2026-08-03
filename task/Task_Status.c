#include "ucos_ii.h"
#include "rn_typedef.h"
#include "alarm.h"
#include "signal_led.h"
#include "key.h"
#include "bsp_can.h"
#include "cmd.h"
#include "bsp_lock.h"
#include "bsp_gpio.h"
#include "bsp_fault.h"
#include "address.h"
#include "lock.h"
#include "thread_check.h"
#include "bsp_wdt.h"
#include "rn_printf.h"
void AlarmUpdate()
{
    if(Bsp_Lock_IsFault())
        Alarm_Set(ALARM_BIT_LOCK);
    else
        Alarm_Clr(ALARM_BIT_LOCK);
}


void StatusAlarmSend()
{
    static uint SaveState = 0;
    uint alarm = Alarm_GetAll();
    if(alarm != 0 && SaveState != alarm)
        Mod_Cmd_RspAlarm(alarm);
    SaveState = alarm;

    //不会一直报警的在这里清掉，会一直报警的直接在报警的else里面清除
    Alarm_Clr(ALARM_BIT_5V);
}

void HeadUpdateState(void)
{

    static uint8_t done_flag = 0;//按键动作标志
    static status_e _xNewState = STATUS_INITIAL;

    int key_sta = Fun_Key_GetState();

    if (Para_head.stat == STATUS_INITIAL)
    {
        return;
    }

    if ((Para_head.en == 0) || (key_sta == KEY_STATUS_DOWN))
    {
        Para_head.stat = STATUS_OFF;        
        Fun_Lock_SetHold();
    }
    else if ((key_sta == KEY_STATUS_NO) && (Para_head.stat == STATUS_OFF))
    {
        Para_head.stat = STATUS_NORMAL;
        Fun_Thread_Rst();
        Para_head.iskey_patch = 0;
//        Fun_Lock_SetRelease();
    }
    else if ((key_sta == KEY_STATUS_UP) && (done_flag == 0))
    {
        done_flag = 1;

        if (Para_head.addr_ing)
        {
            Fun_Addr_SetAddrDeal();
        }
        else if (Para_head.stat == STATUS_NORMAL)
        {
            Para_head.stat = STATUS_PATCH;
            Para_emb.stat = STATUS_PATCH;
            Para_head.iskey_patch = 1;
        }
        else
        {
            Para_head.stat = STATUS_NORMAL;
            Fun_Thread_ClrBrk();
            Para_head.iskey_patch = 0;
        }
    }
    else if (key_sta == KEY_STATUS_CHANGE)
    {
        done_flag = 0;
    }

    if ((Para_emb.stat == STATUS_NORMAL)
        && (Para_head.stat == STATUS_PATCH)
        && Para_emb.IsBrk ==0)
    {
        Para_head.stat = STATUS_NORMAL;
        Para_head.iskey_patch = 0;
        Fun_Lock_SetPatchNo(0);
    }


    static int32_t send_delay_cnt = -2000;//收初始命令2秒后再发，按键是1秒后

    if(Para_head.mainshaft_run == 0)
    {
        if (_xNewState != Para_head.stat)
        {
            send_delay_cnt++;
            if (send_delay_cnt > Para_head.addr)
            {
                _xNewState = Para_head.stat;
                Mod_Cmd_RspHeadStat(_xNewState);
            }
        }
        else
            send_delay_cnt = 0;
    }

}



void McuLed(void)
{
    static INT16U _ucCnt = 0;

    if (++_ucCnt == 1000) {//200ms
				_ucCnt = 0;
        MCU_LED_TOGGLE();
    }
}
int Fun_Test_IsRun(void);

void TaskStatus(void *pdata)
{

    OSTimeDlyHMSM(0, 0, 0, 10);

    while (1) {
        wdt_feed();
        McuLed();
        /* 机头灯 */
		if(Fun_Test_IsRun()==0)
		{
       		 Fun_Led_Task();
		}
        /* 机头开关 */
        Fun_Key_UpdateState();
        /* 设置机头状态 */
        HeadUpdateState();
        /* 报警状态 */
        AlarmUpdate();
        StatusAlarmSend(); //向主控板发送错误信息


        Fun_Addr_Task(Fun_Key_GetState());

        OSTimeDlyHMSM(0, 0, 0, 1);
    }
}
