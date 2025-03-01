/**
 * @file eeprom_dev.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-01
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_EEPROM_DEV_H
#define DEV_EEPROM_DEV_H

#include "dev/eeprom.h"

/* eeprom memory chips */
extern eeprom_dev_t eeprom;

/**
 * @brief initialize all the chips
 *
 * @return err_t error code
 */
err_t EEPROMDev_Init(void);


#endif /* DEV_EEPROM_DEV_H */
