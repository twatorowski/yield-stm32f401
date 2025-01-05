/**
 * @file lfsr32.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-03
 * 
 * @copyright Copyright (c) 2025
 */

#include "util/lfsr32.h"

/* get the next value from the register taps: (32, 22, 2, 1, 0)*/
uint32_t LFSR32_Next(uint32_t x)
{
    /* get the bit generated by the taps */
    uint32_t bit = ((x >> 0) ^ (x >> 10) ^ (x >> 30) ^ (x >> 31)) & 1;
    /* push the data around */
    return ((x >> 1) | (bit << 31));
}