/**
 * @file checksum.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-31
 * 
 * @brief TCP/IP Stack: Inet checksum algorithm
 */

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "compiler.h"
#include "util/endian.h"

/* calculate checksum */
uint16_t OPTIMIZE("Os") TCPIPChecksum_Checksum(uint16_t sum, const void *ptr, size_t size)
{
    /* bytewise pointer */
    const union { uint16_t u16; uint8_t u8[2]; } PACKED *p = ptr;

    /* go over two-byte chunks */
    for (; size >= 2; p++, size -= 2) {
        /* get value */
        uint16_t val = BETOH16(p->u16);
        /* add one on overflows */
        if ((sum += val) < val)
            sum += 1;
    }

    /* handle last byte case */
    if (size) {
        /* construct the value from a single byte */
        uint16_t val = BETOH16(p->u8[0]);
        /* add one on overflows */
        if ((sum += val) < val)
            sum += 1;
    }

    /* return the checksum */
    return sum;
}