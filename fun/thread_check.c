#include "rn_typedef.h"
#include "os_cfg.h"
#include "ucos_ii.h"
#include "cmd.h"
#include "para.h"
#include "thread_check.h"
#include "bsp_thread.h"


#define BOT_MAX_FILTER 9  // 底检最大去抖动针数

typedef enum {
    THREAD_CHECK_NO,
    THREAD_CHECK_TOP,   // 面检
    THREAD_CHECK_BOT,   // 底检
    THREAD_CHECK_BOTH,  // 面检和底检
} THREAD_CHECK_TYPE;

typedef enum {
    CHECK_NONE,
    CHECK_START_COUNT, // 开始计数
} CHECK_FLAG;


// 参数结构体
typedef struct {
    THREAD_CHECK_TYPE type;            //
    uint32_t           selfcheck_flag;  // 自检标志
    uint32_t           bot_exist;       // 是否存在底检 =1存在 =0不
    uint32_t           isbrk;           // 断线标志 =0未断 =1断
    uint32_t           en;           // 断检使能标志 =0未 =1打开
    uint32_t           ndl_channel;  // 断检通道选择针号
    uint32_t check_flag;  // 面检检测标志 =0不判断 =1开始计数 =2开始判断
    uint32_t IscheckNow;  // 另开面检检测标志，原先存在多任务影响
    uint32_t top_cnt;         //
    uint32_t top_bkcnt;       //
    uint32_t top_mscnt;       // 检测次数，次/ms
    uint32_t top_buf[4];      // 面检计数数组
    uint32_t top_sense;       // 面线检测灵敏度
    uint32_t top_filter;      // 断线去抖动针数
    uint32_t top_selfcnt;     // 自检时间计数
    uint32_t top_filter_adj;  // 断线去抖动调节针数

    uint32_t bot_cnt;      //
    uint32_t bot_bkcnt;    //
    uint32_t bot_mscnt;    // 检测次数，次/ms
    uint32_t bot_buf[4];   // 底检计数数组
    uint32_t bot_sense;    // 底线检测灵敏度
    uint32_t bot_filter;   // 断线去抖动针数
    uint32_t bot_selfcnt;  // 自检时间计数

    uint32_t bot_all_len;   // 底检滚动总针长
    uint32_t bot_last_len;  // 上一次针长
    uint32_t bot_cnt_flag;  // 滚动计数达到标志

    uint32_t ndl_cur_len;                        // 当前针长
    uint32_t ndl_all_len;                        // 总针长
    uint32_t ndl_all_num;                        // 总针数
    uint32_t judge_len;                          // 判断针长
    uint32_t judge_ndl;                          // 判断针数
    uint32_t judge_ndl_buf[BOT_MAX_FILTER + 1];  // 判断针数
    uint32_t judge_ndl_buf_id;                   // 判断针数数组id
    uint32_t ndl_num;                            // 机型针数
    uint32_t ndl_cur_num;                        // 主控下发的当前针号
    uint32_t ndl_start_filter;                   // 起绣不检测断线针数，设置下来的参数
    uint32_t ndl_patch_filter_cnt;  // 补绣后正常机头起绣不检测针数计数，每次启动重置为ndl_start_filter
    uint32_t run_ndl_num;           // 主轴重新启动后的针数
    uint32_t  splice_no_judge_flag;  // 拼接头断检不判断标志
    uint32_t  splice_head_ndl_num;  // 拼接机头针棒(单个机头)
    uint32_t is_indep_check;  // 是否独立断检，1：是 0：不是
    uint32_t ndl_num_reverse;
    uint32_t ndl_num_mode;
    uint32_t ndl_num_type;
    uint32_t _a_opto_num;
    uint32_t _b_opto_num;
} thread_param_s;

