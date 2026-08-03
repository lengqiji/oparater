#include "rn_typedef.h"

#include "jump_motor.h"
#include "para.h"
#include "qf.h"
#include "stepper_motor.h"
#include "ucos_ii.h"
#include "rn_printf.h"
#include "bsp_gpio.h"   //SW_DOWN_YES
#define MOTOR_JUMP_ID       A_MOTOR
#define MOTOR_JUMP_CUR_HOLD 8
#define MOTOR_JUMP_CUR_WORK 17

#define MOTOR_JUMP_DIV 32  // 细分数

#define MOTOR_JUMP_DEFAULT_STEP(div) (12 * (div))

#define JUMP_ANGLE_2_STEP(angle) ((angle) * MOTOR_JUMP_DIV * 200 / 360)
#define JUMP_DEFAULT_SPEED(div) (100 * (div))
#define JUMP_MAX_SPEED    20000 //10000;1000跑不了1000转

#define JUMP_LOCK_START_QF   240
#define JUMP_DEALY_QF        150  // 超过150个QF 就不动作了
#define JUMP_UNLOCK_START_QF JUMP_LOCK_START_QF

#define NDL_DOWN_JUMP_MS 2500  // 2.5秒

// 参数结构体
typedef struct {
    uint32_t qf_offset;  // 间隔
    uint32_t cnt;        // 当前频率运行脉冲数
} JUMP_TABLE;

typedef struct {
    int lock_flag;  //
    int qei_start_adj;
    int qei_start_unlock;
    int jump_set;
    int force_jump_set;
    int all_step;
    int lock_start;     // 开启锁针
    int unlock_start;   // 关闭锁针
    int ndl_down_flag;  // 针停下位标记，关闭的机头不下针
    int default_step;   // 跳跃默认行程
    int default_step_adj;  // 跳跃默认行程微调
    struct {
        JUMP_TABLE *p_addr;
        int         len;
    } flw_tab;
} jump_param;

static jump_param Jump_Param;

const uint8_t default_angle_tab[] = {5, 10, 15, 20};
static int speed_table[]
      = {500,  557,  637,  723,  815,  914,  1019, 1130, 1247,
       1372, 1502, 1639, 1782, 1931, 2087, 2168, 2249, 2333,
       2418, 2505, 2593, 2683, 2774, 2868, 2962, 3000, 3100,
       3200,3300,3400,3600,3800,4000,
	    4200,4400,4600,4800,5000,5200,5400,5600,5800,6000,
	    6200,6400,6600,6800,7000,7200,7400,7600,7800,8000,
	    8200,8400,8600,8800,9000,9200,9400,9600,9800,10000};

static int follow_table[] = {JUMP_MAX_SPEED, JUMP_MAX_SPEED};

static JUMP_TABLE JumpTable_ERED[][9] = {
    // 80*8+60*8+40*8+24*8+20*8+16*8+12*8+8*8+4*89 = 2436
    {{80, 8},
     {60, 8},
     {40, 8},
     {24, 8},
     {20, 8},
     {16, 8},
     {12, 8},
     {8, 8},
     {4, JUMP_ANGLE_2_STEP(5)} },
    // 80*4+60*4+40*8+24*8+20*8+16*8+12*8+8*8+4*178 = 2232
    {{80, 4},
     {60, 4},
     {40, 8},
     {24, 8},
     {20, 8},
     {16, 8},
     {12, 8},
     {8, 8},
     {4, JUMP_ANGLE_2_STEP(10)}},
    // 80*2+60*4+40*4+24*4+20*8+16*8+12*8+8*8+4*266 = 2168
    {{80, 2},
     {60, 4},
     {40, 4},
     {24, 4},
     {20, 8},
     {16, 8},
     {12, 8},
     {8, 8},
     {4, JUMP_ANGLE_2_STEP(15)}},
    // 80*2+60*4+40*4+24*4+20*4+16*4+12*4+8*4+4*354 = 2296
    {{80, 2},
     {60, 4},
     {40, 4},
     {24, 4},
     {20, 4},
     {16, 4},
     {12, 4},
     {8, 4},
     {4, JUMP_ANGLE_2_STEP(20)}}//最快不能小于4，脉冲频率来不及
};

