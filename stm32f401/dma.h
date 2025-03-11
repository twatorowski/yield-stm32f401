/**
 * @file dma.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_DMA_H
#define STM32F401_DMA_H

#include "stm32f401.h"

/* base addresses */
#define DMA1_BASE                                           0x40026000U
#define DMA2_BASE                                           0x40026400U

/* instances */
#define DMA1                                                ((dma_t *)DMA1_BASE)
#define DMA2                                                ((dma_t *)DMA2_BASE)


/* register bank */
typedef struct {
  reg32_t LISR;
  reg32_t HISR;
  reg32_t LIFCR;
  reg32_t HIFCR;
} dma_t;

/********************  Bits definition for DMA_LISR register  *****************/
#define DMA_LISR_TCIF3                                       0x08000000
#define DMA_LISR_HTIF3                                       0x04000000
#define DMA_LISR_TEIF3                                       0x02000000
#define DMA_LISR_DMEIF3                                      0x01000000
#define DMA_LISR_FEIF3                                       0x00400000
#define DMA_LISR_TCIF2                                       0x00200000
#define DMA_LISR_HTIF2                                       0x00100000
#define DMA_LISR_TEIF2                                       0x00080000
#define DMA_LISR_DMEIF2                                      0x00040000
#define DMA_LISR_FEIF2                                       0x00010000
#define DMA_LISR_TCIF1                                       0x00000800
#define DMA_LISR_HTIF1                                       0x00000400
#define DMA_LISR_TEIF1                                       0x00000200
#define DMA_LISR_DMEIF1                                      0x00000100
#define DMA_LISR_FEIF1                                       0x00000040
#define DMA_LISR_TCIF0                                       0x00000020
#define DMA_LISR_HTIF0                                       0x00000010
#define DMA_LISR_TEIF0                                       0x00000008
#define DMA_LISR_DMEIF0                                      0x00000004
#define DMA_LISR_FEIF0                                       0x00000001

/********************  Bits definition for DMA_HISR register  *****************/
#define DMA_HISR_TCIF7                                       0x08000000
#define DMA_HISR_HTIF7                                       0x04000000
#define DMA_HISR_TEIF7                                       0x02000000
#define DMA_HISR_DMEIF7                                      0x01000000
#define DMA_HISR_FEIF7                                       0x00400000
#define DMA_HISR_TCIF6                                       0x00200000
#define DMA_HISR_HTIF6                                       0x00100000
#define DMA_HISR_TEIF6                                       0x00080000
#define DMA_HISR_DMEIF6                                      0x00040000
#define DMA_HISR_FEIF6                                       0x00010000
#define DMA_HISR_TCIF5                                       0x00000800
#define DMA_HISR_HTIF5                                       0x00000400
#define DMA_HISR_TEIF5                                       0x00000200
#define DMA_HISR_DMEIF5                                      0x00000100
#define DMA_HISR_FEIF5                                       0x00000040
#define DMA_HISR_TCIF4                                       0x00000020
#define DMA_HISR_HTIF4                                       0x00000010
#define DMA_HISR_TEIF4                                       0x00000008
#define DMA_HISR_DMEIF4                                      0x00000004
#define DMA_HISR_FEIF4                                       0x00000001

/********************  Bits definition for DMA_LIFCR register  ****************/
#define DMA_LIFCR_CTCIF3                                     0x08000000
#define DMA_LIFCR_CHTIF3                                     0x04000000
#define DMA_LIFCR_CTEIF3                                     0x02000000
#define DMA_LIFCR_CDMEIF3                                    0x01000000
#define DMA_LIFCR_CFEIF3                                     0x00400000
#define DMA_LIFCR_CTCIF2                                     0x00200000
#define DMA_LIFCR_CHTIF2                                     0x00100000
#define DMA_LIFCR_CTEIF2                                     0x00080000
#define DMA_LIFCR_CDMEIF2                                    0x00040000
#define DMA_LIFCR_CFEIF2                                     0x00010000
#define DMA_LIFCR_CTCIF1                                     0x00000800
#define DMA_LIFCR_CHTIF1                                     0x00000400
#define DMA_LIFCR_CTEIF1                                     0x00000200
#define DMA_LIFCR_CDMEIF1                                    0x00000100
#define DMA_LIFCR_CFEIF1                                     0x00000040
#define DMA_LIFCR_CTCIF0                                     0x00000020
#define DMA_LIFCR_CHTIF0                                     0x00000010
#define DMA_LIFCR_CTEIF0                                     0x00000008
#define DMA_LIFCR_CDMEIF0                                    0x00000004
#define DMA_LIFCR_CFEIF0                                     0x00000001

