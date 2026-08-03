#include "rn_typedef.h"
#include "hc32_ddl.h"
#include "board_config.h"
#include "bsp_can.h"
#include "bsp_flash.h"
#include "cmd_def.h"
#include "key.h"

static uint8_t up_data_buf[30] = {0};  // 升级标志存储
static int     Cmd_up_cnt      = 0;    // 下发升级命令次数

static void cmd_rsp_up_start(void)
{
    uint8_t buf[8] = {0};

    buf[0] = 0xe6;
    Bsp_Can_Send(CMD_UP_ACK_ID, buf, 4);
}

// 升级命令
void Task_Cmd_Update(struct can_message *pFrame)
{
    uint8_t scmd, len;
    uint8_t flag = 0;

    scmd = pFrame->data[1];
    len  = pFrame->len;

    switch (scmd) {
        case CMD_MAIN_UP_START:
            if (len == 2) {
                Cmd_up_cnt = 0;
            }
            break;
        case CMD_MAIN_UP_START_CODE:
        case CMD_MAIN_UP_CHECK:
        case CMD_MAIN_UP_LEN:
        case CMD_MAIN_UP_SEQ1:
            if (len == 8) {
                Cmd_up_cnt++;
                memcpy(up_data_buf + (scmd - CMD_MAIN_UP_START_CODE) * 6,
                       pFrame->data + 2,
                       6);
                //                shell_printf("up %X\r\n", pFrame->data[1]);
            }
            break;
        case CMD_MAIN_UP_SEQ2:
            if (len == 8) {
                if ((Cmd_up_cnt == 4) && (Fun_Key_GetState() != KEY_STATUS_DOWN)) {
                    if (memcmp(up_data_buf,
                               RN_JUMP_FLAG,
                               strlen(RN_JUMP_FLAG)) == 0) {
                        flag = 1;
                    }

                    if (!flag) {
                        Cmd_up_cnt = 0;
                        //	                    shell_printf("up jump flag
                        // err\n"); hexdump(up_data_buf, sizeof(up_data_buf));
                        return;
                    }

                    cmd_rsp_up_start();
                    SysTick_Delay(10u);
                    flash_set_up_param();
                    SysTick_Delay(10u);
                    NVIC_SystemReset();
                } else
                    Cmd_up_cnt = 0;
            }
            break;
        default:
            Cmd_up_cnt = 0;
            break;
    }
}
