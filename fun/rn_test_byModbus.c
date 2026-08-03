#include "rn_typedef.h"

#include "bsp_uart.h"
#include "rn_test_byModbus.h"

void (*ModubsSend)(uint8_t *tx_data, uint16_t len) = Bsp_Usart_Send;

#define MB_REG_SIZE      40
#define MODBUS_BUFF_SIZE 32
/* ----------------------- Defines ------------------------------------------*/
#define MB_ADDRESS_BROADCAST                 (0) /*! Modbus broadcast address. */
#define MB_ADDRESS_MIN                       (1) /*! Smallest possible slave address. */
#define MB_ADDRESS_MAX                       (247) /*! Biggest possible slave address. */
#define MB_FUNC_NONE                         (0)
#define MB_FUNC_READ_COILS                   (1)
#define MB_FUNC_READ_DISCRETE_INPUTS         (2)
#define MB_FUNC_WRITE_SINGLE_COIL            (5)
#define MB_FUNC_WRITE_MULTIPLE_COILS         (15)
#define MB_FUNC_READ_HOLDING_REGISTER        (3)
#define MB_FUNC_READ_INPUT_REGISTER          (4)
#define MB_FUNC_WRITE_REGISTER               (6)
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS     (16)
#define MB_FUNC_READWRITE_MULTIPLE_REGISTERS (23)
#define MB_FUNC_DIAG_READ_EXCEPTION          (7)
#define MB_FUNC_DIAG_DIAGNOSTIC              (8)
#define MB_FUNC_DIAG_GET_COM_EVENT_CNT       (11)
#define MB_FUNC_DIAG_GET_COM_EVENT_LOG       (12)
#define MB_FUNC_OTHER_REPORT_SLAVEID         (17)
#define MB_FUNC_ERROR                        (128)

/* ----------------------- Type definitions ---------------------------------*/
typedef enum {
    MB_EX_NONE                 = 0x00,
    MB_EX_ILLEGAL_FUNCTION     = 0x01,
    MB_EX_ILLEGAL_DATA_ADDRESS = 0x02,
    MB_EX_ILLEGAL_DATA_VALUE   = 0x03,
    MB_EX_SLAVE_DEVICE_FAILURE = 0x04,
    MB_EX_ACKNOWLEDGE          = 0x05,
    MB_EX_SLAVE_BUSY           = 0x06,
    MB_EX_MEMORY_PARITY_ERROR  = 0x08,
    MB_EX_GATEWAY_PATH_FAILED  = 0x0A,
    MB_EX_GATEWAY_TGT_FAILED   = 0x0B
} eMBException;

typedef enum _frameGet {
    eFRAME_GET_IDLE = 0,
    eFRAME_GET_BUSY,
    eFRAME_GET_READY
} FrameGet_e;
// 字地址 0 - 255 (只取低8位)
// 位地址 0 - 255 (只取低8位)

/* CRC 高位字节值表 */
const UINT8 auchCRCHi[]
    = {0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
       0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
       0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
       0x00, 0xC1, 0x81, 0x40};
/* CRC低位字节值表*/
const UINT8 auchCRCLo[]
    = {0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
       0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
       0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
       0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
       0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
       0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
       0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
       0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
       0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
       0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
       0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
       0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
       0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
       0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
       0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
       0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
       0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
       0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
       0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
       0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
       0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
       0x41, 0x81, 0x80, 0x40};

const char RELEASEDATE[16] = __DATE__;
const char RELEASETIME[16] = __TIME__;

UINT8  mbcoil             = 0;
UINT16 mbreg[MB_REG_SIZE] = {0};
UINT8  addr               = 1;
UINT16 mbreadaddr         = 0;
UINT16 mbreadsize         = 0;
UINT16 crc;

unsigned char modbusBuff[MODBUS_BUFF_SIZE];
unsigned char modbusBuffIdx  = 0;
FrameGet_e    modbusFrameGet = eFRAME_GET_IDLE;

static inline UINT16 crc16(UINT8 *puchMsg, UINT8 usDataLen)
{
    UINT8 uchCRCHi = 0xFF; /* 高CRC字节初始化 */
    UINT8 uchCRCLo = 0xFF; /* 低CRC 字节初始化 */
    UINT8 uIndex;          /* CRC循环中的索引 */
    while (usDataLen--)    /* 传输消息缓冲区 */
    {
        uIndex   = uchCRCHi ^ *puchMsg++; /* 计算CRC */
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex];
    }
    return (uchCRCHi << 8 | uchCRCLo);
}

void Test_ByModbus_Init(void)
{
    char *str;
    // 软件版本
    // p = (char*)0x00000320;    // 引导程序名字
    // memcpy(&mbreg[8],p,16);
    // p = (char*)0x00000310;    // 引导程序日期
    // memcpy(&mbreg[16],p,16);
    // str = (char *) ProgName;  // 应用程序名字
    // memcpy(&mbreg[24], str, 16);
    str = (char *) RELEASEDATE;  // 应用程序日期
    memcpy(&mbreg[32], str, 16);
}

void Test_ByModbus_Update(UINT16 *date, UINT8 idx, UINT8 len)
{
    UINT16 *pReg = &mbreg[idx];
    while (len--) {
        *pReg++ = *date++;
    }
}