static JUMP_TABLE JumpTable[] = {//384
#if (MOTOR_JUMP_DIV == 32)
    /*{30, 15},
    {26, 15},
    {22, 15},
    {18, 15},
    {12, 15},
    {10, 15},
    { 8, 15},
    { 6, 15},
    { 4, 15},
    { 4, 15},//150----1710
    //最快不能小于4，脉冲频率来不及
    { 4, MOTOR_JUMP_DEFAULT_STEP(MOTOR_JUMP_DIV)},//234---936*/

     /*{80, 8},
     {60, 8},0度机直接补在参数晚100度*/
     {40, 8},
     {24, 8},
     {16, 8},
     {10, 8},
     {8, 8},
     {4, 50}, //2104 -- 106
     {4, MOTOR_JUMP_DEFAULT_STEP(MOTOR_JUMP_DIV)},  //1112 -- 278

#elif (MOTOR_JUMP_DIV == 8)
    // 2598,96
    {150,  3},
    {130,  3},
    {110,  3},
    { 90,  3},
    { 60,  3},
    { 50,  3},
    { 40,  3},
    { 30,  3},
    { 20,  3},
    { 10,  3},
    {  8, 66},
#endif
};
static JUMP_TABLE JumpTable_QIHONG[] = {//384
#if (MOTOR_JUMP_DIV == 32)
    /*{30, 15},
    {26, 15},
    {22, 15},
    {18, 15},
    {12, 15},
    {10, 15},
    { 8, 15},
    { 6, 15},
    { 4, 15},
    { 4, 15},//150----1710
    //最快不能小于4，脉冲频率来不及
    { 4, MOTOR_JUMP_DEFAULT_STEP(MOTOR_JUMP_DIV)},//234---936*/

     /*{80, 8},
     {60, 8},0度机直接补在参数晚100度*/
     {40, 8},
     {24, 8},
     {16, 8},
     {10, 8},
     {8, 8},
     {4, 90}, //2104 -- 106
     {4, MOTOR_JUMP_DEFAULT_STEP(MOTOR_JUMP_DIV)},  //1112 -- 278

#elif (MOTOR_JUMP_DIV == 8)
    // 2598,96
    {150,  3},
    {130,  3},
    {110,  3},
    { 90,  3},
    { 60,  3},
    { 50,  3},
    { 40,  3},
    { 30,  3},
    { 20,  3},
    { 10,  3},
    {  8, 66},
#endif
};
static void JumpMotor_ChangeHoldI();
static void JumpMotor_StepCalc(void);

