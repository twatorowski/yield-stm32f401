/**
 * @file charger.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "dev/charger.h"
#include "dev/gpio_signals.h"

/* charger pin definitions */
/* charging current setting */
#define GPIO_CURRENT_LO                 (gpio_signal_t)GPIO_SIGNAL_A6
#define GPIO_CURRENT_MID                (gpio_signal_t)GPIO_SIGNAL_A5
#define GPIO_CURRENT_HI                 (gpio_signal_t)GPIO_SIGNAL_A4
/* charging enable */
#define GPIO_EN                         (gpio_signal_t)GPIO_SIGNAL_A3
/* charging status */
#define GPIO_STAT                       (gpio_signal_t)GPIO_SIGNAL_A7


/* initialize the charger control */
err_t Charger_Init(void)
{
    /* setup pins that control the charging current */
    GPIOSig_CfgOutput(GPIO_CURRENT_LO, GPIO_OTYPE_OD, 1);
    GPIOSig_CfgOutput(GPIO_CURRENT_MID, GPIO_OTYPE_OD, 1);
    GPIOSig_CfgOutput(GPIO_CURRENT_HI, GPIO_OTYPE_OD, 1);

    /* setup pin that control the charging enable/disable */
    GPIOSig_CfgOutput(GPIO_EN, GPIO_OTYPE_PP, 0);

    /* setup charging status pin */
    GPIOSig_CfgInput(GPIO_STAT);
    GPIOSig_CfgPull(GPIO_STAT, GPIO_PULL_UP);

    /* return the status */
    return EOK;
}

/* enable/disable charger */
err_t Charger_Enable(int enable)
{
    /* drive the control pin */
    GPIOSig_Set(GPIO_EN, enable ? 1 : 0);
    /* not much can go wrong here */
    return EOK;
}

/* set the charging current */
err_t Charger_SetChargingCurrent(charger_current_t current)
{
    /* this will store the state of the pins */
    uint32_t bits = 0;

    /* decode enum into bit flags */
    switch (current) {
    case CHARGER_CURRENT_182MA: bits = 0; break;
    case CHARGER_CURRENT_515MA: bits = 1; break;
    case CHARGER_CURRENT_770MA: bits = 2; break;
    case CHARGER_CURRENT_1103MA: bits = 3; break;
    case CHARGER_CURRENT_1394MA: bits = 4; break;
    case CHARGER_CURRENT_1727MA: bits = 5; break;
    case CHARGER_CURRENT_1982MA: bits = 6; break;
    case CHARGER_CURRENT_2316MA: bits = 7; break;
    default: return EARGVAL;
    }

    /* set the pins */
    GPIOSig_Set(GPIO_CURRENT_LO,  !(bits & 0x1));
    GPIOSig_Set(GPIO_CURRENT_MID, !(bits & 0x2));
    GPIOSig_Set(GPIO_CURRENT_HI,  !(bits & 0x4));

    /* not much can go wrong here */
    return EOK;
}

/* is the charger charging? */
int Charger_IsCharging(void)
{
    /* if stat is pulled low then the chip is charging the battery */
    return GPIOSig_Get(GPIO_STAT) ? 0 : 1;
}