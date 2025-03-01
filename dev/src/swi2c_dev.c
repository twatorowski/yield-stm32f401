/**
 * @file swi2c_dev.c
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-07-03
 *
 * @brief device list for the swi2c
 */

#include "dev/swi2c.h"

/* i2c0 device */
swi2c_dev_t swi2c0 = {
    .scl = GPIO_SIGNAL_BLACKPILL_B12,
    .sda = GPIO_SIGNAL_BLACKPILL_B13,
};

/* display driver communication device */
swi2c_dev_t swi2c_disp = {
    .scl = GPIO_SIGNAL_C5,
    .sda = GPIO_SIGNAL_B0,
};

/* display driver communication device */
swi2c_dev_t swi2c_nau = {
    .scl = GPIO_SIGNAL_B14,
    .sda = GPIO_SIGNAL_B13,
};

/* eeprom communication device */
swi2c_dev_t swi2c_eeprom = {
    .scl = GPIO_SIGNAL_B9,
    .sda = GPIO_SIGNAL_B8,
};


/* initialize all the devices */
err_t SwI2CDev_Init(void)
{
    /* initialize devices */
    // SwI2C_DevInit(&swi2c0);
    SwI2C_DevInit(&swi2c_disp);
    SwI2C_DevInit(&swi2c_nau);
    SwI2C_DevInit(&swi2c_eeprom);

    /* report ok eve if one of the devices failes */
    return EOK;
}