void Fun_JumpMotor_Init(void)
{
    Mod_Motor_SetHoldI(MOTOR_JUMP_ID, MOTOR_JUMP_CUR_HOLD);
    Mod_Motor_SetWorkI(MOTOR_JUMP_ID, MOTOR_JUMP_CUR_WORK);

    Bsp_Motor_Current_Set(MOTOR_JUMP_ID, MOTOR_JUMP_CUR_HOLD);

    Mod_Motor_SetSpeedTable(MOTOR_JUMP_ID,
                            &speed_table[0],
                            sizeof(speed_table) / sizeof(int));

    Jump_Param.qei_start_adj    = JUMP_LOCK_START_QF;
    Jump_Param.qei_start_unlock = JUMP_UNLOCK_START_QF;
    Jump_Param.all_step         = MOTOR_JUMP_DEFAULT_STEP(MOTOR_JUMP_DIV);
    if (Para_head.fac_id == CLIENT_TYPE_ERED) {
        Jump_Param.flw_tab.p_addr = &JumpTable_ERED[0][0];
        Jump_Param.flw_tab.len
            = sizeof(JumpTable_ERED[0]) / sizeof(JumpTable_ERED[0][0]);
        Jump_Param.default_step = JUMP_ANGLE_2_STEP(default_angle_tab[0]);
    }else if (Para_head.fac_id == CLIENT_TYPE_QIHONG) {
        Jump_Param.flw_tab.p_addr = &JumpTable_QIHONG[0];
        Jump_Param.flw_tab.len
            = sizeof(JumpTable_QIHONG) / sizeof(JumpTable_QIHONG[0]);
        Jump_Param.default_step = MOTOR_JUMP_DEFAULT_STEP(MOTOR_JUMP_DIV);;
    } else {
        Jump_Param.flw_tab.p_addr = &JumpTable[0];
        Jump_Param.flw_tab.len    = sizeof(JumpTable) / sizeof(JumpTable[0]);
        Jump_Param.default_step   = MOTOR_JUMP_DEFAULT_STEP(MOTOR_JUMP_DIV);
    }
}
void Jump_SetSpeedTable(int is_follow)
{
    if (is_follow) {
        Mod_Motor_SetSpeedTable(MOTOR_JUMP_ID,
                                &follow_table[0],
                                sizeof(follow_table) / sizeof(int));
    } else {
        Mod_Motor_SetSpeedTable(MOTOR_JUMP_ID,
                                &speed_table[0],
                                sizeof(speed_table) / sizeof(int));
    }
}

void Motor_Jump_Lock(void)
{
    if (Jump_Param.lock_flag != 1) {
        if(Jump_Param.force_jump_set)
        {
			if(false == Mod_Motor_ActAbs(MOTOR_JUMP_ID,JUMP_DEFAULT_SPEED(64),Jump_Param.all_step))//快速打开锁头。
	                return;//没完成
	            Jump_SetSpeedTable(0);
        }
        else{

	        if (Para_head.mainshaft_run == 0) {
                uint16_t speed = JUMP_DEFAULT_SPEED(MOTOR_JUMP_DIV);
                if (Para_head.fac_id == CLIENT_TYPE_FORTEVER) {
                } else {
                    speed /= 4;  // 匹配80转关机头的500ms，384的800频率够。
                }
                if(false == Mod_Motor_ActAbs(MOTOR_JUMP_ID,speed,Jump_Param.all_step))
	                return;//没完成
	            Jump_SetSpeedTable(0);
	        } else {
	            Jump_SetSpeedTable(1);
	            Jump_Param.lock_start = 1;
	//            if(Mod_Qf_AngleRead() < 320 && Jump_Param.jump_set)//120 + 界面范围0~200，兼容绣做中手动开关头有时候会下针处理
	//            {
	//                Jump_Param.qei_start_adj = Mod_Qf_Read();
	//            }
	        }
        }
        Jump_Param.lock_flag = 1;
    }
}

void Motor_Jump_Unlock(void)
{
    if (Jump_Param.lock_flag != 0) {


        if (Para_head.mainshaft_run == 0) {
            if(false == Mod_Motor_ActAbs(MOTOR_JUMP_ID, JUMP_DEFAULT_SPEED(MOTOR_JUMP_DIV), 0))
                return;//没完成
            Jump_SetSpeedTable(0);
        } else {
            Jump_SetSpeedTable(1);
            Jump_Param.unlock_start = 1;
//            if(Mod_Qf_AngleRead() <  320 && Jump_Param.jump_set)//120 + 界面范围0~200，兼容绣做中手动开关头有时候会下针处理
//            {
//                Jump_Param.qei_start_unlock = Mod_Qf_Read();
//            }
        }
        Jump_Param.lock_flag = 0;
    }
}

