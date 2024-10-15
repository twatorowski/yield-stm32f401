/**
 * @file spi.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_SPI_H
#define STM32F401_SPI_H

#include "stm32f401.h"


/* register base */
#define SPI1_BASE                               (0x40013000)
#define SPI2_I2S2_BASE                          (0x40003800)
#define SPI3_I2S3_BASE                          (0x40003C00)
#define SPI4_BASE                               (0x40013400)   


/* registers */
#define SPI1                                    ((spi_t *) SPI1_BASE)
#define SPI2_I2S2                               ((spi_t *) SPI2_I2S2_BASE)
#define SPI3_I2S3                               ((spi_t *) SPI3_I2S3_BASE)
#define SPI4                                    ((spi_t *) SPI4_BASE)


/* register base */
typedef struct {
    reg32_t CR1;
    reg32_t CR2;
    reg32_t SR;
    reg32_t DR;
    reg32_t CRCPR;
    reg32_t RXCRCR;
    reg32_t TXCRCR;
    reg32_t I2SCFGR;
    reg32_t I2SPR;
} spi_t;

/*******************  Bit definition for SPI_CR1 register  ********************/
#define SPI_CR1_CPHA                                         0x00000001
#define SPI_CR1_CPOL                                         0x00000002
#define SPI_CR1_MSTR                                         0x00000004
#define SPI_CR1_BR                                           0x00000038
#define SPI_CR1_BR_0                                         0x00000008
#define SPI_CR1_BR_1                                         0x00000010
#define SPI_CR1_BR_2                                         0x00000020
#define SPI_CR1_SPE                                          0x00000040
#define SPI_CR1_LSBFIRST                                     0x00000080
#define SPI_CR1_SSI                                          0x00000100
#define SPI_CR1_SSM                                          0x00000200
#define SPI_CR1_RXONLY                                       0x00000400
#define SPI_CR1_DFF                                          0x00000800
#define SPI_CR1_CRCNEXT                                      0x00001000
#define SPI_CR1_CRCEN                                        0x00002000
#define SPI_CR1_BIDIOE                                       0x00004000
#define SPI_CR1_BIDIMODE                                     0x00008000

/*******************  Bit definition for SPI_CR2 register  ********************/
#define SPI_CR2_RXDMAEN                                      0x00000001
#define SPI_CR2_TXDMAEN                                      0x00000002
#define SPI_CR2_SSOE                                         0x00000004
#define SPI_CR2_FRF                                          0x00000010
#define SPI_CR2_ERRIE                                        0x00000020
#define SPI_CR2_RXNEIE                                       0x00000040
#define SPI_CR2_TXEIE                                        0x00000080

/********************  Bit definition for SPI_SR register  ********************/
#define SPI_SR_RXNE                                          0x00000001
#define SPI_SR_TXE                                           0x00000002
#define SPI_SR_CHSIDE                                        0x00000004
#define SPI_SR_UDR                                           0x00000008
#define SPI_SR_CRCERR                                        0x00000010
#define SPI_SR_MODF                                          0x00000020
#define SPI_SR_OVR                                           0x00000040
#define SPI_SR_BSY                                           0x00000080
#define SPI_SR_FRE                                           0x00000100

/********************  Bit definition for SPI_DR register  ********************/
#define SPI_DR_DR                                            0x0000ffff

/*******************  Bit definition for SPI_CRCPR register  ******************/
#define SPI_CRCPR_CRCPOLY                                    0x0000ffff

/******************  Bit definition for SPI_RXCRCR register  ******************/
#define SPI_RXCRCR_RXCRC                                     0x0000ffff

/******************  Bit definition for SPI_TXCRCR register  ******************/
#define SPI_TXCRCR_TXCRC                                     0x0000ffff

/******************  Bit definition for SPI_I2SCFGR register  *****************/
#define SPI_I2SCFGR_CHLEN                                    0x00000001
#define SPI_I2SCFGR_DATLEN                                   0x00000006
#define SPI_I2SCFGR_DATLEN_0                                 0x00000002
#define SPI_I2SCFGR_DATLEN_1                                 0x00000004
#define SPI_I2SCFGR_CKPOL                                    0x00000008
#define SPI_I2SCFGR_I2SSTD                                   0x00000030
#define SPI_I2SCFGR_I2SSTD_0                                 0x00000010
#define SPI_I2SCFGR_I2SSTD_1                                 0x00000020
#define SPI_I2SCFGR_PCMSYNC                                  0x00000080
#define SPI_I2SCFGR_I2SCFG                                   0x00000300
#define SPI_I2SCFGR_I2SCFG_0                                 0x00000100
#define SPI_I2SCFGR_I2SCFG_1                                 0x00000200
#define SPI_I2SCFGR_I2SE                                     0x00000400
#define SPI_I2SCFGR_I2SMOD                                   0x00000800

/******************  Bit definition for SPI_I2SPR register  *******************/
#define SPI_I2SPR_I2SDIV                                     0x000000ff
#define SPI_I2SPR_ODD                                        0x00000100
#define SPI_I2SPR_MCKOE                                      0x00000200



#endif /* STM32F401_SPI_H */
