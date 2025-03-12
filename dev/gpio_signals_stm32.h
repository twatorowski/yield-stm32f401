/**
 * @file gpio_pins.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-06-12
 * 
 * @brief Signal locations definitions done with gpio.h (Arduino)
 */

#ifndef DEV_GPIO_SIGNALS_STM32_H
#define DEV_GPIO_SIGNALS_STM32_H


/* ports for 'A' signals */
#define GPIO_SIGNAL_A0_PORT                                 GPIOA
#define GPIO_SIGNAL_A1_PORT                                 GPIOA
#define GPIO_SIGNAL_A2_PORT                                 GPIOA
#define GPIO_SIGNAL_A3_PORT                                 GPIOA
#define GPIO_SIGNAL_A4_PORT                                 GPIOA
#define GPIO_SIGNAL_A5_PORT                                 GPIOA
#define GPIO_SIGNAL_A6_PORT                                 GPIOA
#define GPIO_SIGNAL_A7_PORT                                 GPIOA
#define GPIO_SIGNAL_A8_PORT                                 GPIOA
#define GPIO_SIGNAL_A9_PORT                                 GPIOA
#define GPIO_SIGNAL_A10_PORT                                GPIOA
#define GPIO_SIGNAL_A11_PORT                                GPIOA
#define GPIO_SIGNAL_A12_PORT                                GPIOA
#define GPIO_SIGNAL_A13_PORT                                GPIOA
#define GPIO_SIGNAL_A14_PORT                                GPIOA
#define GPIO_SIGNAL_A15_PORT                                GPIOA

/* ports for 'B' signals */
#define GPIO_SIGNAL_B0_PORT                                 GPIOB
#define GPIO_SIGNAL_B1_PORT                                 GPIOB
#define GPIO_SIGNAL_B2_PORT                                 GPIOB
#define GPIO_SIGNAL_B3_PORT                                 GPIOB
#define GPIO_SIGNAL_B4_PORT                                 GPIOB
#define GPIO_SIGNAL_B5_PORT                                 GPIOB
#define GPIO_SIGNAL_B6_PORT                                 GPIOB
#define GPIO_SIGNAL_B7_PORT                                 GPIOB
#define GPIO_SIGNAL_B8_PORT                                 GPIOB
#define GPIO_SIGNAL_B9_PORT                                 GPIOB
#define GPIO_SIGNAL_B10_PORT                                GPIOB
#define GPIO_SIGNAL_B11_PORT                                GPIOB
#define GPIO_SIGNAL_B12_PORT                                GPIOB
#define GPIO_SIGNAL_B13_PORT                                GPIOB
#define GPIO_SIGNAL_B14_PORT                                GPIOB
#define GPIO_SIGNAL_B15_PORT                                GPIOB

/* ports for 'C' signals */
#define GPIO_SIGNAL_C0_PORT                                 GPIOC
#define GPIO_SIGNAL_C1_PORT                                 GPIOC
#define GPIO_SIGNAL_C2_PORT                                 GPIOC
#define GPIO_SIGNAL_C3_PORT                                 GPIOC
#define GPIO_SIGNAL_C4_PORT                                 GPIOC
#define GPIO_SIGNAL_C5_PORT                                 GPIOC
#define GPIO_SIGNAL_C6_PORT                                 GPIOC
#define GPIO_SIGNAL_C7_PORT                                 GPIOC
#define GPIO_SIGNAL_C8_PORT                                 GPIOC
#define GPIO_SIGNAL_C9_PORT                                 GPIOC
#define GPIO_SIGNAL_C10_PORT                                GPIOC
#define GPIO_SIGNAL_C11_PORT                                GPIOC
#define GPIO_SIGNAL_C12_PORT                                GPIOC
#define GPIO_SIGNAL_C13_PORT                                GPIOC
#define GPIO_SIGNAL_C14_PORT                                GPIOC
#define GPIO_SIGNAL_C15_PORT                                GPIOC

/* ports for 'D' signals */
#define GPIO_SIGNAL_D2_PORT                                 GPIOD

/* ports for 'H' signals */
#define GPIO_SIGNAL_H0_PORT                                 GPIOH
#define GPIO_SIGNAL_H1_PORT                                 GPIOH


