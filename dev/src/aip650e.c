/**
 * @file aip650e.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "dev/swi2c.h"
#include "dev/swi2c_dev.h"

#define DEBUG DLVL_INFO
#include "debug.h"

/* device descriptor */
typedef struct aip650e_dev {
    swi2c_dev_t *swi2c;
} aip650e_dev_t;

/* commands */
#define AIP650E_CMD_WRITE_DATA_DIG1             (0x68 >> 1)
#define AIP650E_CMD_WRITE_DATA_DIG2             (0x6a >> 1)
#define AIP650E_CMD_WRITE_DATA_DIG3             (0x6c >> 1)
#define AIP650E_CMD_WRITE_DATA_DIG4             (0x6e >> 1)
#define AIP650E_CMD_SET_PARAM                   (0x48 >> 1)
#define AIP650E_CMD_GET_KEY                     (0x49 >> 1)

/* enumeration for the digits */
typedef enum aip650e_digit {
    AIP650E_DIGIT_1,
    AIP650E_DIGIT_2,
    AIP650E_DIGIT_3,
    AIP650E_DIGIT_4,
} aip650e_digit_t;


/* enumebration of segments */
typedef enum aip650e_segments {
    AIP650E_SEGMENT_A = BIT_VAL(0),
    AIP650E_SEGMENT_B = BIT_VAL(1),
    AIP650E_SEGMENT_C = BIT_VAL(2),
    AIP650E_SEGMENT_D = BIT_VAL(3),
    AIP650E_SEGMENT_E = BIT_VAL(4),
    AIP650E_SEGMENT_F = BIT_VAL(5),
    AIP650E_SEGMENT_G = BIT_VAL(6),
    AIP650E_SEGMENT_DP = BIT_VAL(7),
    /* all of the segments */
    AIP650E_SEGMENT_ALL = AIP650E_SEGMENT_A | AIP650E_SEGMENT_B |
        AIP650E_SEGMENT_C | AIP650E_SEGMENT_D | AIP650E_SEGMENT_E |
        AIP650E_SEGMENT_F | AIP650E_SEGMENT_G | AIP650E_SEGMENT_DP
} aip650e_segments_t;

/* enumeration for the digits */
typedef enum aip650e_config {
    /* display on/off */
    AIP650E_CFG_DISPLAY_OFF = 0,
    AIP650E_CFG_DISPLAY_ON = BIT_VAL(0),
    /* sleep mode */
    AIP650E_CFG_SLEEP_OFF = 0,
    AIP650E_CFG_SLEEP_ON = BIT_VAL(2),

    /* brightness levels */
    AIP650E_CFG_BRIGHTNESS =   7 << 4,
    AIP650E_CFG_BRIGHTNESS_1 = 1 << 4,
    AIP650E_CFG_BRIGHTNESS_2 = 2 << 4,
    AIP650E_CFG_BRIGHTNESS_3 = 3 << 4,
    AIP650E_CFG_BRIGHTNESS_4 = 4 << 4,
    AIP650E_CFG_BRIGHTNESS_5 = 5 << 4,
    AIP650E_CFG_BRIGHTNESS_6 = 6 << 4,
    AIP650E_CFG_BRIGHTNESS_7 = 7 << 4,
    AIP650E_CFG_BRIGHTNESS_8 = 0 << 4,
} aip650e_config_t;

/* configure the operation of the chip */
static err_t AIP650E_Configure(aip650e_dev_t *dev, aip650e_config_t cfg)
{
    /* error code and the placeholder for the data */
    err_t ec = EOK; uint32_t u8 = cfg;

    /* do the read */
    with_sem (&dev->swi2c->sem)
        ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_WR,
                AIP650E_CMD_SET_PARAM, &u8, sizeof(u8));

    /* return the error code */
    return ec;
}

/* read the keys */
static err_t AIP650E_ReadKeays(aip650e_dev_t *dev)
{
    /* error code and the placeholder for the data */
    err_t ec = EOK; uint8_t u8;

    /* do the read */
    with_sem (&dev->swi2c->sem)
        ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_RD,
                AIP650E_CMD_GET_KEY, &u8, sizeof(u8));

    /* return the error code */
    return ec;
}

/* set the segments */
static err_t AIP650E_SetSegments(aip650e_dev_t *dev, aip650e_digit_t digit,
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
        ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_WR, addr, &u8, sizeof(u8));

    /* return the error code */
    return ec;
}


/* initialize common part of the device driver */
err_t AIP650E_Init(void)
{
    /* report status */
    return EOK;
}

/* initialize particular device */
err_t AIP650E_DevInit(aip650e_dev_t *dev)
{
    /* report status */
    return EOK;
}


#include "sys/sleep.h"
/* initialize particular device */
err_t AIP650E_Test(void)
{
    /* device driver */
    aip650e_dev_t dev = {
        .swi2c = &swi2c_disp
    };



    for (;; Sleep(1000)) {
        err_t ec = AIP650E_ReadKeays(&dev);
        AIP650E_SetSegments(&dev, 0, AIP650E_SEGMENT_ALL);
        AIP650E_SetSegments(&dev, 1, AIP650E_SEGMENT_ALL);
        AIP650E_SetSegments(&dev, 2, AIP650E_SEGMENT_ALL);
        AIP650E_SetSegments(&dev, 3, AIP650E_SEGMENT_ALL);
        AIP650E_Configure(&dev, AIP650E_CFG_DISPLAY_ON);


        dprintf_i("reading status = %d\n", ec);
    }

}