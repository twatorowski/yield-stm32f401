/**
 * @file gpio.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_GPIO_H
#define STM32F401_GPIO_H

#include "stm32f401.h"

/* base addresses */
#define GPIOA_BASE                                           0x40020000
#define GPIOB_BASE                                           0x40020400
#define GPIOC_BASE                                           0x40020800
#define GPIOD_BASE                                           0x40020C00
#define GPIOE_BASE                                           0x40021000
#define GPIOH_BASE                                           0x40021C00

/* instances */
#define GPIOA                                                ((gpio_t *)GPIOA_BASE)
#define GPIOB                                                ((gpio_t *)GPIOB_BASE)
#define GPIOC                                                ((gpio_t *)GPIOC_BASE)
#define GPIOD                                                ((gpio_t *)GPIOD_BASE)
#define GPIOE                                                ((gpio_t *)GPIOE_BASE)
#define GPIOF                                                ((gpio_t *)GPIOF_BASE)
#define GPIOG                                                ((gpio_t *)GPIOG_BASE)
#define GPIOH                                                ((gpio_t *)GPIOH_BASE)


/* register bank */
typedef struct {
    reg32_t MODER;
    reg32_t OTYPER;
    reg32_t OSPEEDR;
    reg32_t PUPDR;
    reg32_t IDR;
    reg32_t ODR;
    reg32_t BSRR;
    reg32_t LCKR;
    reg32_t AFRL;
    reg32_t AFRH;
} gpio_t;

/******************  Bits definition for GPIO_MODER register  *****************/
#define GPIO_MODER_MODER0                                    0x00000003
#define GPIO_MODER_MODER0_0                                  0x00000001
#define GPIO_MODER_MODER0_1                                  0x00000002
#define GPIO_MODER_MODER1                                    0x0000000C
#define GPIO_MODER_MODER1_0                                  0x00000004
#define GPIO_MODER_MODER1_1                                  0x00000008
#define GPIO_MODER_MODER2                                    0x00000030
#define GPIO_MODER_MODER2_0                                  0x00000010
#define GPIO_MODER_MODER2_1                                  0x00000020
#define GPIO_MODER_MODER3                                    0x000000C0
#define GPIO_MODER_MODER3_0                                  0x00000040
#define GPIO_MODER_MODER3_1                                  0x00000080
#define GPIO_MODER_MODER4                                    0x00000300
#define GPIO_MODER_MODER4_0                                  0x00000100
#define GPIO_MODER_MODER4_1                                  0x00000200
#define GPIO_MODER_MODER5                                    0x00000C00
#define GPIO_MODER_MODER5_0                                  0x00000400
#define GPIO_MODER_MODER5_1                                  0x00000800
#define GPIO_MODER_MODER6                                    0x00003000
#define GPIO_MODER_MODER6_0                                  0x00001000
#define GPIO_MODER_MODER6_1                                  0x00002000
#define GPIO_MODER_MODER7                                    0x0000C000
#define GPIO_MODER_MODER7_0                                  0x00004000
#define GPIO_MODER_MODER7_1                                  0x00008000
#define GPIO_MODER_MODER8                                    0x00030000
#define GPIO_MODER_MODER8_0                                  0x00010000
#define GPIO_MODER_MODER8_1                                  0x00020000
#define GPIO_MODER_MODER9                                    0x000C0000
#define GPIO_MODER_MODER9_0                                  0x00040000
#define GPIO_MODER_MODER9_1                                  0x00080000
#define GPIO_MODER_MODER10                                   0x00300000
#define GPIO_MODER_MODER10_0                                 0x00100000
#define GPIO_MODER_MODER10_1                                 0x00200000
#define GPIO_MODER_MODER11                                   0x00C00000
#define GPIO_MODER_MODER11_0                                 0x00400000
#define GPIO_MODER_MODER11_1                                 0x00800000
#define GPIO_MODER_MODER12                                   0x03000000
#define GPIO_MODER_MODER12_0                                 0x01000000
#define GPIO_MODER_MODER12_1                                 0x02000000
#define GPIO_MODER_MODER13                                   0x0C000000
#define GPIO_MODER_MODER13_0                                 0x04000000
#define GPIO_MODER_MODER13_1                                 0x08000000
#define GPIO_MODER_MODER14                                   0x30000000
#define GPIO_MODER_MODER14_0                                 0x10000000
#define GPIO_MODER_MODER14_1                                 0x20000000
#define GPIO_MODER_MODER15                                   0xC0000000
#define GPIO_MODER_MODER15_0                                 0x40000000
#define GPIO_MODER_MODER15_1                                 0x80000000

/* Legacy defines */
#define GPIO_MODER_MODE0                                     GPIO_MODER_MODER0
#define GPIO_MODER_MODE0_0                                   GPIO_MODER_MODER0_0
#define GPIO_MODER_MODE0_1                                   GPIO_MODER_MODER0_1
#define GPIO_MODER_MODE1                                     GPIO_MODER_MODER1
#define GPIO_MODER_MODE1_0                                   GPIO_MODER_MODER1_0
#define GPIO_MODER_MODE1_1                                   GPIO_MODER_MODER1_1
#define GPIO_MODER_MODE2                                     GPIO_MODER_MODER2
#define GPIO_MODER_MODE2_0                                   GPIO_MODER_MODER2_0
#define GPIO_MODER_MODE2_1                                   GPIO_MODER_MODER2_1
#define GPIO_MODER_MODE3                                     GPIO_MODER_MODER3
#define GPIO_MODER_MODE3_0                                   GPIO_MODER_MODER3_0
#define GPIO_MODER_MODE3_1                                   GPIO_MODER_MODER3_1
#define GPIO_MODER_MODE4                                     GPIO_MODER_MODER4
#define GPIO_MODER_MODE4_0                                   GPIO_MODER_MODER4_0
#define GPIO_MODER_MODE4_1                                   GPIO_MODER_MODER4_1
#define GPIO_MODER_MODE5                                     GPIO_MODER_MODER5
#define GPIO_MODER_MODE5_0                                   GPIO_MODER_MODER5_0
#define GPIO_MODER_MODE5_1                                   GPIO_MODER_MODER5_1
#define GPIO_MODER_MODE6                                     GPIO_MODER_MODER6
#define GPIO_MODER_MODE6_0                                   GPIO_MODER_MODER6_0
#define GPIO_MODER_MODE6_1                                   GPIO_MODER_MODER6_1
#define GPIO_MODER_MODE7                                     GPIO_MODER_MODER7
#define GPIO_MODER_MODE7_0                                   GPIO_MODER_MODER7_0
#define GPIO_MODER_MODE7_1                                   GPIO_MODER_MODER7_1
#define GPIO_MODER_MODE8                                     GPIO_MODER_MODER8
#define GPIO_MODER_MODE8_0                                   GPIO_MODER_MODER8_0
#define GPIO_MODER_MODE8_1                                   GPIO_MODER_MODER8_1
#define GPIO_MODER_MODE9                                     GPIO_MODER_MODER9
#define GPIO_MODER_MODE9_0                                   GPIO_MODER_MODER9_0
#define GPIO_MODER_MODE9_1                                   GPIO_MODER_MODER9_1
#define GPIO_MODER_MODE10                                    GPIO_MODER_MODER10
#define GPIO_MODER_MODE10_0                                  GPIO_MODER_MODER10_0
#define GPIO_MODER_MODE10_1                                  GPIO_MODER_MODER10_1
#define GPIO_MODER_MODE11                                    GPIO_MODER_MODER11
#define GPIO_MODER_MODE11_0                                  GPIO_MODER_MODER11_0
#define GPIO_MODER_MODE11_1                                  GPIO_MODER_MODER11_1
#define GPIO_MODER_MODE12                                    GPIO_MODER_MODER12
#define GPIO_MODER_MODE12_0                                  GPIO_MODER_MODER12_0
#define GPIO_MODER_MODE12_1                                  GPIO_MODER_MODER12_1
#define GPIO_MODER_MODE13                                    GPIO_MODER_MODER13
#define GPIO_MODER_MODE13_0                                  GPIO_MODER_MODER13_0
#define GPIO_MODER_MODE13_1                                  GPIO_MODER_MODER13_1
#define GPIO_MODER_MODE14                                    GPIO_MODER_MODER14
#define GPIO_MODER_MODE14_0                                  GPIO_MODER_MODER14_0
#define GPIO_MODER_MODE14_1                                  GPIO_MODER_MODER14_1
#define GPIO_MODER_MODE15                                    GPIO_MODER_MODER15
#define GPIO_MODER_MODE15_0                                  GPIO_MODER_MODER15_0
#define GPIO_MODER_MODE15_1                                  GPIO_MODER_MODER15_1

