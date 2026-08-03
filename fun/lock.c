#include "rn_typedef.h"
#include "rn_printf.h"

#include "bsp_lock.h"
#include "lock.h"
#include "para.h"

#define LOCK_SELFHOLD_CNT_BASE 65
#define LOCK_PWM_CNT_BASE      20
#define LOCK_PWM_DUTY_BASE     50
// #define LOCK_PWM_DUTY_BASE 80

// 状态
typedef enum {
    LOCK_TYPE_SELFHOLD,  // 自保持类型
    LOCK_TYPE_PWM,       // pwm控制类型，即非自保持类型
} LOCK_TYPE_E;

// 状态
typedef enum {
    LOCK_STATUS_UNLOCK_OFF,  // 已解锁
    LOCK_STATUS_UNLOCK_ON,   // 正解锁
    LOCK_STATUS_LOCK_OFF,    // 已锁
    LOCK_STATUS_LOCK_ON,     // 正锁
} LOCK_STATUS;

// 参数结构体
typedef struct {
    LOCK_STATUS flag;    // 自保持锁线电磁铁状态标志
    uint32_t open_flag;  // 非自保持锁线电磁铁打开标志 =0未 =1打开
    uint32_t cnt;        // 计时，*10ms
    uint32_t hold_cnt;      // 激励保持时间，*1ms
    uint32_t duty;          // 非自保持占空比
    uint32_t dir;           // 方向
    uint32_t power_adjust;  // 面线夹持电磁铁电压
    uint32_t emb_flag;      //
    uint32_t lock_patch_no;  // 操作盒下发补绣时不补绣的机头锁住面线夹
    uint32_t fault;  // 硬件错误报警标志 =1有报警
    uint32_t lock_delay_open;  // 补绣完切换正常绣过后一针再进行锁线标志
    LOCK_TYPE_E type;        // 锁线电磁铁类型
    uint32_t    cmd_unlock;  // 命令松线一次
} lock_param_s;

static lock_param_s lock_param;

// 锁线打开
void lock_hold(void)
{
    if (lock_param.flag != LOCK_STATUS_LOCK_ON) {
        if (lock_param.dir == 0) {
            Bsp_Lock_SetDir(1);
        } else {
            Bsp_Lock_SetDir(0);
        }
        Bsp_Lock_SetEn(1);
        lock_param.cnt  = 0;
        lock_param.flag = LOCK_STATUS_LOCK_ON;
        INFO_PRINT("lock hold\r\n");
    }
}

// 锁线释放
void lock_release(void)
{
    if (lock_param.flag != LOCK_STATUS_UNLOCK_ON) {
        if (lock_param.dir == 0) {
            Bsp_Lock_SetDir(0);
        } else {
            Bsp_Lock_SetDir(1);
        }
        Bsp_Lock_SetEn(1);
        lock_param.cnt  = 0;
        lock_param.flag = LOCK_STATUS_UNLOCK_ON;
        INFO_PRINT("lock release\r\n");
    }
}

// 锁线自保持任务
void lock_selfhold_task(void)
{
    if ((lock_param.flag == LOCK_STATUS_LOCK_ON)
        || (lock_param.flag == LOCK_STATUS_UNLOCK_ON)) {
        if (lock_param.cnt < lock_param.hold_cnt) {
            lock_param.cnt++;
            if (lock_param.cnt >= lock_param.hold_cnt) {
                Bsp_Lock_SetEn(0);
                INFO_PRINT("lock self hold/release done\r\n");
            }
        }
    }
}

// 锁线判断
// 是否判断组合头情况？
void lock_judge(void)
{
    if (lock_param.emb_flag == 0) {
        // if ((head_param.stat == STATUS_OFF) || ((emb_param.stat ==
        // STATUS_PATCH) && (head_param.stat == STATUS_NORMAL)))
        //{
        // } else
        if (Para_head.stat == STATUS_OFF) {
        } else if ((Para_emb.stat == STATUS_PATCH)
                   && (Para_head.stat == STATUS_NORMAL)) {
            if ((lock_param.lock_patch_no)
                && (Para_head.mainshaft_run))  // 不补绣的机头锁住面线夹
            {
                lock_hold();
            }
        } else {
            // if(lock_param.lock_delay_open == 0)
            // 230831 wyx处理剪线后停车会弹开问题
            if (lock_param.lock_delay_open == 0) {
                if (Para_emb.stat != STATUS_STOP || lock_param.cmd_unlock == 1)
                    lock_release();
            }
        }
        lock_param.cmd_unlock = 0;
    } else {
        lock_hold();
    }
}

// 锁线任务
void Fun_Lock_TaskMs(void)  // 必须1ms一次
{
    lock_judge();
    lock_selfhold_task();
}

// 锁线初始化
void Fun_Lock_Init(void)
{
    lock_param.hold_cnt  = LOCK_SELFHOLD_CNT_BASE + lock_param.power_adjust;
    lock_param.open_flag = 0;

    lock_param.dir = 0;
}

void Fun_Lock_Hold(void)
{
    lock_param.flag       = LOCK_STATUS_LOCK_OFF;
    lock_param.emb_flag   = 1;
    lock_param.cmd_unlock = 0;
}
void Fun_Lock_Release(void)
{
    lock_param.flag       = LOCK_STATUS_UNLOCK_OFF;
    lock_param.emb_flag   = 0;
    lock_param.cmd_unlock = 1;
}

void Fun_Lock_SetHold(void)
{
	lock_param.emb_flag = 1;
    lock_param.cmd_unlock = 0;
}

void Fun_Lock_SetRelease(void)
{
	lock_param.emb_flag = 0;
    lock_param.cmd_unlock = 1;
}

void Fun_Lock_DelayOpenSet(void)
{
    if (Para_emb.stat == STATUS_PATCH)  // 上个状态是补绣状态，现在切换为正常绣
    {
        lock_param.lock_delay_open = 2;  // 锁线电磁铁释放等一针
    }
}

void Fun_Lock_DelayOpenCount(void)
{
    if (lock_param.lock_delay_open > 0) {
        lock_param.lock_delay_open--;
    }
}
void Fun_Lock_SetLockPower(uint8_t param)
{
    lock_param.power_adjust = param * 5;
    lock_param.hold_cnt     = LOCK_SELFHOLD_CNT_BASE + lock_param.power_adjust;
}
void Fun_Lock_SetDir(uint8_t param)
{
    if (param <= 1)
        lock_param.dir = param;
}

void Fun_Lock_SetPatchNo(uint8_t param)
{
    lock_param.lock_patch_no = param;
}

// 锁线类型切换
void Fun_Lock_SetType(uint8_t type) {}

void Fun_Lock_SetPwmDuty(uint8_t duty)
{
    if (duty <= 100) {
        lock_param.duty = duty;
    }
}
