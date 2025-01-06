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


/* offset error code from the base error code value */
#define FROM_BASE(base, offset)         ((base) - (offset))

/* base error code values */
enum err_base {
    /* usb errors base error code */
    EUSB_BASE = -20,
    /* i2c errors base error code */
    EI2C_BASE = -40
};

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
    /** unknown protocol */
    EUNKPROT = -7,
    /** unknown request */
    EUNKREQ = -8,
    /** malformed packet */
    EMALFORMED = -8,
    /** unknown address */
    EUNKADDR = -9,
    /** address unreachable */
    EUNREACHABLE = -10,
    /** no connection is established */
    ENOCONNECT = -11,


    /* usb errors */
    /* usb reset has occured */
    EUSB_RESET = FROM_BASE(EUSB_BASE, 0),
    /* usb endpoint disabled */
    EUSB_EP_DIS = FROM_BASE(EUSB_BASE, 1),
    /* usb endpoint disabled */
    EUSB_INACTIVE = FROM_BASE(EUSB_BASE, 2),


    /** i2c errors  */
    /** bus arbitration lost */
    EI2C_ARB_LOST = FROM_BASE(EI2C_BASE, 0),
    /** bus timeout while waiting for scl to free up */
    EI2C_BUS_TIMEOUT = FROM_BASE(EI2C_BASE, 1),
    /** nack received */
    EI2C_NACK = FROM_BASE(EI2C_BASE, 2),
    /** error during start condition state */
    EI2C_START = FROM_BASE(EI2C_BASE, 3),
    /** error during arressing phase */
    EI2C_ADDR = FROM_BASE(EI2C_BASE, 4),
    /**  error during stop condition */
    EI2C_STOP = FROM_BASE(EI2C_BASE, 5),
    
} err_t;


#endif /** ERR_H */
