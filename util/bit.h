
/**
 * @file bit.h
 * 
 * @date 2020-07-10
 * twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief Bit operation macros, use with care
 */

#ifndef UTIL_BIT_H
#define UTIL_BIT_H

#include "util/msblsb.h"

/** @brief returns value of bit placed at position 'pos' */
#define BIT_VAL(pos)                                        \
    (1UL << (pos))

/** @brief sets the bit at given position */
#define BIT_SET(x, pos)                                     \
    ((x) |  (1 << (pos)))

/** @brief clears the bit at given position */
#define BIT_CLR(x, pos)                                     \
    ((x) & ~(1 << (pos)))

/** @brief writes the bit of value 'value' at position 'pos' */
#define BIT_WR(x, pos, value)                               \
    ((value) ? BIT_SET((x), (pos)) : BIT_CLR((x), (pos)))

/** @brief returns 0 if bit at position 'pos' is not set, 1 otherwise. */
#define BIT_RD(x, pos)                                      \
    (!!(x & (1 << (pos))))


/** @brief returns value that results from placing 'value' at bits represented 
 * by 'pos_mask' starting from lsb of pos_mask */
#define BITS_VAL(pos_mask, value)                           \
    (((val) << LSB(pos_mask)) & (pos_mask))

/** @brief write value at area represented by the position mask */
#define BITS_WR(x, pos_mask, value)                         \
    (((x) & ~(pos_mask)) | (((value) << LSB(pos_mask)) & (pos_mask)))

/** @brief read value from bits represented by the mask */
#define BITS_RD(x, pos_mask)                                \
    (((x) & (pos_mask)) >> LSB(pos_mask))

/** @brief clear bits in masked area */
#define BITS_CLR(x, pos_mask)                               \
    ((x) & ~(pos_mask))

/** @brief set bits in masked area */
#define BITS_SET(x, pos_mask)                               \
    ((x) | (pos_mask))

#endif /* UTIL_BIT_H */
