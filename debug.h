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

/** levels of debug  */
#define DLVL_DEBUG                                          1
#define DLVL_INFO                                           10
#define DLVL_WARN                                           20
#define DLVL_ERROR                                          30


/**
 * @brief initialize debugging mode
 *
 * @return err_t error code
 */
err_t Debug_Init(void);

/**
 * @brief returns the string name of given debug level
 *
 * @param lvl level value (from enum)
 * @return const char* pointer to the string name
 */
const char * Debug_GetLevelName(int lvl);

/**
 * @brief set global debug level
 *
 * @param lvl level to be set
 * @return err_t error code
 */
err_t Debug_SetGlobalLevel(int lvl);

/**
 * @brief send message over debug interfaces
 *
 * @param ptr pointer to the message
 * @param size message length
 *
 * @return err_t error code
 */
err_t Debug_Send(const void *ptr, size_t size);


/* debug enabled? */
#ifdef DEBUG

/* debug buffer */
extern char debug_buf[];
/** #brief length of the data in debug buffer */
extern size_t debug_buf_len;
/* global level of debug */
extern int debug_global_lvl;

/* this is a fix for debug being defined with no value */
#if ~(~DEBUG + 0) == 0 && ~(~DEBUG + 1) == 1
    /* we want to override the value of the debug */
    #undef DEBUG
    /* here we asume that no value means user wants everything */
    #define DEBUG debug_global_lvl
#endif


/* debug message prefix */
#define DBG_MSG_PRFX                                                        \
    "+D: [" __FILE__ ":" CONCATSTR(__LINE__)":%d:%s]"
/**
 * @brief debug printf-like function
 */
#define dprintf(__lvl, fmt, ...)                                            \
    /* encapsulated in a loop, to make it compiler-proof :) */              \
    do {                                                                    \
        if ((__lvl) < (DEBUG))                                              \
            break;                                                          \
        /* debug buffer is occupied? */                                     \
        while (debug_buf_len)                                               \
            Yield();                                                        \
        /* produce string */                                                \
        debug_buf_len = snprintf(debug_buf, DEBUG_MAX_LINE_LEN,             \
            DBG_MSG_PRFX fmt, time(0), Debug_GetLevelName(__lvl), ## __VA_ARGS__); \
        /* try to send debug over the tp */                                 \
        Debug_Send(debug_buf, debug_buf_len);                               \
        /* release the buffer */                                            \
        debug_buf_len = 0;                                                  \
    } while (0)

// /** helper macros */
// #define dprintf_d(fmt, ...)             dprintf(DLVL_DEBUG, fmt, __VA_ARGS__)
// #define dprintf_i(fmt, ...)             dprintf(DLVL_INFO, fmt, __VA_ARGS__)
// #define dprintf_w(fmt, ...)             dprintf(DLVL_WARN, fmt, __VA_ARGS__)
// #define dprintf_e(fmt, ...)             dprintf(DLVL_ERROR, fmt, __VA_ARGS__)
#else
/**
 * @brief debug printf-like function
 */
#define dprintf(__lvl, fmt, ...)                                            \
    /* encapsulated in a loop, to make it compiler-proof :) */              \
    do {                                                                    \
    } while (0)

#endif

/** helper macros */
#define dprintf_d(fmt, ...)             dprintf(DLVL_DEBUG, fmt, __VA_ARGS__)
#define dprintf_i(fmt, ...)             dprintf(DLVL_INFO, fmt, __VA_ARGS__)
#define dprintf_w(fmt, ...)             dprintf(DLVL_WARN, fmt, __VA_ARGS__)
#define dprintf_e(fmt, ...)             dprintf(DLVL_ERROR, fmt, __VA_ARGS__)

#endif /* DEBUG_H */
