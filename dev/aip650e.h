/**
 * @file aip650e.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_AIP650E_H
#define DEV_AIP650E_H

#include "err.h"

/**
 * @brief initialize common part of the device driver
 *
 * @return err_t error code
 */
err_t AIP650E_Init(void);

// TODO:
err_t AIP650E_Test(void);


#endif /* DEV_AIP650E_H */