/********************  Bits definition for DMA_HIFCR  register  ****************/
#define DMA_HIFCR_CTCIF7                                     0x08000000
#define DMA_HIFCR_CHTIF7                                     0x04000000
#define DMA_HIFCR_CTEIF7                                     0x02000000
#define DMA_HIFCR_CDMEIF7                                    0x01000000
#define DMA_HIFCR_CFEIF7                                     0x00400000
#define DMA_HIFCR_CTCIF6                                     0x00200000
#define DMA_HIFCR_CHTIF6                                     0x00100000
#define DMA_HIFCR_CTEIF6                                     0x00080000
#define DMA_HIFCR_CDMEIF6                                    0x00040000
#define DMA_HIFCR_CFEIF6                                     0x00010000
#define DMA_HIFCR_CTCIF5                                     0x00000800
#define DMA_HIFCR_CHTIF5                                     0x00000400
#define DMA_HIFCR_CTEIF5                                     0x00000200
#define DMA_HIFCR_CDMEIF5                                    0x00000100
#define DMA_HIFCR_CFEIF5                                     0x00000040
#define DMA_HIFCR_CTCIF4                                     0x00000020
#define DMA_HIFCR_CHTIF4                                     0x00000010
#define DMA_HIFCR_CTEIF4                                     0x00000008
#define DMA_HIFCR_CDMEIF4                                    0x00000004
#define DMA_HIFCR_CFEIF4                                     0x00000001



/* channel base addresses */
#define DMA1S0_BASE                                         (DMA1_BASE + 0x10)
#define DMA1S1_BASE                                         (DMA1_BASE + 0x28)
#define DMA1S2_BASE                                         (DMA1_BASE + 0x40)
#define DMA1S3_BASE                                         (DMA1_BASE + 0x58)
#define DMA1S4_BASE                                         (DMA1_BASE + 0x70)
#define DMA1S5_BASE                                         (DMA1_BASE + 0x88)
#define DMA1S6_BASE                                         (DMA1_BASE + 0xA0)
#define DMA1S7_BASE                                         (DMA1_BASE + 0xB8)

/* channel base addresses */
#define DMA2S0_BASE                                         (DMA2_BASE + 0x10)
#define DMA2S1_BASE                                         (DMA2_BASE + 0x28)
#define DMA2S2_BASE                                         (DMA2_BASE + 0x40)
#define DMA2S3_BASE                                         (DMA2_BASE + 0x58)
#define DMA2S4_BASE                                         (DMA2_BASE + 0x70)
#define DMA2S5_BASE                                         (DMA2_BASE + 0x88)
#define DMA2S6_BASE                                         (DMA2_BASE + 0xA0)
#define DMA2S7_BASE                                         (DMA2_BASE + 0xB8)

/* channel instances */
#define DMA1S0                                              ((dma_stream_t *)DMA1S0_BASE)
#define DMA1S1                                              ((dma_stream_t *)DMA1S1_BASE)
#define DMA1S2                                              ((dma_stream_t *)DMA1S2_BASE)
#define DMA1S3                                              ((dma_stream_t *)DMA1S3_BASE)
#define DMA1S4                                              ((dma_stream_t *)DMA1S4_BASE)
#define DMA1S5                                              ((dma_stream_t *)DMA1S5_BASE)
#define DMA1S6                                              ((dma_stream_t *)DMA1S6_BASE)
#define DMA1S7                                              ((dma_stream_t *)DMA1S7_BASE)

/* channel instances */
#define DMA2S0                                              ((dma_stream_t *)DMA2S0_BASE)
#define DMA2S1                                              ((dma_stream_t *)DMA2S1_BASE)
#define DMA2S2                                              ((dma_stream_t *)DMA2S2_BASE)
#define DMA2S3                                              ((dma_stream_t *)DMA2S3_BASE)
#define DMA2S4                                              ((dma_stream_t *)DMA2S4_BASE)
#define DMA2S5                                              ((dma_stream_t *)DMA2S5_BASE)
#define DMA2S6                                              ((dma_stream_t *)DMA2S6_BASE)
#define DMA2S7                                              ((dma_stream_t *)DMA2S7_BASE)


/* stream register bank */
typedef struct {
  reg32_t CR;
  reg32_t NDTR;
  reg32_t PAR;
  reg32_t M0AR;
  reg32_t M1AR;
  reg32_t FCR;
} dma_stream_t;


