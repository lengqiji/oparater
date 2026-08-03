#include "rn_typedef.h"
#include "cmd_rst.h"
#include "bsp_can.h"
#include "cmd_def.h"
#include "para.h"
#include "key.h"


void Task_Cmd_Rst(struct can_message *pFrame)
{
    uint8_t cmd, scmd;

    cmd  = pFrame->data[0];
    scmd = pFrame->data[1];

    if (Para_head.stat == STATUS_INITIAL) {
        // 只判断最后一帧
        if ((pFrame->len == 2) && (cmd == 0x1f) && (scmd == 0x60)) {
            if (Fun_Key_GetState() == KEY_STATUS_DOWN)
                Para_head.stat = STATUS_OFF;
            else
                Para_head.stat = STATUS_NORMAL;
            Para_emb.stat = STATUS_STOP;
            Para_head.jump_no_follow =0;
        }
    }
    return;

}
