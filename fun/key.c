#include "rn_typedef.h"

#include "bsp_gpio.h"
#include "cmd.h"
#include "key.h"
#include "para.h"

// 参数结构体
typedef struct {
    KEY_STATUS sta;          // 当前状态
    KEY_STATUS pin_sta;      // 当前引脚状态
    KEY_STATUS pin_pre_sta;  // 上次引脚状态
    uint32_t   cnt;          // 状态计数
} key_param_s;

key_param_s key_param;
// 读取按键值
static KEY_STATUS key_read(void)
{
    if (SW_UP_YES) {
        return KEY_STATUS_UP;
    } else if (SW_DOWN_YES) {
        return KEY_STATUS_DOWN;
    }

    return (KEY_STATUS_NO);
}

void Fun_Key_SetState(uint8_t _key_state)
{
     (key_param.sta =_key_state );
}

KEY_STATUS Fun_Key_GetState(void)
{
    return (key_param.sta);
}
void Fun_Key_UpdateState(void)
{
    static KEY_STATUS _xNewState = KEY_STATUS_CHANGE;

    if (Para_head.is_splice && (Para_head.splice_master == 0)
        && (Para_head.addr_ing == 0)) {
        return;
    }
    key_param.cnt++;
    key_param.pin_sta = key_read();
    if (key_param.pin_pre_sta != key_param.pin_sta) {
        key_param.pin_pre_sta = key_param.pin_sta;
        key_param.cnt         = 0;
        key_param.sta         = KEY_STATUS_CHANGE;
    } else {
        if (key_param.cnt >= 3) {
            key_param.sta = key_param.pin_sta;
        }
    }
   if(Para_head.stat != STATUS_INITIAL){  
	    if (_xNewState != key_param.sta) {
	        _xNewState = key_param.sta;
	        Mod_Cmd_RspKeyStat(_xNewState);
	    }
   }
}
