/**
 * @file usart.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include <stdint.h>
#include <stddef.h>

#include "config.h"
#include "err.h"
#include "dev/dma.h"
#include "dev/gpio.h"
#include "dev/gpio_signals.h"
#include "dev/usart.h"
#include "stm32f401/dma.h"
#include "stm32f401/nvic.h"
#include "stm32f401/rcc.h"
#include "stm32f401/usart.h"
#include "sys/critical.h"
#include "sys/sem.h"
#include "sys/yield.h"
#include "util/elems.h"
#include "util/minmax.h"
#include "util/msblsb.h"
#include "util/string.h"


/* initialize common part of the driver */
err_t USART_Init(void)
{
    /* enter critical section */
    Critical_Enter();

    /* enable power supplies for the usarts located at apb1 bus */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    /* enable power supplies for the usarts located at apb2 bus */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_USART6EN;

    /* exit critical section */
    Critical_Exit();

    /* return error code */
    return EOK;
}


/* initialize particular usart device */
err_t USART_DevInit(usart_dev_t *dev)
{
    /* determine alternate function to be applied to gpio pins */
    gpio_af_t af = dev->usart == USART6 ? GPIO_AF_USART6 : 
        GPIO_AF_SPI3_I2S3_USART1_USART2;
    /* get the clock base for given peripherals */
    int bus_clock = dev->usart == USART2 ? APB1CLOCK_HZ : APB2CLOCK_HZ;

    /* enter critical section */
    Critical_Enter();

    /* configure pins */
    GPIOSig_CfgAltFunction(dev->rxd, af);
    GPIOSig_CfgAltFunction(dev->txd, af);
    /* setup pull up on rxd line */
    GPIOSig_CfgPull(dev->rxd, GPIO_PULL_UP);

    /* get dma streams for rx and tx */
    dev->tx.stream = DMA_GetStream(DMA_GetDMA(dev->tx.dma_num), 
        dev->tx.stream_num);
    dev->rx.stream = DMA_GetStream(DMA_GetDMA(dev->rx.dma_num), 
        dev->rx.stream_num);

    /* tx dma configuration */
    /* configure transfer for the tx stream */
    DMA_CfgTransfer(dev->tx.stream, DMA_TFER_FLAG_MINC | DMA_TFER_FLAG_MSIZE_1B |
        DMA_TFER_FLAG_PSIZE_1B | DMA_TFER_FLAG_DIR_M2P);
    /* setup peripheral address */
    DMA_CfgPeriphAddr(dev->tx.stream, (void *)&dev->usart->DR);
    /* use appropriate channel */
    DMA_CfgChannel(dev->tx.stream, dev->tx.channel_num);
    
        /* rx dma configuration */
    DMA_CfgTransfer(dev->rx.stream, DMA_TFER_FLAG_MINC | DMA_TFER_FLAG_CIRC |
        DMA_TFER_FLAG_MSIZE_1B | DMA_TFER_FLAG_PSIZE_1B | DMA_TFER_FLAG_DIR_P2M);
    /* setup peripheral address */
    DMA_CfgPeriphAddr(dev->rx.stream,  (void *)&(dev->usart->DR));
    /* configure memory address */
    DMA_CfgMemAddr(dev->rx.stream, dev->circ);
    /* configure the size of the circular buffer */
    DMA_CfgSize(dev->rx.stream, sizeof(dev->circ));
    /* select channel */
    DMA_CfgChannel(dev->rx.stream,  dev->rx.channel_num);
    /* enable rx stream */
    DMA_CfgEnable(dev->rx.stream, 1);

    /* configure baud rate */
    dev->usart->BRR = bus_clock / dev->baudrate;
    /* x-mission dma request generation enable */
    dev->usart->CR3 = USART_CR3_DMAR | USART_CR3_DMAT;
    /* start the receiver and transmitter */
    dev->usart->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;

    /* exit critical section */
    Critical_Exit();

    return EOK;
}

/* send data */
err_t USART_Send(usart_dev_t *dev, const void *ptr, size_t size, dtime_t timeout)
{
    /* current time */
    time_t ts = time(0);
    /* shorthand */
    dma_stream_t *tx_dma = dev->tx.stream;

    /* clear status flags */
    DMA_ClearStatus(tx_dma, DMA_STATUS_FLAG_ALL);
    /* setup source address */
    DMA_CfgMemAddr(tx_dma, (void *)ptr);
    /* setup transfer size */
    DMA_CfgSize(tx_dma, size);
    /* enable dma */
    DMA_CfgEnable(tx_dma, 1);

    /* poll as long as there is data in buffer */
    while (!(DMA_GetStatus(tx_dma) & DMA_STATUS_FLAG_FULL_TFER) ||
           !(dev->usart->SR & USART_SR_TC)) {
        /* check for timeout */
        if (timeout && dtime(time(0), ts) > timeout)
            return ETIMEOUT;
        /* no timeout, continue waiting */
        Yield();
    }

    /* return information about how many bytes were sent */
    return size;
}

/* receive data */
err_t USART_Recv(usart_dev_t *dev, void *ptr, size_t size, dtime_t timeout)
{
    /* byte-wise pointer */
    uint8_t *p = ptr; 
    /* current time */
    time_t ts = time(0);
    /* shorthand */
    dma_stream_t *rx_dma = dev->rx.stream;

    /* wait for data loop */
    while ((dev->head = elems(dev->circ) - DMA_GetSize(rx_dma)) - dev->tail == 0) {
        /* check for timeout */
        if (timeout && dtime(time(0), ts) > timeout)
            return ETIMEOUT;
        /* no timeout, continue waiting */
        Yield();
    }
    
    /* get number of bytes to copy */
    uint32_t bnum = min((dev->head - dev->tail) % elems(dev->circ), size);
    /* number of bytes before circ buffer wraps */
    uint32_t wrap = min(bnum, elems(dev->circ) - dev->tail);

    /* copy data before wrapping */
    memcpy(p, dev->circ + dev->tail, wrap);
    /* copy data after wrapping */
    if (bnum - wrap)
        memcpy(p + wrap, dev->circ, bnum - wrap);
    
    /* update tail pointer */
    dev->tail = (dev->tail + bnum) % elems(dev->circ);

    /* report the number of bytes read */
    return bnum;
}
