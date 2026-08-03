#ifndef __TEST_BY_MODBUS_H__
#define __TEST_BY_MODBUS_H__

enum _regIdx {
    eIDX_CAN = 0,
    eIDX_QF,
    eIDX_DET_VOLT_5V,
    eIDX_DET_VOLT_12V,
    eIDX_DET_VOLT_24V,
    eIDX_BOT_CNT,
    eIDX_FAULT,
    eIDX_TOP_DET,
    eIDX_BOT_DET,
    eIDX_CN_IN,
    eIDX_KEY_STATE,
    eIDX_NUM
};

extern void Test_ByModbus_Poll(void);
extern void Test_ByModbus_Update(UINT16 *date, UINT8 idx, UINT8 len);
extern void Test_ByModbus_Init(void);

#endif
