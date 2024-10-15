/**
 * @file reset.h
 * 
 * @date 2019-11-13
 * twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief Routine to reset the MCU
 */
#ifndef RESET_H
#define RESET_H

#include "compiler.h"

/**
 * @brief Resets the MCU
 */
void NORETURN Reset_ResetMCU(void);

#endif /* RESET_H */
