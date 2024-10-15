/**
 * @file spi.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-09-21
 * 
 * @brief SPI Driver
 */

#include "config.h"
#include "err.h"
#include "dev/dma.h"
#include "dev/gpio_signals.h"
#include "dev/spi.h"
#include "stm32f401/rcc.h"
#include "stm32f401/spi.h"
#include "sys/critical.h"
#include "sys/sem.h"
#include "sys/yield.h"
#include "sys/sleep.h"

/* initialize common parts of the driver */
err_t SPI_Init(void)
{
    /* enter cirtical section */
    Critical_Enter();

    /* enable spi1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    /* enable clocks for spi2 & 3 */
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN | RCC_APB1ENR_SPI3EN;
    /* enable clocks for spi4 & 5 & 6*/
    RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;

    /* exit critical section */
    Critical_Exit();
    /* return error code */
    return EOK;
}

/* initialize spi device */
err_t SPI_DevInit(spi_dev_t *dev)
{
    /* enter cirtical section  */
    Critical_Enter();

    /* select alternate function for all three pins */
    GPIOSig_CfgAltFunction(dev->sclk, GPIO_AF_SPI1_SPI2_I2S2_SPI3_I2S3_SPI4);
    GPIOSig_CfgAltFunction(dev->miso, GPIO_AF_SPI1_SPI2_I2S2_SPI3_I2S3_SPI4);
    GPIOSig_CfgAltFunction(dev->mosi, GPIO_AF_SPI1_SPI2_I2S2_SPI3_I2S3_SPI4);
    /* setup pull-up on the miso line */
    GPIOSig_CfgPull(dev->miso, GPIO_PULL_UP);
    /* setup full speed on sck and mosi */
    GPIOSig_CfgOutputSpeed(dev->sclk, GPIO_OSPEED_VERY_HIGH);
    GPIOSig_CfgOutputSpeed(dev->mosi, GPIO_OSPEED_VERY_HIGH);

    /* prepare dma stream pointers */
    dev->rx.stream = DMA_GetStream(DMA_GetDMA(dev->rx.dma_num), dev->rx.stream_num);
    dev->tx.stream = DMA_GetStream(DMA_GetDMA(dev->tx.dma_num), dev->tx.stream_num);

    /* setup peripheral address */
    DMA_CfgPeriphAddr(dev->tx.stream, (void *)&dev->spi->DR);
    DMA_CfgPeriphAddr(dev->rx.stream, (void *)&dev->spi->DR);
    /* select channel */
    DMA_CfgChannel(dev->rx.stream, dev->rx.channel_num);
    DMA_CfgChannel(dev->tx.stream, dev->tx.channel_num);

    /* configure for master mode, mode 3 */
    dev->spi->CR1 =  SPI_CR1_SSM | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_MSTR |
        SPI_CR1_BR | SPI_CR1_SSI;
    
    /* exit critical section */
    Critical_Exit();

    /* report status */
    return EOK;
}

/* perform spi transfer */
err_t SPI_Transfer(spi_dev_t *dev, spi_speed_t speed, spi_mode_t mode, 
    const void *tx, void *rx, size_t size)
{
    /* dummy receive buffer */
    uint8_t dummy_rx, dummy_tx = 0xff;

    /* no point in operation */
    if (size == 0)
        return EOK;

    /* these are placed on the apb2 which is 2x as fast */
    if (dev->spi == SPI1 || dev->spi == SPI4)
        speed += 1 << LSB(SPI_CR1_BR);

    /* disable the spi */
    dev->spi->CR1 &= ~SPI_CR1_SPE;

    /* disable dma requrest generation */
    dev->spi->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
    /* 0. setup speed */
    dev->spi->CR1 = (dev->spi->CR1 & ~(SPI_CR1_BR | SPI_CR1_CPOL | SPI_CR1_CPHA)) | 
        speed | mode;

    /* 1. enable rx dma events generation */
    dev->spi->CR2 |= SPI_CR2_RXDMAEN;

    /* clear dma flags */
    DMA_ClearStatus(dev->rx.stream, DMA_STATUS_FLAG_ALL);
    DMA_ClearStatus(dev->tx.stream, DMA_STATUS_FLAG_ALL);
    
    /* setup rx/tx stream address */
    DMA_CfgMemAddr(dev->rx.stream, (void *)(rx ? rx : &dummy_rx));
    DMA_CfgMemAddr(dev->tx.stream, (void *)(tx ? tx : &dummy_tx));
    
    /* setup transfer size */
    DMA_CfgSize(dev->tx.stream, size); 
    DMA_CfgSize(dev->rx.stream, size);
    
    /* configure transfer to support the dummy writes/reads */
    DMA_CfgTransfer(dev->tx.stream, DMA_TFER_FLAG_DIR_M2P | 
        (tx ? DMA_TFER_FLAG_MINC : 0) | DMA_TFER_FLAG_MSIZE_1B | 
        DMA_TFER_FLAG_PSIZE_1B);
    DMA_CfgTransfer(dev->rx.stream, DMA_TFER_FLAG_DIR_P2M |
        (rx ? DMA_TFER_FLAG_MINC : 0) | DMA_TFER_FLAG_MSIZE_1B |
        DMA_TFER_FLAG_PSIZE_1B);
    
    /* enable transfers */
    DMA_CfgEnable(dev->tx.stream, 1); 
    DMA_CfgEnable(dev->rx.stream, 1);

    /* enable tx dma events generation */
    dev->spi->CR2 |= SPI_CR2_TXDMAEN;
    /* enable the spi */
    dev->spi->CR1 |= SPI_CR1_SPE;

    /* get the microsecods counter value */
    uint16_t us = Time_GetUS();
    /* wait for the transfer to end */
    while (!(DMA_GetStatus(dev->rx.stream) & DMA_STATUS_FLAG_FULL_TFER) ||
           !(dev->spi->SR & SPI_SR_TXE)) {
        /* allow other processes to take place */
        if (Time_GetUS() - us > 50) {
            Yield(); us = Time_GetUS();
        }
    }

    /* report status */
    return EOK;
}