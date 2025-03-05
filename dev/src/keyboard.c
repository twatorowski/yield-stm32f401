/**
 * @file keyboard.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"

#include "dev/keyboard.h"
#include "dev/gpio_signals.h"


/* key signals */
#define GPIO_KEY_UP                     (gpio_signal_t)GPIO_SIGNAL_B10
#define GPIO_KEY_MID                    (gpio_signal_t)GPIO_SIGNAL_A8
#define GPIO_KEY_LEFT                   (gpio_signal_t)GPIO_SIGNAL_B15
#define GPIO_KEY_RIGHT                  (gpio_signal_t)GPIO_SIGNAL_C8


/* initialize keyboard driver */
err_t Kbd_Init(void)
{

    /* configure as inputs */
    GPIOSig_CfgInput(GPIO_KEY_UP);
    GPIOSig_CfgInput(GPIO_KEY_MID);
    GPIOSig_CfgInput(GPIO_KEY_LEFT);
    GPIOSig_CfgInput(GPIO_KEY_RIGHT);

    /* enable pull downs (in case keyboard pcb is missing )*/
    GPIOSig_CfgPull(GPIO_KEY_UP, GPIO_PULL_DN);
    GPIOSig_CfgPull(GPIO_KEY_MID, GPIO_PULL_DN);
    GPIOSig_CfgPull(GPIO_KEY_LEFT, GPIO_PULL_DN);
    GPIOSig_CfgPull(GPIO_KEY_RIGHT, GPIO_PULL_DN);

    /* return error code */
    return EOK;
}

/* get current state of the keyboard */
kbd_mask_t Kbd_GetState(void)
{
    /* keyboard bit mask */
    kbd_mask_t mask = 0;
    /* read the state of the buttons */
    if (GPIOSig_Get(GPIO_KEY_UP)) mask |= KBD_MASK_UP;
    if (GPIOSig_Get(GPIO_KEY_MID)) mask |= KBD_MASK_MID;
    if (GPIOSig_Get(GPIO_KEY_LEFT)) mask |= KBD_MASK_LEFT;
    if (GPIOSig_Get(GPIO_KEY_RIGHT)) mask |= KBD_MASK_RIGHT;

    /* return the mask */
    return mask;
}