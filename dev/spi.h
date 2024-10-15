/**
 * @file spi.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-09-21
 * 
 * @brief SPI Driver
 */

#ifndef DEV_SPI_H
#define DEV_SPI_H

#include <stddef.h>

#include "config.h"
#include "dev/dma.h"
#include "dev/gpio_signals.h"
#include "stm32f401/spi.h"
#include "sys/sem.h"

/** sanitize the clock settings  */
#if (APB1CLOCK_HZ != 42000000) || (APB2CLOCK_HZ != 84000000)
    #error "please update the speed enum to match new clock settings"
#endif

/** spi speeds */
typedef enum spi_speed {
    SPI_SPEED_21M = 0,
    SPI_SPEED_10M5 = (SPI_CR1_BR_0),
    SPI_SPEED_5M25 = (SPI_CR1_BR_1),
    SPI_SPEED_2M625 = (SPI_CR1_BR_1 | SPI_CR1_BR_0),
} spi_speed_t;

/** modes of clock phase/polarity */
typedef enum spi_mode {
    SPI_MODE_0 = 0,
    SPI_MODE_1 = SPI_CR1_CPHA,
    SPI_MODE_2 = SPI_CR1_CPOL,
    SPI_MODE_3 = SPI_CR1_CPHA | SPI_CR1_CPOL,
} spi_mode_t;

/* spi device driver */
typedef struct spi_dev {
    /* spi peripheral */
    spi_t *spi;
    /* dma channels for rx and tx */
    struct {
        /* peripheral and stream number */
        dma_num_t dma_num; dma_stream_num_t stream_num; 
        /* dma channel number associated with given spi */
        dma_channel_num_t channel_num;
        /* dma stream pointer */
        dma_stream_t *stream;
    } rx, tx;
    /* gpios */
    gpio_signal_t sclk, miso, mosi;
    /* semaphore */
    sem_t sem;
} spi_dev_t;

/**
 * @brief initialize spi driver
 *
 * @return err_t
 */
err_t SPI_Init(void);

/**
 * @brief initialize spi device 
 * 
 * @param dev device descritptor
 * 
 * @return err_t error code
 */
err_t SPI_DevInit(spi_dev_t *dev);

/**
 * @brief Perform the transfer on the spi bus
 *
 * @param dev device descriptor
 * @param speed transfer speed
 * @param mode mode of clock phase/polarity
 * @param tx buffer for the data to be sent (or null)
 * @param rx buffer for the received data (or null)
 * @param size size of the transfer
 *
 * @return err_t error code
 */
err_t SPI_Transfer(spi_dev_t *dev, spi_speed_t speed, spi_mode_t mode, 
    const void *tx, void *rx, size_t size);



#endif /* DEV_SPI_H */