#define NDL_PARAM_TEST_NUM 50
typedef struct {// 保存判断信息，测试
    uint16_t head;                         // 下发针长头位置
    uint16_t bot_alarm_ndl;                // 底检报警针数
    uint16_t check_head;                   // 断检判断的参数头位置
    uint16_t ndl[NDL_PARAM_TEST_NUM];      // 针长
    uint16_t len_all[NDL_PARAM_TEST_NUM];  // 总针长
    uint16_t bot[NDL_PARAM_TEST_NUM];
    uint16_t top[NDL_PARAM_TEST_NUM];
} ndl_param_s;

#define BOT_BUF_SIZE (15)
#define BOT_AVGE_NUM (5)
typedef struct {
    uint8_t  bot_cnt_num;
    uint8_t  bot_len_num;
    uint8_t  bot_avge_num;
    uint8_t  bot_ndl_cnt;  // 间隔针数判断
    uint16_t bot_cnt_buf[BOT_BUF_SIZE];
    uint16_t bot_len_buf[BOT_BUF_SIZE];
    uint16_t last_avge[BOT_AVGE_NUM];

} BOT_DATA_BUF_S;



#define BOT_JUDGE_LEN (300)  //(400)
#define BOT_JUDGE_CNT (15)
#define BOT_JUDGE_MIN (5)
/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
thread_param_s thread_param ;  // 断检参数
BOT_DATA_BUF_S thread_bot_data = {0};
static uint32_t new_ndl_len_cnt = 0;	//针长次数



// 选择针号（面检和底检同时设置）
void thread_chose_ndl(uint8_t n)
{
    uint8_t ndl_num         = 0;
    uint8_t head_splice_num = 0;
    uint8_t head_loc        = 0;

    if (Para_head.is_splice == 0) {
        if ((n == 0) || (n > 16)) {
            return;
        }
    }

    if (n > thread_param.ndl_num) {
        // shell_printf("ndl chose err, all=%d, set=%d\n", thread_param.ndl_num,
        // n);
    }

    // 拼接机头
    if (Para_head.is_splice) {
        // 当前针号在单个机头的中最后一针
        if ((n % thread_param.splice_head_ndl_num) == 0) {
            // 当前针号在组合头中的第几块板
            head_splice_num = n / thread_param.splice_head_ndl_num;
        } else {
            head_splice_num = (n / thread_param.splice_head_ndl_num) + 1;
        }
        // shell_printf("head_splice_num:%d\r\n", head_splice_num);

        // 当前机头在组合头中的位置(第几块板)
        if ((Para_head.addr % Para_head.splice_head_num) == 0) {
            head_loc = Para_head.splice_head_num;
        } else {
            head_loc = Para_head.addr % Para_head.splice_head_num;
        }

        // 当前机头和当前针号所在位置相同时需要断线检测
        if (head_loc == head_splice_num) {
            if (n % thread_param.splice_head_ndl_num) {
                n %= thread_param.splice_head_ndl_num;
            } else {
                n = thread_param.splice_head_ndl_num;
            }

            thread_param.splice_no_judge_flag = 0;
            // shell_printf("channel:%d\r\n", n);
        } else {
            thread_param.splice_no_judge_flag = 1;
        }
        // if (Para_head.addr == 1)
        //{
        //     cmd_send_debug((thread_param.splice_head_num <<
        //     8)+thread_param.splice_head_ndl_num);
        // }
    }

    thread_param.ndl_channel = n;


    if (Para_head.fac_id == CLIENT_TYPE_ERED){
        
	    if(thread_param.ndl_num_reverse) {
	        ndl_num = thread_param.ndl_num - n;
	    } else {
	        ndl_num = n - 1;
	    }

	    if(thread_param.ndl_num==9)
	    {
			thread_param._a_opto_num = 5;
			thread_param._b_opto_num = 4;
	    }
	    else if(thread_param.ndl_num==12)//12针
	    {
			thread_param._a_opto_num = 6;
			thread_param._b_opto_num = 6;
	    }
	    else//15针
	    {
			thread_param._a_opto_num = 8;
			thread_param._b_opto_num = 7;
	    }
	    Bsp_Thread_ERED_SelectNdl(ndl_num,thread_param.ndl_num_mode,thread_param.ndl_num_type,thread_param._a_opto_num,thread_param._b_opto_num);

    }else{
       #ifdef NDL_NUM_NEGATIVE //针棒号和断检逆序
	    ndl_num = thread_param.ndl_num - n;
		#else
	    if(thread_param.ndl_num_reverse) {
	        ndl_num = thread_param.ndl_num - n;
	    } else {
	        ndl_num = n - 1;
	    }
		#endif
   	    Bsp_Thread_SelectNdl(ndl_num);
    }


}