/******************  Bits definition for GPIO_OTYPER register  ****************/
#define GPIO_OTYPER_OT0                                      0x00000001
#define GPIO_OTYPER_OT1                                      0x00000002
#define GPIO_OTYPER_OT2                                      0x00000004
#define GPIO_OTYPER_OT3                                      0x00000008
#define GPIO_OTYPER_OT4                                      0x00000010
#define GPIO_OTYPER_OT5                                      0x00000020
#define GPIO_OTYPER_OT6                                      0x00000040
#define GPIO_OTYPER_OT7                                      0x00000080
#define GPIO_OTYPER_OT8                                      0x00000100
#define GPIO_OTYPER_OT9                                      0x00000200
#define GPIO_OTYPER_OT10                                     0x00000400
#define GPIO_OTYPER_OT11                                     0x00000800
#define GPIO_OTYPER_OT12                                     0x00001000
#define GPIO_OTYPER_OT13                                     0x00002000
#define GPIO_OTYPER_OT14                                     0x00004000
#define GPIO_OTYPER_OT15                                     0x00008000

/* Legacy defines */
#define GPIO_OTYPER_OT_0                                     GPIO_OTYPER_OT0
#define GPIO_OTYPER_OT_1                                     GPIO_OTYPER_OT1
#define GPIO_OTYPER_OT_2                                     GPIO_OTYPER_OT2
#define GPIO_OTYPER_OT_3                                     GPIO_OTYPER_OT3
#define GPIO_OTYPER_OT_4                                     GPIO_OTYPER_OT4
#define GPIO_OTYPER_OT_5                                     GPIO_OTYPER_OT5
#define GPIO_OTYPER_OT_6                                     GPIO_OTYPER_OT6
#define GPIO_OTYPER_OT_7                                     GPIO_OTYPER_OT7
#define GPIO_OTYPER_OT_8                                     GPIO_OTYPER_OT8
#define GPIO_OTYPER_OT_9                                     GPIO_OTYPER_OT9
#define GPIO_OTYPER_OT_10                                    GPIO_OTYPER_OT10
#define GPIO_OTYPER_OT_11                                    GPIO_OTYPER_OT11
#define GPIO_OTYPER_OT_12                                    GPIO_OTYPER_OT12
#define GPIO_OTYPER_OT_13                                    GPIO_OTYPER_OT13
#define GPIO_OTYPER_OT_14                                    GPIO_OTYPER_OT14
#define GPIO_OTYPER_OT_15                                    GPIO_OTYPER_OT15

/******************  Bits definition for GPIO_OSPEEDR register  ***************/
#define GPIO_OSPEEDR_OSPEED0                                 0x00000003
#define GPIO_OSPEEDR_OSPEED0_0                               0x00000001
#define GPIO_OSPEEDR_OSPEED0_1                               0x00000002
#define GPIO_OSPEEDR_OSPEED1                                 0x0000000C
#define GPIO_OSPEEDR_OSPEED1_0                               0x00000004
#define GPIO_OSPEEDR_OSPEED1_1                               0x00000008
#define GPIO_OSPEEDR_OSPEED2                                 0x00000030
#define GPIO_OSPEEDR_OSPEED2_0                               0x00000010
#define GPIO_OSPEEDR_OSPEED2_1                               0x00000020
#define GPIO_OSPEEDR_OSPEED3                                 0x000000C0
#define GPIO_OSPEEDR_OSPEED3_0                               0x00000040
#define GPIO_OSPEEDR_OSPEED3_1                               0x00000080
#define GPIO_OSPEEDR_OSPEED4                                 0x00000300
#define GPIO_OSPEEDR_OSPEED4_0                               0x00000100
#define GPIO_OSPEEDR_OSPEED4_1                               0x00000200
#define GPIO_OSPEEDR_OSPEED5                                 0x00000C00
#define GPIO_OSPEEDR_OSPEED5_0                               0x00000400
#define GPIO_OSPEEDR_OSPEED5_1                               0x00000800
#define GPIO_OSPEEDR_OSPEED6                                 0x00003000
#define GPIO_OSPEEDR_OSPEED6_0                               0x00001000
#define GPIO_OSPEEDR_OSPEED6_1                               0x00002000
#define GPIO_OSPEEDR_OSPEED7                                 0x0000C000
#define GPIO_OSPEEDR_OSPEED7_0                               0x00004000
#define GPIO_OSPEEDR_OSPEED7_1                               0x00008000
#define GPIO_OSPEEDR_OSPEED8                                 0x00030000
#define GPIO_OSPEEDR_OSPEED8_0                               0x00010000
#define GPIO_OSPEEDR_OSPEED8_1                               0x00020000
#define GPIO_OSPEEDR_OSPEED9                                 0x000C0000
#define GPIO_OSPEEDR_OSPEED9_0                               0x00040000
#define GPIO_OSPEEDR_OSPEED9_1                               0x00080000
#define GPIO_OSPEEDR_OSPEED10                                0x00300000
#define GPIO_OSPEEDR_OSPEED10_0                              0x00100000
#define GPIO_OSPEEDR_OSPEED10_1                              0x00200000
#define GPIO_OSPEEDR_OSPEED11                                0x00C00000
#define GPIO_OSPEEDR_OSPEED11_0                              0x00400000
#define GPIO_OSPEEDR_OSPEED11_1                              0x00800000
#define GPIO_OSPEEDR_OSPEED12                                0x03000000
#define GPIO_OSPEEDR_OSPEED12_0                              0x01000000
#define GPIO_OSPEEDR_OSPEED12_1                              0x02000000
#define GPIO_OSPEEDR_OSPEED13                                0x0C000000
#define GPIO_OSPEEDR_OSPEED13_0                              0x04000000
#define GPIO_OSPEEDR_OSPEED13_1                              0x08000000
#define GPIO_OSPEEDR_OSPEED14                                0x30000000
#define GPIO_OSPEEDR_OSPEED14_0                              0x10000000
#define GPIO_OSPEEDR_OSPEED14_1                              0x20000000
#define GPIO_OSPEEDR_OSPEED15                                0xC0000000
#define GPIO_OSPEEDR_OSPEED15_0                              0x40000000
#define GPIO_OSPEEDR_OSPEED15_1                              0x80000000

