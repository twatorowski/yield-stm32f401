/**
 * @file stm32f401.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_STM32F401_H
#define STM32F401_STM32F401_H


#include <stdint.h>
#include "../compiler.h"

/* register type 8 bit */
typedef volatile uint8_t reg8_t;
/* register type 16 bit */
typedef volatile uint16_t reg16_t;
/* register type */
typedef volatile uint32_t reg32_t;


/* stack pointer */
/* base position for the stack pointer entry */
#define STM32_VECTOR_STACK_PTR_BASE                         0
/* base of the exception vectors */
#define STM32_VECTOR_EXC_BASE                               1
/* base position for the interrupt vectors */
#define STM32_VECTOR_INT_BASE                               16


/* stack pointer */
/* stack pointer entry id */
#define STM32_STACK_PTR                                     0

/* core exeptions */
/* reset routine */
#define STM32_EXC_RESET                                     0
/* Non maskable interrupt. The RCC Clock Security System (CSS) is
 * linked to the NMI vector. */
#define STM32_EXC_NMI                                       1
/* All class of fault */
#define STM32_EXC_HARDFAULT                                 2
/* Memory management */
#define STM32_EXC_MEMMANAGE                                 3
/* Pre-fetch fault, memory access fault */
#define STM32_EXC_BUSFAULT                                  4
/* Undefined instruction or illegal state */
#define STM32_EXC_USAGEFAULT                                5
/* System service call via SWI instruction */
#define STM32_EXC_SVC                                       10
/* Debug Monitor */
#define STM32_EXC_DEBUGMON                                  11
/* Pendable request for system service */
#define STM32_EXC_PENDSV                                    13
/* System tick timer */
#define STM32_EXC_SYSTICK                                   14




/** Window Watchdog interrupt */
#define STM32_INT_WWDG                                      0
/** PVD through EXTI line */
#define STM32_INT_EXTI16_PVD                                1
/** RTC tamper, timestamp */
#define STM32_INT_EXTI21_TAMP_STAMP                         2
/** RTC Wakeup interrupt */
#define STM32_INT_EXTI22_RTC_WKUP                           3
/** Flash memory */
#define STM32_INT_FLASH                                     4
/** RCC global interrupt */
#define STM32_INT_RCC                                       5
/** EXTI Line 0 interrupt */
#define STM32_INT_EXTI0                                     6
/** EXTI Line 1 interrupt */
#define STM32_INT_EXTI1                                     7
/** EXTI Line 2 interrupt */
#define STM32_INT_EXTI2                                     8
/** EXTI Line 3 interrupt */
#define STM32_INT_EXTI3                                     9
/** EXTI Line 4 interrupt */
#define STM32_INT_EXTI4                                     10
/** DMA1 Stream 0 */
#define STM32_INT_DMA1_STR0                                 11
/** DMA1 Stream 1 */
#define STM32_INT_DMA1_STR1                                 12
/** DMA1 Stream 2 */
#define STM32_INT_DMA1_STR2                                 13
/** DMA1 Stream 3 */
#define STM32_INT_DMA1_STR3                                 14
/** DMA1 Stream 4 */
#define STM32_INT_DMA1_STR4                                 15
/** DMA1 Stream 5 */
#define STM32_INT_DMA1_STR5                                 16
/** DMA1 Stream 6 */
#define STM32_INT_DMA1_STR6                                 17
/** ADC */
#define STM32_INT_ADC                                       18

/** EXTI Line[9:5] interrupts */
#define STM32_INT_EXTI9_5                                   23
/** TIM1 Break interrupt and TIM9 global interrupt */
#define STM32_INT_TIM1_BRK_TIM9                             24
/** TIM1 TIM1 Update interrupt and TIM10 global interrupt */
#define STM32_INT_TIM1_UP_TIM10                             25
/** TIM1 TIM1 Trigger and Commutation interrupts and TIM11 global interrupt */
#define STM32_INT_TIM1_TRG_COM_TIM11                        26
/** TIM1 capture / compare */
#define STM32_INT_TIM1_CC                                   27
/** TIM2 global interrupt */
#define STM32_INT_TIM2                                      28
/** TIM3 global interrupt */
#define STM32_INT_TIM3                                      29
/** TIM4 global interrupt */
#define STM32_INT_TIM4                                      30
/** I2C1 event interrupt */
#define STM32_INT_I2C1_EV                                   31
/** I2C1 error interrupt */
#define STM32_INT_I2C1_ER                                   32
/** I2C2 event interrupt */
#define STM32_INT_I2C2_EV                                   33
/** I2C2 error interrupt */
#define STM32_INT_I2C2_ER                                   34
/** SPI1 global interrupt */
#define STM32_INT_SPI1                                      35
/** SPI2 global interrupt */
#define STM32_INT_SPI2                                      36
/** USART1 global interrupt */
#define STM32_INT_USART1                                    37
/** USART2 global interrupt */
#define STM32_INT_USART2                                    38

/** EXTI Line[15:10] interrupts */
#define STM32_INT_EXTI15_10                                 40
/** RTC alarms (A and B) */
#define STM32_INT_EXTI17_RTC_ALARM                          41
/** EXTI Line 18 interrupt / USB On-The-Go FS Wakeup through EXTI line interrupt */
#define STM32_INT_EXTI18_OTG_FS_WKUP                        42

/** DMA1 Stream 7 */
#define STM32_INT_DMA1_STR7                                 47

/** SDIO global interrupt */
#define STM32_INT_SDIO                                      49
/** TIM5 global interrupt */
#define STM32_INT_TIM5                                      50
/** SPI3 global interrupt */
#define STM32_INT_SPI3                                      51

/** DMA2 Stream 0 interrupt */
#define STM32_INT_DMA2_STR0                                 56
/** DMA2 Stream 1 interrupt */
#define STM32_INT_DMA2_STR1                                 57
/** DMA2 Stream 2 interrupt */
#define STM32_INT_DMA2_STR2                                 58
/** DMA2 Stream 3 interrupt */
#define STM32_INT_DMA2_STR3                                 59
/** DMA2 Stream 4 interrupt */
#define STM32_INT_DMA2_STR4                                 60

/** USB On The Go FS global interrupt */
#define STM32_INT_OTG_FS                                    67
/** DMA2 Stream 5 interrupt */
#define STM32_INT_DMA2_STR5                                 68
/** DMA2 Stream 6 interrupt */
#define STM32_INT_DMA2_STR6                                 69
/** DMA2 Stream 7 interrupt */
#define STM32_INT_DMA2_STR7                                 70
/** USART6 global interrupt */
#define STM32_INT_USART6                                    71
/** I2C3 event interrupt */
#define STM32_INT_I2C3_EV                                   72
/** I2C3 error interrupt */
#define STM32_INT_I2C3_ER                                   73

/** Floating point unit interrupt */
#define STM32_INT_FPU                                       81
/** SPI4 global interrupt */
#define STM32_INT_SPI4                                      84



/* enable interrupts globally */
#define STM32_ENABLEINTS()                      \
    __asm__ volatile ("cpsie i")

/* disable interrupts globally */
#define STM32_DISABLEINTS()                     \
    __asm__ volatile ("cpsid i")




#endif /* STM32F401_STM32F401_H */