// 获取底检计数
void thread_get_bot_count(void)
{
    thread_param.bot_cnt = Bsp_Thread_GetBotCount();

}

// 清除底检计数
void thread_clear_bot_count(void)
{
    Bsp_Thread_ClearBotCount();
}

// 判断底检是否存在
uint8_t thread_judge_bot(void)
{
    return Bsp_Thread_BotCnDetYes();
}




void thread_top_param_init(void)
{
    thread_param.top_buf[3] = 50;
    thread_param.top_buf[2] = 50;
    thread_param.top_buf[1] = 50;
    thread_param.top_buf[0] = 50;
}

// 底检判断针数计算
void thread_cal_bot_judge(void)
{
    uint8_t i, n, step, m;

    n    = thread_param.bot_filter + 1;
    step = (15 - thread_param.bot_sense + n) / n;

    for (i = 0; i < n; ++i) {
        thread_param.judge_ndl_buf[i] = step;
    }

    if ((15 - thread_param.bot_sense + n) > step * n) {
        m = (15 - thread_param.bot_sense + n) - step * n;
        for (i = 0; i < m; ++i) {
            thread_param.judge_ndl_buf[i]++;
        }
    }
    thread_param.judge_ndl_buf_id = 0;

    if (thread_param.bot_filter < 3) {
        for (i = 0; i < n; ++i) {
            thread_param.judge_ndl_buf[i] =
                (thread_param.judge_ndl_buf[i] + thread_param.bot_filter) / 2;
        }
    }
    thread_param.judge_ndl_buf_id = 0;
    thread_param.judge_ndl        = thread_param.judge_ndl_buf[0];

    //    shell_printf("bot sens=%d, filter=%d\njudge ndl: ",
    //                 thread_param.bot_sense,
    //                 thread_param.bot_filter);
    //    for (i = 0; i < n; ++i) {
    //        shell_printf("%d ", thread_param.judge_ndl_buf[i]);
    //    }
    //    shell_printf("\n");
}

// 参数初始化
void Fun_Thread_Init(void)
{
    thread_param.ndl_num          = 15;
    thread_param.ndl_cur_num      = 1;
    thread_param.type             = THREAD_CHECK_BOTH;
    thread_param.top_sense        = 3; /* 面线检测灵敏度           */
    thread_param.bot_sense        = 2; /* 底线检测灵敏度           */
    thread_param.top_filter       = 3; /* 面线断线去抖动针数       */
    thread_param.top_filter_adj   = 3;
    thread_param.bot_filter       = 1; /* 底线断线去抖动针数       */
    thread_param.ndl_start_filter = 6;
    thread_param.ndl_patch_filter_cnt = 0;
    thread_param.ndl_num_reverse = false;
	thread_param.ndl_num_mode = 0;//         Oxaa底检方式值
    thread_param.ndl_num_type = 0;//	     Oxbb 底检模式值
    thread_param.judge_len = 35 - thread_param.bot_sense * 2;

    thread_cal_bot_judge();


    thread_top_param_init();
    thread_param.bot_exist = thread_judge_bot();


}

