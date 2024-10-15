/**
 * @file spi_dev.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef DEV_SPI_DEV_H
#define DEV_SPI_DEV_H

#include "err.h"
#include "dev/spi.h"


/** spi1 device */
extern spi_dev_t spi1;


/**
 * @brief initialize all the devices
 * 
 * @return err_t error code
 */
err_t SPIDev_Init(void);


#endif /* DEV_SPI_DEV_H */
