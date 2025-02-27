/**
 * @file swi2c_dev.h
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-07-03
 *
 * @brief devices present in system
 */
#ifndef DEV_SWI2C_DEV_H
#define DEV_SWI2C_DEV_H

#include "dev/swi2c.h"

/* i2c0 device */
extern swi2c_dev_t swi2c0, swi2c_disp;

/**
 * @brief Perform the initialization of all the devices
 *
 * @return err_t
 */
err_t SwI2CDev_Init(void);

#endif /* DEV_SWI2C_DEV_H */