// 断检参数复位
void Fun_Thread_Rst(void)
{
    thread_param.top_mscnt        = 0;
    thread_param.top_cnt          = 0;
    thread_param.top_bkcnt        = 0;
    thread_param.bot_cnt          = 0;
    thread_param.bot_bkcnt        = 0;
    thread_param.isbrk            = 0;
    thread_param.ndl_all_len      = 0;
    thread_param.ndl_all_num      = 0;
    thread_param.judge_ndl_buf_id = 0;

    thread_param.bot_all_len  = 0;
    thread_param.bot_last_len = 0;
    thread_param.bot_cnt_flag = 0;

    thread_bot_data.bot_avge_num = 0;
    thread_bot_data.bot_cnt_num  = 0;
    thread_bot_data.bot_len_num  = 0;
    thread_bot_data.bot_ndl_cnt  = 0;

    thread_param.bot_cnt_flag = 0;

    memset(thread_bot_data.bot_cnt_buf, 0, BOT_BUF_SIZE);
    memset(thread_bot_data.bot_len_buf, 0, BOT_BUF_SIZE);
    new_ndl_len_cnt = 0;
}

// 自检测试
void thread_brk_selfcheck(void)
{
    uint8_t        brk = 0;
    static uint8_t pre = 0;

    if ((thread_param.type == THREAD_CHECK_TOP) ||
        (thread_param.type == THREAD_CHECK_BOTH)) {
        if (Bsp_Thread_TopDetYes() == 0) {
            thread_param.top_selfcnt = OSTimeGet();
        } else {
            if ((OSTimeGet() - thread_param.top_selfcnt) >
                500 * (OS_TICKS_PER_SEC / 1000)) {
                brk = 1;
            }
        }
    }

    if ((thread_param.type == THREAD_CHECK_BOT) ||
        (thread_param.type == THREAD_CHECK_BOTH)) {
        if ((thread_param.ndl_all_len >= thread_param.judge_len) &&
            (thread_param.bot_exist)) {
            thread_get_bot_count();
            thread_clear_bot_count();
            if (thread_param.bot_cnt >= 2) {
                thread_param.bot_selfcnt = OSTimeGet();
            } else {
                if ((OSTimeGet() - thread_param.bot_selfcnt) >
                    500 * (OS_TICKS_PER_SEC / 1000)) {
                    brk = 1;
                }
            }
        }
    }

    if (brk)  // 有断线
    {
        thread_param.isbrk = 1;
        Para_head.stat    = STATUS_PATCH;
        // cmd_rsp_brk();
    } else {
        thread_param.isbrk = 0;
        Para_head.stat    = STATUS_NORMAL;
    }

    if (pre != brk) {
        pre = brk;
        // shell_printf("brk=%d\n", brk);
    }
}

// 底检断线判断
uint8_t thread_bot_judge(void)
{
    int     tmp;
    uint8_t brk = 0;


    if (thread_param.bot_cnt > 50)
        thread_param.bot_cnt = 50;


    if ((thread_param.ndl_all_len >= 50) && (thread_param.bot_sense > 5) &&
        (thread_param.ndl_all_num > 1))
        tmp = 2;
    else
        tmp = 1;

    if (thread_param.bot_cnt < tmp) {
        thread_param.bot_bkcnt++;
    } else {
        thread_param.bot_bkcnt  = 0;
    }

    if (thread_param.bot_bkcnt > thread_param.bot_filter) {
        brk++;
    } else {
        if (Para_head.mainshaft_run == 0) {
            brk = 0;
        }
    }

    thread_param.bot_cnt     = 0;
    thread_param.ndl_all_len = 0;
    thread_param.ndl_all_num = 0;
    if (++thread_param.judge_ndl_buf_id > thread_param.bot_filter)
        thread_param.judge_ndl_buf_id = 0;
    thread_param.judge_ndl =
        thread_param.judge_ndl_buf[thread_param.judge_ndl_buf_id];

    return brk;
}