/* Legacy defines */
#define GPIO_OSPEEDER_OSPEEDR0                               GPIO_OSPEEDR_OSPEED0
#define GPIO_OSPEEDER_OSPEEDR0_0                             GPIO_OSPEEDR_OSPEED0_0
#define GPIO_OSPEEDER_OSPEEDR0_1                             GPIO_OSPEEDR_OSPEED0_1
#define GPIO_OSPEEDER_OSPEEDR1                               GPIO_OSPEEDR_OSPEED1
#define GPIO_OSPEEDER_OSPEEDR1_0                             GPIO_OSPEEDR_OSPEED1_0
#define GPIO_OSPEEDER_OSPEEDR1_1                             GPIO_OSPEEDR_OSPEED1_1
#define GPIO_OSPEEDER_OSPEEDR2                               GPIO_OSPEEDR_OSPEED2
#define GPIO_OSPEEDER_OSPEEDR2_0                             GPIO_OSPEEDR_OSPEED2_0
#define GPIO_OSPEEDER_OSPEEDR2_1                             GPIO_OSPEEDR_OSPEED2_1
#define GPIO_OSPEEDER_OSPEEDR3                               GPIO_OSPEEDR_OSPEED3
#define GPIO_OSPEEDER_OSPEEDR3_0                             GPIO_OSPEEDR_OSPEED3_0
#define GPIO_OSPEEDER_OSPEEDR3_1                             GPIO_OSPEEDR_OSPEED3_1
#define GPIO_OSPEEDER_OSPEEDR4                               GPIO_OSPEEDR_OSPEED4
#define GPIO_OSPEEDER_OSPEEDR4_0                             GPIO_OSPEEDR_OSPEED4_0
#define GPIO_OSPEEDER_OSPEEDR4_1                             GPIO_OSPEEDR_OSPEED4_1
#define GPIO_OSPEEDER_OSPEEDR5                               GPIO_OSPEEDR_OSPEED5
#define GPIO_OSPEEDER_OSPEEDR5_0                             GPIO_OSPEEDR_OSPEED5_0
#define GPIO_OSPEEDER_OSPEEDR5_1                             GPIO_OSPEEDR_OSPEED5_1
#define GPIO_OSPEEDER_OSPEEDR6                               GPIO_OSPEEDR_OSPEED6
#define GPIO_OSPEEDER_OSPEEDR6_0                             GPIO_OSPEEDR_OSPEED6_0
#define GPIO_OSPEEDER_OSPEEDR6_1                             GPIO_OSPEEDR_OSPEED6_1
#define GPIO_OSPEEDER_OSPEEDR7                               GPIO_OSPEEDR_OSPEED7
#define GPIO_OSPEEDER_OSPEEDR7_0                             GPIO_OSPEEDR_OSPEED7_0
#define GPIO_OSPEEDER_OSPEEDR7_1                             GPIO_OSPEEDR_OSPEED7_1
#define GPIO_OSPEEDER_OSPEEDR8                               GPIO_OSPEEDR_OSPEED8
#define GPIO_OSPEEDER_OSPEEDR8_0                             GPIO_OSPEEDR_OSPEED8_0
#define GPIO_OSPEEDER_OSPEEDR8_1                             GPIO_OSPEEDR_OSPEED8_1
#define GPIO_OSPEEDER_OSPEEDR9                               GPIO_OSPEEDR_OSPEED9
#define GPIO_OSPEEDER_OSPEEDR9_0                             GPIO_OSPEEDR_OSPEED9_0
#define GPIO_OSPEEDER_OSPEEDR9_1                             GPIO_OSPEEDR_OSPEED9_1
#define GPIO_OSPEEDER_OSPEEDR10                              GPIO_OSPEEDR_OSPEED10
#define GPIO_OSPEEDER_OSPEEDR10_0                            GPIO_OSPEEDR_OSPEED10_0
#define GPIO_OSPEEDER_OSPEEDR10_1                            GPIO_OSPEEDR_OSPEED10_1
#define GPIO_OSPEEDER_OSPEEDR11                              GPIO_OSPEEDR_OSPEED11
#define GPIO_OSPEEDER_OSPEEDR11_0                            GPIO_OSPEEDR_OSPEED11_0
#define GPIO_OSPEEDER_OSPEEDR11_1                            GPIO_OSPEEDR_OSPEED11_1
#define GPIO_OSPEEDER_OSPEEDR12                              GPIO_OSPEEDR_OSPEED12
#define GPIO_OSPEEDER_OSPEEDR12_0                            GPIO_OSPEEDR_OSPEED12_0
#define GPIO_OSPEEDER_OSPEEDR12_1                            GPIO_OSPEEDR_OSPEED12_1
#define GPIO_OSPEEDER_OSPEEDR13                              GPIO_OSPEEDR_OSPEED13
#define GPIO_OSPEEDER_OSPEEDR13_0                            GPIO_OSPEEDR_OSPEED13_0
#define GPIO_OSPEEDER_OSPEEDR13_1                            GPIO_OSPEEDR_OSPEED13_1
#define GPIO_OSPEEDER_OSPEEDR14                              GPIO_OSPEEDR_OSPEED14
#define GPIO_OSPEEDER_OSPEEDR14_0                            GPIO_OSPEEDR_OSPEED14_0
#define GPIO_OSPEEDER_OSPEEDR14_1                            GPIO_OSPEEDR_OSPEED14_1
#define GPIO_OSPEEDER_OSPEEDR15                              GPIO_OSPEEDR_OSPEED15
#define GPIO_OSPEEDER_OSPEEDR15_0                            GPIO_OSPEEDR_OSPEED15_0
#define GPIO_OSPEEDER_OSPEEDR15_1                            GPIO_OSPEEDR_OSPEED15_1

