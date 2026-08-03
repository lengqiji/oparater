#pragma once

#define PRINT_USE_UART      0
#define PRINT_USE_JLINK_RTT 1
#define DEBUG_TOOL          PRINT_USE_JLINK_RTT

#if (DEBUG_TOOL == PRINT_USE_JLINK_RTT)
    #include "SEGGER_RTT.h"
    #define PRINT_FUNC(fmt, ...) SEGGER_RTT_printf(0, fmt, ##__VA_ARGS__)
#elif (DEBUG_TOOL == PRINT_USE_UART)
    #define PRINT_FUNC(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

//#define DEBUG_ON
//extern INT32U  OSTimeGet (void);
#ifdef DEBUG_ON

    #define shell_printf(fmt, ...)                                             \
        do {                                                                   \
            PRINT_FUNC(fmt, ##__VA_ARGS__);                                    \
        } while (0)

    #define INFO_PRINT(fmt, ...)                                               \
        do {                                                                   \
            PRINT_FUNC("Info %s,%s,%d,%d: " fmt "",                            \
                       __FILE__,                                               \
                       __FUNCTION__,                                           \
                       __LINE__,                                               \
                       OSTimeGet(),                                            \
                       ##__VA_ARGS__);                                         \
        } while (0)

    #define WARNING_PRINT(fmt, ...)                                            \
        do {                                                                   \
            PRINT_FUNC("Warning %s,%s,%d,%d: " fmt "",                         \
                       __FILE__,                                               \
                       __FUNCTION__,                                           \
                       __LINE__,                                               \
                       OSTimeGet(),                                            \
                       ##__VA_ARGS__);                                         \
        } while (0)

    #define DEBUG_PRINT(fmt, ...)                                              \
        do {                                                                   \
            PRINT_FUNC("Debug %s,%s,%d,%d: " fmt "",                           \
                       __FILE__,                                               \
                       __FUNCTION__,                                           \
                       __LINE__,                                               \
                       OSTimeGet(),                                            \
                       ##__VA_ARGS__);                                         \
        } while (0)

    #define ERROR_PRINT(fmt, ...)                                              \
        do {                                                                   \
            PRINT_FUNC("Error %s,%s,%d,%d: " fmt "",                           \
                       __FILE__,                                               \
                       __FUNCTION__,                                           \
                       __LINE__,                                               \
                       OSTimeGet(),                                            \
                       ##__VA_ARGS__);                                         \
        } while (0)

#else

    #define shell_printf(fmt, ...)                                             \
        do {                                                                   \
        } while (0)

    #define INFO_PRINT(fmt, ...)                                               \
        do {                                                                   \
        } while (0)

    #define WARNING_PRINT(fmt, ...)                                            \
        do {                                                                   \
        } while (0)

    #define DEBUG_PRINT(fmt, ...)                                              \
        do {                                                                   \
        } while (0)

    #define ERROR_PRINT(fmt, ...)                                              \
        do {                                                                   \
        } while (0)

#endif
