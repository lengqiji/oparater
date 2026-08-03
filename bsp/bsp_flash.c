#include "bsp_flash.h"
#include "bsp_flash_new.h"
#include "hc32_ddl.h"
#include "para.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define BOOT_PARA_ADDRESS (0x8000)
#define APP_CAN_UP_FLAG   ((uint32_t) 0x674E4143)
// 共页，每页字节
#define VER_ADDR            (0x3B00)  // 硬件版本号，loader版本号存储位置
#define FLASH_UP_ADDR       (0x3B00)  // 第16k地址，升级信息
#define FLASH_PAY_ADDR      (0xF400)  // 分期付款地址
#define FLASH_PAY_BACK_ADDR (0xF800)  // 分期付款备份地址
#define FLASH_BOARD_ADDR    (0x7E00)  // 第64k地址，板级信息


/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

flash_up_param_s    flash_up_param;
flash_board_param_s flash_board_param;

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部变量申明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型申明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型申明                             *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : bsp_flash_read
 功能描述  : flash读操作
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2021年11月18日
    作    者   : zhaodeming
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_flash_read(const uint32_t addr, uint8_t *const buf, uint32_t len)
{
    memcpy(buf, (void *)addr, len);
}

/*****************************************************************************
 函 数 名  : bsp_flash_erase
 功能描述  : flash擦除操作
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2021年11月18日
    作    者   : zhaodeming
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_flash_erase(const uint32_t addr)
{
    // EFM_SectorErase(addr);
    efm_EraseSector(addr);
}

/*****************************************************************************
 函 数 名  : bsp_flash_write
 功能描述  : flash写操作
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2021年11月18日
    作    者   : zhaodeming
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_flash_write(const uint32_t addr, uint8_t *const buf, uint32_t len)
{
    // EFM_SequenceProgram(addr, len, buf);
    efm_WriteFlash(addr, buf, len);
}

/*****************************************************************************
 函 数 名  : bsp_flash_init
 功能描述  : flash初始化
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2021年11月18日
    作    者   : zhaodeming
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_flash_init(void)
{
    /* Unlock EFM. */
    EFM_Unlock();

    /* Enable flash. */
    EFM_FlashCmd(Enable);
    /* Wait flash ready. */
    while (Set != EFM_GetFlagStatus(EFM_FLAG_RDY)) {
        ;
    }
}

/*****************************************************************************
 函 数 名  : flash_set_up_param
 功能描述  : 设置参数
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2021年11月18日
    作    者   : zhaodeming
    修改内容   : 新生成函数

*****************************************************************************/
void flash_set_up_param(void)
{
    uint32_t u32Temp = APP_CAN_UP_FLAG;
    //	EFM_Unlock();
    //    EFM_FlashCmd(Enable);
    //    while(Set != EFM_GetFlagStatus(EFM_FLAG_RDY));

    bsp_flash_erase(BOOT_PARA_ADDRESS);
    bsp_flash_write(BOOT_PARA_ADDRESS, (uint8_t *) &u32Temp, 4);

    //	EFM_Lock();
}

/*****************************************************************************
 函 数 名  : flash_get_up_param
 功能描述  : 读取升级标志
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2021年11月18日
    作    者   : zhaodeming
    修改内容   : 新生成函数

*****************************************************************************/
void flash_get_up_param(void)
{
    bsp_flash_read(BOOT_PARA_ADDRESS,
                   (uint8_t *) &flash_up_param,
                   sizeof(flash_up_param_s));
}

/*****************************************************************************
 函 数 名  : flash_set_upflag
 功能描述  : 设置有APP标志
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2021年11月18日
    作    者   : zhaodeming
    修改内容   : 新生成函数

*****************************************************************************/
void flash_set_upflag(uint8_t flag)
{
    flash_get_up_param();
    if (flag) {
        // memset(flash_up_param.up_flag, 0, sizeof(flash_up_param));
        // memcpy(flash_up_param.up_flag, "APP_OK", 6);
        // flash_set_up_param((uint8_t*)&flash_up_param);
    } else {
        bsp_flash_erase(BOOT_PARA_ADDRESS);
    }
}

/*****************************************************************************
 函 数 名  : flash_set_board_param
 功能描述  : 设置参数
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2021年11月18日
    作    者   : zhaodeming
    修改内容   : 新生成函数

*****************************************************************************/
void flash_set_board_param(uint8_t *p)
{
    //    EFM_Unlock();
    //    EFM_FlashCmd(Enable);
    //    while(Set != EFM_GetFlagStatus(EFM_FLAG_RDY));

    //__set_PRIMASK(1);
    //__disable_irq();
    bsp_flash_erase(FLASH_BOARD_ADDR);
    bsp_flash_write(FLASH_BOARD_ADDR, p, sizeof(flash_board_param_s));
    //__enable_irq();
    //__set_PRIMASK(0);

    //    EFM_Lock();
}

/*****************************************************************************
 函 数 名  : flash_get_board_param
 功能描述  : 读取地址
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2021年11月18日
    作    者   : zhaodeming
    修改内容   : 新生成函数

*****************************************************************************/
void flash_get_board_param(void)
{
    uint8_t type_set = 0;
    bsp_flash_read(FLASH_BOARD_ADDR,
                       (uint8_t *) &flash_board_param,
                       sizeof(flash_board_param_s));

    uint32_t uid = EFM_ReadUID().uniqueID1+EFM_ReadUID().uniqueID2-EFM_ReadUID().uniqueID3;
    if(flash_board_param.uid == 0xF1F2F3F4)
    {
        flash_board_param.uid = uid;
        flash_set_board_param((uint8_t*)&flash_board_param);
    }
    else
    {
        // if(flash_board_param.uid != uid)
        //     while(1);//直接不跑了
    }



    /* head type */

	/*type_set = flash_board_param.type & 0xFF;
	if (type_set == JUMP_TYPE_MOTOR)
	{
		head_param.jump_param.type = JUMP_TYPE_MOTOR;
	} else
	{
		head_param.jump_param.type = JUMP_TYPE_MAG;
	}*/


    type_set = (flash_board_param.type>>8) & 0xFF;
    if (type_set == HEAD_TYPE_TAPE)
    {
       Para_head.type = HEAD_TYPE_TAPE;
    } else
    if (type_set == HEAD_TYPE_TUFTING)
    {
       Para_head.type = HEAD_TYPE_TUFTING;
    } else
    {
       Para_head.type = HEAD_TYPE_FLAT;
    }

    /* head addr */
    if (flash_board_param.addr >= 500)
    {
       Para_head.addr = 0;
    }else
    {
       Para_head.addr = flash_board_param.addr;
    }

    Para_head.fac_id = flash_board_param.fac_id;
    Para_head.mach_type = flash_board_param.mach_type;
}

void Bsp_Flash_SetFacId(uint32_t id)
{
    if (flash_board_param.fac_id != id) {
        flash_board_param.fac_id = id;
        Para_head.fac_id         = flash_board_param.fac_id;
        flash_set_board_param((uint8_t *) &flash_board_param);
    }
}

void Bsp_Flash_SetMachType(uint32_t type)
{
    if (flash_board_param.mach_type != type) {
        flash_board_param.mach_type = type;
        Para_head.mach_type         = flash_board_param.mach_type;
        flash_set_board_param((uint8_t *) &flash_board_param);
    }
}