UINT8
MBFuncReadCoils(void)
{
    UINT8 err = MB_EX_NONE;
    if ((mbreadaddr >= eIDX_NUM) || ((mbreadaddr + mbreadsize) > eIDX_NUM)) {
        return (MB_EX_ILLEGAL_DATA_ADDRESS);
    }
    modbusBuff[0] = addr;
    modbusBuff[1] = MB_FUNC_READ_COILS;
    modbusBuff[2] = 1; /* 字节计数*/
    modbusBuff[3] = mbcoil >> mbreadaddr;
    crc           = crc16(modbusBuff, 4);
    modbusBuff[4] = crc >> 8;
    modbusBuff[5] = crc & 0xFF;

    modbusBuffIdx  = 1;
    modbusFrameGet = eFRAME_GET_READY;

    ModubsSend(modbusBuff, 6);

    return (err);
}

UINT8
MBFuncReadHoldingReg(void)
{
    UINT8 i, j;
    UINT8 err = MB_EX_NONE;
    if ((mbreadaddr >= MB_REG_SIZE)
        || (mbreadaddr + mbreadsize > MB_REG_SIZE)) {
        return (MB_EX_ILLEGAL_DATA_ADDRESS);
    }
    if (mbreadsize > eIDX_NUM) {
        return (MB_EX_ILLEGAL_DATA_ADDRESS);  // modbusBuff限长
    }

    modbusBuff[0] = addr;
    modbusBuff[1] = MB_FUNC_READ_HOLDING_REGISTER;
    modbusBuff[2] = mbreadsize * 2; /* 字节计数*/
    j             = 3;
    for (i = 0; i < mbreadsize; i++) {
        modbusBuff[j++] = mbreg[mbreadaddr] >> 8;
        modbusBuff[j++] = mbreg[mbreadaddr] & 0xFF;
        mbreadaddr++;
    }
    crc             = crc16(&modbusBuff[0], j);
    modbusBuff[j++] = crc >> 8;
    modbusBuff[j++] = crc & 0xFF;

    modbusBuffIdx  = 1;
    modbusFrameGet = eFRAME_GET_READY;

    ModubsSend(modbusBuff, j);

    return (err);
}

UINT8
MBFuncWriteMultiReg(void)
{
    UINT8 i, j, k;
    UINT8 err = MB_EX_NONE;
    if ((mbreadaddr >= MB_REG_SIZE)
        || (mbreadaddr + mbreadsize > MB_REG_SIZE)) {
        return (MB_EX_ILLEGAL_DATA_ADDRESS);
    }

    i = mbreadaddr;
    j = mbreadsize;
    k = eIDX_NUM - 1;
    while (j) {
        mbreg[i] = modbusBuff[k] * 0x100 + modbusBuff[k + 1];
        i++;
        k = k + 2;
        j--;
    }

    modbusBuff[0] = addr;
    modbusBuff[1] = MB_FUNC_WRITE_MULTIPLE_REGISTERS;
    modbusBuff[2] = mbreadaddr >> 8;
    modbusBuff[3] = mbreadaddr;
    modbusBuff[4] = mbreadsize >> 8;
    modbusBuff[5] = mbreadsize;
    crc           = crc16(&modbusBuff[0], 6);
    modbusBuff[6] = crc >> 8;
    modbusBuff[7] = crc & 0xFF;

    modbusBuffIdx  = 1;
    modbusFrameGet = eFRAME_GET_READY;

    ModubsSend(modbusBuff, 8);

    return (err);
}

void Test_ByModbus_Poll(void)
{
    static UINT8 _idx = 0, _cnt = 0;

    UINT8 err = MB_EX_NONE;
    if (_idx != uart_rx_idx) {
        _idx = uart_rx_idx;
        _cnt = 0;
    } else {
        if (++_cnt > 3) {
            if (uart_rx_idx >= 4) {
                memcpy(modbusBuff, uart_buff, uart_rx_idx);
                modbusBuffIdx  = uart_rx_idx;
                modbusFrameGet = eFRAME_GET_BUSY;
            }
            uart_rx_idx = 0;
            _idx        = 0;
        }
    }
    if (modbusFrameGet == eFRAME_GET_BUSY) {
        if (addr != modbusBuff[0]) {
            modbusBuffIdx  = 0;
            modbusFrameGet = eFRAME_GET_IDLE;
            return;
        }

        crc = crc16(&modbusBuff[0], modbusBuffIdx - 2);
        if (crc
            == ((modbusBuff[modbusBuffIdx - 1] & 0xFF)
                + (modbusBuff[modbusBuffIdx - 2] * 0x100))) {
            mbreadaddr = (modbusBuff[3] & 0xFF) + (modbusBuff[2] * 0x100);
            mbreadsize = (modbusBuff[5] & 0xFF) + (modbusBuff[4] * 0x100);
            switch (modbusBuff[1]) {
                case MB_FUNC_READ_COILS:
                    err = MBFuncReadCoils();
                    break;
                case MB_FUNC_READ_HOLDING_REGISTER:
                    err = MBFuncReadHoldingReg();
                    break;
                case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
                    err = MBFuncWriteMultiReg();
                    break;
                default:
                    err = MB_EX_ILLEGAL_FUNCTION;
                    break;
            }
            if (err != MB_EX_NONE) {
                modbusBuff[0] = addr;
                modbusBuff[1] |= 0x80;
                modbusBuff[2] = err;
                crc           = crc16(modbusBuff, 3);
                modbusBuff[3] = crc >> 8;
                modbusBuff[4] = crc & 0xFF;

                modbusBuffIdx  = 1;
                modbusFrameGet = eFRAME_GET_READY;

                ModubsSend(modbusBuff, 5);
            }
        } else {
            modbusBuffIdx  = 0;
            modbusFrameGet = eFRAME_GET_IDLE;
        }
    }
}
