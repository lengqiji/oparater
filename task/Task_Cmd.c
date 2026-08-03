#include "ucos_ii.h"
#include "rn_typedef.h"
#include "bsp_can.h"
#include "cmd/cmd_process.h"
#include "cmd.h"
#include "Task_Cmd.h"
#include "alarm.h"

extern void Mod_Cmd_SetCallBack(void (*callback)(struct can_message*));


//can发送缓冲区
typedef struct t_can_tx_data 
{
	uint32_t id;		//
	uint8_t data[8];	//数据
	uint8_t len;		//数据长度
}can_tx_data;

#define  MAX_CAN_TX_FRAME   50 //50个缓冲区

static can_tx_data CanTxFrame[MAX_CAN_TX_FRAME] = {0};//实际数据
static OS_MEM *pCanTxFrame;//用来管理数据
static OS_EVENT   *pQCanTxFrame;//用来管理消息指针
static void       *QCanTxFrameTb[MAX_CAN_TX_FRAME];//消息指针队列


void Cmd_Init()
{
    INT8U err;
    pCanTxFrame = OSMemCreate(CanTxFrame, MAX_CAN_TX_FRAME, sizeof(can_tx_data), &err);
    pQCanTxFrame = OSQCreate(&QCanTxFrameTb[0], MAX_CAN_TX_FRAME);

}


void Cmd_Post(struct can_message *pFrame)
{//放到发送缓冲区，等下一起发送
    INT8U err;
    can_tx_data *pt = OSMemGet(pCanTxFrame,&err);//申请空间
    if (err != OS_ERR_NONE)
    {
        Alarm_Set(ALARM_BIT_5V);//发送缓冲区满，临时用这个报警
        return;
    }
    pt->id = pFrame->id;
    memcpy(pt->data,pFrame->data,pFrame->len);
    pt->len = pFrame->len;    
    OSQPost(pQCanTxFrame, (void *)pt);
}

void Cmd_Send_BuffData()
{
    INT8U err;
    can_tx_data *pt = OSQAccept(pQCanTxFrame,&err);
    if (err != OS_ERR_NONE)
        return;

    Bsp_Can_Send(pt->id, pt->data, pt->len);

    OSMemPut(pCanTxFrame, pt);//释放空间

}


void TaskCmd(void *pdata)
{
    (void)pdata; 
    Cmd_Init();
    Mod_Cmd_SetCallBack(Task_Cmd_ProcessCallback);
    Mod_Cmd_TxCallBack(Cmd_Post);
    OSTimeDlyHMSM(0, 0, 0, 10);//等待其他认为初始化

    int cnt = 0;
    while(1)
    {
        cnt ++;
        Mod_Cmd_Recv();
        if(cnt % 10 == 0)
        {//发送太快会丢
            Cmd_Send_BuffData();
        }
//        OSTimeDlyHMSM(0, 0, 0, 1);
        OSTimeDly(1);
    }
}

