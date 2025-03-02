/**
 * @file husb238.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-01
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_HUSB238_H
#define DEV_HUSB238_H

#include "dev/swi2c.h"


/** device descriptor  */
typedef struct husb238_dev_t {
    /* bus */
    swi2c_dev_t *swi2c;
} husb238_dev_t;


/** voltage setting enumeration  */
typedef enum husb328_volts {
    HUSB_VOLTS_UNKNOWN,
    HUSB_VOLTS_5V,
    HUSB_VOLTS_9V,
    HUSB_VOLTS_12V,
    HUSB_VOLTS_15V,
    HUSB_VOLTS_18V,
    HUSB_VOLTS_20V,
} husb328_volts_t;

/** voltage setting enumeration */
typedef enum husb328_amps {
    HUSB_AMPS_UNKNOWN,
    HUSB_AMPS_0A5,
    HUSB_AMPS_0A7,
    HUSB_AMPS_1A,
    HUSB_AMPS_1A25,
    HUSB_AMPS_1A5,
    HUSB_AMPS_1A75,
    HUSB_AMPS_2A,
    HUSB_AMPS_2A25,
    HUSB_AMPS_2A5,
    HUSB_AMPS_2A75,
    HUSB_AMPS_3A,
    HUSB_AMPS_3A25,
    HUSB_AMPS_3A5,
    HUSB_AMPS_4A,
    HUSB_AMPS_4A5,
    HUSB_AMPS_5A,
} husb328_amps_t;


/**
 * @brief initialize common parts of the driver
 *
 * @return err_t error code
 */
err_t HUSB238_Init(void);

/**
 * @brief initialize particular device
 *
 * @param dev device descriptor
 * @return err_t error code
 */
err_t HUSB238_DevInit(husb238_dev_t *dev);

/**
 * @brief perform a hard reset command
 *
 * @param dev device descriptor
 * @return err_t error code
 */
err_t HUSB238_QuerySource(husb238_dev_t *dev);

/**
 * @brief perform a hard reset command
 *
 * @param dev device descriptor
 * @return err_t error code
 */
err_t HUSB238_HardReset(husb238_dev_t *dev);

/* get the current contract */
err_t HUSB238_GetCurrentContract(husb238_dev_t *dev, husb328_volts_t *volts,
    husb328_amps_t *amps);

#endif /* DEV_HUSB238_H */
