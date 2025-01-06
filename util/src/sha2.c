/**
 * @file sha2.c
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
#include "util/sha2.h"
#include "util/string.h"


/* round constants */
static const uint32_t sha2_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/* left-rotate 32 bit word by given number of bits */
static inline uint32_t SHA2_RRot32(uint32_t x, uint32_t bits)
{
    return (x >> bits) | (x << (32 - bits));
}

/* extend the message: 512 bits of input message are getting extened into
 * 80 32-bit words */
static void SHA2_ScheduleMessage(const uint32_t in[16], uint32_t out[64])
{
    uint32_t s0, s1, o2, o15;

    /* first 16 words are just a copy of the input data */
    for (int i = 0; i < 16; i++)
        out[i] = in[i];

    /* subsequent words are some weird combination of the previous words */
    for (int i = 16; i < 64; i++) {
        /* preload previous values */
        o2 = out[i - 2], o15 = out[i - 15];
        s0 = SHA2_RRot32(o15,  7) ^ SHA2_RRot32(o15, 18) ^ (o15 >> 3);
        s1 = SHA2_RRot32(o2 , 17) ^ SHA2_RRot32(o2 , 19) ^ (o2 >> 10);
        /* store the final value */
        out[i] = out[i - 16] + s0 + out[i - 7] + s1;
    }
}

/* do the rounds */
static void SHA2_Rounds(sha2_state_t *s, uint32_t w[64])
{
    /* load the state variables */
    uint32_t a = s->h[0], b = s->h[1], c = s->h[2], d = s->h[3];
    uint32_t e = s->h[4], f = s->h[5], g = s->h[6], h = s->h[7];
    /* temporary variables */
    uint32_t s0, s1, ch, t1, t2, ma;

    /* do the 64 rounds */
    for (int round = 0; round < 64; round++) {
        /* compute nonlinear functions */
        s1 = SHA2_RRot32(e, 6) ^ SHA2_RRot32(e, 11) ^ SHA2_RRot32(e, 25);
        ch = (e & f) ^ ((~e) & g);
        t1 = h + s1 + ch + sha2_k[round] + w[round];

        s0 = SHA2_RRot32(a, 2) ^ SHA2_RRot32(a, 13) ^ SHA2_RRot32(a, 22);
        ma = (a & b) ^ (a & c) ^ (b & c);
        t2 = s0 + ma;

        /* pass the state registers around */
        h = g; g = f; f = e;
        e = d + t1;
        d = c; c = b; b = a;
        a = t1 + t2;
    }

    /* update hash value */
    s->h[0] += a; s->h[1] += b; s->h[2] += c; s->h[3] += d;
    s->h[4] += e; s->h[5] += f; s->h[6] += g; s->h[7] += h;
}

/* prepare the initialize state vector */
err_t SHA2_InitState(sha2_state_t *s, sha2_type_t type)
{
    /* initialiation depends on type */
    switch (type) {
    /* 256 bit hash */
    case SHA2_TYPE_256: {
        *s = (sha2_state_t) {
            .h[0] = 0x6a09e667, .h[1] = 0xbb67ae85, .h[2] = 0x3c6ef372,
            .h[3] = 0xa54ff53a, .h[4] = 0x510e527f, .h[5] = 0x9b05688c,
            .h[6] = 0x1f83d9ab, .h[7] = 0x5be0cd19,
        };
    } break;
    /* 224 bit hash */
    case SHA2_TYPE_224: {
        *s = (sha2_state_t) {
            .h[0] = 0xc1059ed8, .h[1] = 0x367cd507, .h[2] = 0x3070dd17,
            .h[3] = 0xf70e5939, .h[4] = 0xffc00b31, .h[5] = 0x68581511,
            .h[6] = 0x64f98fa7, .h[7] = 0xbefa4fa4
        };
    } break;
    /* unsupported type */
    default: return EARGVAL;
    }
    /* store the type information */
    s->type = type;
    /* return success */
    return EOK;
}

/* compute hash over the buffer */
err_t SHA2_Digest(sha2_state_t *s, int final, const void *ptr, size_t size)
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
        uint32_t w[64]; SHA2_ScheduleMessage(s->w.u32, w);
        /* do the rounds */
        SHA2_Rounds(s, w);

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
err_t SHA2_GetHashVal(sha2_state_t *s, void *ptr, size_t size)
{
    /* number of bytes that the full hash will take */
    size_t byte_cnt;

    /* state is not finalized, so it does not contain the proper hash value */
    if (!s->finalized)
        return EARGVAL;

    /* determine the number of bytes */
    switch (s->type) {
    case SHA2_TYPE_224: byte_cnt = 7 * 4 * 2; break;
    case SHA2_TYPE_256: byte_cnt = 8 * 4 * 2; break;
    default: return EARGVAL;
    }

    /* limit the size */
    size = min(size, byte_cnt);
    /* copy the data */
    memcpy(ptr, s->h, size);

    /* return the number of bytes written */
    return size;
}


/* get the string version of the hash */
err_t SHA2_GetHashStr(sha2_state_t *s, char *str, size_t size)
{
    /* get the byte-wise pointer */
    uint8_t *h = (uint8_t *)s->h; char *sptr;
    /* set of digits */
    static const char digits[] = "0123456789abcdef";
    /* number of digits to be printed */
    size_t char_cnt;

    /* determine the number of digits */
    switch (s->type) {
    case SHA2_TYPE_224: char_cnt = 7 * 4 * 2; break;
    case SHA2_TYPE_256: char_cnt = 8 * 4 * 2; break;
    default: return EARGVAL;
    }

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