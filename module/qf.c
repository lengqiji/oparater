#include "rn_typedef.h"
#include "bsp_qf.h"
#include "qf.h"
#include "para.h"
#include "alarm.h"

#define MAINSHAFT_PPR (4000)  // 主轴光电编码器四倍频后一转脉冲数

const uint16_t c_usQfErrMax = MAINSHAFT_PPR * 1.05;
const uint16_t c_usQfErrMin = MAINSHAFT_PPR * 0.95;

static uint Qf_Rpm = 0;

// 参数结构体
typedef struct {
    uint32_t          pulse_width;  // 脉宽
    uint32_t          cnt;          // 计数
    volatile uint16_t qf_err_cnt;   // 四倍频错误计数
    volatile uint16_t qf_no_cnt;    // 无四倍频计数
} qf_param_s;
qf_param_s qf_param;

void Mod_Qf_ErrJudge(void)
{
    qf_param.cnt = Bsp_Qf_Read();

    if (Para_head.mainshaft_run) {

        if ((qf_param.cnt < c_usQfErrMax) && (qf_param.cnt > c_usQfErrMin))  // 有四倍频脉冲
        {
            qf_param.qf_err_cnt = 0;
            qf_param.qf_no_cnt  = 0;
        } else if (qf_param.cnt > c_usQfErrMax) {
            qf_param.qf_err_cnt += 1;
            qf_param.qf_no_cnt = 0;
        } else if (qf_param.cnt < c_usQfErrMin) {
            qf_param.qf_no_cnt += 1;
            qf_param.qf_err_cnt = 0;
        } else {
            qf_param.qf_err_cnt += 1;
        }

        // 连续30次四倍频超过4200或者1001-3799报四倍频错误
        if (qf_param.qf_err_cnt > 50) {
            Alarm_Set(ALARM_BIT_QF_ERR);
            // shell_printf("***ALARM_BIT_QF_ERR\r\n");
        }

        // 连续30次四倍频少于1000报无四倍频
        if (qf_param.qf_no_cnt > 50) {
            Alarm_Set(ALARM_BIT_NO_QF);
            // shell_printf("***ALARM_BIT_NO_QF\r\n");
        }
    }
}

INT32U Qf_AngleToCnt(INT32S v)
{
    return (v * MAINSHAFT_PPR / 360);
}
INT32U Qf_CntToAngle(INT32S v)
{
    return (v * 360 / MAINSHAFT_PPR);
}



void Mod_Qf_ErrClear(void)
{
    qf_param.qf_err_cnt = 0;
    qf_param.qf_no_cnt  = 0;

}

INT32U Mod_Qf_Read()//余每圈qf
{
    return Bsp_Qf_Read()%MAINSHAFT_PPR;
}

INT32U Mod_Qf_AngleRead()
{// 平绣主控是在100度的时候qf清0，qf为0 的时候其实是100度
    return (Qf_CntToAngle(Mod_Qf_Read())+100)%360;
}

void Mod_Qf_RpmUpdateMs()
{
    static int MsCount = 0;
    static int Qf_last = 0;

    MsCount ++;
    if(MsCount >= 10)//暂时按最高2000转，时间至少有30ms
    {
        MsCount = 0;
        int newQf = Mod_Qf_Read();
        int qfOffset = newQf + MAINSHAFT_PPR - Qf_last;
        qfOffset %= MAINSHAFT_PPR;

        Qf_Rpm = qfOffset*6000/MAINSHAFT_PPR;
        Qf_last = newQf;
    }
    
}
INT32U Mod_Qf_RpmRead()
{
    return Qf_Rpm;
}