uint8_t thread_bot_check(void)
{
    uint16_t tmp_cnt = 0;
//    uint16_t tmp_avge = 0;
    uint16_t tmp_len = 0;
    uint8_t tmp = 0;
    //uint8_t cur_avge = 0;
    //uint8_t judge_avge = 0;
    uint8_t len_ok = 0;
    //static volatile uint8_t ndl_num = 0;
    static uint8_t brk_num = 0;

    if (thread_param.bot_cnt_flag)
    {
        /* cacu addup len and botcnt */
        len_ok = BOT_BUF_SIZE;
        tmp = thread_bot_data.bot_cnt_num;
        //while (tmp_len < (BOT_JUDGE_LEN-(thread_param.bot_sense*20)))
        while (tmp_len < BOT_JUDGE_LEN)
        {
            if (tmp == 0)
            {
                tmp = BOT_BUF_SIZE;
            }
            tmp--;
            tmp_len += thread_bot_data.bot_len_buf[tmp];
            tmp_cnt += thread_bot_data.bot_cnt_buf[tmp];
            len_ok--;
            if (len_ok == 0)
            {
                break;
            }
        }

        //if (len_ok)
        {
            if (tmp_cnt < (BOT_JUDGE_MIN+thread_param.bot_sense*4*tmp_len/BOT_JUDGE_LEN))
            {//15针的总量，小于跟灵敏度相关的一个计算公式
                brk_num++;
                if (brk_num > thread_param.bot_filter)
                {
                    return 1;
                } else
                {
                    return 0;
                }
            } else
            {
                brk_num = 0;
            }
        }
    }

    return 0;
}



// 面检计数
void thread_top_count(void)
{
    static uint8_t pre = 0;
    uint8_t        i;

    if (thread_param.check_flag == CHECK_START_COUNT) {
        thread_param.top_mscnt++;
        i = Bsp_Thread_TopDetYes();
        if (i != pre) {
            pre = i;
        } else {
            if (i == 0) {
                thread_param.top_cnt++;
            }
        }
    }
}
int Thread_BotCheckNew()
{
    static int LastNdlLen = 0;
    static int LastBotCnt = 0;
    static int LastMinCnt = 0;
    int brk = 0;
    thread_get_bot_count();
    //新处理
    //int len = thread_param.ndl_cur_len;//3mm = 30 ndl_cur_len
    int len = LastNdlLen;//上一针的才对应
    //不打滑差不多1mm 1个值，灵敏度10接近不打滑，毛巾可能要用15
    int curCnt = len*thread_param.bot_sense/100;//灵敏度1~15
    int minCnt = Min(curCnt,LastMinCnt);//判断条件突变时，使用不容易报警的

    int filterPin = thread_param.bot_filter;

    if(minCnt <= 1)
    {
        minCnt = 1;
        if(filterPin < 5)//计数值太少的时候去抖针数不能太小
            filterPin = 5;
    }
    /*容易误报else if(thread_param.bot_cnt == 0 && LastBotCnt == 0)
    {//非较小针距，连续两针0，直接报警
        brk = 1;
    }*/

    if(thread_param.bot_cnt < minCnt)
    {
        thread_param.bot_bkcnt++;
    }
    else
        thread_param.bot_bkcnt = 0;

    if(thread_param.bot_bkcnt >= filterPin)//去抖1~9
    {
        brk = 1;
    }

    LastMinCnt = curCnt;
    LastBotCnt = thread_param.bot_cnt;
    LastNdlLen = thread_param.ndl_cur_len;
    thread_clear_bot_count();//判断完了就清除
    return brk;

}

