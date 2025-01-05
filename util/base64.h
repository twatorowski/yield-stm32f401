/**
 * @file base64.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-12-26
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef UTIL_BASE64_H
#define UTIL_BASE64_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief encode buffer with base64
 *
 * @param in source data
 * @param in_size source data size
 * @param out output buffer
 * @param out_size output buffer size
 *
 * @return int number of bytes produced in the output buffer
 */
err_t Base64_Encode(const void *in, size_t in_size, void *out, size_t out_size);

/**
 * @brief decode base 64-string
 *
 * @param in string to be decoded
 * @param in_size
 * @param out
 * @param out_size
 * @return int
 */
err_t Base64_Decode(const void *in, size_t in_size, void *out, size_t out_size);


#endif /* UTIL_BASE64_H */
