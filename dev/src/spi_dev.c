/**
 * @file spi_dev.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-09-23
 * 
 * @brief SPI device instances
 */

#include "config.h"
#include "dev/spi.h"

/* spi1 instance */
spi_dev_t spi1 = {
    /* setup gpios */
    .sclk = GPIO_SIGNAL_BLACKPILL_B3,
    .miso = GPIO_SIGNAL_BLACKPILL_B4,
    .mosi = GPIO_SIGNAL_BLACKPILL_B5,
    /* setup peripheral instance */
    .spi = SPI1,
    /* dma config for rx */
    .rx = { .dma_num = DMA_SPI1_RX_PERIPH, .stream_num = DMA_SPI1_RX_STREAM, 
        .channel_num = DMA_SPI1_RX_CHANNEL },
    /* dma config for tx */
    .tx = { .dma_num = DMA_SPI1_TX_PERIPH, .stream_num = DMA_SPI1_TX_STREAM, 
        .channel_num = DMA_SPI1_TX_CHANNEL }
};

/* initialize all the devices */
err_t SPIDev_Init(void)
{
    /* initialize devices */
    SPI_DevInit(&spi1);

    /* report ok eve if one of the devices failes */
    return EOK;
}