/**
 * @file sha1.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-12-26
 *
 * @copyright Copyright (c) 2024
 */

#include <stdint.h>
#include <stddef.h>

#include "assert.h"
#include "err.h"

#include "util/endian.h"
#include "util/elems.h"
#include "util/minmax.h"
#include "util/sha1.h"
#include "util/string.h"

/* left-rotate 32 bit word by given number of bits */
static inline uint32_t SHA1_LRot32(uint32_t x, uint32_t bits)
{
    return (x << bits) | (x >> (32 - bits));
}

/* return the result of applying non-linear function to the state vector */
static inline uint32_t SHA1_F(int round, uint32_t b, uint32_t c, uint32_t d)
{
    /* non-linear function depends on the round number */
    if (round <= 19) return (b & c) | ((~b) & d);
    if (round <= 39) return (b ^ c ^ d);
    if (round <= 59) return (b & c) | (b & d) | (c & d);
    if (round <= 79) return (b ^ c ^ d);
    /* wtf? */
    assert(0, "invalid round number");
}

/* extend the message: 512 bits of input message are getting extened into
 * 80 32-bit words */
static void SHA1_ScheduleMessage(const uint32_t in[16], uint32_t out[80])
{
    /* first 16 words are just a copy of the input data */
    for (int i = 0; i < 16; i++)
        out[i] = in[i];
    /* subsequent words are some weird combination of the previous words */
    for (int i = 16; i < 80; i++)
        out[i] = SHA1_LRot32((out[i-3] ^ out[i-8] ^ out[i-14] ^ out[i-16]), 1);
}

/* do the rounds */
static void SHA1_Rounds(sha1_state_t *s, uint32_t w[80])
{
    /* copy of the state variables */
    uint32_t a = s->h[0], b = s->h[1], c = s->h[2], d = s->h[3], e = s->h[4];

    /* do the 80 rounds */
    for (int round = 0; round < 80; round++) {
        /* K_t */
        uint32_t k;

        /* get the K_t value for given round */
        if (round <= 19) k = 0x5A827999;
        else if (round <= 39) k = 0x6ED9EBA1;
        else if (round <= 59) k = 0x8F1BBCDC;
        else  k = 0xCA62C1D6;

        /* compute the result of non-linear function */
        uint32_t f = SHA1_F(round, b, c, d);

        /* calculate the temporary value */
        uint32_t t = SHA1_LRot32(a, 5) + f + e + k + w[round];
        /* update state */
        e = d;
        d = c;
        c = SHA1_LRot32(b, 30);
        b = a;
        a = t;
    }

    /* update the output vector */
    s->h[0] += a;
    s->h[1] += b;
    s->h[2] += c;
    s->h[3] += d;
    s->h[4] += e;
}

/* prepare the initialize state vector */
void SHA1_InitState(sha1_state_t *s)
{
    /* return the initialized state vector */
    *s = (sha1_state_t) {
        /* initialization constants */
        .h[0] = 0x67452301, .h[1] = 0xEFCDAB89,
        .h[2] = 0x98BADCFE, .h[3] = 0x10325476,
        .h[4] = 0xC3D2E1F0
    };
}


/* compute hash over the buffer */
err_t SHA1_Digest(sha1_state_t *s, int final, const void *ptr, size_t size)
{
    /* data pointer and the number of bytes that remain */
    const uint8_t *p = ptr; size_t size_rem = size;
    /* is the trailing bit already added? */
    int trailing_bit_added = 0, data_size_added = 0;

    /* we shall not do any more digesting on this state vector since it was
     * already finalized */
    if (s->finalized)
        return EARGVAL;

    /* update the size of the entire message */
    s->total_len += size;
    /* this will mark the data as finalized if the final flag is set */
    s->finalized = final;

    /* loop until you process all of the data that can fill up the
     * internal buffer */
    for (; !data_size_added; ) {
        /* bytes to copy in current iteration */
        size_t b_copy = min(sizeof(s->w.u8) - s->w_len, size_rem);
        /* bytes that will remain as free */
        size_t b_left = sizeof(s->w.u8) - s->w_len - b_copy;

        /* anything to copy to our internal buffer */
        if (b_copy) {
            /* we copy to the internal buffer to ensure that data is 32-bit
             * aligned for the rest of the processing */
            memcpy(s->w.u8 + s->w_len, p, b_copy);
            /* update the pointers */
            p += b_copy, size_rem -= b_copy; s->w_len += b_copy;
        }

        /* internal buffer is nor full and this is not the last data chunk that
         * we work with, so prevent further execution */
        if (b_left && !final)
            return EOK;

        /* not entire buffer was filled */
        if (b_left) {
            /* zero out the rest of the bytes */
            memset(s->w.u8 + s->w_len, 0, b_left);
            /* store the trailing '1' */
            if (!trailing_bit_added)
                s->w.u8[s->w_len] = 0x80, trailing_bit_added = 1;
        }

        /* do we have the space for endoding the number of bits? (here we also
         * take into account the byte containing the trailing bit hence we
         * use '>' instead of '>=') */
        if (b_left > 8) {
            /* compute the number of bits */
            uint64_t total_bits = (uint64_t)s->total_len * 8;
            /* if so, then encode them as big endian 64-bit number */
            s->w.u32[14] = HTOBE32(total_bits >> 32);
            s->w.u32[15] = HTOBE32(total_bits);
            /* data size field was added */
            data_size_added = 1;
        }

        /* do the endianness */
        for (int i = 0; i < elems(s->w.u32); i++)
            s->w.u32[i] = HTOBE32(s->w.u32[i]);

        /* schedule the message words for the upcoming rounds of sha1
         * algorithm */
        uint32_t w[80]; SHA1_ScheduleMessage(s->w.u32, w);
        /* do the rounds */
        SHA1_Rounds(s, w);

        /* buffer is now processed, we can release it */
        s->w_len = 0;
    }

    /* this part of the code is only reached when final bit is set.Bring the
     * state vector to the local endiannes */
    for (int i = 0; i < elems(s->h); i++)
        s->h[i] = BETOH32(s->h[i]);

    /* return status */
    return EOK;
}


/* return the hash value */
err_t SHA1_GetHashVal(sha1_state_t *s, void *ptr, size_t size)
{
    /* state is not finalized, so it does not contain the proper hash value */
    if (!s->finalized)
        return EARGVAL;
    /* limit the size */
    size = min(size, sizeof(s->h));
    /* copy the data */
    memcpy(ptr, s->h, size);

    /* return the number of bytes written */
    return size;
}

/* get the string version of the hash */
err_t SHA1_GetHashStr(sha1_state_t *s, char *str, size_t size)
{
    /* get the byte-wise pointer */
    uint8_t *h = (uint8_t *)s->h; char *sptr;
    /* set of digits */
    static const char digits[] = "0123456789abcdef";
    /* number of digits to be printed */
    size_t char_cnt = 5 * 4 * 2;

    /* hashing was not finalized */
    if (!s->finalized)
        return EARGVAL;

    /* sanitize the size field */
    if (!size) {
        return 0;
    /* limit the size that we are going to use */
    } else if (size > char_cnt + 1) {
        size = char_cnt + 1;
    }

    /* render the string */
    for (sptr = str; size > 1; size--, sptr++)
        *sptr = (sptr - str) % 2 ? digits[*(h++) & 0xf] : digits[*h >> 4];

    /* zero terminate */
    *sptr++ = 0;
    /* return the number of bytes that we produced */
    return sptr - str;
}