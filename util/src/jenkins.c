/**
 * @file jenkins.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-03
 * 
 * @copyright Copyright (c) 2025
 */

#include <stdint.h>
#include <stddef.h>

/* Bob Jenkins One at a Time hash function */
uint32_t Jenkins_OAAT(uint32_t iv, const uint8_t* key, size_t size)
{
    /* hash containing register */
    uint32_t hash = iv;
    /* go through the data */
    for (size_t i = 0; i != size; i++) {
        hash += key[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    /* final shuffle */
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    /* return the result */
    return hash;
}