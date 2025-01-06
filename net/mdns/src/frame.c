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
#include "net/mdns/frame.h"

/* encode domain name in mdns format */
err_t MDNSFrame_EncodeName(const char *name, uint8_t *dst, size_t dst_size)
{
    /* source pointer and the destination pointer */
    const char *s = name; uint8_t *d = dst, *dl;

    /* process as long as */
    while (*s) {
        /* not enough space */
        if (dst_size < 1)
            return EFATAL;
        /* copy data */
        for (dl = d + 1; *s && *s != '.' && (dst_size - 1); dst_size--)
            *dl++ = *s++;
        /* we did not reach the end of the name part OR the name is too long
         * to be encoded OR empty label (consecutive two dots) */
        if ((*s != 0 && *s != '.') || (dl - d > 0x40) || (*s && (dl - d == 1)))
            return EFATAL;
        /* encode the size */
        *d = dl - d - 1; d = dl;
        /* skip over dot */
        if (*s)
            s++;
    }

    /* not enough space for termination */
    if (!dst_size)
        return EFATAL;
    /* terminate the label chain */
    *d++ = 0;
    /* return the number of bytes produced */
    return d - dst;
}

/* function for decoding dns record name (either compressed or plain) */
err_t MDNSFrame_DecodeName(const uint8_t *src, const mdns_frame_t *frame,
    size_t frame_size, char *dst, size_t dst_size)
{
    /* get the offset of the source data within the entire frame */
    size_t offset = (uintptr_t)src - (uintptr_t)frame;

    /* source pointer, source end pointer, source start pointer */
    const uint8_t *s = src, *se = src + frame_size - offset, *ss = src - offset;
    /* destination pointer */
    char *d = dst, *ds = dst, *de = dst + dst_size;
    /*number of bytes consumed */
    size_t bconsumed = 0;

    /* get the number of bytes that are left in the source buffer */
    for (int pointer_was_used = 0; ; ) {
        /* helper variables */
        size_t l_size, l_offs, bleft = se > s ? se - s : 0;
        /* normal encoding */
        if (bleft >= 1 && *s < 0x40) {
            /* not enough bytes in the source or in the destination */
            if (((l_size = *s++) > --bleft) || ((de - d) < l_size + 1))
                return EARGVAL;
            /* store the number of bytes to be consumed */
            if (!pointer_was_used)
                bconsumed += l_size + 1;
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
            l_offs = ((s[0] & 0x3f) << 8) | s[1];
            /* store the number of bytes to be consumed */
            if (!pointer_was_used)
                bconsumed += 2;
            pointer_was_used = 1;
            /* redo the logic */
            s = ss + l_offs;
        /* unsupported size delimiter */
        } else {
            return EFATAL;
        }
    }

    /* zero-terminate */
    d - ds ? (d[-1] = 0) : (*d++ = 0);
    /* return the number of bytes consumed from the record */
    return bconsumed;
}