/* pins for 'A' signals */
#define GPIO_SIGNAL_A0_PIN                                  GPIO_PIN_0
#define GPIO_SIGNAL_A1_PIN                                  GPIO_PIN_1
#define GPIO_SIGNAL_A2_PIN                                  GPIO_PIN_2
#define GPIO_SIGNAL_A3_PIN                                  GPIO_PIN_3
#define GPIO_SIGNAL_A4_PIN                                  GPIO_PIN_4
#define GPIO_SIGNAL_A5_PIN                                  GPIO_PIN_5
#define GPIO_SIGNAL_A6_PIN                                  GPIO_PIN_6
#define GPIO_SIGNAL_A7_PIN                                  GPIO_PIN_7
#define GPIO_SIGNAL_A8_PIN                                  GPIO_PIN_8
#define GPIO_SIGNAL_A9_PIN                                  GPIO_PIN_9
#define GPIO_SIGNAL_A10_PIN                                 GPIO_PIN_10
#define GPIO_SIGNAL_A11_PIN                                 GPIO_PIN_11
#define GPIO_SIGNAL_A12_PIN                                 GPIO_PIN_12
#define GPIO_SIGNAL_A13_PIN                                 GPIO_PIN_13
#define GPIO_SIGNAL_A14_PIN                                 GPIO_PIN_14
#define GPIO_SIGNAL_A15_PIN                                 GPIO_PIN_15

/* pins for 'B' signals */
#define GPIO_SIGNAL_B0_PIN                                  GPIO_PIN_0
#define GPIO_SIGNAL_B1_PIN                                  GPIO_PIN_1
#define GPIO_SIGNAL_B2_PIN                                  GPIO_PIN_2
#define GPIO_SIGNAL_B3_PIN                                  GPIO_PIN_3
#define GPIO_SIGNAL_B4_PIN                                  GPIO_PIN_4
#define GPIO_SIGNAL_B5_PIN                                  GPIO_PIN_5
#define GPIO_SIGNAL_B6_PIN                                  GPIO_PIN_6
#define GPIO_SIGNAL_B7_PIN                                  GPIO_PIN_7
#define GPIO_SIGNAL_B8_PIN                                  GPIO_PIN_8
#define GPIO_SIGNAL_B9_PIN                                  GPIO_PIN_9
#define GPIO_SIGNAL_B10_PIN                                 GPIO_PIN_10
#define GPIO_SIGNAL_B11_PIN                                 GPIO_PIN_11
#define GPIO_SIGNAL_B12_PIN                                 GPIO_PIN_12
#define GPIO_SIGNAL_B13_PIN                                 GPIO_PIN_13
#define GPIO_SIGNAL_B14_PIN                                 GPIO_PIN_14
#define GPIO_SIGNAL_B15_PIN                                 GPIO_PIN_15

/* pins for 'C' signals */
#define GPIO_SIGNAL_C0_PIN                                  GPIO_PIN_0
#define GPIO_SIGNAL_C1_PIN                                  GPIO_PIN_1
#define GPIO_SIGNAL_C2_PIN                                  GPIO_PIN_2
#define GPIO_SIGNAL_C3_PIN                                  GPIO_PIN_3
#define GPIO_SIGNAL_C4_PIN                                  GPIO_PIN_4
#define GPIO_SIGNAL_C5_PIN                                  GPIO_PIN_5
#define GPIO_SIGNAL_C6_PIN                                  GPIO_PIN_6
#define GPIO_SIGNAL_C7_PIN                                  GPIO_PIN_7
#define GPIO_SIGNAL_C8_PIN                                  GPIO_PIN_8
#define GPIO_SIGNAL_C9_PIN                                  GPIO_PIN_9
#define GPIO_SIGNAL_C10_PIN                                 GPIO_PIN_10
#define GPIO_SIGNAL_C11_PIN                                 GPIO_PIN_11
#define GPIO_SIGNAL_C12_PIN                                 GPIO_PIN_12
#define GPIO_SIGNAL_C13_PIN                                 GPIO_PIN_13
#define GPIO_SIGNAL_C14_PIN                                 GPIO_PIN_14
#define GPIO_SIGNAL_C15_PIN                                 GPIO_PIN_15

/* pins for 'D' signals */
#define GPIO_SIGNAL_D2_PIN                                  GPIO_PIN_2

/* pins for 'H' signals */
#define GPIO_SIGNAL_H0_PIN                                  GPIO_PIN_0
#define GPIO_SIGNAL_H1_PIN                                  GPIO_PIN_1


/* port A signals */
#define GPIO_SIGNAL_A0                          \
    GPIO_SIGNAL(GPIO_SIGNAL_A0_PORT, GPIO_SIGNAL_A0_PIN)
#define GPIO_SIGNAL_A1                          \
    GPIO_SIGNAL(GPIO_SIGNAL_A1_PORT, GPIO_SIGNAL_A1_PIN)
