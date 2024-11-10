/**
 * @file gpio.h
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-05-06
 *
 * @brief General Purpose Input-Output
 */
#ifndef DEV_GPIO_H
#define DEV_GPIO_H

#include "compiler.h"
#include "err.h"
#include "stm32f401/gpio.h"

/** gpio pin enumeration */
typedef enum gpio_pin {
    GPIO_PIN_0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
} gpio_pin_t;

/** pull up / pull down bit masks */
typedef enum gpio_pull {
    GPIO_PULL_UP = GPIO_PUPDR_PUPD0_0,
    GPIO_PULL_DN = GPIO_PUPDR_PUPD0_1,
} gpio_pull_t;

/** output type - push-pull, open drain */
typedef enum gpio_otype {
    GPIO_OTYPE_PP = 0,
    GPIO_OTYPE_OD = GPIO_OTYPER_OT0,
} gpio_otype_t;

/** gpio speed settings */
typedef enum gpio_ospeed {
    GPIO_OSPEED_LOW = 0,
    GPIO_OSPEED_MED = GPIO_OSPEEDR_OSPEED0_0,
    GPIO_OSPEED_HIGH = GPIO_OSPEEDR_OSPEED0_1,
    GPIO_OSPEED_VERY_HIGH = GPIO_OSPEEDR_OSPEED0,
} gpio_ospeed_t;

/** alternate functions */
typedef enum gpio_af_t {
    GPIO_AF_SYS = 0,
    GPIO_AF_TIM1_TIM2,
    GPIO_AF_TIM3_TIM4_TIM5,
    GPIO_AF_TIM9_TIM10_TIM11,
    GPIO_AF_I2C1_I2C2_I2C3,
    GPIO_AF_SPI1_SPI2_I2S2_SPI3_I2S3_SPI4,
    GPIO_AF_SPI2_I2S2_SPI3_I2S3,
    GPIO_AF_SPI3_I2S3_USART1_USART2,
    GPIO_AF_USART6,
    GPIO_AF_I2C2_I2C3,
    GPIO_AF_OTG1_FS,
    GPIO_AF_SDIO = 12,
    GPIO_AF_EVNOUT = 15,
} gpio_af_t;

/**
 * @brief Initialize gpio driver
 *
 * @return err_t status code
 */
err_t GPIO_Init(void);

/**
 * @brief Configure given pin in gpio as output and set it's initial state
 *
 * @param gpio gpio port
 * @param pin pin number
 * @param otype output type
 * @param initial_value initial output value
 *
 * @return err_t status
 */
err_t GPIO_CfgOutput(gpio_t *gpio, gpio_pin_t pin, gpio_otype_t otype,
    int initial_value);

/**
 * @brief configure the output type
 *
 * @param gpio gpio port
 * @param pin pin number
 * @param otype output type
 *
 * @return err_t error code
 */
err_t GPIO_CfgOutputType(gpio_t *gpio, gpio_pin_t pin, gpio_otype_t otype);

/**
 * @brief configure the output speed
 *
 * @param gpio gpio
 * @param pin pin number
 * @param speed speed
 *
 * @return err_t error code
 */
err_t GPIO_CfgOutputSpeed(gpio_t *gpio, gpio_pin_t pin, gpio_ospeed_t speed);


/**
 * @brief Configure given pin as input
 *
 * @param gpio gpio port
 * @param pin pin number
 *
 * @return err_t status code
 */
err_t GPIO_CfgInput(gpio_t *gpio, gpio_pin_t pin);

/**
 * @brief Setup pull resistor type on given pin
 *
 * @param gpio gpio port
 * @param pin pin number
 * @param pull pull up/dn configuration
 *
 * @return err_t status code
 */
err_t GPIO_CfgPull(gpio_t *gpio, gpio_pin_t pin, gpio_pull_t pull);

/**
 * @brief configure given pin as analog input
 *
 * @param gpio gpio port
 * @param pin pin number
 *
 * @return err_t status code
 */
err_t GPIO_CfgAnalog(gpio_t *gpio, gpio_pin_t pin);

/**
 * @brief configure given pin as alternate function
 *
 * @param gpio gpio port
 * @param pin pin number
 * @param af alternate function
 *
 * @return err_t status code
 */
err_t GPIO_CfgAltFunction(gpio_t *gpio, gpio_pin_t pin, gpio_af_t af);

/**
 * @brief Set pin state
 *
 * @param gpio gpio port
 * @param pin pin number
 * @param value pin value
 */
static inline ALWAYS_INLINE void GPIO_Set(gpio_t *gpio, gpio_pin_t pin,
    int value)
{
    /* write to bsrr register in order to get the pin state */
    gpio->BSRR = (value ? GPIO_BSRR_BS0 : GPIO_BSRR_BR0) << pin;
}

/**
 * @brief Get pin state
 *
 * @param gpio gpio port
 * @param pin pin number
 */
static inline ALWAYS_INLINE int GPIO_Get(gpio_t *gpio, gpio_pin_t pin)
{
    /* get the pin state */
    return !!(gpio->IDR & (GPIO_IDR_ID0 << pin));
}


#endif /* DEV_GPIO_H */
