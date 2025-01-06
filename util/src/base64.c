/**
 * @file base64.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-12-26
 * 
 * @copyright Copyright (c) 2024
 */


#include <stddef.h>
#include <stdint.h>

#include "err.h"

/* encoding */
static const uint8_t enc[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

/* decoder array (offset by '+') */
static const uint8_t dec[] = {
	62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64,
	64, 64, 0,  64, 64, 64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	64, 64, 64, 64, 64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
	36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
};

/* encode with base64 */
err_t Base64_Encode(const void *in, size_t in_size, void *out, size_t out_size)
{
	/* pointers */
	const uint8_t *inp = in; uint8_t *outp = out;
	/* packing register */
	register uint32_t value;

	/* unable to encode due to the size limit */
	if (out_size < 4 * (in_size + 2) / 3)
		return EFATAL;

	/* encode 3 bytes at once and produce 4 encoded characters */
	for (; in_size >= 3; inp += 3, in_size -= 3) {
		/* pack value */
		value = inp[0] << 16 | inp[1] << 8 | inp[2] << 0;
		/* encode */
		*outp++ = enc[(value >> 18) & 0x3f];
		*outp++ = enc[(value >> 12) & 0x3f];
		*outp++ = enc[(value >>  6) & 0x3f];
		*outp++ = enc[(value >>  0) & 0x3f];
	}

	/* got remaining data to pack? */
	if (in_size) {
		/* last remaining bits */
		value  = inp[0] << 16;
		/* at most two bytes are remaining */
		if (in_size >= 2)
			value |= inp[1] << 8;

		/* at least two charactes must be built */
		*outp++ = enc[(value >> 18) & 0x3f];
		*outp++ = enc[(value >> 12) & 0x3f];
		/* trailing '=' go here */
		*outp++ = in_size >= 2 ? enc[(value >> 6) & 0x3f] : '=';
		*outp++ = '=';
	}

	/* return encoded data size */
	return outp - (uint8_t *)out;
}

/* decode base 64-string */
err_t Base64_Decode(const void *in, size_t in_size, void *out, size_t out_size)
{
	/* pointers */
	const uint8_t *inp = in; uint8_t *outp = out;
	/* packing register */
	register uint32_t value;

	/* invalid input size */
	if (in_size & 0x3)
		return EFATAL;

	/* number of trailing '=' determines the size of the last block */
	size_t last_block_size = 3;
	/* '=' is a trailing character */
	if (*(inp + in_size - 1) == '=') last_block_size--;
	if (*(inp + in_size - 2) == '=') last_block_size--;
	/* running low on output data space? */
	if (out_size < in_size * 3 / 4 - (3 - last_block_size))
		return EFATAL;

	/* process whole blocks */
	for (; in_size > 4; in_size -= 4) {
		/* decode */
		value  = dec[*inp++ - '+'] << 18;
		value |= dec[*inp++ - '+'] << 12;
		value |= dec[*inp++ - '+'] <<  6;
		value |= dec[*inp++ - '+'] <<  0;
		/* unpack */
		*outp++ = value >> 16;
		*outp++ = value >>	8;
		*outp++ = value >>	0;
	}

	/* got some trailing chars? */
	if (in_size) {
		/* decode */
		value  = dec[*inp++ - '+'] << 18;
		value |= dec[*inp++ - '+'] << 12;
		value |= dec[*inp++ - '+'] <<  6;
		value |= dec[*inp++ - '+'] <<  0;

		/* there is always at least one complete byte */
		*outp++ = value >> 16;
		/* '=' are automatically decodes as zeors */
		if (last_block_size > 1) *outp++ = value >> 8;
		if (last_block_size > 2) *outp++ = value >> 0;
	}

	/* report the number of bytes */
	return outp - (uint8_t *)out;
}