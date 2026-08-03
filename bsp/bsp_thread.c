#include "bsp_thread.h"
#include "hc32_ddl.h"
#include "para.h"
//#define MACHINE_SLIDE_DETECT//易瑞德滑动式光耦底检专用
// 底检通道选择
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

void Bsp_Thread_BotInit(void);

static uint16_t s_usBotCnt = 0;

static void BotDetCnt_Callback(void)
{
    s_usBotCnt++;
}

void Bsp_Thread_Init()
{
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;

    PORT_Init(A0_DRV_PORT, A0_DRV_PIN, &stcPortInit);
    PORT_Init(A1_DRV_PORT, A1_DRV_PIN, &stcPortInit);
    PORT_Init(A2_DRV_PORT, A2_DRV_PIN, &stcPortInit);

    PORT_Init(OEL_PORT, OEL_PIN, &stcPortInit);
    PORT_Init(OEH_PORT, OEH_PIN, &stcPortInit);
    A0_DRV_OFF;
    A1_DRV_OFF;
    A2_DRV_OFF;
    OEL_OFF;
    OEH_OFF;

    stcPortInit.enPinMode = Pin_Mode_In;
    PORT_Init(TOP_DET_PORT, TOP_DET_PIN, &stcPortInit);
    // PORT_Init(BOT_DET_PORT, BOT_DET_PIN, &stcPortInit);
    PORT_Init(CN_DET_PORT, CN_DET_PIN, &stcPortInit);

    Bsp_Thread_BotInit();
}

void Bsp_Thread_BotInit(void)
{
    stc_exint_config_t  stcExtiConfig;
    stc_port_init_t     stcPortInit;
    stc_irq_regi_conf_t stcIrqRegiConf;

    MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcPortInit);
    MEM_ZERO_STRUCT(stcIrqRegiConf);

    stcExtiConfig.enExitCh = BOT_DET_EXT_CH;
    /* Filter setting */
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk   = Pclk3Div8;
    stcExtiConfig.enExtiLvl  = ExIntFallingEdge;
    EXINT_Init(&stcExtiConfig);
    /* Set External Int */
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enExInt   = Enable;
    stcPortInit.enPullUp  = Enable;
    PORT_Init(BOT_DET_PORT, BOT_DET_PIN, &stcPortInit);
    /* Select External Int Ch.13 */
    stcIrqRegiConf.enIntSrc = BOT_DET_EIQ;
    /* Register External Int to Vect.No.000 */
    stcIrqRegiConf.enIRQn = BOT_DET_IRQHandler;
    /* Callback function */
    stcIrqRegiConf.pfnCallback = BotDetCnt_Callback;
    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);
    /* Clear pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, BOT_DET_PRIORITY);
    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
}

uint16_t Bsp_Thread_GetBotCount(void)
{
    return s_usBotCnt;
}

void Bsp_Thread_ClearBotCount(void)
{
    s_usBotCnt = 0;
}

void Bsp_Thread_SelectNdl(uint8_t ndl_num)
{
    // 根据针号开启通道
    OEL_OFF;
    OEH_OFF;

    if (ndl_num & 0x01)
        A0_DRV_ON;
    else
        A0_DRV_OFF;

    if (ndl_num & 0x02)
        A1_DRV_ON;
    else
        A1_DRV_OFF;

    if (ndl_num & 0x04)
        A2_DRV_ON;
    else
        A2_DRV_OFF;

    if (ndl_num < 8) {
        OEL_ON;
        OEH_OFF;
    } else {
        OEL_OFF;
        OEH_ON;
    }
}

//易瑞德纯底检需求
void Bsp_Thread_ERED_SelectNdl(uint8_t ndl_num,uint8_t _ndl_num_mode,uint8_t _ndl_num_type,uint8_t _a_opto_num,uint8_t _b_opto_num)
{
#define ODD_A 0

#define MODE_0 0
#define MODE_1 1

#define TYPE_0 0//A+B+
#define TYPE_1 1//A+B-
#define TYPE_2 2//A-B+
#define TYPE_3 3//A-B-

	uint8_t A_NDL_NUM =(_a_opto_num);//A底检板通道数
	uint8_t B_NDL_NUM =(_b_opto_num);//B底检板通道数
	uint8_t aim_ndl_num = 0;//目标针位	
	uint8_t is_combine_ndl_equal = 0;//是否针位组合相等
	uint8_t is_ndl_polar_A = ODD_A;
	is_ndl_polar_A = ndl_num%2;//A底检板为0 ,B底检板为1
	
	if (A_NDL_NUM==B_NDL_NUM)
	{
		is_combine_ndl_equal = 0;
	}
	else
	{
		is_combine_ndl_equal = 1;
	}
	/*		 Oxaa底检方式值
	  A    	 B 		 thread_param.ndl_num_mode    说明
	奇数  	偶数			0							默认
	偶数		奇数			1


	A         B       thread_param.ndl_num_type    说明
	正装      正装                0						默认
	正装      反装                1         	
	反装      正装           	  2         	
	反装      反装                3         	
	 */