/******************  Bits definition for GPIO_PUPDR register  *****************/
#define GPIO_PUPDR_PUPD0                                     0x00000003
#define GPIO_PUPDR_PUPD0_0                                   0x00000001
#define GPIO_PUPDR_PUPD0_1                                   0x00000002
#define GPIO_PUPDR_PUPD1                                     0x0000000C
#define GPIO_PUPDR_PUPD1_0                                   0x00000004
#define GPIO_PUPDR_PUPD1_1                                   0x00000008
#define GPIO_PUPDR_PUPD2                                     0x00000030
#define GPIO_PUPDR_PUPD2_0                                   0x00000010
#define GPIO_PUPDR_PUPD2_1                                   0x00000020
#define GPIO_PUPDR_PUPD3                                     0x000000C0
#define GPIO_PUPDR_PUPD3_0                                   0x00000040
#define GPIO_PUPDR_PUPD3_1                                   0x00000080
#define GPIO_PUPDR_PUPD4                                     0x00000300
#define GPIO_PUPDR_PUPD4_0                                   0x00000100
#define GPIO_PUPDR_PUPD4_1                                   0x00000200
#define GPIO_PUPDR_PUPD5                                     0x00000C00
#define GPIO_PUPDR_PUPD5_0                                   0x00000400
#define GPIO_PUPDR_PUPD5_1                                   0x00000800
#define GPIO_PUPDR_PUPD6                                     0x00003000
#define GPIO_PUPDR_PUPD6_0                                   0x00001000
#define GPIO_PUPDR_PUPD6_1                                   0x00002000
#define GPIO_PUPDR_PUPD7                                     0x0000C000
#define GPIO_PUPDR_PUPD7_0                                   0x00004000
#define GPIO_PUPDR_PUPD7_1                                   0x00008000
#define GPIO_PUPDR_PUPD8                                     0x00030000
#define GPIO_PUPDR_PUPD8_0                                   0x00010000
#define GPIO_PUPDR_PUPD8_1                                   0x00020000
#define GPIO_PUPDR_PUPD9                                     0x000C0000
#define GPIO_PUPDR_PUPD9_0                                   0x00040000
#define GPIO_PUPDR_PUPD9_1                                   0x00080000
#define GPIO_PUPDR_PUPD10                                    0x00300000
#define GPIO_PUPDR_PUPD10_0                                  0x00100000
#define GPIO_PUPDR_PUPD10_1                                  0x00200000
#define GPIO_PUPDR_PUPD11                                    0x00C00000
#define GPIO_PUPDR_PUPD11_0                                  0x00400000
#define GPIO_PUPDR_PUPD11_1                                  0x00800000
#define GPIO_PUPDR_PUPD12                                    0x03000000
#define GPIO_PUPDR_PUPD12_0                                  0x01000000
#define GPIO_PUPDR_PUPD12_1                                  0x02000000
#define GPIO_PUPDR_PUPD13                                    0x0C000000
#define GPIO_PUPDR_PUPD13_0                                  0x04000000
#define GPIO_PUPDR_PUPD13_1                                  0x08000000
#define GPIO_PUPDR_PUPD14                                    0x30000000
#define GPIO_PUPDR_PUPD14_0                                  0x10000000
#define GPIO_PUPDR_PUPD14_1                                  0x20000000
#define GPIO_PUPDR_PUPD15                                    0xC0000000
#define GPIO_PUPDR_PUPD15_0                                  0x40000000
#define GPIO_PUPDR_PUPD15_1                                  0x80000000

/* Legacy defines */
#define GPIO_PUPDR_PUPDR0                                    GPIO_PUPDR_PUPD0
#define GPIO_PUPDR_PUPDR0_0                                  GPIO_PUPDR_PUPD0_0
#define GPIO_PUPDR_PUPDR0_1                                  GPIO_PUPDR_PUPD0_1
#define GPIO_PUPDR_PUPDR1                                    GPIO_PUPDR_PUPD1
#define GPIO_PUPDR_PUPDR1_0                                  GPIO_PUPDR_PUPD1_0
#define GPIO_PUPDR_PUPDR1_1                                  GPIO_PUPDR_PUPD1_1
#define GPIO_PUPDR_PUPDR2                                    GPIO_PUPDR_PUPD2
#define GPIO_PUPDR_PUPDR2_0                                  GPIO_PUPDR_PUPD2_0
#define GPIO_PUPDR_PUPDR2_1                                  GPIO_PUPDR_PUPD2_1
#define GPIO_PUPDR_PUPDR3                                    GPIO_PUPDR_PUPD3
#define GPIO_PUPDR_PUPDR3_0                                  GPIO_PUPDR_PUPD3_0
#define GPIO_PUPDR_PUPDR3_1                                  GPIO_PUPDR_PUPD3_1
#define GPIO_PUPDR_PUPDR4                                    GPIO_PUPDR_PUPD4
#define GPIO_PUPDR_PUPDR4_0                                  GPIO_PUPDR_PUPD4_0
#define GPIO_PUPDR_PUPDR4_1                                  GPIO_PUPDR_PUPD4_1
#define GPIO_PUPDR_PUPDR5                                    GPIO_PUPDR_PUPD5
#define GPIO_PUPDR_PUPDR5_0                                  GPIO_PUPDR_PUPD5_0
#define GPIO_PUPDR_PUPDR5_1                                  GPIO_PUPDR_PUPD5_1
#define GPIO_PUPDR_PUPDR6                                    GPIO_PUPDR_PUPD6
#define GPIO_PUPDR_PUPDR6_0                                  GPIO_PUPDR_PUPD6_0
#define GPIO_PUPDR_PUPDR6_1                                  GPIO_PUPDR_PUPD6_1
#define GPIO_PUPDR_PUPDR7                                    GPIO_PUPDR_PUPD7
#define GPIO_PUPDR_PUPDR7_0                                  GPIO_PUPDR_PUPD7_0
#define GPIO_PUPDR_PUPDR7_1                                  GPIO_PUPDR_PUPD7_1
#define GPIO_PUPDR_PUPDR8                                    GPIO_PUPDR_PUPD8
#define GPIO_PUPDR_PUPDR8_0                                  GPIO_PUPDR_PUPD8_0
#define GPIO_PUPDR_PUPDR8_1                                  GPIO_PUPDR_PUPD8_1
#define GPIO_PUPDR_PUPDR9                                    GPIO_PUPDR_PUPD9
#define GPIO_PUPDR_PUPDR9_0                                  GPIO_PUPDR_PUPD9_0
#define GPIO_PUPDR_PUPDR9_1                                  GPIO_PUPDR_PUPD9_1
#define GPIO_PUPDR_PUPDR10                                   GPIO_PUPDR_PUPD10
#define GPIO_PUPDR_PUPDR10_0                                 GPIO_PUPDR_PUPD10_0
#define GPIO_PUPDR_PUPDR10_1                                 GPIO_PUPDR_PUPD10_1
#define GPIO_PUPDR_PUPDR11                                   GPIO_PUPDR_PUPD11
#define GPIO_PUPDR_PUPDR11_0                                 GPIO_PUPDR_PUPD11_0
#define GPIO_PUPDR_PUPDR11_1                                 GPIO_PUPDR_PUPD11_1
#define GPIO_PUPDR_PUPDR12                                   GPIO_PUPDR_PUPD12
#define GPIO_PUPDR_PUPDR12_0                                 GPIO_PUPDR_PUPD12_0
#define GPIO_PUPDR_PUPDR12_1                                 GPIO_PUPDR_PUPD12_1
#define GPIO_PUPDR_PUPDR13                                   GPIO_PUPDR_PUPD13
#define GPIO_PUPDR_PUPDR13_0                                 GPIO_PUPDR_PUPD13_0
#define GPIO_PUPDR_PUPDR13_1                                 GPIO_PUPDR_PUPD13_1
#define GPIO_PUPDR_PUPDR14                                   GPIO_PUPDR_PUPD14
#define GPIO_PUPDR_PUPDR14_0                                 GPIO_PUPDR_PUPD14_0
#define GPIO_PUPDR_PUPDR14_1                                 GPIO_PUPDR_PUPD14_1
#define GPIO_PUPDR_PUPDR15                                   GPIO_PUPDR_PUPD15
#define GPIO_PUPDR_PUPDR15_0                                 GPIO_PUPDR_PUPD15_0
#define GPIO_PUPDR_PUPDR15_1                                 GPIO_PUPDR_PUPD15_1

