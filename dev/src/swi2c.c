/**
 * @file swi2c.c
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-09-06
 *
 * @brief Software based (bit-banged) i2c controller
 */

#include <stdint.h>
#include <stdlib.h>

#include "compiler.h"
#include "err.h"
#include "dev/swi2c.h"
#include "sys/sleep.h"
#include "sys/yield.h"

/* default clock streching timeout */
#define SWI2C_TIMEOUT                   10

/* macro shorthands used for controlling the signal levels */
#define W_SDA(dev, x)                   GPIO_Set(dev->sda.gpio, dev->sda.pin, x)
#define W_SCL(dev, x)                   GPIO_Set(dev->scl.gpio, dev->scl.pin, x)
#define R_SDA(dev)                      GPIO_Get(dev->sda.gpio, dev->sda.pin)
#define R_SCL(dev)                      GPIO_Get(dev->scl.gpio, dev->scl.pin)
/* macro for dealy generation */
#define DELAY(dev)                      Time_DelayUS(5)

/* wait for the clock to*/
static inline ALWAYS_INLINE err_t SwI2C_ClockStrech(const swi2c_dev_t *dev,
    dtime_t timeout)
{
    /* clock already released */
    if (R_SCL(dev))
        return EOK;

    uint16_t us = Time_GetUS();
    /* poll as long as clock is held low and timeout did not occur */
    for (time_t ts = time(0); !R_SCL(dev) && dtime(time(0), ts) < timeout; ) {
        /* yield from time to time */
        if (Time_GetUS() - us > 100)
            Yield(), us = Time_GetUS();
    }

    /* return status */
    return R_SCL(dev) ? EOK : EFATAL;
}

/* reset the bus */
static err_t SwI2C_Reset(const swi2c_dev_t *dev)
{
    /* release the sda */
    W_SDA(dev, 1); DELAY(dev);
    /* clock some dummy pulses */
    for (int i = 0; i < 16; i++) {
        W_SCL(dev, 0); DELAY(dev);
        W_SCL(dev, 1); DELAY(dev);
    }

    /* report stattus */
    return EOK;
}

/* emit start condition onto the bus */
static err_t SwI2C_Start(const swi2c_dev_t *dev)
{
    /* reset bus */
    W_SDA(dev, 1); DELAY(dev);
    W_SCL(dev, 1); DELAY(dev);

    /* clock streching */
    if (SwI2C_ClockStrech(dev, SWI2C_TIMEOUT) != EOK)
        return EI2C_ARB_LOST;

    /* send start condition bus */
    W_SDA(dev, 0); DELAY(dev);
    W_SCL(dev, 0); DELAY(dev);

    /* start sent */
    return EOK;
}

/* emit stop condition onto the bus */
static err_t SwI2C_Stop(const swi2c_dev_t *dev)
{
    /* clear sda & set scl */
    W_SDA(dev, 0); DELAY(dev);
    W_SCL(dev, 1); DELAY(dev);

    /* clock streching */
    if (SwI2C_ClockStrech(dev, SWI2C_TIMEOUT) != EOK) {
        /* release the line before bailing out */
        W_SDA(dev, 1); return EI2C_ARB_LOST;
    }

    /* set the sda */
    W_SDA(dev, 1); DELAY(dev);
    /* sda is being held by remote party*/
    if (R_SDA(dev) == 0)
        return EI2C_ARB_LOST;

    /* stop condition sent */
    return EOK;
}

/* writes a single byte on the bus */
static err_t SwI2C_WriteByte(const swi2c_dev_t *dev, uint8_t byte)
{
    /* acknowledge status */
    int ack;

    /* 8 bits to be pushed */
    for (int i = 0; i < 8; i++, byte <<= 1) {
        /* derive bit value */
        int bit_val = !!(byte & 0x80);
        /* set bit, clock bit */
        W_SDA(dev, bit_val); DELAY(dev);
        W_SCL(dev, 1); DELAY(dev);
        /* wait for the clock signal to become released */
        if (SwI2C_ClockStrech(dev, SWI2C_TIMEOUT) != EOK) {
            /* release the bus before quitting */
            W_SDA(dev, 1); return EI2C_BUS_TIMEOUT;
        }
        /* oops, someone is drivint the sda! */
        if (bit_val != R_SDA(dev)) {
            W_SDA(dev, 1); return EI2C_ARB_LOST;
        }
        /* clear clock */
        W_SCL(dev, 0);
    }

    /* let the slave drive the bus so that we can receive the ack pulse */
    W_SDA(dev, 1); DELAY(dev);
    W_SCL(dev, 1); DELAY(dev);
    /* clock stretching */
    if (SwI2C_ClockStrech(dev, SWI2C_TIMEOUT) != EOK)
        return EI2C_BUS_TIMEOUT;

    /* read ack value */
    DELAY(dev); ack = R_SDA(dev);
    /* bring the clock back to the default state */
    W_SCL(dev, 0); DELAY(dev);


    /* report sno ack if the sda was high during ack phase */
    return ack == 0 ? EOK : EI2C_NACK;
}

