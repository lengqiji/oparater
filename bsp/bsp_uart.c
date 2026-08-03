/****************************************Copyright (c)**************************************************
**                                 福建睿能科技有限公司
**                                     研    发    部
**
**
**                                 http://www.burnon.com
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名:uart.c
**创   建   人: Luogxw
**最后修改日期:
**描        述:	串口
**
**
********************************************************************************************************/
#include <stdint.h>

#include "bsp_uart.h"
#include "hc32_ddl.h"

#define DEBUG_BAUD                          (115200ul)//(460800ul) //(1500000ul) //


/* USART channel definition */
#define DEBUG_USART_CH                        (M4_USART1)
#define DEBUG_USART_FCG_PERIPH                PWC_FCG1_PERIPH_USART1
/* USART baudrate definition */

/* USART RX Port/Pin definition */
#define DEBUG_USART_RX_PORT                   (PortA)
#define DEBUG_USART_RX_PIN                    (Pin13)
#define DEBUG_USART_RX_FUNC                   (Func_Usart1_Rx)
#define USART_RI_NUM                          (INT_USART1_RI)
/* USART TX Port/Pin definition */
#define DEBUG_USART_TX_PORT                   (PortA)
#define DEBUG_USART_TX_PIN                    (Pin14)
#define DEBUG_USART_TX_FUNC                   (Func_Usart1_Tx)

/* DMA_USART_ADDR*/
#define DEBUG_USART_DMA_RX_ADDR               (((unsigned int)&M4_USART1->DR) + 2)
#define DEBUG_USART_DMA_TX_ADDR               (((unsigned int)&M4_USART1->DR))

/* DMA_USART_TRIGGER_SRC*/
#define DEBUG_USART_DMA_RX_TRIGGER            (EVT_USART1_RI)
#define DEBUG_USART_DMA_TX_TRIGGER            (EVT_USART1_TI)

/* USART DMA_UNIT */
#define DEBUG_DMA_UNIT                       (M4_DMA2)
#define DEBUG_DMA_TX_CH                      (DmaCh3)
#define DEBUG_DMA_FCG_PERIPH                 (PWC_FCG0_PERIPH_DMA2)
#define DEBUG_DMA_TX_TRIGGER                 (EVT_USART1_TI)

#define UART_RX_BUFF_SIZE   32

uint8_t uart_buff[UART_RX_BUFF_SIZE];
uint8_t uart_rx_idx = 0;

static void Bsp_Dma_Debug_Init(void);


static void bsp_usart_rx_callback(void)
{
    uint16_t u16Data = USART_RecData(DEBUG_USART_CH);

    //USART_SendData(USART_CH, u16Data);
    if (uart_rx_idx < UART_RX_BUFF_SIZE)
    {
        uart_buff[uart_rx_idx++] = u16Data;
    }
    //(void)RingBufWrite(&m_stcRingBuf, (uint8_t)u16Data);
}

void BSP_UartInit(void)
{
    PORT_DebugPortSetting(TCK_SWCLK, Disable);
    PORT_DebugPortSetting(TMS_SWDIO, Disable);

    PWC_Fcg1PeriphClockCmd( DEBUG_USART_FCG_PERIPH, Enable);


    stc_usart_uart_init_t   stcInitCfg;
    MEM_ZERO_STRUCT(stcInitCfg);
    stcInitCfg.enClkDiv     = UsartClkDiv_1;
    stcInitCfg.enClkMode    = UsartIntClkCkNoOutput;
    stcInitCfg.enDataLength = UsartDataBits8;
    stcInitCfg.enDetectMode = UsartStartBitFallEdge;
    stcInitCfg.enDirection  = UsartDataLsbFirst;
    stcInitCfg.enHwFlow     = UsartRtsEnable;
    stcInitCfg.enParity     = UsartParityNone;
    stcInitCfg.enStopBit    = UsartOneStopBit;
    stcInitCfg.enSampleMode = UsartSampleBit16;
    /* Initialize UART */
    while(USART_UART_Init(DEBUG_USART_CH, &stcInitCfg) != Ok){};
    while(USART_SetBaudrate(DEBUG_USART_CH, DEBUG_BAUD) != Ok){};


    PORT_SetFunc(DEBUG_USART_RX_PORT, DEBUG_USART_RX_PIN, DEBUG_USART_RX_FUNC, Disable);
    PORT_SetFunc(DEBUG_USART_TX_PORT, DEBUG_USART_TX_PIN, DEBUG_USART_TX_FUNC, Disable);

//    PORT_SetFunc(DEBUGJLINK_USART_TX_PORT, DEBUGJLINK_USART_TX_PIN, DEBUGJLINK_USART_TX_FUNC, Disable);

#if 1
	/* Set USART RX IRQ */
	stc_irq_regi_conf_t stcIrqRegiCfg;
    MEM_ZERO_STRUCT(stcIrqRegiCfg);
    stcIrqRegiCfg.enIRQn = Int018_IRQn;
    stcIrqRegiCfg.pfnCallback = &bsp_usart_rx_callback;
    stcIrqRegiCfg.enIntSrc = USART_RI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_00);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    /*Enable RX && TX function*/
    USART_FuncCmd(DEBUG_USART_CH, UsartRx, Enable);
    USART_FuncCmd(DEBUG_USART_CH, UsartRxInt, Enable);