/******************  Bits definition for GPIO_IDR register  *******************/
#define GPIO_IDR_ID0                                         0x00000001
#define GPIO_IDR_ID1                                         0x00000002
#define GPIO_IDR_ID2                                         0x00000004
#define GPIO_IDR_ID3                                         0x00000008
#define GPIO_IDR_ID4                                         0x00000010
#define GPIO_IDR_ID5                                         0x00000020
#define GPIO_IDR_ID6                                         0x00000040
#define GPIO_IDR_ID7                                         0x00000080
#define GPIO_IDR_ID8                                         0x00000100
#define GPIO_IDR_ID9                                         0x00000200
#define GPIO_IDR_ID10                                        0x00000400
#define GPIO_IDR_ID11                                        0x00000800
#define GPIO_IDR_ID12                                        0x00001000
#define GPIO_IDR_ID13                                        0x00002000
#define GPIO_IDR_ID14                                        0x00004000
#define GPIO_IDR_ID15                                        0x00008000

/* Legacy defines */
#define GPIO_IDR_IDR_0                                       GPIO_IDR_ID0
#define GPIO_IDR_IDR_1                                       GPIO_IDR_ID1
#define GPIO_IDR_IDR_2                                       GPIO_IDR_ID2
#define GPIO_IDR_IDR_3                                       GPIO_IDR_ID3
#define GPIO_IDR_IDR_4                                       GPIO_IDR_ID4
#define GPIO_IDR_IDR_5                                       GPIO_IDR_ID5
#define GPIO_IDR_IDR_6                                       GPIO_IDR_ID6
#define GPIO_IDR_IDR_7                                       GPIO_IDR_ID7
#define GPIO_IDR_IDR_8                                       GPIO_IDR_ID8
#define GPIO_IDR_IDR_9                                       GPIO_IDR_ID9
#define GPIO_IDR_IDR_10                                      GPIO_IDR_ID10
#define GPIO_IDR_IDR_11                                      GPIO_IDR_ID11
#define GPIO_IDR_IDR_12                                      GPIO_IDR_ID12
#define GPIO_IDR_IDR_13                                      GPIO_IDR_ID13
#define GPIO_IDR_IDR_14                                      GPIO_IDR_ID14
#define GPIO_IDR_IDR_15                                      GPIO_IDR_ID15

/******************  Bits definition for GPIO_ODR register  *******************/
#define GPIO_ODR_OD0                                         0x00000001
#define GPIO_ODR_OD1                                         0x00000002
#define GPIO_ODR_OD2                                         0x00000004
#define GPIO_ODR_OD3                                         0x00000008
#define GPIO_ODR_OD4                                         0x00000010
#define GPIO_ODR_OD5                                         0x00000020
#define GPIO_ODR_OD6                                         0x00000040
#define GPIO_ODR_OD7                                         0x00000080
#define GPIO_ODR_OD8                                         0x00000100
#define GPIO_ODR_OD9                                         0x00000200
#define GPIO_ODR_OD10                                        0x00000400
#define GPIO_ODR_OD11                                        0x00000800
#define GPIO_ODR_OD12                                        0x00001000
#define GPIO_ODR_OD13                                        0x00002000
#define GPIO_ODR_OD14                                        0x00004000
#define GPIO_ODR_OD15                                        0x00008000
/* Legacy defines */
#define GPIO_ODR_ODR_0                                       GPIO_ODR_OD0
#define GPIO_ODR_ODR_1                                       GPIO_ODR_OD1
#define GPIO_ODR_ODR_2                                       GPIO_ODR_OD2
#define GPIO_ODR_ODR_3                                       GPIO_ODR_OD3
#define GPIO_ODR_ODR_4                                       GPIO_ODR_OD4
#define GPIO_ODR_ODR_5                                       GPIO_ODR_OD5
#define GPIO_ODR_ODR_6                                       GPIO_ODR_OD6
#define GPIO_ODR_ODR_7                                       GPIO_ODR_OD7
#define GPIO_ODR_ODR_8                                       GPIO_ODR_OD8
#define GPIO_ODR_ODR_9                                       GPIO_ODR_OD9
#define GPIO_ODR_ODR_10                                      GPIO_ODR_OD10
#define GPIO_ODR_ODR_11                                      GPIO_ODR_OD11
#define GPIO_ODR_ODR_12                                      GPIO_ODR_OD12
#define GPIO_ODR_ODR_13                                      GPIO_ODR_OD13
#define GPIO_ODR_ODR_14                                      GPIO_ODR_OD14
#define GPIO_ODR_ODR_15                                      GPIO_ODR_OD15

