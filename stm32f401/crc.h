/**
 * @file crc.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_CRC_H
#define STM32F401_CRC_H


#include "stm32f401.h"

/* base addresses */
#define CRC_BASE                                             0x40023000

/* instances */
#define CRC                                     ((crc_t *)CRC_BASE)

/* register bank */
typedef struct {
    reg32_t DR;
    reg8_t IDR;
    reg8_t RESERVED0;
    reg16_t RESERVED1;
    reg32_t CR;
} crc_t;

/*******************  Bit definition for CRC_DR register  *********************/
#define CRC_DR_DR                                            0xffffffff


/*******************  Bit definition for CRC_IDR register  ********************/
#define CRC_IDR_IDR                                          0x000000ff


/********************  Bit definition for CRC_CR register  ********************/
#define CRC_CR_RESET                                         0x00000001




#endif /* STM32F401_CRC_H */
