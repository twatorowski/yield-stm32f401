/**
 * @file debug.c
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-05
 *
 * @brief Printf Debugging
 */

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "config.h"

#include "debug.h"
#include "stm32f401/scb.h"

/* debug interfaces  */
#include "dev/usart.h"
#include "dev/usart_dev.h"
#include "dev/usb_vcp.h"


/* debug line buffer */
char debug_buf[DEBUG_MAX_LINE_LEN];
/* current debug buffer contents length */
size_t debug_buf_len;

/* global level of debug */
int debug_global_lvl = DEBUG_DEFAULT_LEVEL;

/* get debug level name */
const char * Debug_GetLevelName(int lvl)
{
    /* switch on the enum and return the string name */
    switch (lvl) {
    case DLVL_DEBUG: return "DEBUG";
    case DLVL_INFO: return "INFO";
    case DLVL_WARN: return "WARN";
    case DLVL_ERROR: return "ERROR";
    }
    /* unknown level name */
    return "UNKNOWN";
}

/* set global debug level */
err_t Debug_SetGlobalLevel(int lvl)
{
    /* switch on the enum and return the string name */
    switch (lvl) {
    case DLVL_DEBUG:
    case DLVL_INFO:
    case DLVL_WARN:
    case DLVL_ERROR:
        debug_global_lvl = lvl; return EOK;
    }

    /* unknown level */
    return EARGVAL;
}

/* send message over debug interfaces */
err_t Debug_Send(const void *ptr, size_t size)
{
    /* send over uart */
    USART_Send(&usart1, debug_buf, debug_buf_len, 0);

    /* report status */
    return EOK;
}

/* initialize debugging mode */
err_t Debug_Init(void)
{
    /* if the development mode is enabled then disable write caching so that we
     * always get precise errors during bus-fault etc. */
    #if DEVELOPMENT
        SCB_SCS->ACTLR |= SCB_ACTLR_DISDEFWBUF;
    #endif

    /* report status */
    return EOK;
}