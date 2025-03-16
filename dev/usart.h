/**
 * @file usart.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef DEV_USART_H
#define DEV_USART_H

#include "err.h"
#include "dev/dma.h"
#include "dev/gpio_signals.h"
#include "stm32f401/usart.h"
#include "sys/time.h"
#include "sys/sem.h"

/** usart device */
typedef struct usart_dev {
    /* pins */
    gpio_signal_t rxd, txd;
    /* baudrate */
    int baudrate;
    /* usart peripheral */
    usart_t *usart;
    /* dma configs for both rx and tx channels */
    struct {
        /* peripheral and stream number */
        dma_num_t dma_num; dma_stream_num_t stream_num; 
        /* dma channel number associated with given spi */
        dma_channel_num_t channel_num;
        /* dma stream pointer */
        dma_stream_t *stream;
    } rx, tx;
    /* circular buffer for reception */
    uint8_t circ[512];
    /* circular buffer indexers */
    uint32_t head, tail;
    /* semaphore */
    sem_t rx_sem, tx_sem;
} usart_dev_t;


/**
 * @brief initialize common part of the driver 
 * 
 * @return err_t error code
 */
err_t USART_Init(void);


/**
 * @brief initialize particular usart device 
 * 
 * @param dev device to be initialized
 * 
 * @return err_t error code
 */
err_t USART_DevInit(usart_dev_t *dev);


/**
 * @brief set the usart baud rate
 *
 * @param dev device descriptor
 * @param baudrate badrate that we want to set
 *
 * @return err_t error code
**/
err_t USART_SetBaudrate(usart_dev_t *dev, int baudrate);


/**
 * @brief Send data over the usart.
 *
 * @param dev device descriptor
 * @param ptr pointer to the data to be sent. User is responsible for keeping the
 * data unchangeable during the send process
 * @param size size of the data to be sent in bytes
 * @param timeout timeout in milliseconds
 *
 * @return err_t negative number indicates errors, positive indicates number of 
 * bytes sent
 */
err_t USART_Send(usart_dev_t *dev, const void *ptr, size_t size, dtime_t timeout);

/**
 * @brief Receive data from the usart
 * 
 * @param dev device descriptor
 * @param ptr pointer to where to store the data
 * @param size data size
 * @param tout timeout in milliseconds
 *
 * @return int number of bytes received (positive) 
 */
err_t USART_Recv(usart_dev_t *dev, void *ptr, size_t size, dtime_t timeout);


#endif /* DEV_USART_H */