/******************  Bits definition for GPIO_BSRR register  ******************/
#define GPIO_BSRR_BS0                                        0x00000001
#define GPIO_BSRR_BS1                                        0x00000002
#define GPIO_BSRR_BS2                                        0x00000004
#define GPIO_BSRR_BS3                                        0x00000008
#define GPIO_BSRR_BS4                                        0x00000010
#define GPIO_BSRR_BS5                                        0x00000020
#define GPIO_BSRR_BS6                                        0x00000040
#define GPIO_BSRR_BS7                                        0x00000080
#define GPIO_BSRR_BS8                                        0x00000100
#define GPIO_BSRR_BS9                                        0x00000200
#define GPIO_BSRR_BS10                                       0x00000400
#define GPIO_BSRR_BS11                                       0x00000800
#define GPIO_BSRR_BS12                                       0x00001000
#define GPIO_BSRR_BS13                                       0x00002000
#define GPIO_BSRR_BS14                                       0x00004000
#define GPIO_BSRR_BS15                                       0x00008000
#define GPIO_BSRR_BR0                                        0x00010000
#define GPIO_BSRR_BR1                                        0x00020000
#define GPIO_BSRR_BR2                                        0x00040000
#define GPIO_BSRR_BR3                                        0x00080000
#define GPIO_BSRR_BR4                                        0x00100000
#define GPIO_BSRR_BR5                                        0x00200000
#define GPIO_BSRR_BR6                                        0x00400000
#define GPIO_BSRR_BR7                                        0x00800000
#define GPIO_BSRR_BR8                                        0x01000000
#define GPIO_BSRR_BR9                                        0x02000000
#define GPIO_BSRR_BR10                                       0x04000000
#define GPIO_BSRR_BR11                                       0x08000000
#define GPIO_BSRR_BR12                                       0x10000000
#define GPIO_BSRR_BR13                                       0x20000000
#define GPIO_BSRR_BR14                                       0x40000000
#define GPIO_BSRR_BR15                                       0x80000000

/* Legacy defines */
#define GPIO_BSRR_BS_0                                       GPIO_BSRR_BS0
#define GPIO_BSRR_BS_1                                       GPIO_BSRR_BS1
#define GPIO_BSRR_BS_2                                       GPIO_BSRR_BS2
#define GPIO_BSRR_BS_3                                       GPIO_BSRR_BS3
#define GPIO_BSRR_BS_4                                       GPIO_BSRR_BS4
#define GPIO_BSRR_BS_5                                       GPIO_BSRR_BS5
#define GPIO_BSRR_BS_6                                       GPIO_BSRR_BS6
#define GPIO_BSRR_BS_7                                       GPIO_BSRR_BS7
#define GPIO_BSRR_BS_8                                       GPIO_BSRR_BS8
#define GPIO_BSRR_BS_9                                       GPIO_BSRR_BS9
#define GPIO_BSRR_BS_10                                      GPIO_BSRR_BS10
#define GPIO_BSRR_BS_11                                      GPIO_BSRR_BS11
#define GPIO_BSRR_BS_12                                      GPIO_BSRR_BS12
#define GPIO_BSRR_BS_13                                      GPIO_BSRR_BS13
#define GPIO_BSRR_BS_14                                      GPIO_BSRR_BS14
#define GPIO_BSRR_BS_15                                      GPIO_BSRR_BS15
#define GPIO_BSRR_BR_0                                       GPIO_BSRR_BR0
#define GPIO_BSRR_BR_1                                       GPIO_BSRR_BR1
#define GPIO_BSRR_BR_2                                       GPIO_BSRR_BR2
#define GPIO_BSRR_BR_3                                       GPIO_BSRR_BR3
#define GPIO_BSRR_BR_4                                       GPIO_BSRR_BR4
#define GPIO_BSRR_BR_5                                       GPIO_BSRR_BR5
#define GPIO_BSRR_BR_6                                       GPIO_BSRR_BR6
#define GPIO_BSRR_BR_7                                       GPIO_BSRR_BR7
#define GPIO_BSRR_BR_8                                       GPIO_BSRR_BR8
#define GPIO_BSRR_BR_9                                       GPIO_BSRR_BR9
#define GPIO_BSRR_BR_10                                      GPIO_BSRR_BR10
#define GPIO_BSRR_BR_11                                      GPIO_BSRR_BR11
#define GPIO_BSRR_BR_12                                      GPIO_BSRR_BR12
#define GPIO_BSRR_BR_13                                      GPIO_BSRR_BR13
#define GPIO_BSRR_BR_14                                      GPIO_BSRR_BR14
#define GPIO_BSRR_BR_15                                      GPIO_BSRR_BR15
#define GPIO_BRR_BR0                                         GPIO_BSRR_BR0
#define GPIO_BRR_BR1                                         GPIO_BSRR_BR1
#define GPIO_BRR_BR2                                         GPIO_BSRR_BR2
#define GPIO_BRR_BR3                                         GPIO_BSRR_BR3
#define GPIO_BRR_BR4                                         GPIO_BSRR_BR4
#define GPIO_BRR_BR5                                         GPIO_BSRR_BR5
#define GPIO_BRR_BR6                                         GPIO_BSRR_BR6
#define GPIO_BRR_BR7                                         GPIO_BSRR_BR7
#define GPIO_BRR_BR8                                         GPIO_BSRR_BR8
#define GPIO_BRR_BR9                                         GPIO_BSRR_BR9
#define GPIO_BRR_BR10                                        GPIO_BSRR_BR10
#define GPIO_BRR_BR11                                        GPIO_BSRR_BR11
#define GPIO_BRR_BR12                                        GPIO_BSRR_BR12
#define GPIO_BRR_BR13                                        GPIO_BSRR_BR13
#define GPIO_BRR_BR14                                        GPIO_BSRR_BR14
#define GPIO_BRR_BR15                                        GPIO_BSRR_BR15
/****************** Bit definition for GPIO_LCKR register *********************/
#define GPIO_LCKR_LCK0                                       0x00000001
#define GPIO_LCKR_LCK1                                       0x00000002
#define GPIO_LCKR_LCK2                                       0x00000004
#define GPIO_LCKR_LCK3                                       0x00000008
#define GPIO_LCKR_LCK4                                       0x00000010
#define GPIO_LCKR_LCK5                                       0x00000020
#define GPIO_LCKR_LCK6                                       0x00000040
#define GPIO_LCKR_LCK7                                       0x00000080
#define GPIO_LCKR_LCK8                                       0x00000100
#define GPIO_LCKR_LCK9                                       0x00000200
#define GPIO_LCKR_LCK10                                      0x00000400
#define GPIO_LCKR_LCK11                                      0x00000800
#define GPIO_LCKR_LCK12                                      0x00001000
#define GPIO_LCKR_LCK13                                      0x00002000
#define GPIO_LCKR_LCK14                                      0x00004000
#define GPIO_LCKR_LCK15                                      0x00008000
#define GPIO_LCKR_LCKK                                       0x00010000
/****************** Bit definition for GPIO_AFRL register *********************/
#define GPIO_AFRL_AFSEL0                                     0x0000000F
#define GPIO_AFRL_AFSEL0_0                                   0x00000001
#define GPIO_AFRL_AFSEL0_1                                   0x00000002
#define GPIO_AFRL_AFSEL0_2                                   0x00000004
#define GPIO_AFRL_AFSEL0_3                                   0x00000008
#define GPIO_AFRL_AFSEL1                                     0x000000F0
#define GPIO_AFRL_AFSEL1_0                                   0x00000010
#define GPIO_AFRL_AFSEL1_1                                   0x00000020
#define GPIO_AFRL_AFSEL1_2                                   0x00000040
#define GPIO_AFRL_AFSEL1_3                                   0x00000080
#define GPIO_AFRL_AFSEL2                                     0x00000F00
#define GPIO_AFRL_AFSEL2_0                                   0x00000100
#define GPIO_AFRL_AFSEL2_1                                   0x00000200
#define GPIO_AFRL_AFSEL2_2                                   0x00000400
#define GPIO_AFRL_AFSEL2_3                                   0x00000800
#define GPIO_AFRL_AFSEL3                                     0x0000F000
#define GPIO_AFRL_AFSEL3_0                                   0x00001000
#define GPIO_AFRL_AFSEL3_1                                   0x00002000
#define GPIO_AFRL_AFSEL3_2                                   0x00004000
#define GPIO_AFRL_AFSEL3_3                                   0x00008000
#define GPIO_AFRL_AFSEL4                                     0x000F0000
#define GPIO_AFRL_AFSEL4_0                                   0x00010000
#define GPIO_AFRL_AFSEL4_1                                   0x00020000
#define GPIO_AFRL_AFSEL4_2                                   0x00040000
#define GPIO_AFRL_AFSEL4_3                                   0x00080000
#define GPIO_AFRL_AFSEL5                                     0x00F00000
#define GPIO_AFRL_AFSEL5_0                                   0x00100000
#define GPIO_AFRL_AFSEL5_1                                   0x00200000
#define GPIO_AFRL_AFSEL5_2                                   0x00400000
#define GPIO_AFRL_AFSEL5_3                                   0x00800000
#define GPIO_AFRL_AFSEL6                                     0x0F000000
#define GPIO_AFRL_AFSEL6_0                                   0x01000000
#define GPIO_AFRL_AFSEL6_1                                   0x02000000
#define GPIO_AFRL_AFSEL6_2                                   0x04000000
#define GPIO_AFRL_AFSEL6_3                                   0x08000000
#define GPIO_AFRL_AFSEL7                                     0xF0000000
#define GPIO_AFRL_AFSEL7_0                                   0x10000000
#define GPIO_AFRL_AFSEL7_1                                   0x20000000
#define GPIO_AFRL_AFSEL7_2                                   0x40000000
#define GPIO_AFRL_AFSEL7_3                                   0x80000000