static void Fun_JumpMotor_HeadOffJudge(uint8_t *jmp)
{
//    static status_e _xOldState   = STATUS_INITIAL;
static status_e pre_sta = 0;
    static uint32_t _ulTimeBegin = 0;

    if (Para_head.mainshaft_run) {
        return;
    }

    if (Para_head.stat == STATUS_INITIAL) {
        return;
    }

    if (OSTimeGet() < (_ulTimeBegin + 3000)) {
        *jmp = 1;
    } else {
        _ulTimeBegin = 0;
    }

    if (SW_DOWN_YES == pre_sta) {
        return;
    }
    pre_sta = SW_DOWN_YES;

    if (SW_DOWN_YES) {
        _ulTimeBegin = OSTimeGet();
    }
}

void Fun_JumpMotor_TaskMs(void)
{
    uint8_t jmp = 0;

    if (Para_head.type_err && (Para_head.type != HEAD_TYPE_TAPE)) {
       return;
    }

    if (Para_head.mainshaft_run)  // 主轴启动
    {
        Jump_Param.ndl_down_flag = 0;
        if (Para_head.stat == STATUS_OFF) {
            jmp = 1;
            Fun_JumpMotor_Small_Cur_Set(1);

        } else if (Para_head.manual_trim)  // 手动剪线状态，允许下针
        {
            // 依据head_param.jump_param.jump状态决定
        } else if ((Para_emb.stat == STATUS_PATCH)
                   && (Para_head.stat == STATUS_NORMAL))  // 正常下，不需要补绣
        {
            jmp = 1;
        }
    } else if (Jump_Param.ndl_down_flag) {  // 特殊操作的时候关闭的机头不下针
        Jump_Param.ndl_down_flag--;
        jmp = 1;
    }

    Fun_JumpMotor_HeadOffJudge(&jmp);

    if (Jump_Param.jump_set) {
        jmp = 1;
    }
 	if (Jump_Param.force_jump_set) {
        jmp = 1;
    }
    if (jmp) {
        Motor_Jump_Lock();
    } else {
        Motor_Jump_Unlock();
    }
    JumpMotor_ChangeHoldI();
}

void Fun_JumpMotor_JumpSet(int para)
{
    Jump_Param.jump_set = para;
//    Jump_Param.qei_start_adj = Mod_Qf_Read();
//    Jump_Param.qei_start_unlock = Jump_Param.qei_start_adj;
}
void Fun_JumpMotor_JumpAngleStart(void)
{
    Jump_Param.qei_start_adj = Mod_Qf_Read();
    Jump_Param.qei_start_unlock = Jump_Param.qei_start_adj;
    INFO_PRINT("JumpSet   -%d--%d-\r\n\r\n",Mod_Qf_Read(),Jump_Param.qei_start_adj);

}
void Fun_JumpMotor_Force_JumpSet(int para)
{
    Jump_Param.force_jump_set = para;
}
int Fun_JumpMotor_Force_JumpGet(void)
{
    int temp;
    temp =Jump_Param.force_jump_set;
    return temp;
}
void Fun_JumpMotor_Small_Cur_Set(uint32_t para)
{
    if (Para_head.fac_id == CLIENT_TYPE_YUEMEI) {
    	Para_head.small_cur_mode = para;
    }
    else{
			Para_head.small_cur_mode = 0;
    }
}
uint32_t Fun_JumpMotor_Small_Cur_Get(void)
{
    uint32_t temp;
    temp =Para_head.small_cur_mode;
    return temp;
}
void Fun_JumpMotor_StepsAdj(int8_t para)
{
#define STEP_COEFF (MOTOR_JUMP_DIV / 2)
    if (Para_head.fac_id == CLIENT_TYPE_FORTEVER)//0度机满行程
        Jump_Param.default_step_adj = (int16_t) para * STEP_COEFF * 2;
    else if(Para_head.fac_id == CLIENT_TYPE_ERED)
        Jump_Param.default_step_adj = (int16_t) para * STEP_COEFF;
    else//用放大2倍的补充，软件通用点
        Jump_Param.default_step_adj = (int16_t) para * STEP_COEFF * 2;

    JumpMotor_StepCalc();  // 重新计算步数
}

