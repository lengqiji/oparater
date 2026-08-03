#pragma once
#include "bsp_can.h"
// 协议命令
/********************一级命令********************/
// 复位
#define CMD_RST_STRRT  (0x00)  // 机头复位开始
#define CMD_RST_STRRT4 (0xe0)  // 机头复位开始
#define CMD_RST_END1   (0xe0)  // 机头复位结束
#define CMD_RST_END2   (0xF4)  // 机头复位结束
#define CMD_RST_END3   (0x1F)  // 机头复位结束

// 主控绣作类
#define CMD_MAIN    (0xE0)  // 主控绣作
#define CMD_NDL_NUM (0x10)  // 机型针数
#define CMD_NDL_CUR (0x20)  // 当前针数

// 机头绣作类
#define CMD_HEAD_NDL_LEN      (0x0C)  // 当前针长 0.1mm
#define CMD_HEAD_PARAM        (0x1C)  // 机头参数命令
#define CMD_HEAD_MOTOR_ADJUST (0x4C)  // 机头电机角度调整
#define CMD_HEAD_EN           (0x3C)  // 机头工作使能命令
#define CMD_HEAD              (0xFC)  // 机头刺绣动作命令
#define CMD_HEAD_PARAM__MASK  0x1f    // 机头参数屏蔽位
#define CMD_HEAD_STATUS_ID    0x401   // 状态反馈id

#define CMD_UP_ACK_ID 0x400  // 升级id

/********************二级命令********************/
// 主控绣作子命令 一级e0
enum _main {
    CMD_MAIN_NEW_NDL = 0x00,       // 新一针
    CMD_MAIN_FLAT,                 // 平绣头
    CMD_MAIN_TAPE,                 // 盘带头
    CMD_MAIN_NORMAL,               // 正常绣
    CMD_MAIN_PATCH,                // 补绣
    CMD_MAIN_STOP,                 // 05主轴停止
    CMD_MAIN_START,                // 06主轴启动
    CMD_MAIN_BRK_SELFCHECK_START,  // 断线部件自检开始
    CMD_MAIN_BRK_SELFCHECK_END,  // 断线部件自检结束，协议里是步进剪线回原点？

    CMD_MAIN_ZERO = 0x30,  // 主轴在零位
    CMD_MAIN_NO_ZERO,      // 主轴不在零位

    CMD_MAIN_TOWEL = 0x34,  // 毛巾绣头
    CMD_MAIN_LOCK_NO = 0x37,  // 补绣时面线夹持全开：否--不补绣的机头锁住面线夹
    CMD_MAIN_RST_END = 0xFE,  // 机头复位结束
};

typedef enum _mainUpdate {
    CMD_MAIN_UP_START      = 0x22,  // 升级开始
    CMD_MAIN_UP_START_CODE = 0xe5,  // 升级开始代码位置
    CMD_MAIN_UP_CHECK,              // 升级数据校验
    CMD_MAIN_UP_LEN,                // 升级数据长度
    CMD_MAIN_UP_SEQ1,               // 升级序列1
    CMD_MAIN_UP_SEQ2,               // 升级序列2
} CMD_MAIN_UP_E;

typedef enum _mainAddr {
    CMD_MAIN_ADDR_SET   = 0xEA,       // 编址或寻址
    CMD_MAIN_ADDR_START = 0xF6,       // 机头编址开始
    CMD_MAIN_ADDR_END,                // 机头编址结束
    CMD_MAIN_ADDR_LOOK_START = 0xF9,  // 机头寻址开始
    CMD_MAIN_ADDR_LOOK_END,           // 机头寻址结束
} CMD_MAIN_ADDR_E;

// 机头子命令 一级fc
enum _headAct {
    CMD_HEAD_JUMP_LOCK = 0x01,  // 打开跳跃电磁铁
    CMD_HEAD_JUMP_UNLOCK,       // 关闭跳跃电磁铁
    CMD_HEAD_THREAD_UNLOCK,     // 锁线电磁铁关
    CMD_HEAD_THREAD_LOCK,       // 锁线电磁铁开

    CMD_HEAD_BRK_CHECK_START,      // 0x05 断线检测开始
    CMD_HEAD_BRK_CHECK_END,        // 断线检测结束
    CMD_HEAD_BRK_SELFCHECK_START,  // 断线自检开始
    CMD_HEAD_BRK_SELFCHECK_END,    // 断线自检结束

