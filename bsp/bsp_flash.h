/******************************************************************************

                  版权所有 (C), 2021-2031, 睿能科技股份有限公司

 ******************************************************************************
  文 件 名   : bsp_flash.h
  版 本 号   : 初稿
  作    者   : zhaodeming
  生成日期   : 2021年9月15日
  最近修改   :
  功能描述   : bsp_flash.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2021年11月18日
    作    者   : zhaodeming
    修改内容   : 创建文件

******************************************************************************/

#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#include <stdint.h>
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/


//升级信息存储到flash的参数
typedef struct
{
	uint8_t up_flag[8];//升级标志 "APP_OK"
}flash_up_param_s;

//机头板信息存储到flash的参数
typedef struct
{
    uint16_t addr;//机头地址
    uint16_t type;//机头地址
    uint32_t uid;//芯片id
    uint32_t fac_id;
    uint8_t  mach_type;
}flash_board_param_s;

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部变量申明                                 *
 *----------------------------------------------*/
extern flash_board_param_s flash_board_param;
extern flash_up_param_s flash_up_param;

/*----------------------------------------------*
 * 外部函数原型申明                             *
 *----------------------------------------------*/
extern void bsp_flash_init(void);
extern void bsp_flash_erase(const uint32_t addr);
extern void bsp_flash_read(const uint32_t addr, uint8_t *const buf, uint32_t len);
extern void bsp_flash_write(const uint32_t addr, uint8_t *const buf, uint32_t len);
extern void flash_set_up_param(void);
extern void flash_set_upflag(uint8_t flag);
extern void flash_set_board_param(uint8_t *p);
extern void flash_get_board_param(void);
extern void Bsp_Flash_SetFacId(uint32_t id);
extern void Bsp_Flash_SetMachType(uint32_t type);
/*----------------------------------------------*
 * 内部函数原型申明                             *
 *----------------------------------------------*/

#endif
