#include "rn_typedef.h"
#include "alarm.h"
#define MAX_ALARM 32

////考虑到多任务调用，改成数组
//static uint alarm_val = 0;//报警值
static uint alarm_state[MAX_ALARM] = {0};//报警值



void Alarm_Set(int loc)
{
    if(loc < MAX_ALARM)
        alarm_state[loc] = 1;
}
uint Alarm_GetAll()
{
    uint alarm_val = 0;
    for(int i=0;i<MAX_ALARM;i++)
    {
        if(alarm_state[i])
            SetBit(alarm_val, i);
    }
    
    return alarm_val;
}

void Alarm_Clr(int loc)
{
    if(loc < MAX_ALARM)
        alarm_state[loc] = 0;
}
void Alarm_ClrAll()
{
    for(int i=0;i<MAX_ALARM;i++)
    {
       alarm_state[i] = 0;
    }

}

