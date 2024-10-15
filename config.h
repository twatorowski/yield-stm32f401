/**
 * @file config.h
 *
 * @date 29.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief Main configuration file
 */

#ifndef CONFIG
#define CONFIG

#include <stdint.h>



/** @name Development configuration */
/** @{ */
#ifndef DEVELOPMENT
/** @brief Development flag. Used to generate the code with all of the printf
 * debugging enabled */
#define DEVELOPMENT                                 1
#endif
/** @} */


/** @name System Clock configuration */
/** @{ */
/** @brief mcu frequency */
#define CPUCLOCK_HZ                                 84000000
/** @brief ahb frequency */
#define AHBCLOCK_HZ                                 (CPUCLOCK_HZ / 1)
/** @brief apb1 bus frequency */
#define APB1CLOCK_HZ                                (CPUCLOCK_HZ / 2)
/** @brief apb2 bus frequency */
#define APB2CLOCK_HZ                                (CPUCLOCK_HZ / 1)
/** @} */


/** @name Debug configuration */
/** @{ */
/** @brief debugging is enabled?  */
#define DEBUG_ENABLED                               DEVELOPMENT
/** @brief maximal length of the debug line string  */
#define DEBUG_MAX_LINE_LEN                          256
/** @} */




/** @name DMA assignment */
/** @{ */
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
/** @} */


/** @name Yield configuration */
/** @{ */
/** @brief Memory for the tasks */
#define SYS_HEAP_SIZE                               (36 * 1024)
/** @brief coroutine stack size */
#define SYS_CORO_STACK_SIZE                         256
/** @brief maximal number of concurrently running coroutines */
#define SYS_CORO_MAX_NUM                            4 
/** @} */


/** @name Interrupt priorities */
/** @{ */
/** @brief systick overflow */
#define INT_PRI_SYSTICK                             0x00
/** @brief context switcher */
#define INT_PRI_YIELD                               0xf0
/** @} */



/** @name USART1 Configuration */
/** @{ */
/** @brief baudrate */
#define USART1_BAURDRATE                            115200
/** @} */



/** @} */

#endif /* CONFIG_H_ */