void Fun_JumpMotor_SetNdlDownFlag(void)
{
    if (Para_head.stat == STATUS_OFF) {
        Jump_Param.ndl_down_flag = NDL_DOWN_JUMP_MS;  // 关闭的机头不下针
    }
}

void Fun_JumpMotor_SetWorkI(uint8_t para)
{
    Mod_Motor_SetWorkI(MOTOR_JUMP_ID, para / 10);
}

void Fun_JumpMotor_SetDir(uint8_t para)
{
    if (para <= 1 && Para_head.type == HEAD_TYPE_FLAT)
        Mod_Motor_SetLogicDir(MOTOR_JUMP_ID, para);
}

void Fun_JumpMotor_SetDefaultStep(uint8_t para)
{
    if (Para_head.fac_id != CLIENT_TYPE_ERED) {
        return;
    }
    int tab_len = sizeof(default_angle_tab) / sizeof(default_angle_tab[0]);
    if (para > (tab_len - 1)) {
        para = tab_len - 1;
    }
    Jump_Param.default_step   = JUMP_ANGLE_2_STEP(default_angle_tab[para]);
    Jump_Param.flw_tab.p_addr = &JumpTable_ERED[para][0];
    Jump_Param.flw_tab.len
        = sizeof(JumpTable_ERED[para]) / sizeof(JumpTable_ERED[0][0]);
    JumpMotor_StepCalc(); // 重新计算步数
}

void Fun_JumpMotor_Follow()
{
#define QF_NOT_SAVE -1
    static int qf_last = 0;
    static int ActStartQf = QF_NOT_SAVE;//初始值
    int        new_qf  = Mod_Qf_Read();

    if (!Para_head.mainshaft_run) {
        Jump_Param.lock_start   = 0;//防止未完成
        Jump_Param.unlock_start = 0;
        return;
    }
    if (qf_last != new_qf) {
        int tableSize    = Jump_Param.flw_tab.len;
        int start_qf     = Jump_Param.qei_start_adj;
        int pulse        = 1;  // 正方向走
        int jumpMotorPos = 0;  // 跳跃电机绝对位置
        int qfActDelay   = 0;  // 最迟启动动作QF差

        if (Jump_Param.unlock_start != 0) {
            start_qf     = Jump_Param.qei_start_unlock;  // 解锁启动角度
            pulse        = -1;                           // 反方向
            jumpMotorPos = Jump_Param.all_step;
        }

        if (Jump_Param.lock_start != 0 || Jump_Param.unlock_start != 0) {
            int isAct = 0;
            if ((new_qf > start_qf
                 && new_qf < start_qf + JUMP_DEALY_QF)  // 超过150没动就不动了，兼容绣做中手动开关头有时候会下针处理
                || Jump_Param.lock_start == 2
                || Jump_Param.unlock_start == 2) {  // 启动角度,或者已经启动
                int i = 0;
                for (; i < tableSize; i++) {
                    for (int cnt = 0; cnt < Jump_Param.flw_tab.p_addr[i].cnt; cnt++) {
                        int nextQf = start_qf + Jump_Param.flw_tab.p_addr[i].qf_offset;

                        jumpMotorPos += pulse;  // 每次只有1个脉冲

                        int new_qf2 = new_qf;
                        if(ActStartQf != QF_NOT_SAVE
                            && new_qf < ActStartQf)//初始值
                        {//过0还在动，只要小于启动点
                            new_qf2 = new_qf + 4000;
                        }

                        if (new_qf2 >= start_qf
                            && new_qf2 < nextQf) {  // 会被多次调用，上个未完成不执行

                            if (Jump_Param.lock_start
                                == 1) {  // 其实是连续运动，不切换保持电流，先设置成工作电流
                                Jump_Param.lock_start = 2;
                            }
                            if (Jump_Param.unlock_start == 1) {
                                Jump_Param.unlock_start = 2;
                            }

                            Mod_Motor_SetHoldI(
                                MOTOR_JUMP_ID,
                                Mod_Motor_GetWorkI(MOTOR_JUMP_ID));
                            Mod_Motor_ActAbs(MOTOR_JUMP_ID,
                                             JUMP_MAX_SPEED,
                                             jumpMotorPos);
                            isAct = 1;

                            if(ActStartQf == QF_NOT_SAVE)//初始值
                            {
                                ActStartQf = new_qf2;
                            }


                        }
                        start_qf = nextQf;  // 更新启动角度
                        if (isAct)
                            break;
                    }
                    if (isAct)
                        break;
                }
                if (isAct == 0) {
                    Jump_Param.lock_start   = 0;
                    Jump_Param.unlock_start = 0;
                    ActStartQf = QF_NOT_SAVE;
                }
            }
        }
    }
    qf_last = new_qf;
}

