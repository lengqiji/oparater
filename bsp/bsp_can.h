#ifndef __BSP_CAN_H__
#define __BSP_CAN_H__

#include <stdint.h>
//帧格式
enum can_format 
{
	CAN_FMT_STDID,		//标准帧，11位id
	CAN_FMT_EXTID		//扩展帧，29位id
};

//消息帧结构体
struct can_message 
{
	uint32_t id;		//
	uint8_t data[8];	//数据
	uint8_t len;		//数据长度
	enum can_format fmt;//帧格式
};
extern void Bsp_Can_Init(void);
extern void Bsp_Can_Send(uint32_t id, uint8_t* send_data, uint8_t len);
extern void Bsp_Can_SetRxCallBack(void (*callback)(struct can_message*));

#endif


