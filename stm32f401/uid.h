/**
 * @file uid.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_UID_H
#define STM32F401_UID_H

#include "stm32f401.h"

/* register base address */
#define UID_BASE                                            0x1FFF7A10

/* instances */
#define UID                                                 ((uid_t *)UID_BASE)

/* register bank */
typedef struct {
    reg32_t U_ID0;
    reg32_t U_ID1;
    reg32_t U_ID2;
} uid_t; 

#endif /* STM32F401_UID_H */
