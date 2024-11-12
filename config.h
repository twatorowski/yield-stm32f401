/**
 * @file config.h
 *
 * @date 29.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * Main configuration file
 */

#ifndef CONFIG
#define CONFIG

#include <stdint.h>



/** Development configuration */
#ifndef DEVELOPMENT
/** Development flag. Used to generate the code with all of the printf
 * debugging enabled */
#define DEVELOPMENT                                 1
#endif


/** System Clock configuration */
/** mcu frequency */
#define CPUCLOCK_HZ                                 84000000
/** ahb frequency */
#define AHBCLOCK_HZ                                 (CPUCLOCK_HZ / 1)
/** apb1 bus frequency */
#define APB1CLOCK_HZ                                (CPUCLOCK_HZ / 2)
/** apb2 bus frequency */
#define APB2CLOCK_HZ                                (CPUCLOCK_HZ / 1)


/** Debug configuration */
/** debugging is enabled?  */
#define DEBUG_ENABLED                               DEVELOPMENT
/** maximal length of the debug line string  */
#define DEBUG_MAX_LINE_LEN                          256



/** DMA assignment */
/** usart1 tx dma peripheral */
#define DMA_USART1_TX_PERIPH                        DMA_2
/** usart1 tx dma stream */
#define DMA_USART1_TX_STREAM                        DMA_STREAM_7
/** usart1 tx dma channel */
#define DMA_USART1_TX_CHANNEL                       DMA2_S7_USART1_TX
/** usart1 rx dma peripheral */
#define DMA_USART1_RX_PERIPH                        DMA_2
/** usart1 rx dma stream */
#define DMA_USART1_RX_STREAM                        DMA_STREAM_2
/** usart1 rx dma channel */
#define DMA_USART1_RX_CHANNEL                       DMA2_S2_USART1_RX

/** spi tx dma peripheral */
#define DMA_SPI1_TX_PERIPH                          DMA_2
/** spi tx dma stream */
#define DMA_SPI1_TX_STREAM                          DMA_STREAM_5
/** spi tx dma channel */
#define DMA_SPI1_TX_CHANNEL                         DMA2_S5_SPI1_TX
/** spi rx dma peripheral */
#define DMA_SPI1_RX_PERIPH                          DMA_2
/** spi rx dma stream */
#define DMA_SPI1_RX_STREAM                          DMA_STREAM_0
/** spi rx dma channel */
#define DMA_SPI1_RX_CHANNEL                         DMA2_S0_SPI1_RX


/** Yield configuration */
/** Memory for the tasks */
#define SYS_HEAP_SIZE                               (36 * 1024)
/** coroutine stack size */
#define SYS_CORO_STACK_SIZE                         256
/** maximal number of concurrently running coroutines */
#define SYS_CORO_MAX_NUM                            4
/** sys max event callback subscribers */
#define SYS_EV_MAX_CBS                              8


/** Interrupt priorities */
/** systick overflow */
#define INT_PRI_SYSTICK                             0x00
/** context switcher */
#define INT_PRI_YIELD                               0xf0



/** USART1 Configuration */
/** baudrate */
#define USART1_BAURDRATE                            115200


/** USB Configuration */
/* usb uses common fifo for reception so we need to set it's size to
 * hold the largest packet possible */
#define USB_RX_FIFO_SIZE                            256
/** control endpoint max packet size */
#define USB_CTRLEP_SIZE                             64
/** virtual com port interrupt endpoint frame size */
#define USB_VCP_INT_SIZE                            8
/** virtual com port transmission frame size */
#define USB_VCP_TX_SIZE                             32
/** virtual com port transmission frame size */
#define USB_VCP_RX_SIZE                             32


/** USBCore Configuration */
/* maximal number of interfaces */
#define USBCORE_MAX_IFACE_NUM                       10


#endif /* CONFIG_H_ */