// 断检判断
int Thread_BotCheck(void)
{

    if (thread_param.bot_exist == 0)
        return 0;

    if (thread_param.type != THREAD_CHECK_BOT && thread_param.type != THREAD_CHECK_BOTH)
        return 0;

    return Thread_BotCheckNew();

    /*旧处理
    int brk = 0;
    thread_get_bot_count();

    // 保存最近20次脉冲数
    thread_bot_data.bot_cnt_buf[thread_bot_data.bot_cnt_num] =
        thread_param.bot_cnt;

    thread_bot_data.bot_len_buf[thread_bot_data.bot_cnt_num] =
        thread_param.ndl_cur_len;

    if (++thread_bot_data.bot_cnt_num >= BOT_BUF_SIZE) {
        thread_bot_data.bot_cnt_num = 0;
    }


    brk += thread_bot_check();//15针的总量,是否小于(灵敏度*针长)

    if ((thread_param.ndl_all_num >= thread_param.judge_ndl))
    {//累加采集的针数 大于 要判断的针数，
        brk += thread_bot_judge();//会清除bot_count,后面不能调用thread_get_bot_count
        thread_clear_bot_count();
    }

    return brk; */

}
int Thread_TopCheck(void)
{
    uint8_t        brk = 0;
    uint16_t       tmp;
    uint16_t       tmp1;
    //计异常时间占比
    tmp = thread_param.top_cnt * 100 / thread_param.top_mscnt;
    thread_param.top_buf[3] = thread_param.top_buf[2];
    thread_param.top_buf[2] = thread_param.top_buf[1];
    thread_param.top_buf[1] = thread_param.top_buf[0];
    thread_param.top_buf[0] = tmp;

    thread_param.top_cnt   = 0;
    thread_param.top_mscnt = 0;


    /* check top thread */
    if ((thread_param.type == THREAD_CHECK_TOP) ||
        (thread_param.type == THREAD_CHECK_BOTH))
    {
        //做平均，当前占比高点
        tmp = (thread_param.top_buf[0] * 2 + thread_param.top_buf[1] +
               thread_param.top_buf[2] + thread_param.top_buf[3]) / 5;

        if (thread_param.run_ndl_num < 5)  // 启动后前几针
        {
            tmp1 = 1;//低档灵敏度
        }
        else
        {
            if (thread_param.ndl_cur_len < 15)  // 当前针长
                tmp1 = 2;//中档灵敏度
            else
                tmp1 = 3;//高灵敏度
        }

        if (tmp < thread_param.top_sense * tmp1)  // thread_param.top_sense最多15
        {
            thread_param.top_bkcnt++;
        } else {
            thread_param.top_bkcnt = 0;
        }

        if (thread_param.top_bkcnt > thread_param.top_filter_adj) {
            brk++;
        } else {
            if (Para_head.mainshaft_run == 0) {
                brk = 0;
            }
        }
    }
    return brk;

}


// 断检判断
void Fun_Thread_BrkCheckMs(void)
{//必须1ms调用1次，要计时

    uint8_t        brk = 0;
    static uint8_t pre_stat = 0;

    Para_emb.IsBrk = thread_param.isbrk;//全局有其他地方用到
    if (thread_param.is_indep_check)  // 独立断检
    {
        return;
    }

    if (Para_head.mainshaft_run)//主轴启动
    {
        if ((Para_emb.stat == STATUS_PATCH) && (Para_head.stat == STATUS_NORMAL))//正常下，不需要补绣
        {//补绣后正常机头起绣不检测针数
            thread_param.ndl_patch_filter_cnt = thread_param.ndl_start_filter;
        }
    }

    thread_top_count();//必须1ms调用1次，要计时

    if ((Para_head.stat == STATUS_OFF)
        || (thread_param.en == 0)
        || (thread_param.type == THREAD_CHECK_NO)
        || ((Para_emb.stat == STATUS_PATCH) && (Para_head.stat == STATUS_NORMAL)))
    {//无需检测
        if (thread_param.isbrk)  // 有断线
        {
            thread_param.isbrk = 0;
            Para_head.stat    = STATUS_NORMAL;
        }

        if (pre_stat == 0)
            pre_stat = 1;

        return;
    }
    else
    {
        if (pre_stat == 1) {//首次开启，初始化
            pre_stat                      = 0;
            thread_param.judge_ndl_buf_id = 0;
            thread_param.ndl_all_len      = 0;
            thread_param.ndl_all_num      = 0;
        }
    }
	if (thread_param.splice_no_judge_flag && Para_head.is_splice)
	{
        if (thread_param.selfcheck_flag == 1)
        {
            thread_param.isbrk = 0;
            Para_head.stat = STATUS_NORMAL;
            pre_stat = 1;
        }
		return;
	}

    // 收到新针长判断一次
    if (thread_param.IscheckNow == 1)
    {
        thread_param.IscheckNow = 0;

        if (thread_param.selfcheck_flag == 1)
        {
            thread_brk_selfcheck();
            return;
        }

        brk = Thread_TopCheck();//----------面检判断
        brk += Thread_BotCheck(); //----------底检判断

        if (brk)  // 有断线
        {
            if (thread_param.isbrk == 0)
                Mod_Cmd_RspBrk();//上传报警
            thread_param.isbrk = 1;
            Para_head.stat    = STATUS_PATCH;
        }
        else if ((Para_emb.stat == STATUS_TSTBRK)||(Para_emb.stat == STATUS_STOP))
        {
            Para_head.stat = STATUS_NORMAL;
        }
    }
}