#define GPIO_SIGNAL_A2                          \
    GPIO_SIGNAL(GPIO_SIGNAL_A2_PORT, GPIO_SIGNAL_A2_PIN)
#define GPIO_SIGNAL_A3                          \
    GPIO_SIGNAL(GPIO_SIGNAL_A3_PORT, GPIO_SIGNAL_A3_PIN)
#define GPIO_SIGNAL_A4                          \
    GPIO_SIGNAL(GPIO_SIGNAL_A4_PORT, GPIO_SIGNAL_A4_PIN)
#define GPIO_SIGNAL_A5                          \
    GPIO_SIGNAL(GPIO_SIGNAL_A5_PORT, GPIO_SIGNAL_A5_PIN)
#define GPIO_SIGNAL_A6                          \
    GPIO_SIGNAL(GPIO_SIGNAL_A6_PORT, GPIO_SIGNAL_A6_PIN)
#define GPIO_SIGNAL_A7                          \
    GPIO_SIGNAL(GPIO_SIGNAL_A7_PORT, GPIO_SIGNAL_A7_PIN)
#define GPIO_SIGNAL_A8                          \
    GPIO_SIGNAL(GPIO_SIGNAL_A8_PORT, GPIO_SIGNAL_A8_PIN)
#define GPIO_SIGNAL_A9                          \
    GPIO_SIGNAL(GPIO_SIGNAL_A9_PORT, GPIO_SIGNAL_A9_PIN)
#define GPIO_SIGNAL_A10                         \
    GPIO_SIGNAL(GPIO_SIGNAL_A10_PORT, GPIO_SIGNAL_A10_PIN)
#define GPIO_SIGNAL_A11                         \
    GPIO_SIGNAL(GPIO_SIGNAL_A11_PORT, GPIO_SIGNAL_A11_PIN)
#define GPIO_SIGNAL_A12                         \
    GPIO_SIGNAL(GPIO_SIGNAL_A12_PORT, GPIO_SIGNAL_A12_PIN)
#define GPIO_SIGNAL_A13                         \
    GPIO_SIGNAL(GPIO_SIGNAL_A13_PORT, GPIO_SIGNAL_A13_PIN)
#define GPIO_SIGNAL_A14                         \
    GPIO_SIGNAL(GPIO_SIGNAL_A14_PORT, GPIO_SIGNAL_A14_PIN)
#define GPIO_SIGNAL_A15                         \
    GPIO_SIGNAL(GPIO_SIGNAL_A15_PORT, GPIO_SIGNAL_A15_PIN)


/* port B signals */
#define GPIO_SIGNAL_B0                          \
    GPIO_SIGNAL(GPIO_SIGNAL_B0_PORT, GPIO_SIGNAL_B0_PIN)
#define GPIO_SIGNAL_B1                          \
    GPIO_SIGNAL(GPIO_SIGNAL_B1_PORT, GPIO_SIGNAL_B1_PIN)
#define GPIO_SIGNAL_B2                          \
    GPIO_SIGNAL(GPIO_SIGNAL_B2_PORT, GPIO_SIGNAL_B2_PIN)
#define GPIO_SIGNAL_B3                          \
    GPIO_SIGNAL(GPIO_SIGNAL_B3_PORT, GPIO_SIGNAL_B3_PIN)
#define GPIO_SIGNAL_B4                          \
    GPIO_SIGNAL(GPIO_SIGNAL_B4_PORT, GPIO_SIGNAL_B4_PIN)
#define GPIO_SIGNAL_B5                          \
    GPIO_SIGNAL(GPIO_SIGNAL_B5_PORT, GPIO_SIGNAL_B5_PIN)
#define GPIO_SIGNAL_B6                          \
    GPIO_SIGNAL(GPIO_SIGNAL_B6_PORT, GPIO_SIGNAL_B6_PIN)
#define GPIO_SIGNAL_B7                          \
    GPIO_SIGNAL(GPIO_SIGNAL_B7_PORT, GPIO_SIGNAL_B7_PIN)
#define GPIO_SIGNAL_B8                          \
    GPIO_SIGNAL(GPIO_SIGNAL_B8_PORT, GPIO_SIGNAL_B8_PIN)
#define GPIO_SIGNAL_B9                          \
    GPIO_SIGNAL(GPIO_SIGNAL_B9_PORT, GPIO_SIGNAL_B9_PIN)
#define GPIO_SIGNAL_B10                         \
    GPIO_SIGNAL(GPIO_SIGNAL_B10_PORT, GPIO_SIGNAL_B10_PIN)
