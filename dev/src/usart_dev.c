/**
 * @file usart_dev.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "config.h"
#include "dev/usart.h"

/** usart instances  */
usart_dev_t usart1 = {
    /* gpio pins that are used */
    .rxd = GPIO_SIGNAL_BLACKPILL_A10,
    .txd = GPIO_SIGNAL_BLACKPILL_A9,
    /* usart device */
    .usart = USART1,
    /* baudrate */
    .baudrate = USART1_BAURDRATE,
        /* dma config for rx */
    .rx = { .dma_num = DMA_USART1_RX_PERIPH, .stream_num = DMA_USART1_RX_STREAM, 
        .channel_num = DMA_USART1_RX_CHANNEL },
    /* dma config for tx */
    .tx = { .dma_num = DMA_USART1_TX_PERIPH, .stream_num = DMA_USART1_TX_STREAM, 
        .channel_num = DMA_USART1_TX_CHANNEL }
};

/** usart 2 device  */
usart_dev_t usart2 = {
    /* gpio pins that are used */
    .rxd = GPIO_SIGNAL_BLACKPILL_A3,
    .txd = GPIO_SIGNAL_BLACKPILL_A2,
    /* usart device */
    .usart = USART2,
    /* baudrate */
    .baudrate = USART2_BAURDRATE,
        /* dma config for rx */
    .rx = { .dma_num = DMA_USART2_RX_PERIPH, .stream_num = DMA_USART2_RX_STREAM,
        .channel_num = DMA_USART2_RX_CHANNEL },
    /* dma config for tx */
    .tx = { .dma_num = DMA_USART2_TX_PERIPH, .stream_num = DMA_USART2_TX_STREAM,
        .channel_num = DMA_USART2_TX_CHANNEL }
};


/* initialize all the devices */
err_t USARTDev_Init(void)
{
    /* initialize devices */
    USART_DevInit(&usart1);
    USART_DevInit(&usart2);

    /* report ok eve if one of the devices failes */
    return EOK;
}