/********************  Bits definition for DMA_CR register  *****************/
#define DMA_CR_CHSEL                                        0x0e000000
#define DMA_CR_CHSEL_0                                      0x02000000
#define DMA_CR_CHSEL_1                                      0x04000000
#define DMA_CR_CHSEL_2                                      0x08000000
#define DMA_CR_MBURST                                       0x01800000
#define DMA_CR_MBURST_0                                     0x00800000
#define DMA_CR_MBURST_1                                     0x01000000
#define DMA_CR_PBURST                                       0x00600000
#define DMA_CR_PBURST_0                                     0x00200000
#define DMA_CR_PBURST_1                                     0x00400000
#define DMA_CR_CT                                           0x00080000
#define DMA_CR_DBM                                          0x00040000
#define DMA_CR_PL                                           0x00030000
#define DMA_CR_PL_0                                         0x00010000
#define DMA_CR_PL_1                                         0x00020000
#define DMA_CR_PINCOS                                       0x00008000
#define DMA_CR_MSIZE                                        0x00006000
#define DMA_CR_MSIZE_0                                      0x00002000
#define DMA_CR_MSIZE_1                                      0x00004000
#define DMA_CR_PSIZE                                        0x00001800
#define DMA_CR_PSIZE_0                                      0x00000800
#define DMA_CR_PSIZE_1                                      0x00001000
#define DMA_CR_MINC                                         0x00000400
#define DMA_CR_PINC                                         0x00000200
#define DMA_CR_CIRC                                         0x00000100
#define DMA_CR_DIR                                          0x000000c0
#define DMA_CR_DIR_0                                        0x00000040
#define DMA_CR_DIR_1                                        0x00000080
#define DMA_CR_PFCTRL                                       0x00000020
#define DMA_CR_TCIE                                         0x00000010
#define DMA_CR_HTIE                                         0x00000008
#define DMA_CR_TEIE                                         0x00000004
#define DMA_CR_DMEIE                                        0x00000002
#define DMA_CR_EN                                           0x00000001

/* Legacy defines */
#define DMA_CR_ACK                                          0x00100000

/********************  Bits definition for DMA_CNDTR register  **************/
#define DMA_NDT                                             0x0000ffff
#define DMA_NDT_0                                           0x00000001
#define DMA_NDT_1                                           0x00000002
#define DMA_NDT_2                                           0x00000004
#define DMA_NDT_3                                           0x00000008
#define DMA_NDT_4                                           0x00000010
#define DMA_NDT_5                                           0x00000020
#define DMA_NDT_6                                           0x00000040
#define DMA_NDT_7                                           0x00000080
#define DMA_NDT_8                                           0x00000100
#define DMA_NDT_9                                           0x00000200
#define DMA_NDT_10                                          0x00000400
#define DMA_NDT_11                                          0x00000800
#define DMA_NDT_12                                          0x00001000
#define DMA_NDT_13                                          0x00002000
#define DMA_NDT_14                                          0x00004000
#define DMA_NDT_15                                          0x00008000

/********************  Bits definition for DMA_FCR register  ****************/
#define DMA_FCR_FEIE                                        0x00000080
#define DMA_FCR_FS                                          0x00000038
#define DMA_FCR_FS_0                                        0x00000008
#define DMA_FCR_FS_1                                        0x00000010
#define DMA_FCR_FS_2                                        0x00000020
#define DMA_FCR_DMDIS                                       0x00000004
#define DMA_FCR_FTH                                         0x00000003
#define DMA_FCR_FTH_0                                       0x00000001
#define DMA_FCR_FTH_1                                       0x00000002

/******************  Bit definition for DMA_PAR register  ********************/
#define DMA_PAR_PA                                          0xffffffff

/******************  Bit definition for DMA_M0AR register  ********************/
#define DMA_M0AR_M0A                                        0xffffffff

/******************  Bit definition for DMA_M1AR register  ********************/
#define DMA_M1AR_M1A                                        0xffffffff

