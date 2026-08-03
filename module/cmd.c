#include "rn_typedef.h"
#include "bsp_can.h"
#include "cmd.h"
#include "cmd_def.h"
#include "para.h"
#include "board_config.h"
#include "hc32_ddl.h"

typedef struct {
    uint8_t  head;  // 头位置
    uint8_t  tail;  // 尾位置
    uint16_t seq;   // 帧序号

    uint8_t rst_cnt;  // 下发的复位次数
    uint8_t up_cnt;   // 下发升级命令次数
} cmd_param_s;
// 帧结构体
typedef struct {
    uint8_t            flag;  // 使用标志 =0未用 =1已用
    struct can_message msg;
} cmd_frame_s;

#define CMD_TOTAL_NUM (50)                              // 命令结构体帧数
static cmd_frame_s cmd_frame_buf[CMD_TOTAL_NUM] = {0};  // 命令帧结构体数组
static cmd_param_s cmd_param = {0};                     // 参数结构体

static void (*Cmd_Recv_Callback)(struct can_message *);  // 动作结束回调
static void (*Cmd_Send_Callback)(struct can_message *);  //

static void callback_null(struct can_message *pFrame){};

void Can_Buff_Post(uint32_t id, uint8_t *send_data, uint8_t len);


void cmd_get_msg(cmd_frame_s *p)
{
    if (cmd_frame_buf[cmd_param.tail].flag == 1) {
        memcpy(p, &(cmd_frame_buf[cmd_param.tail]), sizeof(cmd_frame_s));
        cmd_frame_buf[cmd_param.tail].flag = 0;
        cmd_param.tail                     = (++cmd_param.tail) % CMD_TOTAL_NUM;
    } else
        p->flag = 0;
}