#endif
    USART_FuncCmd(DEBUG_USART_CH, UsartTx, Enable);
    USART_FuncCmd(DEBUG_USART_CH, UsartNoiseFilter, Enable);

//    USART_FuncCmd(DEBUGJLINK_USART_CH, UsartTx, Enable);

//    Bsp_Dma_Debug_Init();


}
void Bsp_Usart_Send(uint8_t *tx_data, uint16_t len)
{
    while (len--)
    {
        USART_SendData(DEBUG_USART_CH, (uint16_t)*tx_data);
        tx_data++;
    }
}

//---------------------------------------------------------------------
//函数名称：void BSPDebugSentB(void) BSPDebugSenString
//
//输入参数：无
//
//输出参数：无
//
//功能描述：调试用
//
//---------------------------------------------------------------------

void BSPDebugSentB(uint16_t u16TD)
{
    if(Set == USART_GetStatus(DEBUG_USART_CH, UsartTxEmpty))  /* Warit Tx data register empty */
    {
        USART_SendData(DEBUG_USART_CH, (u16TD&0xff));//等于0 则死循环
    }
//    if(Set == USART_GetStatus(DEBUGJLINK_USART_CH, UsartTxEmpty))  /* Warit Tx data register empty */
//    {
//        USART_SendData(DEBUGJLINK_USART_CH, (u16TD&0xff));//等于0 则死循环
//    }


    //无等待
}

void BSPDebugSenString(uint16_t u16TD)
{

    USART_SendData(DEBUG_USART_CH, (u16TD&0xff));
}

int16_t BSP_DEBUG_UART_Read(void)
{
    if (Set == USART_GetStatus(DEBUG_USART_CH, UsartFrameErr))
    {
        USART_ClearStatus(DEBUG_USART_CH, UsartFrameErr);
    }
    if (Set == USART_GetStatus(DEBUG_USART_CH, UsartParityErr))
    {
        USART_ClearStatus(DEBUG_USART_CH, UsartParityErr);
    }
    if (Set == USART_GetStatus(DEBUG_USART_CH, UsartOverrunErr))
    {
        USART_ClearStatus(DEBUG_USART_CH, UsartOverrunErr);
    }

    if (Set == USART_GetStatus(DEBUG_USART_CH, UsartRxNoEmpty))
    {
        return  USART_RecData(DEBUG_USART_CH) & 0xFF;
    }

    return -1;
}


void Bsp_Dma_Debug_Init(void)
{

    PWC_Fcg0PeriphClockCmd(DEBUG_DMA_FCG_PERIPH, Enable);
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);

    //模块复位
    DMA_Cmd(DEBUG_DMA_UNIT, Disable);
    DMA_DeInit(DEBUG_DMA_UNIT, DEBUG_DMA_TX_CH);
    DMA_Cmd(DEBUG_DMA_UNIT, Enable);

    stc_dma_config_t stc_dma_config;