    CMD_HEAD_NORMAL_FLAT = 0x0b,  // 正常绣平针
    CMD_HEAD_NORMAL_JUMP,         // 正常绣跳针
    CMD_HEAD_PATCH_FLAT,          // 补绣平针
    CMD_HEAD_PATCH_JUMP,          // 补绣跳针

    CMD_HEAD_JUMP,  // 0f停车锁针

    CMD_HEAD_BRK_CHECK = 0x15,  // 断线检测开始
    CMD_HEAD_NDL_DOWN  = 0x16,  // 针停下位
};

// 机头参数子命令 一级1c
enum _headParam {
    CMD_HEAD_PARAM_MAG_POWER        = 0x00,  // 跳跃电磁铁电压
    CMD_HEAD_PARAM_START_FILTER_NDL = 0x20,  // 起针不检测断线针数
    CMD_HEAD_PARAM_BRK_CHECK_TYPE   = 0x40,  // 断线检测类型
    CMD_HEAD_PARAM_TOP_SENSE        = 0x60,  // 面检灵敏度
    CMD_HEAD_PARAM_BOT_SENSE        = 0x80,  // 底检灵敏度
    CMD_HEAD_PARAM_TOP_FILTER_NDL   = 0xA0,  // 面检去抖动针数
    CMD_HEAD_PARAM_BOT_FILTER_NDL   = 0xC0,  // 底检去抖动针数
    CMD_HEAD_PARAM_LOCK_POWER       = 0xE0,  // 锁线电磁铁电压
};

/**********自主添加扩展帧命令**********/

// 机头扩展帧id
enum _extId {
    CANID_CMD_HEAD_AUTO_ADDR_START = 0xc00,  // 机头自动编址开始
    CANID_CMD_HEAD_AUTO_ADDR_END,            // 机头自动编址结束
    CANID_CMD_HEAD_AUTO_ADDR_RSP,            // 机头自动编址
    CANID_CMD_HEAD_CLEAR_ALARM,              // 清除机头报警
    CANID_CMD_HEAD_C04 = 0xC04,              // 独立压脚针停下位C04

    CANID_CMD_HEAD_PARAM_PWR = 0xc10,  // 设置电压报警比例
    // CANID_CMD_HEAD_PARAM_24V, //设置24V报警比例
    CANID_CMD_HEAD_PARAM_JUMP_TYPE,     // 设置跳跃类型
    CANID_CMD_HEAD_PARAM_LOCK_PARA,     // 设置面线夹参数
    CANID_CMD_HEAD_PARAM_SMT_PARA,      // 设置电机参数
    CANID_CMD_HEAD_PARAM_COMBINE_PARA,  // 设置组合头参数

    CANID_CMD_HEAD_FOOT_PARAM = 0xC16,  // 压脚参数
    CANID_CMD_HEAD_FOOT_ANGLE = 0xC17,  // 压脚角度参数
    CANID_CMD_HEAD_FOOT_HOOK  = 0xC18,  // 压脚勾线高度

    CMDID_CMD_HEAD_STATUS = 0xC19,  // 机头状态

    CANID_CMD_HEAD_FOOT_LOC = 0xC20,  // 查询当前压脚位置

    CANID_CMD_HEAD_VALVE_JUMP_S     = 0xC21,  // 查询气动跳跃状态
    CANID_CMD_HEAD_SEND_TRIM        = 0XC25,  // 机头通知剪线
    CANID_CMD_HEAD_TYPE             = 0xC29,  // 机头类型
    CANID_CMD_HEAD_FOOT_TEST_MODE   = 0xC2D,  // 压脚测试
    CANID_CMD_HEAD_MANUAL_TRIM      = 0xC2E,  // 主控通知手动剪线状态
    CAN_ID_CMD_HEAD_PARAM_FOOT_CTRL = 0xCE1,  // 命令跳跃气阀

    CAN_CMD_HEAD_EN_G1 = 0xC30,  // 使能机头组1(1-64)
    CAN_CMD_HEAD_EN_G2,          // 使能机头组2(65-128)
    CAN_CMD_HEAD_EN_G3,          // 使能机头组3(129-192)
    CAN_CMD_HEAD_EN_G4,          // 使能机头组4(193-256)
    CAN_CMD_HEAD_EN_G5,          // 使能机头组5(257-320)

