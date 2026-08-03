/****************************************Copyright (c)**************************************************
**                                 ������ܿƼ����޹�˾
**                                     ��    ��    ��
**
**
**                                 http://www.burnon.com
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��:uart.h
**��   ��   ��: Luogxw
**����޸�����:
**��        ��:
**
**
********************************************************************************************************/
#ifndef _BSP_UART_H
#define _BSP_UART_H

//-----------------------����
int16_t BSP_DEBUG_UART_Read(void);
extern void BSPDebugSentB(uint16_t u16TD);
extern void BSPDebugSenString(uint16_t u16TD);


//-----------------------old
extern void BSP_UartInit(void);
extern unsigned int Bsp_IsUartTxIdle(void);
extern void Bsp_UartTxTrigle(void);
//-------------------------
extern void Bsp_Usart_Send(uint8_t *tx_data, uint16_t len);

extern uint8_t uart_buff[];
extern uint8_t uart_rx_idx;
#endif
