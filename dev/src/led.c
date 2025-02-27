/**
 * @file led.c
 *
 * @date 2020-03-12
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief On-Board LED Driver
 */

#include "err.h"
#include "dev/gpio.h"
#include "dev/led.h"
#include "sys/critical.h"
#include "dev/gpio_signals.h"

/* initialize led driver */
int Led_Init(void)
{
    /* enter critcal section */
    Critical_Enter();

    /* setup signal */
    GPIO_CfgOutput(GPIOC, GPIO_PIN_11, GPIO_OTYPE_OD, 1);

    /* exti critical section */
    Critical_Exit();
    /* report status */
    return EOK;
}