static void JumpMotor_ChangeHoldI()
{  // 跟随动作时保持原先和工作电流一样，这里要还原保持电流
#define JUMP_HOLD_I (Mod_Motor_GetWorkI(MOTOR_JUMP_ID)/2)
    static uint workTime = 0;

    if (Mod_Motor_IsRun(MOTOR_JUMP_ID)
        || Jump_Param.lock_start != 0 //跟随脉冲最高速太快，这里扫描不到run
        || Jump_Param.unlock_start != 0)
    {
        workTime = OSTimeGet();
    }
    else if (workTime != 0 ) {
        int offset = OSTimeGet() - workTime;  // 1个100us
        if (abs(offset) > 1000)               // 100ms切换保持电流
        {
            workTime = 0;

            Mod_Motor_SetHoldI(MOTOR_JUMP_ID, JUMP_HOLD_I);

            if (Jump_Param.lock_flag == 0)  // 保持电流直接0，有弹簧
                Bsp_Motor_Current_Set(MOTOR_JUMP_ID, 0);
            else
            {
				if (Fun_JumpMotor_Small_Cur_Get() == 1)//
				{
				  Bsp_Motor_Current_Set(MOTOR_JUMP_ID, 2);//停止下来，测试跳跃时切0.2A，声音小。
				}
				else
				{
				  Bsp_Motor_Current_Set(MOTOR_JUMP_ID, JUMP_HOLD_I);
				}

            }
        }
    }
    else if(Bsp_Motor_Current_Get(MOTOR_JUMP_ID) != 0
        && Jump_Param.lock_flag == 0)
    {//防止没扫到启动标志等问题
        Bsp_Motor_Current_Set(MOTOR_JUMP_ID, 0);//跟随
    }
    else if (Bsp_Motor_Current_Get(MOTOR_JUMP_ID) != JUMP_HOLD_I
        && Jump_Param.lock_flag == 1)
    {//防止没扫到启动标志等问题
//        Bsp_Motor_Current_Set(MOTOR_JUMP_ID, JUMP_HOLD_I);
				if (Fun_JumpMotor_Small_Cur_Get() == 1)//
				{
				  Bsp_Motor_Current_Set(MOTOR_JUMP_ID, 2);//停止下来，测试跳跃时切0.2A，声音小。
				}
				else
				{
				  Bsp_Motor_Current_Set(MOTOR_JUMP_ID, JUMP_HOLD_I);
				}
    }
}

static void JumpMotor_StepCalc(void)
{
    int follow_step_sum = 0;

    Jump_Param.all_step = Jump_Param.default_step + Jump_Param.default_step_adj;

    for (int i = 0; i < Jump_Param.flw_tab.len - 1; i++) {
        follow_step_sum += Jump_Param.flw_tab.p_addr[i].cnt;
    }

    if (Jump_Param.all_step < follow_step_sum) {
        Jump_Param.all_step = follow_step_sum;
    }

    Jump_Param.flw_tab.p_addr[Jump_Param.flw_tab.len - 1].cnt
        = Jump_Param.all_step - follow_step_sum;
        INFO_PRINT("step -%d-\r\n\r\n",Jump_Param.flw_tab.p_addr[Jump_Param.flw_tab.len - 1].cnt);
}
