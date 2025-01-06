/**
 * @file usart_dev.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef DEV_USART_DEV_H
#define DEV_USART_DEV_H

#include "err.h"
#include "dev/usart.h"


/** list of devices present in the system  */
extern usart_dev_t usart1, usart2;

/**
 * @brief initialize all the devices
 * 
 * @return err_t error code
 */
err_t USARTDev_Init(void);


#endif /* DEV_USART_DEV_H */
