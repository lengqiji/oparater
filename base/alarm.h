#pragma once

// 报警位
enum _alarmBit {
	ALARM_BIT_5V = 0x00,			//5v报警
	ALARM_BIT_24V,					//24v报警
	ALARM_BIT_JUMP,					//步进报警
	ALARM_BIT_MAG,					//跳跃报警
	ALARM_BIT_LOCK,					//锁线报警
	ALARM_BIT_NO_QF,				//无四倍频报警
	ALARM_BIT_QF_ERR,				//四倍频错误报警，如一圈8000个脉冲等
	ALARM_BIT_NO_BOT = 7,			//无底检
	ALARM_BIT_HOOK,					//勾线步进报警
	ALARM_BIT_PAY,					//分期付款到期报警
	ALARM_BIT_SPM_ZERO_ERR,			//送片电机找零失败
	ALARM_BIT_VALVE,				//气阀报警
	ALARM_BIT_QEP_FLT,				//QEP_FLT报警
	ALARM_BIT_OVER_CURRENT,			//闭环电机过流报警
	ALARM_BIT_ADC_OFT_ERR,			//闭环电机ADC偏置过大报警
	ALARM_BIT_MOTOR_ZERO_ERR = 15,	//闭环电机找零失败报警
	ALARM_BIT_OVER_LOAD,			//闭环电机过载报警
	ALARM_BIT_POSITION_CTR_ERR,		//闭环电机位置超差报警
	ALARM_BIT_FOOT_RUN_ERR,			//压脚电机运行出错
	ALARM_BIT_AIR_PRESSURE_ERR,		//气压报警

};



extern void Alarm_Set(int loc);

extern uint Alarm_GetAll(void);

extern void Alarm_Clr(int loc);

extern void Alarm_ClrAll(void);

