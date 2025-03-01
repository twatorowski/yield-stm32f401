/**
 * @file eeprom_dev.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-01
 * 
 * @copyright Copyright (c) 2025
 */

#include "dev/eeprom.h"
#include "dev/swi2c_dev.h"


/* on-board eeprom chip */
eeprom_dev_t eeprom = {
    /* i2c configuration */
    .swi2c = &swi2c_eeprom, .a2a1a0 = 0,
    /* memory size */
    .capacity = 16 * 1024, .page_size = 64,
    /* write protecto pin */
    .wp = GPIO_SIGNAL_C13,
};

/* initialize all the chips */
err_t EEPROMDev_Init(void)
{
    /* initialize on-board chips */
    EEPROM_DevInit(&eeprom);
    /* return status */
    return EOK;
}