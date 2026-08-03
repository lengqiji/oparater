#include "rn_typedef.h"
#include "module.h"
#include "cmd.h"
#include "cmd_def.h"
#include "stepper_motor.h"
#include "qf.h"


void Module_Init()
{
    Mod_Cmd_Init();
    Mod_Motor_Init();
}
