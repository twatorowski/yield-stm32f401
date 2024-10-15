/**
 * @file strerr.c
 *
 * @date 2019-09-09
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief Translate error code to human readable form
 */

#include "err.h"
#include "util/elems.h"

#define PRFX(str)                               str": "

/* error string prefixes */

/* can errors */
#define PRFX_CAN                                "can: "

/* max31865 sensor errors */
#define PRFX_MAX31865                           "max31865: "

/* at parser */
#define PRFX_AT                                 PRFX("at")
/* i2c errors */
#define PRFX_I2C                                PRFX("i2c")
/* temperature sensor */
#define PRFX_TMP100                             PRFX("tmp100")
/* max31856 sensor errors */
#define PRFX_MAX31856                           PRFX("max31856")
/* lepton infrared camera */
#define PRFX_LEPTON                             PRFX("lepton")
/* pyrometer error prefix */
#define PRFX_MLX                                PRFX("mlx90614")

/* error strings */
static const char *err_str[] = {
    /* general errors */
    [-EOK] =
        "success",
    [-EFATAL] =
        "fatal error",
    [-EBUSY] =
        "resource busy",
    [-EARGVAL] =
        "invalid argument value",
    [-ETIMEOUT] =
        "operation timeout",

    /* i2c errors */
    [-EI2C_ADDR] =
        PRFX_I2C "i2c address did not respond",
    [-EI2C_NACK] =
        PRFX_I2C "i2c nack received",
    [-EI2C_BUS_TIMEOUT] =
        PRFX_I2C "i2c bus timeout",

};

/* return the error string for given error code (or empty string
 * if not found) */
const char * strerr(err_t ec)
{
    /* we use negative error codes in the system, so lets negate */
    ec = -ec;
    /* outside the boundaries? */
    if (ec < 0 || ec >= (int)elems(err_str))
        return "unknown error";
    /* no valid error string? */
    if (!err_str[ec])
        return "unknown error";
    /* got valid error string */
    return err_str[ec];
}