    CANID_CMD_FACTORY_ID = 0xCF1, //厂家ID

    CANID_CMD_HEAD_SEND_ALARM = 0xD00,  // 机头上传报警信息

    CAN_CMD_PAY_CMDTYPE   = 0xE00,  // 分期付款命令类型
    CAN_CMD_PAY_DATALEN   = 0xE01,  // 分期付款数据长度
    CAN_CMD_PAY_DATASTART = 0xE02,  // 分期付款数据长度起始
    CAN_CMD_PAY_DATAEND   = 0xE0F,  // 分期付款数据长度最大id

    CAN_CMD_HEAD_READ_VERSION = 0xFE0,  // 读取机头版本信息
    CAN_CMD_HEAD_READ_RECORD  = 0xFE1,  // 读取机头统计信息
    CAN_CMD_HEAD_RSP_RECORD   = 0xFE2,  // 回复机头统计信息
    CAN_CMD_HEAD_RSP_VERSION  = 0xFF0,  // 机头板回复版本信息

    CANID_CMD_HEAD_HOOK_SET    = 0x1500,  // 勾线电机设置
    CANID_CMD_HEAD_HOOK_UP_STA = 0x1501,  // 勾线电机回复状态
    CANID_CMD_HEAD_HOOK_STA    = 0x1700,
    // CANID_CMD_HEAD_SEND_RST = 0xe00,		//机头上传复位信息
    CANID_CMD_TAPE_MOTOR_LOGIC_DIR = 0x1D06, /* 盘带跳跃电机逻辑方向 */

    CANID_CMD_HEAD_BRUSH = 0x4581,  // 画笔功能

    CAN_CMD_MAINSHAFT_SPEED = 0xE0E0,  // 主轴速度广播

    CANID_CMD_HEAD_HEART     = 0x4000000,  // 心跳包
    CANID_CMD_HEAD_RSP_HEART = 0x4000001,  // 心跳包回复
    CANID_FACTORY_TEST       = 0x1F900000,  // 出厂测试
    CANID_RSP_FACTORY_TEST   = 0x1F900001,  // 出厂测试回复
};

// 0xC14子命令
typedef enum _paramCombine {
    C14_COMBINE_HEAD = 0x01,  // 组合头
    C14_SPLICE_HEAD,          // 拼接机头
    C14_INDEP_CHECK,          // 独立断检
    C14_SPLICE_CH_NUM,  // 拼接报警板通道数：1-32；（代表有多少个报警通道数组合）
    C14_BRK_GREEN_OFF,  // 断线是未断线的机头绿灯关闭
    C14_SGL_NDL_TYPE,   // 单针类型
    C14_DOUBLE_HEAD_MODE = 0x07,  // 双针断检组合模式(4+3；3+2)
    C14_NDL_NUM_REVERSE,		// 针棒逆序
    C14_JUMP_MOTOR_DEFAULT_STEP =  0x0A,// 跳跃电机默认行程
    C14_NDL_NUM_MODE =  0x0B,//底线检测方式，模式 Oxaa Oxbb           Oxaa底检方式值<br/>Oxbb 底检模式值
} CMD_HEAD_PARAM_C14_SCMD;

// 机头扩展帧参数电压子命令
enum _paramVoltAlarm {
    CMD_HEAD_PARAM_5V_ALARM = 0x01,  // 5V报警电压
    CMD_HEAD_PARAM_24V_ALARM,        // 24V报警电压
};

// 机头扩展帧参数子命令
enum _paramJumpType {
    CMD_HEAD_PARAM_JUMP_MAG = 0x01,  // 电磁铁跳跃
    CMD_HEAD_PARAM_JUMP_MOTOR,       // 步进跳跃
};

// 机头扩展帧参数子命令
enum _paramLock {
    CMD_HEAD_PARAM_LOCK_TYPE = 0x01,  // 面线夹类型
    CMD_HEAD_PARAM_LOCK_DUTY,         // 面线夹占空比
    CMD_HEAD_PARAM_LOCK_DIR,          // 面线夹方向
};

// 机头扩展帧参数子命令
enum _paramMotor {
    CMD_HEAD_PARAM_MOTOR_CUR_JUMP = 0x01,  // 跳跃电机电流
    CMD_HEAD_PARAM_MOTOR_CUR_HOOK,         // 勾线电机电流
    CMD_HEAD_PARAM_MOTOR_LOGIC_DIR,        // 电机逻辑方向
    CMD_HEAD_PARAM_FOOT_COM_CFG            // 集中式压脚参数
};