/******************  Channel/Stream Allocation DMA1  ********************/
#define DMA1_S0_SPI3_RX                                      0x00000000
#define DMA1_S0_I2C1_RX                                      0x00000001
#define DMA1_S0_TIM4_CH1                                     0x00000002
#define DMA1_S0_I2S3_EXT_RX                                  0x00000003
#define DMA1_S0_TIM5_CH3_TIM5_UP                             0x00000006
#define DMA1_S1_I2C3_RX                                      0x00000001
#define DMA1_S1_TIM2_UP_TIM2_CH3                             0x00000003
#define DMA1_S1_TIM5_CH4_TIM5_TRIG                           0x00000006
#define DMA1_S2_SPI3_RX                                      0x00000000
#define DMA1_S2_I2S3_EXT_RX                                  0x00000002
#define DMA1_S2_I2C3_RX                                      0x00000003
#define DMA1_S2_TIM3_CH4_TIM3_UP                             0x00000005
#define DMA1_S2_TIM5_CH1                                     0x00000006
#define DMA1_S2_I2C2_RX                                      0x00000007
#define DMA1_S3_SPI2_RX                                      0x00000000
#define DMA1_S3_TIM4_CH2                                     0x00000002
#define DMA1_S3_I2S2_EXT_RX                                  0x00000003
#define DMA1_S3_TIM5_CH4_TIM5_TRIG                           0x00000006
#define DMA1_S3_I2C2_RX                                      0x00000007
#define DMA1_S4_SPI2_TX                                      0x00000000
#define DMA1_S4_I2S2_EXT_TX                                  0x00000002
#define DMA1_S4_I2C3_TX                                      0x00000003
#define DMA1_S4_TIM3_CH1_TIM3_TRIG                           0x00000005
#define DMA1_S4_TIM5_CH2                                     0x00000006
#define DMA1_S5_SPI3_TX                                      0x00000000
#define DMA1_S5_I2C1_RX                                      0x00000001
#define DMA1_S5_I2S3_EXT_TX                                  0x00000002
#define DMA1_S5_TIM2_CH1                                     0x00000003
#define DMA1_S5_USART2_RX                                    0x00000004
#define DMA1_S5_TIM3_CH2                                     0x00000005
#define DMA1_S5_I2C3_TX                                      0x00000006
#define DMA1_S6_I2C1_TX                                      0x00000001
#define DMA1_S6_TIM4_UP                                      0x00000002
#define DMA1_S6_TIM2_CH3_TIM2_CH4                            0x00000003
#define DMA1_S6_USART2_TX                                    0x00000004
#define DMA1_S6_TIM5_UP                                      0x00000006
#define DMA1_S7_SPI3_TX                                      0x00000000
#define DMA1_S7_I2C1_TX                                      0x00000001
#define DMA1_S7_TIM4_CH3                                     0x00000002
#define DMA1_S7_TIM2_UP_TIM2_CH4                             0x00000003
#define DMA1_S7_TIM3_CH3                                     0x00000005
#define DMA1_S7_I2C2_TX                                      0x00000007

/******************  Channel/Stream Allocation DMA2  ********************/
#define DMA2_S0_ADC1                                         0x00000000
#define DMA2_S0_SPI1_RX                                      0x00000003
#define DMA2_S0_SPI4_RX                                      0x00000004
#define DMA2_S0_TIM1_TRIG                                    0x00000006
#define DMA2_S1_SPI4_TX                                      0x00000004
#define DMA2_S1_USART6_RX                                    0x00000005
#define DMA2_S1_TIM1_CH1                                     0x00000006
#define DMA2_S2_SPI1_RX                                      0x00000003
#define DMA2_S2_USART1_RX                                    0x00000004
#define DMA2_S2_USART6_RX                                    0x00000005
#define DMA2_S2_TIM1_CH2                                     0x00000006
#define DMA2_S3_SPI1_TX                                      0x00000003
#define DMA2_S3_SDIO                                         0x00000004
#define DMA2_S3_SPI4_TX                                      0x00000005
#define DMA2_S3_TIM1_CH1                                     0x00000006
#define DMA2_S4_ADC1                                         0x00000000
#define DMA2_S4_SPI4_TX                                      0x00000005
#define DMA2_S4_TIM1_CH4_TIM1_TRIG_TIM1_COM                  0x00000006
#define DMA2_S5_SPI1_TX                                      0x00000003
#define DMA2_S5_USART1_RX                                    0x00000004
#define DMA2_S5_TIM1_UP                                      0x00000006
#define DMA2_S6_TIM1_CH1_TIM1_CH2_TIM1_CH3                   0x00000000
#define DMA2_S6_SDIO                                         0x00000004
#define DMA2_S6_USART6_TX                                    0x00000005
#define DMA2_S6_TIM1_CH3                                     0x00000006
#define DMA2_S7_USART1_TX                                    0x00000004
#define DMA2_S7_USART6_TX                                    0x00000005



#endif /* STM32F401_DMA_H */