#define GPIO_SIGNAL_B11                         \
    GPIO_SIGNAL(GPIO_SIGNAL_B11_PORT, GPIO_SIGNAL_B11_PIN)
#define GPIO_SIGNAL_B12                         \
    GPIO_SIGNAL(GPIO_SIGNAL_B12_PORT, GPIO_SIGNAL_B12_PIN)
#define GPIO_SIGNAL_B13                         \
    GPIO_SIGNAL(GPIO_SIGNAL_B13_PORT, GPIO_SIGNAL_B13_PIN)
#define GPIO_SIGNAL_B14                         \
    GPIO_SIGNAL(GPIO_SIGNAL_B14_PORT, GPIO_SIGNAL_B14_PIN)
#define GPIO_SIGNAL_B15                         \
    GPIO_SIGNAL(GPIO_SIGNAL_B15_PORT, GPIO_SIGNAL_B15_PIN)


/* port C signals */
#define GPIO_SIGNAL_C0                          \
    GPIO_SIGNAL(GPIO_SIGNAL_C0_PORT, GPIO_SIGNAL_C0_PIN)
#define GPIO_SIGNAL_C1                          \
    GPIO_SIGNAL(GPIO_SIGNAL_C1_PORT, GPIO_SIGNAL_C1_PIN)
#define GPIO_SIGNAL_C2                          \
    GPIO_SIGNAL(GPIO_SIGNAL_C2_PORT, GPIO_SIGNAL_C2_PIN)
#define GPIO_SIGNAL_C3                          \
    GPIO_SIGNAL(GPIO_SIGNAL_C3_PORT, GPIO_SIGNAL_C3_PIN)
#define GPIO_SIGNAL_C4                          \
    GPIO_SIGNAL(GPIO_SIGNAL_C4_PORT, GPIO_SIGNAL_C4_PIN)
#define GPIO_SIGNAL_C5                          \
    GPIO_SIGNAL(GPIO_SIGNAL_C5_PORT, GPIO_SIGNAL_C5_PIN)
#define GPIO_SIGNAL_C6                          \
    GPIO_SIGNAL(GPIO_SIGNAL_C6_PORT, GPIO_SIGNAL_C6_PIN)
#define GPIO_SIGNAL_C7                          \
    GPIO_SIGNAL(GPIO_SIGNAL_C7_PORT, GPIO_SIGNAL_C7_PIN)
#define GPIO_SIGNAL_C8                          \
    GPIO_SIGNAL(GPIO_SIGNAL_C8_PORT, GPIO_SIGNAL_C8_PIN)
#define GPIO_SIGNAL_C9                          \
    GPIO_SIGNAL(GPIO_SIGNAL_C9_PORT, GPIO_SIGNAL_C9_PIN)
#define GPIO_SIGNAL_C10                         \
    GPIO_SIGNAL(GPIO_SIGNAL_C10_PORT, GPIO_SIGNAL_C10_PIN)
#define GPIO_SIGNAL_C11                         \
    GPIO_SIGNAL(GPIO_SIGNAL_C11_PORT, GPIO_SIGNAL_C11_PIN)
#define GPIO_SIGNAL_C12                         \
    GPIO_SIGNAL(GPIO_SIGNAL_C12_PORT, GPIO_SIGNAL_C12_PIN)
#define GPIO_SIGNAL_C13                         \
    GPIO_SIGNAL(GPIO_SIGNAL_C13_PORT, GPIO_SIGNAL_C13_PIN)
#define GPIO_SIGNAL_C14                         \
    GPIO_SIGNAL(GPIO_SIGNAL_C14_PORT, GPIO_SIGNAL_C14_PIN)
#define GPIO_SIGNAL_C15                         \
    GPIO_SIGNAL(GPIO_SIGNAL_C15_PORT, GPIO_SIGNAL_C15_PIN)


/* port D signals */
#define GPIO_SIGNAL_D2                          \
    GPIO_SIGNAL(GPIO_SIGNAL_D2_PORT, GPIO_SIGNAL_D2_PIN)

/* port H signals */
#define GPIO_SIGNAL_H0                          \
    GPIO_SIGNAL(GPIO_SIGNAL_H0_PORT, GPIO_SIGNAL_H0_PIN)
#define GPIO_SIGNAL_H1                          \
    GPIO_SIGNAL(GPIO_SIGNAL_H1_PORT, GPIO_SIGNAL_H1_PIN)


#endif /* DEV_GPIO_SIGNALS_STM32_H */
