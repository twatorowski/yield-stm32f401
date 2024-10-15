/**
 * @file debug.h
 *
 * @date 29.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief Debugging macros
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stddef.h>

#include "config.h"
#include "compiler.h"
#include "dev/usart.h"
#include "dev/usart_dev.h"
#include "util/concatstr.h"
#include "util/stdio.h"
#include "sys/yield.h"
#include "sys/time.h"

/* disables debug globally */
#if !(DEVELOPMENT)
    #undef DEBUG
#endif

/* debug enabled? */
#ifdef DEBUG

/* debug buffer */
extern char debug_buf[];
/** #brief length of the data in debug buffer */
extern size_t debug_buf_len;

/* debug message prefix */
#define DBG_MSG_PRFX                                                        \
    "+D: [" __FILE__ ":" CONCATSTR(__LINE__)":%d]"

/**
 * @brief debug printf-like function
 */
#define dprintf(fmt, ...)                                                   \
    /* encapsulated in a loop, to make it compiler-proof :) */              \
    do {                                                                    \
        /* debug buffer is occupied? */                                     \
        while (debug_buf_len)                                               \
            Yield();                                                        \
        /* produce string */                                                \
        debug_buf_len = snprintf(debug_buf, DEBUG_MAX_LINE_LEN,             \
            DBG_MSG_PRFX fmt, time(0), ## __VA_ARGS__);                     \
        /* try to send debug over the tp */                                 \
        USART_Send(&usart1, debug_buf, debug_buf_len, 0);                   \
        /* release the buffer */                                            \
        debug_buf_len = 0;                                                  \
    } while (0)
#else
/**
 * @brief debug printf-like function
 */
#define dprintf(fmt, ...)                                                   \
    /* encapsulated in a loop, to make it compiler-proof :) */              \
    do {                                                                    \
    } while (0)

#endif


#endif /* DEBUG_H */
