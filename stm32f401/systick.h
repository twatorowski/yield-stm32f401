/**
 * @file systick.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef STM32F401_SYSTICK_H
#define STM32F401_SYSTICK_H


#include "stm32f401.h"

/* base addresses */
#define SYSTICK_BASE                                        (0xE000E010)

/* instances */
#define SYSTICK                                             ((systick_t *)SYSTICK_BASE)     

/* register bank */
typedef struct {
    reg32_t CTRL;               
    reg32_t LOAD;
    reg32_t VAL;               
    reg32_t CALIB;
} systick_t;


/* SysTick Control / Status Register Definitions */
#define SYSTICK_CTRL_COUNTFLAG                               0x00010000
#define SYSTICK_CTRL_CLKSOURCE                               0x00000004
#define SYSTICK_CTRL_TICKINT                                 0x00000002
#define SYSTICK_CTRL_ENABLE                                  0x00000001

/* SysTick Reload Register Definitions */
#define SYSTICK_LOAD_RELOAD                                  0x00ffffff

/* SysTick Current Register Definitions */
#define SYSTICK_VAL_CURRENT                                  0x00ffffff

/* SysTick Calibration Register Definitions */
#define SYSTICK_CALIB_NOREF                                  0x80000000
#define SYSTICK_CALIB_SKEW                                   0x40000000
#define SYSTICK_CALIB_TENMS                                  0x00ffffff


#endif /* STM32F401_SYSTICK_H */