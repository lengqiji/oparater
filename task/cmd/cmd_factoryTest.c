#include "bsp_can.h"
#include "cmd_def.h"
#include "factory_test.h"

void Task_Cmd_FactoryTest(struct can_message *pFrame)
{
    void (*test_op)(struct can_message *) = Fun_Test_Op[pFrame->data[0]];
    if (test_op) {
        test_op(pFrame);
    }
}