cmd_frame_s *cmd_frame_new(void)
{
    cmd_frame_s *p = NULL;

    if (cmd_frame_buf[cmd_param.head].flag == 0) {
        cmd_frame_buf[cmd_param.head].flag = 1;
        p                                  = &cmd_frame_buf[cmd_param.head];
        cmd_param.head                     = (++cmd_param.head) % CMD_TOTAL_NUM;
    } else {
        // shell_printf("cmd new err\r\n");
    }

    return p;
}
#if 1
static int cmd_scroll_id_judge(struct can_message *pFrame)
{
    /* Mod_Cmd_RspBrk */
    if ((pFrame->id >= 0x401) && (pFrame->id <= 0x4FF)) {
        return 1;
    }

    if (pFrame->id <= 0x1c1) {
        if ((pFrame->id % 0x40) == 0)  // 广播命令
        {
            return 1;
        } else if ((pFrame->id % 0x40) == 1)  // 机头命令
        {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
#endif
void cmd_recv(struct can_message *msg)
{
    cmd_frame_s *p;

#if 1
    if (msg->fmt == CAN_FMT_STDID) {
        if (cmd_scroll_id_judge(msg) == 0) {  // 不处理的命令直接扔掉
            return;
        }
    }
#endif
    p = cmd_frame_new();
    if (p) {
        memcpy(&(p->msg), msg, sizeof(struct can_message));
    }
}

void Mod_Cmd_Init()
{
    Cmd_Recv_Callback = callback_null;
    Cmd_Send_Callback = callback_null;
    Bsp_Can_SetRxCallBack(cmd_recv);
}

void Mod_Cmd_SetCallBack(void (*callback)(struct can_message *))
{
    Cmd_Recv_Callback = callback;
}

void Mod_Cmd_Recv(void)
{
    for (int i = 0; i < CMD_TOTAL_NUM; i++)  // 1ms任务 50条命令
    {
        cmd_frame_s CanRxFrame;
        cmd_get_msg(&CanRxFrame);
        if (CanRxFrame.flag) {
            Cmd_Recv_Callback(&(CanRxFrame.msg));
        }
    }
}




// 编址反馈
void Mod_Cmd_RspSetAddr(void)
{
    uint8_t buf[8] = {0};

    buf[0] = 0x15;
    buf[1] = 0x01;
    buf[2] = Para_head.type;
    buf[3] = Para_head.addr;
    Can_Buff_Post(CMD_HEAD_STATUS_ID + Para_head.addr * 8, buf, 8);
}

// 自动编址反馈
void Mod_Cmd_RspAutoSetAddr(void)
{
    uint8_t buf[8] = {0};

    buf[0] = 0x15;
    buf[1] = 0x01;
    buf[2] = Para_head.type;
    buf[3] = Para_head.addr;
    Can_Buff_Post(CANID_CMD_HEAD_AUTO_ADDR_RSP, buf, 8);
}
/* 寻址反馈 */
void Mod_Cmd_RspLookAddr(void)
{
    uint8_t buf[8] = {0};

    buf[0] = 0x15;
    buf[1] = 0x02;
    buf[2] = Para_head.type;
    buf[3] = Para_head.addr;
    Can_Buff_Post(CMD_HEAD_STATUS_ID + Para_head.addr * 8, buf, 8);
}



void Mod_Cmd_RspAlarm(uint16_t alarm_var)
{//

    uint8_t buf[4] = {0};

    buf[0] = eHEAD_ALARM;
    buf[1] = alarm_var&0xff;
    buf[2] = (alarm_var>>8)&0xff;
    buf[3] = (alarm_var>>16)&0xff;
    Can_Buff_Post(CANID_CMD_HEAD_SEND_ALARM+Para_head.addr,buf, sizeof(buf));
}

void Mod_Cmd_RspHeadStat(uint8_t status)
{
	uint8_t buf[8] = {0};

	buf[0] = eSTATUS_HEAD;
	buf[1] = status;
    Can_Buff_Post(CANID_CMD_HEAD_SEND_ALARM+Para_head.addr, buf, 2);
}

void Mod_Cmd_RspKeyStat(uint8_t status)
{
	uint8_t buf[8] = {0};

	buf[0] = eSTATUS_KEY;
	buf[1] = status;
    Can_Buff_Post(CANID_CMD_HEAD_SEND_ALARM+Para_head.addr, buf, 2);
}

void Mod_Cmd_RspBrk(void)
{
    uint8_t buf[8] = {0};

    buf[0] = 0x1c;

    buf[1] = Para_head.addr;

    Can_Buff_Post(CMD_HEAD_STATUS_ID + Para_head.addr, buf, 2);
}

void Mod_Cmd_RspHookState(uint8_t status)
{
    uint8_t buf[8] = {0};

    buf[0] = status;
    Can_Buff_Post(CANID_CMD_HEAD_HOOK_UP_STA + Para_head.addr, buf, 1);
}

void Mod_Cmd_RspHeart(struct can_message *pFrame)
{
	uint8_t buf[8] = {0};
    uint32_t    idno;
	static uint8_t heart_cnt = 0;

	//判断地址
	if ((pFrame->data[0] == 0x1) && (pFrame->data[1] == Para_head.addr))
	{
		buf[0] = Para_head.type;		//机头类型
		buf[1] = Para_head.addr;		//机头地址
		buf[2] = heart_cnt;				//心跳计数
        //2022.03.24 mod --> tow type head on one machione
		//can_msg_send(CAN_FMT_EXTID, CANID_CMD_HEAD_RSP_HEART, buf, 3);
        idno = CANID_CMD_HEAD_RSP_HEART + Para_head.type - 1;
		Can_Buff_Post(idno, buf, 3);

		heart_cnt += 1;
	}

}

void Rsp_Board_Time_Type(uint8_t buf[])
{
extern const char RELEASEDATE[16];
extern const char RELEASETIME[16];
//#define RN_SOFT_VER 		(0xE0450520UL)	//
    uint8_t v;
#ifdef DEBUG_ON
		buf[3] = 0xD2;
#else
		buf[3] = 0xC2;
#endif
    v = 4;
    v = v << 4; //ID
    v += (RELEASEDATE[10]-0x30);     //年
//    v = (RELEASEDATE[10]-0x30) << 4;
	 buf[2] = v;//ID+年

	 v = 0;
    if ((RELEASEDATE[0]=='J') && (RELEASEDATE[1]=='a') && (RELEASEDATE[2]=='n'))
    {
        v = 0x01;
    }
    if ((RELEASEDATE[0]=='F') && (RELEASEDATE[1]=='e') && (RELEASEDATE[2]=='b'))
    {
        v = 0x02;
    }
    if ((RELEASEDATE[0]=='M') && (RELEASEDATE[1]=='a') && (RELEASEDATE[2]=='r'))
    {
        v = 0x03;
    }
    if ((RELEASEDATE[0]=='A') && (RELEASEDATE[1]=='p') && (RELEASEDATE[2]=='r'))
    {
        v = 0x04;
    }
    if ((RELEASEDATE[0]=='M') && (RELEASEDATE[1]=='a') && (RELEASEDATE[2]=='y'))
    {
        v = 0x05;
    }
    if ((RELEASEDATE[0]=='J') && (RELEASEDATE[1]=='u') && (RELEASEDATE[2]=='n'))
    {
        v = 0x06;
    }
    if ((RELEASEDATE[0]=='J') && (RELEASEDATE[1]=='u') && (RELEASEDATE[2]=='l'))
    {
        v = 0x07;
    }
    if ((RELEASEDATE[0]=='A') && (RELEASEDATE[1]=='u') && (RELEASEDATE[2]=='g'))
    {
        v += 0x08;
    }
    if ((RELEASEDATE[0]=='S') && (RELEASEDATE[1]=='e') && (RELEASEDATE[2]=='p'))
    {
        v = 0x09;
    }
    if ((RELEASEDATE[0]=='O') && (RELEASEDATE[1]=='c') && (RELEASEDATE[2]=='t'))
    {
        v = 0x10;
    }
    if ((RELEASEDATE[0]=='N') && (RELEASEDATE[1]=='o') && (RELEASEDATE[2]=='v'))
    {
        v = 0x11;
    }
    if ((RELEASEDATE[0]=='D') && (RELEASEDATE[1]=='e') && (RELEASEDATE[2]=='c'))
    {
        v = 0x12;
    }

    buf[1] = v;
    v = 0;
    if (RELEASEDATE[4] != 0x20)
    {
        v += (RELEASEDATE[4] - 0x30) << 4;
    }
    v += (RELEASEDATE[5] - 0x30);
    //具体ri
     buf[0] = v;

}

// 扩展帧回复机头版本信息
void Mod_Cmd_RspVerExt(struct can_message *pFrame)
{
    uint8_t  buf[8]    = {0};
    uint32_t head_addr = RN_SOFT_VER;
    uint8_t verion[4] = {0};
    Rsp_Board_Time_Type(verion);
    // 判断长度、类型、地址
    if ((pFrame->len == 2) && (pFrame->data[0] == (Para_head.type)) &&
        (pFrame->data[1] == Para_head.addr)) {
        buf[0] = Para_head.type;  // 机头类型
        buf[1] = Para_head.addr;      // 机头地址
//        memcpy(&buf[2], &head_addr, 4);
		 memcpy(&buf[2], &verion, 4);
        Can_Buff_Post(CAN_CMD_HEAD_RSP_VERSION, buf, 6);
    }
}


void Mod_Cmd_RspResetProc(void)
{
	uint8_t buf[8] = {0};
	uint8_t reset = 0;

	stc_rmu_rstcause_t stcRmuRstCause;

	/* Get RMU information */
    RMU_GetResetCause(&stcRmuRstCause);
	RMU_ClrResetFlag();

	if (Set == stcRmuRstCause.enMultiRst)		//发生两个及两个以上复
    {
        //shell_printf("Multiply Reset\r\n");
    }

	if (Set == stcRmuRstCause.enXtalErr)		//外部高速振荡器异常停振复位
    {
        //shell_printf("Xtal error reset\r\n");
    }

	if (Set == stcRmuRstCause.enClkFreqErr)		//时钟频率异常复位
    {
        //shell_printf(" Clk freqence error reset\r\n");
    }

	if (Set == stcRmuRstCause.enRamEcc)			//RAM ECC 错误复位
    {
        //shell_printf("Ram ECC reset\r\n");
    }

	if (Set == stcRmuRstCause.enRamParityErr)	//RAM 奇偶校验复位
    {
        //shell_printf("Ram parity error reset\r\n");
    }

	if (Set == stcRmuRstCause.enMpuErr)			//MPU 错误复位
    {
        //shell_printf("Software reset\r\n");
    }

	if (Set == stcRmuRstCause.enSoftware)		//软件复位
    {
        //shell_printf("Software reset\r\n");
    }

	if (Set == stcRmuRstCause.enPowerDown)		//掉电唤醒复位
    {
        //shell_printf("Power down reset\r\n");
		reset |= (1<<2);
    }

	if (Set == stcRmuRstCause.enSwdt)			//专用看门狗复位
    {
        //shell_printf("Special watchdog timer reset\r\n");
    }

	if (Set == stcRmuRstCause.enWdt)			//看门狗复位
    {
        //shell_printf("Special watchdog timer reset\r\n");
		reset |= (1<<5);
    }

	if (Set == stcRmuRstCause.enPvd2)			//可编程电压检测 2 复位
    {
        //shell_printf("Program voltage Dectection 2 reset\r\n");
    }

	if (Set == stcRmuRstCause.enPvd1)			//可编程电压检测 1 复位
    {
        //shell_printf("Program voltage Dectection 1 reset\r\n");
    }

	if (Set == stcRmuRstCause.enBrownOut)		//欠压复位
    {
        //shell_printf("Brown out reset\r\n");
    }

	if (Set == stcRmuRstCause.enRstPin)			//NRST 引脚复位
    {
        //shell_printf("Reset pin reset\r\n");
		reset |= (1<<4);
    }

	if (Set == stcRmuRstCause.enPowerOn)		//上电复位
    {
        //shell_printf("Power on reset\r\n");

		reset |=1;
    }


    buf[0] = eERR_SYS_RST;
    buf[1] = reset;
	Can_Buff_Post(CANID_CMD_HEAD_SEND_ALARM+Para_head.addr, buf, 2);
}

void Mod_Cmd_RspFootStatus(uint8_t status, uint8_t cnt)
{
	static uint8_t pre_status;
	uint8_t buf[8] = {0};

	buf[0] = 0x09;

	if (pre_status != status)
	{
		buf[1] = status;
		buf[2] = cnt;
		pre_status = status;
		Can_Buff_Post(CANID_CMD_HEAD_SEND_ALARM+Para_head.addr, buf, 3);
	}
}


void Mod_Cmd_RspFootLoc(uint8_t foot_loc)
{//扩展帧 机头回复压脚位置
	uint8_t buf[8] = {0};

	buf[0] = eSTATUS_FOOT_LOC;
	buf[1] = foot_loc;

    Can_Buff_Post(CANID_CMD_HEAD_SEND_ALARM+Para_head.addr, buf, 2);

}


void Can_Buff_Post(uint32_t id, uint8_t *send_data, uint8_t len)
{//放到发送缓冲区，等下一起发送
    struct can_message msg;
    msg.id = id;
    memcpy(msg.data,send_data,len);
    msg.len = len;
    Cmd_Send_Callback(&msg);
}

void Mod_Cmd_TxCallBack(void (*callback)(struct can_message *))
{
    Cmd_Send_Callback = callback;
}