// 机头子命令
enum _alarmClr {
    CMD_HEAD_CLEAR_ALARM = 0x01,  // 清除报警
};

enum
{//CAN_ID_CMD_HEAD_PARAM_FOOT_CTRL 子命令
    FOOT_TO_A = 0x01,       //压脚运行到A点
    FOOT_TO_B,              //压脚运行到B点
    FOOT_TO_D,              //压脚运行到D点
    FOOT_JUMP_ON,           //压脚动作时打开跳跃
    FOOT_JUMP_OFF,          //压脚动作时关闭跳跃
    FOOT_TO_HOOK,           //压脚运行到勾线位置
    FOOT_VALVE_ON,          //打开跳跃气阀
    FOOT_VALVE_OFF,         //关闭跳跃气阀
    FOOT_TO_STANDBY,        //压脚运行到待机位置
    FOOT_JUMP_UNLOCK = 0x0a, //换色前跳跃电机解锁
    FOOT_JUMP_RST = 0x0b,    //换色结束，跳跃电机恢复换色前状态
	FOOT_MAG_ON,			//压脚电磁铁打开
	FOOT_MAG_OFF,			//压脚电磁铁关闭
    FOOT_FIND_ZERO = 0x20,  //独立压脚找零
};

enum _infoReport {
    eERR_PC = 0x01,  // PC指针错误
    eERR_SYS_RST,  // 系统复位原因：bit0上电复位，bit1核心掉电，bit2掉电，bit4外部复位，bit5看门狗复位，bit6系统复位
    eSTATUS_KEY_POS,  // 机头开关状态（变化时主动上报）,0x0：机头中位,0x1：机头上位,0x2：机头下位
    eSTATUS_FOOT_LOC,  // 独立压脚当前位置状态,0x1：A位置,0x2：B位置,0x3：D位置.0x6：C位置
    eSTATUS_VALVE_JUMP,  // 气动跳跃状态,1：idle；0：busy；
    eSTATUS_HEAD,  // 机头状态(0:初始化,1:正常,2:补绣,3:停车,4:机头关闭,5:测断线)
    eSTATUS_KEY,   // 机头按键状态
    eSTATUS_LOCK_MOTOR,  // 电机锁线零位状态,2：找零出错；1：找零成功；0：找零未完成；
    eHEAD_ALARM = 0xFA
};

enum _factoryTest {
    eFACTORY_TEST_BEGIN = 0,
    eFACTORY_TEST_END,
    eFACTORY_TEST_GET_INPUT,
    eFACTORY_TEST_SET_OUTPUT,
    eFACTORY_TEST_GET_ADC_VAL,
    eFACTORY_TEST_SET_VOLT,
    eFACTORY_TEST_M_ENCODE_BEGIN,
    eFACTORY_TEST_M_ENCODE_END,
    eFACTORY_TEST_M_RUN_CW,
    eFACTORY_TEST_M_RUN_CCW,
    eFACTORY_TEST_GET_CLOSE_M_INFO,
    eFACTORY_TEST_QF_BEGIN,
    eFACTORY_TEST_QF_END,
    eFACTORY_TEST_I2C_COMM,
    eFACTORY_TEST_USART_COMM,
    eFACTORY_TEST_GET_SOFT_VER,
    eFACTORY_TEST_USART_RECV_DATA
};
//0xC29机头类型
//0:平绣 1:大毛巾 2:激光 3:烫钻 4:热切割 5:雕孔 6:盘带1 7:盘带2 8:植绒绣
typedef enum
{
	HDTYPE_FLAT = 0x00,		//平绣
	HDTYPE_TOWEL,			//毛巾
	HDTYPE_LASER,			//激光
	HDTYPE_RSTONE,			//烫钻
	HDTYPE_HOTCUT,			//热切割
	HDTYPE_PHOLE,			//雕孔
	HDTYPE_TAPE1,			//盘带1
	HDTYPE_TAPE2,			//盘带2
	HDTYPE_TUFTING,			//植绒绣
	HDTYPE_INDHOLE,			//独立雕孔
}CMD_HEAD_TYPE_C29;