/* Legacy defines */
#define GPIO_AFRL_AFRL0                                      GPIO_AFRL_AFSEL0
#define GPIO_AFRL_AFRL0_0                                    GPIO_AFRL_AFSEL0_0
#define GPIO_AFRL_AFRL0_1                                    GPIO_AFRL_AFSEL0_1
#define GPIO_AFRL_AFRL0_2                                    GPIO_AFRL_AFSEL0_2
#define GPIO_AFRL_AFRL0_3                                    GPIO_AFRL_AFSEL0_3
#define GPIO_AFRL_AFRL1                                      GPIO_AFRL_AFSEL1
#define GPIO_AFRL_AFRL1_0                                    GPIO_AFRL_AFSEL1_0
#define GPIO_AFRL_AFRL1_1                                    GPIO_AFRL_AFSEL1_1
#define GPIO_AFRL_AFRL1_2                                    GPIO_AFRL_AFSEL1_2
#define GPIO_AFRL_AFRL1_3                                    GPIO_AFRL_AFSEL1_3
#define GPIO_AFRL_AFRL2                                      GPIO_AFRL_AFSEL2
#define GPIO_AFRL_AFRL2_0                                    GPIO_AFRL_AFSEL2_0
#define GPIO_AFRL_AFRL2_1                                    GPIO_AFRL_AFSEL2_1
#define GPIO_AFRL_AFRL2_2                                    GPIO_AFRL_AFSEL2_2
#define GPIO_AFRL_AFRL2_3                                    GPIO_AFRL_AFSEL2_3
#define GPIO_AFRL_AFRL3                                      GPIO_AFRL_AFSEL3
#define GPIO_AFRL_AFRL3_0                                    GPIO_AFRL_AFSEL3_0
#define GPIO_AFRL_AFRL3_1                                    GPIO_AFRL_AFSEL3_1
#define GPIO_AFRL_AFRL3_2                                    GPIO_AFRL_AFSEL3_2
#define GPIO_AFRL_AFRL3_3                                    GPIO_AFRL_AFSEL3_3
#define GPIO_AFRL_AFRL4                                      GPIO_AFRL_AFSEL4
#define GPIO_AFRL_AFRL4_0                                    GPIO_AFRL_AFSEL4_0
#define GPIO_AFRL_AFRL4_1                                    GPIO_AFRL_AFSEL4_1
#define GPIO_AFRL_AFRL4_2                                    GPIO_AFRL_AFSEL4_2
#define GPIO_AFRL_AFRL4_3                                    GPIO_AFRL_AFSEL4_3
#define GPIO_AFRL_AFRL5                                      GPIO_AFRL_AFSEL5
#define GPIO_AFRL_AFRL5_0                                    GPIO_AFRL_AFSEL5_0
#define GPIO_AFRL_AFRL5_1                                    GPIO_AFRL_AFSEL5_1
#define GPIO_AFRL_AFRL5_2                                    GPIO_AFRL_AFSEL5_2
#define GPIO_AFRL_AFRL5_3                                    GPIO_AFRL_AFSEL5_3
#define GPIO_AFRL_AFRL6                                      GPIO_AFRL_AFSEL6
#define GPIO_AFRL_AFRL6_0                                    GPIO_AFRL_AFSEL6_0
#define GPIO_AFRL_AFRL6_1                                    GPIO_AFRL_AFSEL6_1
#define GPIO_AFRL_AFRL6_2                                    GPIO_AFRL_AFSEL6_2
#define GPIO_AFRL_AFRL6_3                                    GPIO_AFRL_AFSEL6_3
#define GPIO_AFRL_AFRL7                                      GPIO_AFRL_AFSEL7
#define GPIO_AFRL_AFRL7_0                                    GPIO_AFRL_AFSEL7_0
#define GPIO_AFRL_AFRL7_1                                    GPIO_AFRL_AFSEL7_1
#define GPIO_AFRL_AFRL7_2                                    GPIO_AFRL_AFSEL7_2
#define GPIO_AFRL_AFRL7_3                                    GPIO_AFRL_AFSEL7_3

