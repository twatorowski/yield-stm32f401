/**
 * @file swi2c.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-09-06
 * 
 * @brief Software based (bit-banged) i2c controller
 */

#ifndef DEV_SWI2C_H
#define DEV_SWI2C_H

#include <stddef.h>

#include "dev/gpio.h"
#include "dev/gpio_signals.h"
#include "sys/sem.h"
#include "util/bit.h"

/** device descriptor for the software i2c */
typedef struct swi2c_dev {
    /** signal location for the sda and scl */
    gpio_signal_t sda, scl;
    /** i2c access lock */
    sem_t sem;
} swi2c_dev_t;

/** swi2c operation */
typedef enum swi2c_oper {
    /** read operation */
    SWI2C_OPER_RD = 0,
    /** write operation */
    SWI2C_OPER_WR = BIT_VAL(0),
    /** do the addressing phase */
    SWI2C_OPER_ADDR = BIT_VAL(1),
    /** finish with i2c stop */
    SWI2C_OPER_STOP = BIT_VAL(2),
} swi2c_oper_t;

/**
 * @brief initialize software i2c
 *
 * @return err_t status code
 */
err_t SwI2C_Init(void);

/**
 * @brief initialize software i2c device
 *
 * @param dev device descriptor
 *
 * @return err_t error code
 */
err_t SwI2C_DevInit(swi2c_dev_t *dev);

/**
 * @brief Performs I2C transfer
 *
 * @param dev device descriptor
 * @param oper operation to be performed
 * @param addr slave address
 * @param ptr data pointer
 * @param size data size
 *
 * @return err_t operation error code
 */
err_t SwI2C_Transfer(swi2c_dev_t *dev, swi2c_oper_t oper, int addr, void *ptr,
    size_t size);


#endif /* DEV_SWI2C_H */
