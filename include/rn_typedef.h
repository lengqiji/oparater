#ifndef _RN_TYPEDEF_H_
#define _RN_TYPEDEF_H_

#include "stdlib.h"
#include <stdbool.h>
#include <string.h>
#include <stdint.h>




//type define
typedef     signed   int                    sint;
typedef     unsigned int                    uint;

typedef     signed char                     int8;
typedef     unsigned char                   uchar;
typedef     signed short                    int16;
typedef     signed long                     int32;
typedef     signed long long                int64;

typedef     signed int                     int32_t;
typedef     unsigned char                   uint8_t;
typedef     unsigned short                  uint16_t;
typedef     unsigned int                   uint32_t;

typedef     unsigned char                   uint8;
typedef     unsigned short                  uint16;
typedef     unsigned long                   uint32;
typedef     unsigned long long              uint64;

typedef     volatile signed char            vint8;
typedef     volatile signed short           vint16;
typedef     volatile signed long            vint32;

typedef     volatile unsigned char          vuint8;
typedef     volatile unsigned short         vuint16;
typedef     volatile unsigned long          vuint32;

typedef     signed const char               intc8;              /* Read Only */
typedef     signed const short              intc16;             /* Read Only */
typedef     signed const long               intc32;             /* Read Only */

typedef     unsigned const char             uintc8;             /* Read Only */
typedef     unsigned const short            uintc16;            /* Read Only */
typedef     unsigned const long             uintc32;            /* Read Only */

typedef     signed   int                    INT;
typedef     unsigned int                    UINT;


typedef     signed char                     INT8;
typedef     signed short                    INT16;
typedef     signed long                     INT32;
typedef     long long                       INT64;


typedef     signed char                     INT8S;
typedef     signed short                    INT16S;
typedef     signed int                      INT32S;
typedef     long long                       INT64S;

typedef     unsigned char                   UINT8;
typedef     unsigned short                  UINT16;
typedef     unsigned int                    UINT32;
typedef     unsigned long long              UINT64;


typedef     unsigned char                   INT8U;
typedef     unsigned short                  INT16U;
typedef     unsigned int                   INT32U;
typedef     unsigned long long              INT64U;


typedef     volatile signed char            VINT8;
typedef     volatile signed short           VINT16;
typedef     volatile signed long            VINT32;

typedef     volatile unsigned char          VUINT8;
typedef     volatile unsigned short         VUINT16;
typedef     volatile unsigned long          VUINT32;

typedef     signed const char               INTC8;              /* Read Only */
typedef     signed const short              INTC16;             /* Read Only */
typedef     signed const long               INTC32;             /* Read Only */

typedef     unsigned const char             UINTC8;             /* Read Only */
typedef     unsigned const short            UINTC16;            /* Read Only */
typedef     unsigned const long             UINTC32;            /* Read Only */


#define RdBit(value, num) (((value)>>(num)) & 0x01)   // 读某位
#define ClrBit(value, num)  ((value) &= ~(0x01<<(num))) // 某位清0
#define SetBit(value, num)  ((value) |= (0x01<<(num))) // 某位置1
#define TglBit(value, num)  ((value) ^= (0x01<<(num)))  // 位取反


#define Min(a, b)           (((a) < (b)) ?  (a) : (b))
#define Max(a, b)           (((a) > (b)) ?  (a) : (b))


/*
********************************************************************************
*
*                         End of typedef.h
*
********************************************************************************
*/
#endif