void Fun_Thread_NewNdlLen(uint8_t param)
{//收到新针长命令，断检处理

    thread_param.ndl_all_len += param;
    thread_param.ndl_all_num++;
    thread_param.ndl_cur_len = param;
    new_ndl_len_cnt++;

    if (new_ndl_len_cnt > BOT_BUF_SIZE + 1) {
        thread_param.bot_cnt_flag = 1;
    }

    if (thread_param.check_flag == CHECK_START_COUNT) {
        thread_param.IscheckNow = 1;
        thread_param.check_flag = CHECK_NONE;
    }




}


void Fun_Thread_NewNdl()
{
    if (thread_param.run_ndl_num++ > 10000)
        thread_param.run_ndl_num = 10000;

    if (thread_param.ndl_patch_filter_cnt > 0)
        thread_param.ndl_patch_filter_cnt--;

}

void Fun_Thread_MainStop()
{
    thread_param.run_ndl_num          = 0;
    thread_param.ndl_patch_filter_cnt = 0;
    thread_top_param_init();

}
void Fun_Thread_SetNdlNum(uint8_t num)
{
    thread_param.ndl_num = num;
    Para_head.ndl_num = num;                           // 机型针数
    
}

void Fun_Thread_SetCurNdlNum(uint8_t num)
{
    thread_param.ndl_cur_num = num;
    thread_chose_ndl(num);//硬件选择对应针号地址
}
void Fun_Thread_SetStartFilter(uint8_t filter)
{
    thread_param.ndl_start_filter = filter;


}
void Fun_Thread_SetType(uint8_t type)
{
    if (type <= THREAD_CHECK_BOTH)
        thread_param.type = (THREAD_CHECK_TYPE)type;
}

void Fun_Thread_SetBotSense(uint8_t param)
{
    thread_param.bot_sense = (param > 15) ? 15 : param;
    if (thread_param.bot_sense < 1)
        thread_param.bot_sense = 1;
    thread_param.judge_len =
        35 - thread_param.bot_sense * 2;
    thread_param.judge_ndl =
        (16 - thread_param.bot_sense + 4) /5;  // 5档为1档共3档

    thread_cal_bot_judge();

}

void Fun_Thread_SetTopFilter(uint8_t param)
{
    thread_param.top_filter = (param < 3) ? 3 : param;
    thread_param.top_filter_adj =
        thread_param.top_filter +
        (20 - thread_param.top_sense) / 5;//重新计算

}
void Fun_Thread_SetTopSense(uint8_t param)
{

    thread_param.top_sense = (param > 15) ? 15 : param;
    thread_param.top_filter_adj =
        thread_param.top_filter +
        (20 - thread_param.top_sense) / 5;//重新计算

}


void Fun_Thread_SetBotFilter(uint8_t param)
{
    thread_param.bot_filter = (param < 1) ? 1 : param;
    if (thread_param.bot_filter > BOT_MAX_FILTER)
        thread_param.bot_filter = BOT_MAX_FILTER;
    thread_cal_bot_judge();

}

