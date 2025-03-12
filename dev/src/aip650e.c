/**
 * @file aip650e.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "dev/aip650e.h"
#include "dev/swi2c.h"
#include "dev/swi2c_dev.h"

#define DEBUG DLVL_INFO
#include "debug.h"

/* commands */
#define AIP650E_CMD_WRITE_DATA_DIG1             (0x68 >> 1)
#define AIP650E_CMD_WRITE_DATA_DIG2             (0x6a >> 1)
#define AIP650E_CMD_WRITE_DATA_DIG3             (0x6c >> 1)
#define AIP650E_CMD_WRITE_DATA_DIG4             (0x6e >> 1)
#define AIP650E_CMD_SET_PARAM                   (0x48 >> 1)
#define AIP650E_CMD_GET_KEY                     (0x49 >> 1)


/* initialize common part of the device driver */
err_t AIP650E_Init(void)
{
    /* report status */
    return EOK;
}

/* initialize particular device */
err_t AIP650E_DevInit(aip650e_dev_t *dev)
{
    /* error code */
    err_t ec = EOK;

    /* turn off the display */
    if (!ec) ec = AIP650E_Configure(dev, AIP650E_CFG_DISPLAY_OFF);
    /* clear the memory */
    if (!ec) ec = AIP650E_SetSegments(dev, AIP650E_DIGIT_1, AIP650E_SEGMENT_NONE);
    if (!ec) ec = AIP650E_SetSegments(dev, AIP650E_DIGIT_2, AIP650E_SEGMENT_NONE);
    if (!ec) ec = AIP650E_SetSegments(dev, AIP650E_DIGIT_3, AIP650E_SEGMENT_NONE);
    if (!ec) ec = AIP650E_SetSegments(dev, AIP650E_DIGIT_4, AIP650E_SEGMENT_NONE);

    /* report status */
    return ec;
}

/* configure the operation of the chip */
err_t AIP650E_Configure(aip650e_dev_t *dev, aip650e_config_t cfg)
{
    /* error code and the placeholder for the data */
    err_t ec = EOK; uint8_t u8 = cfg;

    /* do the read */
    with_sem (&dev->swi2c->sem)
        ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_ADDR | SWI2C_OPER_WR |
            SWI2C_OPER_STOP, AIP650E_CMD_SET_PARAM, &u8, sizeof(u8));

    /* return the error code */
    return ec;
}

/* read the keys */
err_t AIP650E_ReadKeys(aip650e_dev_t *dev, aip650e_digit_t *digit,
    aip650e_key_t *key)
{
    /* error code and the placeholder for the data */
    err_t ec = EOK; uint8_t u8;
    /* key and digit placeholders */
    aip650e_key_t k; aip650e_digit_t d;

    /* do the read */
    with_sem (&dev->swi2c->sem)
        ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_ADDR | SWI2C_OPER_RD |
            SWI2C_OPER_STOP, AIP650E_CMD_GET_KEY, &u8, sizeof(u8));

    /* i2c error */
    if (ec < EOK)
        return ec;

    /* check the value of the fixed fields */
    if ((u8 & 0xc4) != 0x44)
        return EFATAL;

    /* decode digit */
    switch (u8 & 0x3) {
    case 0: d = AIP650E_DIGIT_1; break;
    case 1: d = AIP650E_DIGIT_2; break;
    case 2: d = AIP650E_DIGIT_3; break;
    case 3: d = AIP650E_DIGIT_4; break;
    default: return EFATAL;
    }

    /* decode keypress */
    switch ((u8 >> 3) & 0x7) {
    case 0: k = AIP650E_KEY_1; break;
    case 1: k = AIP650E_KEY_2; break;
    case 2: k = AIP650E_KEY_3; break;
    case 3: k = AIP650E_KEY_4; break;
    case 4: k = AIP650E_KEY_5; break;
    case 5: k = AIP650E_KEY_6; break;
    case 6: k = AIP650E_KEY_7; break;
    case 7: k = AIP650E_KEY_1_2; break;
    default: return EFATAL;
    }

    /* return informatin */
    if (key) *key = k;
    if (digit) *digit = d;

    /* return the error code */
    return ec;
}

/* set the segments */
err_t AIP650E_SetSegments(aip650e_dev_t *dev, aip650e_digit_t digit,
    aip650e_segments_t segments)
{
    /* address to which we write */
    int addr = 0; err_t ec = EOK;

    /* get the address */
    switch (digit) {
    case AIP650E_DIGIT_1: addr = AIP650E_CMD_WRITE_DATA_DIG1; break;
    case AIP650E_DIGIT_2: addr = AIP650E_CMD_WRITE_DATA_DIG2; break;
    case AIP650E_DIGIT_3: addr = AIP650E_CMD_WRITE_DATA_DIG3; break;
    case AIP650E_DIGIT_4: addr = AIP650E_CMD_WRITE_DATA_DIG4; break;
    /* unknown digit */
    default: return EARGVAL;
    }

    /* convert to segments representation */
    uint8_t u8 = segments;
    /* truncation modified the data? */
    if (u8 != segments)
        return EARGVAL;

    /* do the transfer  */
    with_sem (&dev->swi2c->sem)
        ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_ADDR | SWI2C_OPER_WR |
            SWI2C_OPER_STOP, addr, &u8, sizeof(u8));

    /* return the error code */
    return ec;
}

