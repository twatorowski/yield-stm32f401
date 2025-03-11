/**
 * @file seed.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-03
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_SEED_H
#define DEV_SEED_H

#include <stdint.h>

#include "err.h"

/**
 * @brief generate the seed value
 *
 * @return err_t error code
 */
err_t Seed_Init(void);

/**
 * @brief return the seed value
 *
 * @return uint32_t seed value
 */
uint32_t Seed_GetSeed(void);

/**
 * @brief get random value
 *
 * @return uint32_t random value generated based on seed
 */
uint32_t Seed_GetRand(void);

/**
 * @brief return random integer from the range a to b (both included)
 *
 * @param a start of the range
 * @param b end of the range
 * @return int random integer or 0 if a > b.
 */
int Seed_GetRandInt(int a, int b);

#endif /* DEV_SEED_H */
