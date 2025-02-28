/**
 * @file pumps.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-28
 *
 * @copyright Copyright (c) 2025
 */

#include "config.h"
#include "err.h"

#include "dev/analog.h"
#include "dev/gpio_signals.h"
#include "dev/pumps.h"
#include "stm32f401/rcc.h"
#include "stm32f401/timer.h"

/* pin definitions for the motor drivers */
#define GPIO_AIR_EN1                            (gpio_signal_t)GPIO_SIGNAL_A15
#define GPIO_AIR_EN2                            (gpio_signal_t)GPIO_SIGNAL_C10
#define GPIO_FLUID_EN1                          (gpio_signal_t)GPIO_SIGNAL_C12
#define GPIO_FLUID_EN2                          (gpio_signal_t)GPIO_SIGNAL_B3

/* analog channels */
#define ANALOG_CH_AIR_ISENSE                    ANALOG_IN11
#define ANALOG_CH_FLUID_ISENSE                  ANALOG_IN10


/* initialize pump motor driver support */
err_t Pumps_Init(void)
{
    /* do we need to multiply the peripheral clock by 2? it is the case when
     * peripheral clock is prescaled by RCC */
    int mult = 1;

    /* clock is prescaled */
	if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
        mult = 2;

    /* enable tim2 */
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* 1us per pulse (multiply by two may be needed)*/
    TIM2->PSC = (APB1CLOCK_HZ * mult) / 1000000 - 1;
    /* setup for 1kHz freq (period = 1000 us) */
    TIM2->ARR = 1000 - 1;
    /* reload prescaler */
    TIM2->EGR = TIM_EGR_UG;

    /* setup the compare value */
    TIM2->CCR1 = 0; TIM2->CCR2 = 0;
    /* configure pwm1 mode (channel is active as long as cnt < ccr) */
    TIM2->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 |
        TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
    /* enable channels */
    TIM2->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;

    /* CH1 = pump1 = AIR */
    /* CH2 = pump2 = FLUID */

    /* enable timer */
    TIM2->CR1 = TIM_CR1_CEN;

    /* configure timer output pins */
    GPIOSig_CfgAltFunction(GPIO_AIR_EN1, GPIO_AF_TIM1_TIM2);
    GPIOSig_CfgAltFunction(GPIO_FLUID_EN2, GPIO_AF_TIM1_TIM2);
    /* configure the gpios that drive the second input pin of the driver */
    GPIOSig_CfgOutput(GPIO_AIR_EN2, GPIO_OTYPE_PP, 0);
    GPIOSig_CfgOutput(GPIO_FLUID_EN1, GPIO_OTYPE_PP, 0);

    /* setup analog inputs for the current measurement */
    Analog_ConfigureGPIO(ANALOG_CH_AIR_ISENSE);
    Analog_ConfigureGPIO(ANALOG_CH_FLUID_ISENSE);

    /* configure sampling time */
    Analog_ConfigureChannel(ANALOG_CH_AIR_ISENSE, ANALOG_SMPL_TIME_112);
    Analog_ConfigureChannel(ANALOG_CH_FLUID_ISENSE, ANALOG_SMPL_TIME_112);

    /* report status */
    return EOK;
}

/* set the duty cycle for any given pump */
err_t Pumps_SetPumpDutyCycle(pumps_pump_t pump, pumps_dir_t direction,
    float duty_cycle)
{
    /* gpio enable pin for given pump, and timer's ccr register pointer */
    gpio_signal_t gpio_en; reg32_t *ccr;

    /* sanitize the input */
    if (duty_cycle < 0 || duty_cycle > 1)
        return EARGVAL;
    /* unknown direction */
    if (direction != PUMPS_DIR_FWD && direction != PUMPS_DIR_BACK)
        return EARGVAL;

    /* switch on the pump */
    switch (pump) {
    /* get the control registers for the fluid pump */
    case PUMPS_PUMP_FLUID: {
        gpio_en = GPIO_FLUID_EN1; ccr = &TIM2->CCR2;
        /* fluid pump has inverted controls: EN2 is controlled by timer, hence
         * the need to invert the direction of rotation */
        direction = direction == PUMPS_DIR_BACK ?
            PUMPS_DIR_FWD : PUMPS_DIR_BACK;
    } break;
    /* get the control registers for the air pump */
    case PUMPS_PUMP_AIR: {
        gpio_en = GPIO_AIR_EN2; ccr = &TIM2->CCR1;
    } break;
    /* unknown pump */
    default: return EFATAL;
    }

    /* compute the timer compare value based on the duty cycle */
    uint16_t ccr_val = TIM2->ARR * duty_cycle;
    /* reverse direction requires inverted pulse generation */
    if (direction == PUMPS_DIR_BACK)
        ccr_val = TIM2->ARR - ccr_val;

    /* reconfigure the time and the gpio signal */
    *ccr = ccr_val; GPIOSig_Set(gpio_en, direction == PUMPS_DIR_BACK);

    /* return status */
    return EOK;
}

/* get the current drawn by the pump motor */
err_t Pumps_GetCurrentDraw(pumps_pump_t pump, float *current_a)
{
    /* conversion value */
    uint16_t adc_val; uint32_t adc_acc = 0, smpl_cnt = 0;
    analog_channel_t channel;

    /* switch on the pump */
    switch (pump) {
    case PUMPS_PUMP_FLUID: channel = ANALOG_CH_FLUID_ISENSE; break;
    case PUMPS_PUMP_AIR: channel = ANALOG_CH_AIR_ISENSE; break;
    /* unknown pump */
    default: return EFATAL;
    }

    /* we measure the current by sampling the adc during 3 timer cycles
     * looking for the maximal value */
    for (int tim_cnt = TIM2->CNT, timer_updates = 0; timer_updates < 10; ) {
        /* do the conversion */
        Analog_Convert(channel, &adc_val);
        /* sum up the current */
        adc_acc += adc_val; smpl_cnt++;

        /* timer wrapped around */
        if (TIM2->CNT < tim_cnt)
            timer_updates += 1;
        /* store the counter value */
        tim_cnt = TIM2->CNT;

    }

    /* convert the readout to mV */
    float mv = 3000.f * adc_acc / ANALOG_MAX_VAL / smpl_cnt;
    /* convert to current flowing through IPROPI resistor (2.2k). Here we
     * divide millivolts by kiloohms resulting in microamps */
    float iprop_ua = mv / 2.2f;
    /* compute the motor current */
    float motor_current = iprop_ua / 205;
    /* adc cannot register currents near 0, so treat everything
     * < 10mA as motor off */
    if (motor_current < 0.01f)
        motor_current = 0;

    /* chip gain is set to be 205uA per 1A flowing through the motor */
    if (current_a) *current_a = motor_current;

    /* return the status */
    return EOK;
}