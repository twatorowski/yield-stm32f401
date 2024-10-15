/**
 * @file err.h
 *
 * @date 2020-03-12
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief Error codes definitions
 */

#ifndef ERR_H
#define ERR_H

#include <stdint.h>

/** error codes present in system */
typedef enum err {
    /** secure the size of enum */
    _MAX_VAL = INT32_MAX,

    /* general errors */
    /** no error */
    EOK = 0,
    /** fatal error */
    EFATAL = -1,
    /** resource busy */
    EBUSY = -2,
    /** argument value error */
    EARGVAL = -3,
    /** routine is to be called again (polled upon) */
    EAGAIN = -4,
    /** timeout has occured */
    ETIMEOUT = -5,
    /** task cancelled */
    ECANCEL = -6,


    /** i2c errors  */
    /** bus arbitration lost */
    EI2C_ARB_LOST = -40,
    /** bus timeout while waiting for scl to free up */
    EI2C_BUS_TIMEOUT = -41,
    /** nack received */
    EI2C_NACK = -42,
    /** error during start condition state */
    EI2C_START = -43,
    /** error during arressing phase */
    EI2C_ADDR = -44,
    /**  error during stop condition */
    EI2C_STOP = -45,
    
} err_t;


#endif /** ERR_H */
