/**
 * @file vusb_detect.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_VUSB_DETECT_H
#define DEV_VUSB_DETECT_H


#include "err.h"

/**
 * @brief initialize vusb detector
 *
 * @return err_t error code
 */
err_t VUSBDet_Init(void);


/**
 * @brief returns the status of vusb voltage
 *
 * @return int 1 - present, 0 - not present
 */
int VUSBDet_IsConnected(void);


#endif /* DEV_VUSB_DETECT_H */
