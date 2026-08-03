#include "bsp_can.h"
#include "hc32_ddl.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define CAN_FILTERS_COUNT (1u)
#define CAN_COMIT         (0)
#define CAN_IRQ_PRIORITY  (6)  // 6 低于motor 的 5

#define CAN_TX_PORT PortB
#define CAN_TX_PIN  Pin12
#define CAN_RX_PORT PortB
#define CAN_RX_PIN  Pin13

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
static stc_can_rxframe_t stcRxFrame;
void (*CanRx_Callback)(struct can_message *);  // 动作结束回调

extern uint32_t CAN_IrqFlgGetAll(void);
extern uint32_t CAN_IrqFlgClrALL(uint32_t flg);

static void cmd_recv_null(struct can_message *msg) {}

static void can_rx_callback(void)
{
    struct can_message     msg;
    en_can_rx_buf_status_t rx_buf_status_t = CanRxBufEmpty;
    uint8_t                repeat          = 0;
    uint32_t               irq_flag        = CAN_IrqFlgGetAll();

    if (true == CAN_IrqFlgGet(CanRxIrqFlg)) {
REPEAT:
        rx_buf_status_t = CAN_Receive(&stcRxFrame);
        if (rx_buf_status_t == CanRxBufEmpty) {
            repeat = 0;
        } else {
            repeat++;
        }

        if (stcRxFrame.Cst.Control_f.IDE) {
            msg.fmt = CAN_FMT_EXTID;
            msg.id  = stcRxFrame.ExtID;
        } else {
            msg.fmt = CAN_FMT_STDID;
            msg.id  = stcRxFrame.StdID;
        }

        msg.len = stcRxFrame.Cst.Control_f.DLC;
        memcpy(msg.data, stcRxFrame.Data, msg.len);

        CanRx_Callback(&msg);

        if (repeat > 0 && repeat < 5) {
            goto REPEAT;
        }

        // CAN_IrqFlgClr(CanRxIrqFlg);
    }

    // if (true == CAN_IrqFlgGet(CanTxPrimaryIrqFlg)) {
    //     // CAN_IrqFlgClr(CanTxPrimaryIrqFlg);
    //     // CAN_IrqCmd(CanRxIrqEn, Enable);
    // }

    // if (true == CAN_IrqFlgGet(CanRxOverIrqFlg)) {
    //     shell_printf("I1\r\n");
    // }

    // if (true == CAN_IrqFlgGet(CanRxBufFullIrqFlg)) {
    //     shell_printf("I2\r\n");
    // }

    // if (true == CAN_IrqFlgGet(CanRxBufAlmostFullIrqFlg)) {
    //     shell_printf("I3\r\n");
    // }

    // if (true == CAN_IrqFlgGet(CanErrorIrqFlg)) {
    //     shell_printf("I4\r\n");
    // }

    // if (true == CAN_IrqFlgGet(CanAbortIrqFlg)) {
    //     shell_printf("I5\r\n");
    // }
    CAN_IrqFlgClrALL(irq_flag);
}

void Bsp_Can_Init(void)
{
    CanRx_Callback = cmd_recv_null;
    stc_can_init_config_t stcCanInitCfg;
    stc_irq_regi_conf_t   stcIrqRegiConf;

    stc_can_filter_t astcFilters[CAN_FILTERS_COUNT] = {
        {0x00000000ul, 0x1FFFFFFFul, CanFilterSel1, CanAllFrames}
    };

    //<< Enable can peripheral clock and buffer(ram)
    PWC_RamOpMdConfig(HighSpeedMd);
    PWC_RamPwrdownCmd(PWC_RAMPWRDOWN_CAN, Enable);
    PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_CAN, Enable);

    //<< CAN GPIO config
    PORT_SetFunc(CAN_TX_PORT, CAN_TX_PIN, Func_Can1_Tx, Disable);
    PORT_SetFunc(CAN_RX_PORT, CAN_RX_PIN, Func_Can1_Rx, Disable);
    // PORT_ResetBits(PortD, Pin15);
    // PORT_OE(PortD, Pin15, Enable);

    MEM_ZERO_STRUCT(stcCanInitCfg);
    //<< Can bit time config
    // 250KHz 采样点87.5%    14/16
    stcCanInitCfg.stcCanBt.PRESC = 4u - 1u;
    stcCanInitCfg.stcCanBt.SEG_1 = 14u - 2u;
    stcCanInitCfg.stcCanBt.SEG_2 = 2u - 1u;
    stcCanInitCfg.stcCanBt.SJW   = 2u - 1u;

    stcCanInitCfg.stcWarningLimit.CanErrorWarningLimitVal = 10u;
    stcCanInitCfg.stcWarningLimit.CanWarningLimitVal      = 16u - 1u;

    stcCanInitCfg.enCanRxBufAll  = CanRxNormal;
    stcCanInitCfg.enCanRxBufMode = CanRxBufOverwritten;  // CanRxBufNotStored;
    stcCanInitCfg.enCanSAck      = CanSelfAckEnable;
    stcCanInitCfg.enCanSTBMode   = CanSTBFifoMode;

    stcCanInitCfg.pstcFilter    = astcFilters;
    stcCanInitCfg.u8FilterCount = CAN_FILTERS_COUNT;

    CAN_Init(&stcCanInitCfg);

    stcIrqRegiConf.enIRQn      = Int000_IRQn;
    stcIrqRegiConf.enIntSrc    = INT_CAN_INT;
    stcIrqRegiConf.pfnCallback = &can_rx_callback;
    enIrqRegistration(&stcIrqRegiConf);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, CAN_IRQ_PRIORITY);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

}

void Bsp_Can_Send(uint32_t id, uint8_t *send_data, uint8_t len)
{
    // 帧格式

    enum can_format fmt;
    // uint8_t u8Idx = 0u;
    stc_can_txframe_t stcTxFrame = {0};

    if (id > 0x7FF)
        fmt = CAN_FMT_EXTID;  // 扩展帧，29位id
    else
        fmt = CAN_FMT_STDID;  // 标准帧，11位id

    if (fmt == CAN_FMT_STDID) {
        stcTxFrame.StdID = id;
    } else {
        stcTxFrame.ExtID         = id;
        stcTxFrame.Control_f.IDE = 1;
    }

    stcTxFrame.Control_f.DLC = len;
    // stcTxFrame.enBufferSel = CanSTBSel;

    memcpy(stcTxFrame.Data, send_data, len);

#if (CAN_COMIT)
    CAN_SetFrame(&stcTxFrame);
    CAN_TransmitCmd(CanPTBTxCmd);
#else
    stcTxFrame.enBufferSel = CanSTBSel;
    CAN_SetFrame(&stcTxFrame);
    CAN_TransmitCmd(CanSTBTxAllCmd);
#endif
}
/**
 * @brief 获取CAN接收错误计数
 * @return uint8_t
 * @author lincq <lincq@raynen.cn>
 * @date 2023-11-04
 */
uint8_t Bsp_Can_RecvErrCnt(void)
{
    return CAN_RxErrorCntGet();
}

void Bsp_Can_SetRxCallBack(void (*callback)(struct can_message *))
{
    CanRx_Callback = callback;
}