void Fun_Thread_SetIndepCheck(uint8_t param)
{
    if ((param == 0xFF)  // 外围板开启独立断检
        || ((param == 1)
            && (thread_param.ndl_num
                > 20))  // 主控在针棒数目大于16时，会下发使用独立断检的命令，导致20针机型出问题
    ) {
        thread_param.is_indep_check = 1;
        thread_param.en             = 0;
    } else {
        thread_param.is_indep_check = 0;
    }
}

void Fun_Thread_SetEn(int en)
{
    if (thread_param.is_indep_check)//独立断检
        return;

    thread_param.en = en;

}
void Fun_Thread_CheckStart()
{
    if (thread_param.is_indep_check)//独立断检
        return;

    thread_param.check_flag = CHECK_START_COUNT;

    //补绣后正常机头不检测针数
    if (thread_param.ndl_patch_filter_cnt)
    {
       thread_param.check_flag = CHECK_NONE;
       thread_param.ndl_all_len = 0;
       thread_param.ndl_all_num = 0;
       thread_clear_bot_count();
    }

}

void Fun_Thread_SetSelfCheck(int en)
{
    thread_param.selfcheck_flag = en;
    thread_param.top_selfcnt = OSTimeGet();
    thread_param.bot_selfcnt = OSTimeGet();
    thread_param.isbrk = 0;

}

void Fun_Thread_ClrBrk()
{
    thread_param.isbrk = 0;
}
void Fun_Thread_SetBrk()
{
    thread_param.isbrk = 1;
}
uint32_t Fun_Thread_GetBrk()
{
    return thread_param.isbrk;
}
void Fun_Thread_SetNdlNumReverse(uint8_t rev)
{
    thread_param.ndl_num_reverse = rev;
}

void Fun_Thread_SetNdlNum_Mode_Type(uint8_t _mode,uint8_t _type)
{
    thread_param.ndl_num_mode = _mode;//         Oxaa底检方式值
    thread_param.ndl_num_type = _type;//	     Oxbb 底检模式值
    Bsp_Thread_ERED_SelectNdl(thread_param.ndl_channel-1,thread_param.ndl_num_mode,thread_param.ndl_num_type,thread_param._a_opto_num,thread_param._b_opto_num);

}

void Fun_thread_ExtInfoBrk(uint32_t brk)
{
    thread_param.isbrk = brk;
    Para_emb.IsBrk = thread_param.isbrk;//马上更新
}

uint32_t Fun_Thread_GetIndepChk(void)
{
    return thread_param.is_indep_check;
}
//拼接断检添加
uint32_t Fun_Thread_Splice_no_judge_flag(void)
{
    return thread_param.splice_no_judge_flag;
}
void Fun_thread_Splice_head_ndl_num(uint32_t splice_head_ndl_num)
{
     thread_param.splice_head_ndl_num = splice_head_ndl_num;
}
/*****************************************************************************
 函 数 名  : Fun_thread_splice_head_group_calc
 功能描述  : 拼接机头组计算
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
void Fun_thread_splice_head_group_calc(void)
{
    if (Para_head.is_splice)
    {
        Para_head.splice_master = 0;
        if ((Para_head.addr)%(Para_head.splice_head_num) == 1)
        {
            Para_head.splice_master = 1;
        }
    }
}
/*****************************************************************************
 函 数 名  : uint8_t Fun_thread_Is_splice_samegroup(uint8_t idno)
 功能描述  : 拼接机头组计算
 输入参数  : idno --> head_addr
 输出参数  :
 返 回 值  : 1 samegroup, 0 notsame
*****************************************************************************/
uint8_t Fun_thread_Is_splice_samegroup(uint8_t idno)
{
	uint8_t splice_group = 0;
	uint8_t splice_tmp = 0;

	splice_group = (Para_head.addr)/(Para_head.splice_head_num);
	if ((Para_head.addr)%(Para_head.splice_head_num))
	{
		splice_group += 1;
	}

	splice_tmp = idno /Para_head.splice_head_num;
	if (idno % Para_head.splice_head_num)
	{
		splice_tmp += 1;
	}

	if (splice_group == splice_tmp)
	{
        return(1);
	} else
    {
        return(0);
    }
}