/* read byte from the bus */
static err_t SwI2C_ReadByte(const swi2c_dev_t *dev, uint8_t *byte, int ack)
{
    /* byte value, timeout, acknowledge status */
    int b = 0;

    /* let the slave drive the bus */
    W_SDA(dev, 1); DELAY(dev);

    /* 8 bits to be pushed */
    for (int i = 0; i < 8; i++) {
        /* set clock */
        W_SCL(dev, 1);
        /* clock stretching */
        if (SwI2C_ClockStrech(dev, SWI2C_TIMEOUT) != EOK)
            return EI2C_BUS_TIMEOUT;

        /* wait for slave to set the bus */
        DELAY(dev);
        /* read bit */
        b = b << 1 | (R_SDA(dev));
        /* clear clock */
        W_SCL(dev, 0); DELAY(dev);
    }

    /* send ack pulse */
    W_SDA(dev, ack ? 0 : 1); DELAY();
    W_SCL(dev, 1); DELAY();

    /* clock stretching */
    if (SwI2C_ClockStrech(dev, SWI2C_TIMEOUT) != EOK)
        return EI2C_BUS_TIMEOUT;

    /* clear the clock*/
    W_SCL(dev, 0);

    /* store byte value */
    if (byte)
        *byte = b;
    /* report status */
    return EOK;
}

/* initialize software i2c */
err_t SwI2C_Init(void)
{
    /* return status */
    return EOK;
}

/* initialize software i2c */
err_t SwI2C_DevInit(swi2c_dev_t *dev)
{
    /* prepare gpios */
    GPIO_CfgOutput(dev->scl.gpio, dev->scl.pin, GPIO_OTYPE_OD, 1);
    GPIO_CfgOutput(dev->sda.gpio, dev->sda.pin, GPIO_OTYPE_OD, 1);
    /* configure pull-ups */
    GPIO_CfgPull(dev->scl.gpio, dev->scl.pin, GPIO_PULL_UP);
    GPIO_CfgPull(dev->sda.gpio, dev->sda.pin, GPIO_PULL_UP);

    /* perform the bus reset */
    SwI2C_Reset(dev);
    /* release the semaphore */
    Sem_Release(&dev->sem);

    /* return status */
    return EOK;
}

/* do the i2c operation */
err_t SwI2C_Transfer(swi2c_dev_t *dev, swi2c_oper_t oper, int addr, void *ptr,
    size_t size)
{
    /* bytwwise data pointer, error code */
    uint8_t *p = ptr; err_t ec;


    /* addressing phase requested? */
    if (oper & SWI2C_OPER_ADDR) {
        /* start condition */
        if ((ec = SwI2C_Start(dev)) != EOK) {
            ec = EI2C_START; goto end;
        }
        /* last bit in address field tells whether it is read or write
         * transfer */
        addr = addr << 1 | (oper & SWI2C_OPER_WR ? 0 : 1);
        /* send address */
        if (SwI2C_WriteByte(dev, addr) != EOK) {
            /* this is needed to ensure compatibility with smbus */
            SwI2C_WriteByte(dev, 0x00);
            SwI2C_Stop(dev);
            /* set the error */
            ec = EI2C_ADDR; goto end;
        }
    }

    /* write transfer? */
    if (oper & SWI2C_OPER_WR) {
        /* write all bytes */
        for (size_t i = 0; i < size; i++)
            if ((ec = SwI2C_WriteByte(dev, *p++)) != EOK)
                goto end;
    /* read transfer? */
    } else {
        /* read all bytes */
        for (size_t i = 0; i < size; i++)
            if ((ec = SwI2C_ReadByte(dev, p++, i < size - 1)) != EOK)
                goto end;
    }

    /* emit stop? */
    if (oper & SWI2C_OPER_STOP) {
        /* check for errors */
        if (SwI2C_Stop(dev) != EOK)
            ec = EI2C_STOP;
    }

    /* to the bus reset if an error occured */
    end: if (ec != EOK);
        // SwI2C_Reset(dev);

    /* yield once per transfer */
    Yield();
    /* report status */
    return ec;
}