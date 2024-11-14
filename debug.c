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