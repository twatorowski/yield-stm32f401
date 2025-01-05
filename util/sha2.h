/**
 * @file sha2.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-12-26
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef UTIL_SHA2_H
#define UTIL_SHA2_H

#include <stdint.h>
#include <stdarg.h>

#include "err.h"

/* type of the algorithm */
typedef enum sha2_type {
    SHA2_TYPE_224,
    SHA2_TYPE_256,
} sha2_type_t;

/* representation of sha2 state */
typedef struct sha2_state {
    /* type of the algorithm */
    sha2_type_t type;
    /* internal state representation */
    uint32_t h[8];
    /* current message byte length */
    uint32_t total_len, w_len;
    /* message buffer represented as byte wise pointer and */
    union {
         uint32_t u32[16]; uint8_t u8[4 * 16];
    } w;
    /* will be set to '1' by calling digest function with final set to '1'
     * this will prevent calling 'final' iteration (that's the one with data
     * padding ) more than once */
    int finalized;
} sha2_state_t;


/**
 * @brief initialize the state vector
 *
 * @param s state vector pointer
 * @param type type of the hashing algorithm
 *
 * @return err_t error code
 */
err_t SHA2_InitState(sha2_state_t *s, sha2_type_t type);

/**
 * @brief compute hash over the buffer
 *
 * @param s state vector
 * @param final is this the final chunk of the data?
 * @param ptr data pointer
 * @param size size of the data
 *
 * @return err_t error code
 */
err_t SHA2_Digest(sha2_state_t *s, int final, const void *ptr, size_t size);

/**
 * @brief return the hash value in binary format
 *
 * @param s state vector
 * @param ptr pointer to where to put the data to
 * @param size size of the buffer
 *
 * @return err_t error code
 */
err_t SHA2_GetHashVal(sha2_state_t *s, void *ptr, size_t size);

/**
 * @brief get the string version of the hash
 *
 * @param s state vector
 * @param str string buffer pointer
 * @param size size of the string buffer pointer
 *
 * @return err_t error code
 */
err_t SHA2_GetHashStr(sha2_state_t *s, char *str, size_t size);


#endif /* UTIL_SHA2_H */
