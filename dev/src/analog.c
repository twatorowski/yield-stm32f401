/**
 * @file analog.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-05-08
 * 
 * @brief Analog to digital converter
 */

#include "assert.h"
#include "err.h"
#include "dev/analog.h"
#include "dev/gpio.h"
#include "dev/gpio_signals.h"
#include "stm32f401/rcc.h"
#include "stm32f401/gpio.h"
#include "stm32f401/adc.h"
#include "sys/critical.h"
#include "util/bit.h"
#include "util/msblsb.h"

/* mapping between adc channels and gpio */
static err_t Analog_GetPinForChannel(analog_channel_t ch, gpio_signal_t *gpio)
{
    /* placeholder for the signal */
    gpio_signal_t signal;
    /* switch on channel number */
    switch (ch) {
        /* all known channels */
        case ANALOG_IN0: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOA, GPIO_PIN_0); break;
        case ANALOG_IN1: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOA, GPIO_PIN_1); break;
        case ANALOG_IN2: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOA, GPIO_PIN_2); break;
        case ANALOG_IN3: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOA, GPIO_PIN_3); break;
        case ANALOG_IN4: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOA, GPIO_PIN_4); break;
        case ANALOG_IN5: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOA, GPIO_PIN_5); break;
        case ANALOG_IN6: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOA, GPIO_PIN_6); break;
        case ANALOG_IN7: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOA, GPIO_PIN_7); break;
        case ANALOG_IN8: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOB, GPIO_PIN_0); break;
        case ANALOG_IN9: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOB, GPIO_PIN_1); break;
        case ANALOG_IN10: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOC, GPIO_PIN_0); break;
        case ANALOG_IN11: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOC, GPIO_PIN_1); break;
        case ANALOG_IN12: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOC, GPIO_PIN_2); break;
        case ANALOG_IN13: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOC, GPIO_PIN_3); break;
        case ANALOG_IN14: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOC, GPIO_PIN_4); break;
        case ANALOG_IN15: signal = (gpio_signal_t)GPIO_SIGNAL(GPIOC, GPIO_PIN_5); break;
        /* invalid argument*/
        default: return EFATAL;
    }

    /* store the info */
    if (gpio)
        *gpio = signal;
    /* return the signal */
    return EOK;
}

/* analog to digital converter driver initialization */
err_t Analog_Init(void)
{
    /* enter critical section */
    Critical_Enter();

    /* enable clock for the adc 1 */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    /* select  hclk/23 as the clock */
    ADC_COMMON->CCR = ADC_CCR_ADCPRE;

    /* disable power down */
    ADC1->CR2 |= ADC_CR2_ADON;
    /* wait for the adc to exit power down */
    while (!(ADC1->CR2 & ADC_CR2_ADON));

    /* conversion on single channel */
    ADC1->SQR1 = 0 << LSB(ADC_SQR1_L);


    /* exit critical section */
    Critical_Exit();
    /* report status */
    return EOK;
}

/* configure gpio pin to work in analog mode */
err_t Analog_ConfigureChannel(analog_channel_t channel,
    analog_sampling_time_t sampling_time)
{
    /* get the masking and the value to be written */
    uint32_t mask = 0x7 << ((channel % 10) * 3);
    uint32_t val = sampling_time << ((channel % 10) * 3);
    /* select the register */
    reg32_t *smpr = channel >= 10 ? &ADC1->SMPR1 : &ADC1->SMPR2;

    /* set the value */
    *smpr = (*smpr & ~mask) | val;

    /* return status */
    return EOK;
}

/* configure gpio pin to work in analog mode */
err_t Analog_ConfigureGPIO(analog_channel_t channel)
{
    /* gpio to be configured */
    gpio_signal_t gpio;
    /* is there a pin that is connected to this channel? */
    err_t ec = Analog_GetPinForChannel(channel, &gpio);
    /* return status */
    return ec == EOK ? GPIOSig_CfgAnalog(gpio) : ec;
}

/* perform conversion on given channel */
err_t Analog_Convert(analog_channel_t channel, uint16_t *value)
{
    /* conversion on single channel */
    ADC1->SQR3 = channel << LSB(ADC_SQR3_SQ1);
    /* start the software-triggered conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART;
    /* wait for the end of conversion */
    while (!(ADC1->SR & ADC_SR_EOC));
    
    /* read the value from the data register */
    *value = ADC1->DR;
    /* return status */
    return EOK;
}

/* enable internal temperature sensor and connect it to the adc_in18 */
err_t Analog_EnableTempSensor(int enable)
{
    /* enable the temperature sensor */
    if (enable) {
        ADC_COMMON->CCR |=  ADC_CCR_TSVREFE;
    /* disable temperature sensor */
    } else {
        ADC_COMMON->CCR &= ~ADC_CCR_TSVREFE;
    }

    /* report status */
    return EOK;
}

/* enable internal temperature sensor and connect it to the adc_in18 */
err_t Analog_EnableVBatBridge(int enable)
{
    /* enable the temperature sensor */
    if (enable) {
        ADC_COMMON->CCR |=  ADC_CCR_VBATE;
    /* disable temperature sensor */
    } else {
        ADC_COMMON->CCR &= ~ADC_CCR_VBATE;
    }

    /* report status */
    return EOK;
}