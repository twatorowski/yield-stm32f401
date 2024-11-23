/**
 * @file frame.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-23
 * 
 * @copyright Copyright (c) 2024
 */

#include <stdint.h>
#include <stddef.h>

#include "err.h"

/* function for decoding dns record name (either compressed or plain) */
err_t MDNSFrame_DecodeName(const uint8_t *src, size_t offset, size_t size,
    uint8_t *dst, size_t dst_size)
{
    /* source pointer, source end pointer, source start pointer */
    const uint8_t *s = src, *se = src + size - offset, *ss = src - offset;
    /* destination pointer */
    uint8_t *d = dst, *ds = dst, *de = dst + dst_size;

    /* get the number of bytes that are left in the source buffer */
    while (1) {
        /* helper variables */
        size_t l_size, l_offs, bleft = se > s ? se - s : 0;
        /* normal encoding */
        if (bleft >= 1 && *s < 0x40) {
            /* not enough bytes in the source or in the destination */
            if (((l_size = *s++) > --bleft) || ((de - d) < l_size + 1))
                return EARGVAL;
            /* end of processing */
            if (l_size == 0)
                break;
            /* copy the label */
            for (size_t i = l_size; i > 0 && i--;)
                *d++ = *s++;
            /* append the separator */
            *d++ = '.';
        /* pointer */
        } else if (bleft >= 2 && *s >= 0xc0) {
            /* get the offset */
            l_offs = ((*s++ & 0x3f) << 8) | *s;
            /* redo the logic */
            s = ss + l_offs;
        /* unsupported size delimiter */
        } else {
            return EFATAL;
        }
    }

    /* zero-terminate */
    d - ds ? (d[-1] = 0) : (*d++ = 0);
    /* return the number of bytes written */
    return d - ds;
}
