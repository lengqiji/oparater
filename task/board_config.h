#pragma once

/**
 * @brief 软件版本号
 * @details C:为正式版；D:为调试版
 */
#ifdef DEBUG_ON
    #define SOFT_TYPE (0xD0000000UL)
#else
    #define SOFT_TYPE (0xC0000000UL)
#endif
#define HW_ID       (0x2400000UL)
#define DATE_INFO   (0x251101UL)	//已弃用，测试工装需要
#define RN_SOFT_VER (SOFT_TYPE | HW_ID | (DATE_INFO & 0xfffff))

#define RN_JUMP_FLAG "RNXHJBINm024head"  // 睿能机头升级标志（固定，不能修改升级工具）

//#define NDL_NUM_NEGATIVE 	//断检和针棒号逆序
#define HDAPP_CFG_TYPE       (HEAD_TYPE_FLAT)