/****************** Bit definition for GPIO_AFRH register *********************/
#define GPIO_AFRH_AFSEL8                                     0x0000000F
#define GPIO_AFRH_AFSEL8_0                                   0x00000001
#define GPIO_AFRH_AFSEL8_1                                   0x00000002
#define GPIO_AFRH_AFSEL8_2                                   0x00000004
#define GPIO_AFRH_AFSEL8_3                                   0x00000008
#define GPIO_AFRH_AFSEL9                                     0x000000F0
#define GPIO_AFRH_AFSEL9_0                                   0x00000010
#define GPIO_AFRH_AFSEL9_1                                   0x00000020
#define GPIO_AFRH_AFSEL9_2                                   0x00000040
#define GPIO_AFRH_AFSEL9_3                                   0x00000080
#define GPIO_AFRH_AFSEL10                                    0x00000F00
#define GPIO_AFRH_AFSEL10_0                                  0x00000100
#define GPIO_AFRH_AFSEL10_1                                  0x00000200
#define GPIO_AFRH_AFSEL10_2                                  0x00000400
#define GPIO_AFRH_AFSEL10_3                                  0x00000800
#define GPIO_AFRH_AFSEL11                                    0x0000F000
#define GPIO_AFRH_AFSEL11_0                                  0x00001000
#define GPIO_AFRH_AFSEL11_1                                  0x00002000
#define GPIO_AFRH_AFSEL11_2                                  0x00004000
#define GPIO_AFRH_AFSEL11_3                                  0x00008000
#define GPIO_AFRH_AFSEL12                                    0x000F0000
#define GPIO_AFRH_AFSEL12_0                                  0x00010000
#define GPIO_AFRH_AFSEL12_1                                  0x00020000
#define GPIO_AFRH_AFSEL12_2                                  0x00040000
#define GPIO_AFRH_AFSEL12_3                                  0x00080000
#define GPIO_AFRH_AFSEL13                                    0x00F00000
#define GPIO_AFRH_AFSEL13_0                                  0x00100000
#define GPIO_AFRH_AFSEL13_1                                  0x00200000
#define GPIO_AFRH_AFSEL13_2                                  0x00400000
#define GPIO_AFRH_AFSEL13_3                                  0x00800000
#define GPIO_AFRH_AFSEL14                                    0x0F000000
#define GPIO_AFRH_AFSEL14_0                                  0x01000000
#define GPIO_AFRH_AFSEL14_1                                  0x02000000
#define GPIO_AFRH_AFSEL14_2                                  0x04000000
#define GPIO_AFRH_AFSEL14_3                                  0x08000000
#define GPIO_AFRH_AFSEL15                                    0xF0000000
#define GPIO_AFRH_AFSEL15_0                                  0x10000000
#define GPIO_AFRH_AFSEL15_1                                  0x20000000
#define GPIO_AFRH_AFSEL15_2                                  0x40000000
#define GPIO_AFRH_AFSEL15_3                                  0x80000000

/* Legacy defines */
#define GPIO_AFRH_AFRH0                                      GPIO_AFRH_AFSEL8
#define GPIO_AFRH_AFRH0_0                                    GPIO_AFRH_AFSEL8_0
#define GPIO_AFRH_AFRH0_1                                    GPIO_AFRH_AFSEL8_1
#define GPIO_AFRH_AFRH0_2                                    GPIO_AFRH_AFSEL8_2
#define GPIO_AFRH_AFRH0_3                                    GPIO_AFRH_AFSEL8_3
#define GPIO_AFRH_AFRH1                                      GPIO_AFRH_AFSEL9
#define GPIO_AFRH_AFRH1_0                                    GPIO_AFRH_AFSEL9_0
#define GPIO_AFRH_AFRH1_1                                    GPIO_AFRH_AFSEL9_1
#define GPIO_AFRH_AFRH1_2                                    GPIO_AFRH_AFSEL9_2
#define GPIO_AFRH_AFRH1_3                                    GPIO_AFRH_AFSEL9_3
#define GPIO_AFRH_AFRH2                                      GPIO_AFRH_AFSEL10
#define GPIO_AFRH_AFRH2_0                                    GPIO_AFRH_AFSEL10_0
#define GPIO_AFRH_AFRH2_1                                    GPIO_AFRH_AFSEL10_1
#define GPIO_AFRH_AFRH2_2                                    GPIO_AFRH_AFSEL10_2
#define GPIO_AFRH_AFRH2_3                                    GPIO_AFRH_AFSEL10_3
#define GPIO_AFRH_AFRH3                                      GPIO_AFRH_AFSEL11
#define GPIO_AFRH_AFRH3_0                                    GPIO_AFRH_AFSEL11_0
#define GPIO_AFRH_AFRH3_1                                    GPIO_AFRH_AFSEL11_1
#define GPIO_AFRH_AFRH3_2                                    GPIO_AFRH_AFSEL11_2
#define GPIO_AFRH_AFRH3_3                                    GPIO_AFRH_AFSEL11_3
#define GPIO_AFRH_AFRH4                                      GPIO_AFRH_AFSEL12
#define GPIO_AFRH_AFRH4_0                                    GPIO_AFRH_AFSEL12_0
#define GPIO_AFRH_AFRH4_1                                    GPIO_AFRH_AFSEL12_1
#define GPIO_AFRH_AFRH4_2                                    GPIO_AFRH_AFSEL12_2
#define GPIO_AFRH_AFRH4_3                                    GPIO_AFRH_AFSEL12_3
#define GPIO_AFRH_AFRH5                                      GPIO_AFRH_AFSEL13
#define GPIO_AFRH_AFRH5_0                                    GPIO_AFRH_AFSEL13_0
#define GPIO_AFRH_AFRH5_1                                    GPIO_AFRH_AFSEL13_1
#define GPIO_AFRH_AFRH5_2                                    GPIO_AFRH_AFSEL13_2
#define GPIO_AFRH_AFRH5_3                                    GPIO_AFRH_AFSEL13_3
#define GPIO_AFRH_AFRH6                                      GPIO_AFRH_AFSEL14
#define GPIO_AFRH_AFRH6_0                                    GPIO_AFRH_AFSEL14_0
#define GPIO_AFRH_AFRH6_1                                    GPIO_AFRH_AFSEL14_1
#define GPIO_AFRH_AFRH6_2                                    GPIO_AFRH_AFSEL14_2
#define GPIO_AFRH_AFRH6_3                                    GPIO_AFRH_AFSEL14_3
#define GPIO_AFRH_AFRH7                                      GPIO_AFRH_AFSEL15
#define GPIO_AFRH_AFRH7_0                                    GPIO_AFRH_AFSEL15_0
#define GPIO_AFRH_AFRH7_1                                    GPIO_AFRH_AFSEL15_1
#define GPIO_AFRH_AFRH7_2                                    GPIO_AFRH_AFSEL15_2
#define GPIO_AFRH_AFRH7_3                                    GPIO_AFRH_AFSEL15_3




#endif /* STM32F401_GPIO_H */