//    //数据接收通道配置
//    MEM_ZERO_STRUCT(stc_dma_config);
//    stc_dma_config.u16BlockSize = 1;
//    stc_dma_config.u16TransferCnt = 0;
//    stc_dma_config.u32SrcAddr = USART_DMA_RX_ADDR;
//    stc_dma_config.u16DesRptSize = 0;                   //设置为1024，即1k数据重载
//    stc_dma_config.stcDmaChCfg.enDesInc = AddressIncrease;
//    stc_dma_config.stcDmaChCfg.enSrcInc = AddressFix;
//    stc_dma_config.stcDmaChCfg.enDesRptEn = Enable;
//    stc_dma_config.stcDmaChCfg.enSrcRptEn = Disable;
//    stc_dma_config.stcDmaChCfg.enTrnWidth = Dma8Bit;
//    stc_dma_config.stcDmaChCfg.enLlpEn = Disable;
//    stc_dma_config.stcDmaChCfg.enLlpMd = LlpWaitNextReq;
//    stc_dma_config.stcDmaChCfg.enIntEn = Disable;
//    DMA_InitChannel(DEBUG_DMA_UNIT, DEBUG_DMA_CH, &stc_dma_config);
//    DMA_SetTriggerSrc(DEBUG_DMA_UNIT, DEBUG_DMA_TX_CH, USART_DMA_RX_TRIGGER);

    //数据发送通道配置
    MEM_ZERO_STRUCT(stc_dma_config);
    stc_dma_config.u16BlockSize = 1;
    stc_dma_config.u16TransferCnt = 0;
    stc_dma_config.u32DesAddr = DEBUG_USART_DMA_TX_ADDR;
    stc_dma_config.u16DesRptSize = 0;                   //设置为1024，即1k数据重载
    stc_dma_config.u16SrcRptSize = 0;
    stc_dma_config.stcDmaChCfg.enDesInc = AddressFix;
    stc_dma_config.stcDmaChCfg.enSrcInc = AddressIncrease;
    stc_dma_config.stcDmaChCfg.enDesRptEn = Disable;
    stc_dma_config.stcDmaChCfg.enSrcRptEn = Enable;
    stc_dma_config.stcDmaChCfg.enTrnWidth = Dma8Bit;
    stc_dma_config.stcDmaChCfg.enLlpEn = Disable;
    stc_dma_config.stcDmaChCfg.enLlpMd = LlpWaitNextReq;
    stc_dma_config.stcDmaChCfg.enIntEn = Disable;
    DMA_InitChannel(DEBUG_DMA_UNIT, DEBUG_DMA_TX_CH, &stc_dma_config);

    DMA_SetTriggerSrc(DEBUG_DMA_UNIT, DEBUG_DMA_TX_CH, DEBUG_DMA_TX_TRIGGER);

}

__attribute__((section("FastRam"))) void Bsp_Dma_Debug_TxSrc(unsigned int src, unsigned int size)
{
    if(0 == size)
    {
        return;
    }

    stc_dma_dtctl_field_t *DTCTL_f, *MONDTCTL_f;
    uint32_t *SrcAdd;

    DTCTL_f = (stc_dma_dtctl_field_t *)((uint32_t)&DEBUG_DMA_UNIT->DTCTL0 + DEBUG_DMA_TX_CH * 0x40);

    SrcAdd = (uint32_t *)((uint32_t)&DEBUG_DMA_UNIT->SAR0 + DEBUG_DMA_TX_CH * 0x40);

    MONDTCTL_f = (stc_dma_dtctl_field_t *)((uint32_t)&DEBUG_DMA_UNIT->MONDTCTL0 + DEBUG_DMA_TX_CH * 0x40);

    DTCTL_f->CNT = size;
    *SrcAdd = src;
    __DSB();__ISB();

    while(MONDTCTL_f->CNT != size)
    {
        DTCTL_f->CNT = size;
        __DSB();__ISB();
    }
    DEBUG_DMA_UNIT->CHEN |= (1<<DEBUG_DMA_TX_CH);
}


int32_t fputc(int32_t ch, FILE *f)
{
    (void)f;  /* Prevent unused argument compilation warning */
	//while((M4_USART1->SR&0X40)==0); //循环发送,直到发送完毕
	for(uint32_t i=0;i<10000;i++)//1ms
    {
        __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
        if((M4_USART1->SR&0X40) != 0)
            break;
    }
	M4_USART1->DR = (uint8_t) ch;
	return ch;;
}


//---------------------------------------------------------------------
