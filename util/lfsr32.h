/**
 * @file lfsr32.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-03
 *
 * @copyright Copyright (c) 2025
 */

#ifndef UTIL_LFSR32_H
#define UTIL_LFSR32_H

#include <stdint.h>

/**
 * @brief get the next value from the lfsr with taps: (32, 22, 2, 1, 0)
 *
 * @param x current value
 * @return uint32_t next value
 */
uint32_t LFSR32_Next(uint32_t x);


#endif /* UTIL_LFSR32_H */