if(is_combine_ndl_equal ==0)//相等，比如6+6
{
	switch(_ndl_num_mode)
	{
	case MODE_0:
		switch(_ndl_num_type)
		{
		case TYPE_0:
			aim_ndl_num = ndl_num;//原始正常的
			break;
		case TYPE_1:
			if(is_ndl_polar_A == ODD_A)
			{
				aim_ndl_num = ndl_num;//原始正常的
			}
			else
			{
				aim_ndl_num = (B_NDL_NUM)*2-ndl_num;
			}
			break;
		case TYPE_2:
			if(is_ndl_polar_A == ODD_A)
			{
				aim_ndl_num = (A_NDL_NUM-1)*2-ndl_num;
			}
			else
			{
				aim_ndl_num = ndl_num;//原始正常的
			}
			break;
		case TYPE_3:
			if(is_ndl_polar_A == ODD_A)
			{
				aim_ndl_num = (A_NDL_NUM-1)*2-ndl_num;
			}
			else
			{
				aim_ndl_num = (B_NDL_NUM)*2-ndl_num;
			}
			break;
		}
		break;
	case MODE_1:
		switch(_ndl_num_type)
		{
		case TYPE_0:
			if(is_ndl_polar_A == ODD_A)
			{
				aim_ndl_num = (A_NDL_NUM-1)*2-ndl_num;
			}
			else
			{
				aim_ndl_num = (B_NDL_NUM)*2-ndl_num;
			}

			break;
		case TYPE_1:
			if(is_ndl_polar_A == ODD_A)
			{
				aim_ndl_num = (A_NDL_NUM-1)*2-ndl_num;
			}
			else
			{
				aim_ndl_num = ndl_num;//原始正常的
			}
			break;
		case TYPE_2:
			if(is_ndl_polar_A == ODD_A)
			{
				aim_ndl_num = ndl_num;//原始正常的
			}
			else
			{
				aim_ndl_num = (B_NDL_NUM)*2-ndl_num;
			}
			break;
		case TYPE_3:
			aim_ndl_num = ndl_num;//原始正常的
			break;
		}
		break;

	}
}
else//这个是AB断检不相等的情况//不相等，比如4+5,7+8
{
	switch(_ndl_num_mode)
	{
	case MODE_0:
		switch(_ndl_num_type)
		{
		case TYPE_0:
			aim_ndl_num = ndl_num;//原始正常的
			break;
		case TYPE_1:
			if(is_ndl_polar_A == ODD_A)
			{
				aim_ndl_num = ndl_num;//原始正常的
			}
			else
			{
				aim_ndl_num = A_NDL_NUM+B_NDL_NUM-1-ndl_num;
			}
			break;
		case TYPE_2:
			if(is_ndl_polar_A == ODD_A)
			{
				aim_ndl_num = A_NDL_NUM+B_NDL_NUM-1-ndl_num;
			}
			else
			{
				aim_ndl_num = ndl_num;//原始正常的
			}
			break;
		case TYPE_3:
			aim_ndl_num = A_NDL_NUM+B_NDL_NUM-1-ndl_num;
			break;
		}
		break;
	case MODE_1:
		switch(_ndl_num_type)
		{
		case TYPE_0:
			aim_ndl_num = A_NDL_NUM+B_NDL_NUM-1-ndl_num;

			break;
		case TYPE_1:
			if(is_ndl_polar_A == ODD_A)
			{
				aim_ndl_num = A_NDL_NUM+B_NDL_NUM-1-ndl_num;
			}
			else
			{
				aim_ndl_num = ndl_num;//原始正常的
			}
			break;
		case TYPE_2:
			if(is_ndl_polar_A == ODD_A)
			{
				aim_ndl_num = ndl_num;//原始正常的
			}
			else
			{
				aim_ndl_num = A_NDL_NUM+B_NDL_NUM-1-ndl_num;
			}
			break;
		case TYPE_3:
			aim_ndl_num = ndl_num;//原始正常的
			break;
		}
		break;

	}
}
#ifdef MACHINE_SLIDE_DETECT
    if (((ndl_num+1) % 2) == 0) {
        A0_DRV_ON; /* 偶数 */
    } else {
        A0_DRV_OFF; /* 奇数 */
    }

    return;
#endif	
	// 根据针号开启通道
	OEL_OFF;
	OEH_OFF;
	if (aim_ndl_num & 0x01)
		A0_DRV_ON;
	else
		A0_DRV_OFF;

	if (aim_ndl_num & 0x02)
		A1_DRV_ON;
	else
		A1_DRV_OFF;

	if (aim_ndl_num & 0x04)
		A2_DRV_ON;
	else
		A2_DRV_OFF;

//	if (aim_ndl_num < 8)
//	{
//		OEL_ON;
//		OEH_OFF;
//	}
//	else
//	{
//		OEL_OFF;
//		OEH_ON;
//	}
	 if (( Para_head.ndl_num < 16) ||
        (Para_head.is_splice && ( Para_head.ndl_num  <= 30) &&
            ( Para_head.ndl_num  != 20))) {
        if (aim_ndl_num > 7) {
            OEL_OFF;
            OEH_ON;
        } else {
            OEL_ON;
            OEH_OFF;
        }
    } 
		
}


uint8_t Bsp_Thread_TopDetYes()
{
    return TOP_DET_YES;
}
uint8_t Bsp_Thread_BotCnDetYes()
{  // 底检
    return CN_DET_YES;
}

uint8_t Bsp_Thread_BotDetYes()
{
    return BOT_DET_YES;
}
