/**
 * @file sha1.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-12-26
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef UTIL_SHA1_H
#define UTIL_SHA1_H


/* representation of sha1 state */
typedef struct sha1_state {
    /* internal state representation */
    uint32_t h[5];
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
} sha1_state_t;

/* prepare the initialize state vector */

/**
 * @brief initialize the state vector for the sha1 algorithm
 *
 * @param s sha1 state vector pointer
 */
void SHA1_InitState(sha1_state_t *s);

/**
 * @brief compute hash over the buffer
 *
 * @param s state vector
 * @param final is this the final chunk of the data?
 * @param ptr data pointer
 * @param size data size
 *
 * @return err_t error code
 */
err_t SHA1_Digest(sha1_state_t *s, int final, const void *ptr, size_t size);

/**
 * @brief return the hash value as the binary data
 *
 * @param s state vector
 * @param ptr pointer to where to put the hash value to
 * @param size size of the buffer
 *
 * @return err_t error ccode
 */
err_t SHA1_GetHashVal(sha1_state_t *s, void *ptr, size_t size);

/**
 * @brief get the hass value from the state vector in string format
 *
 * @param s state vector
 * @param str string to store the data to
 * @param size max size of the data to store
 *
 * @return err_t
 */
err_t SHA1_GetHashStr(sha1_state_t *s, char *str, size_t size);


#endif /* UTIL_SHA1_H */
