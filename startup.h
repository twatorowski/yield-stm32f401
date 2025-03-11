/**
 * @file startup.h
 *
 * @date 2019-09-19
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief MCU startup routines. This is where the story starts
 */

#ifndef STARTUP_H
#define STARTUP_H

#include <stdint.h>

/**
 * @brief first function to be executed after the reset. Shall initialize the
 * chip to it's default state
 */
void Startup_ResetHandler(void);

/**
 * @brief reset the mcu and execute application located at given address
 *
 * @param addr application address
 */
void Startup_ResetAndJump(uint32_t addr);

#endif /* STARTUP_H */
