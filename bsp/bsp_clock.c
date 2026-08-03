#include "hc32_ddl.h"


void Bsp_Clock_Init(void)
{
    stc_clk_sysclk_cfg_t stcSysClkCfg;
    stc_clk_xtal_cfg_t   stcXtalCfg; /* 配置外部 Xtal */
    stc_clk_mpll_cfg_t   stcMpllCfg;
    stc_sram_config_t    stcSramConfig;
    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);
    MEM_ZERO_STRUCT(stcSramConfig);
    /* Set bus clk div. */
    stcSysClkCfg.enHclkDiv = ClkSysclkDiv1;  // 当前 200MHz，最高频率 200MHz
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;  // 当前 100MHz，最高频率 100MHz
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;  // 当前 200MHz，最高频率 200MHz
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;  // 当前 100MHz，最高频率 100MHz
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;  // 当前 50MHz， 最高频率 60MHz
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;  // 当前 50MHz， 最高频率 50MHz
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;  // 当前 100MHz，最高频率 100MHz
    CLK_SysClkConfig(&stcSysClkCfg);
    // XTAL的配置
    stcXtalCfg.enMode = ClkXtalModeOsc;  // 使用外部振荡器模式
    stcXtalCfg.enDrv  = ClkXtalMidDrv;   // 中驱动能力(16-20MHz晶振)
    stcXtalCfg.enFastStartup = Enable;   // 允许超高速驱动
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* 配置 HRC 经过 PLL 后作为系统时钟，而不是直接使用 HRC（HRC
     * 是可以直接作为系统时钟） */

    /* 1. 启动 HRC， MCU 启动后默认以 MRC 来工作，以下开始切换到 HRC */
    // CLK_HrcCmd(Enable);

    /* 根据手册，需要等待 HRC Ready 后才可以正常使用 */
    // while(Set != CLK_GetFlagStatus(ClkFlagHRCRdy));

    /* 2. 设置 PLL 的时钟源为 XTAL */
    CLK_SetPllSource(ClkPllSrcXTAL);
    /* 3. MPLL config (主晶振 / pllmDiv * plln / PllpDiv = 128M). */
#if 0
    stcMpllCfg.pllmDiv = 16ul;
    stcMpllCfg.plln    = 256ul;
    stcMpllCfg.PllpDiv = 2ul;
    stcMpllCfg.PllqDiv = 8ul;
    stcMpllCfg.PllrDiv = 2ul;
#endif
    // 16/2*50/2 = 200MHz
    stcMpllCfg.pllmDiv = 2ul;
    stcMpllCfg.plln    = 50ul;
    stcMpllCfg.PllpDiv = 2ul;
    stcMpllCfg.PllqDiv = 8ul;
    stcMpllCfg.PllrDiv = 2ul;
    CLK_MpllConfig(&stcMpllCfg);
    /* Enable MPLL. */
    CLK_MpllCmd(Enable);
    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_3);
    EFM_Lock();
    /* sram init include read/write wait cycle setting */
    stcSramConfig.u8SramIdx     = Sram12Idx | Sram3Idx | SramHsIdx | SramRetIdx;
    stcSramConfig.enSramRC      = SramCycle2;
    stcSramConfig.enSramWC      = SramCycle2;
    stcSramConfig.enSramEccMode = EccMode3;
    stcSramConfig.enSramEccOp   = SramNmi;
    stcSramConfig.enSramPyOp    = SramNmi;
    SRAM_Init(&stcSramConfig);
    /* Wait MPLL ready. */
    while (Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
        ;
    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);
    SystemCoreClockUpdate();
}


