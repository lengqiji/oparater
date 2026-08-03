#ifndef __MOTOR_SPDTAB_CALC_H__
#define __MOTOR_SPDTAB_CALC_H__

#include <math.h>
#include <stdint.h>

#define RPM_2_PLS(speed, sglPls) ((speed) * (sglPls) / 60) /* rpm->step/s */

static inline void Calc_S_Curve(int           *Fre,
                                const uint16_t tabLen,
                                float          rpmBegin,
                                float          rpmEnd,
                                uint16_t       usSglPls,
                                uint8_t        A,
                                uint8_t        B)
{
    const float c_fNum = (float) tabLen / B;
    float       X;
    float       StartFre;
    float       EndFre;
    bool        bIsDec = false;

    if (rpmBegin > rpmEnd) { /* 减速曲线 */
        bIsDec   = true;
        StartFre = RPM_2_PLS(rpmEnd, usSglPls);
        EndFre   = RPM_2_PLS(rpmBegin, usSglPls);
    } else { /* 加速曲线 */
        bIsDec   = false;
        StartFre = RPM_2_PLS(rpmBegin, usSglPls);
        EndFre   = RPM_2_PLS(rpmEnd, usSglPls);
    }

    for (uint16_t i = 0; i < tabLen; i++) {
        X = A * (i - c_fNum) / c_fNum;
        /*
         * 当x趋向于负无穷时，SIGMOID趋向于0，
         * 当x等于0时候，SIGMOID等于0.5，
         * 当x趋向于正无穷时，SIGMOID趋向于1
         */
        Fre[i] = StartFre + (EndFre - StartFre) / (1 + expf(-X));
    }
    /*减速运动，倒序排列*/
    if (bIsDec == true) {
        uint16_t tmp_pxSpeed = 0;
        /* 倒序排序 */
        for (uint16_t i = 0; i < (tabLen / 2); i++) {
            tmp_pxSpeed         = Fre[i];
            Fre[i]              = Fre[tabLen - 1 - i];
            Fre[tabLen - 1 - i] = tmp_pxSpeed;
        }
    }
}

#endif
