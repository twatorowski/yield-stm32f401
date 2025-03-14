/**
 * @file string.h
 *
 * @date 28.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief String manipulation functions
 */

#ifndef UTIL_STRING_H
#define UTIL_STRING_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Compare two memory regions
 *
 * @param ptr1 pointer to the first memory region
 * @param ptr2 pointer to the second memory region
 * @param num number of bytes to be compared
 *
 * @return 0 if regions contain the same data, non-0 otherwise
 */
int memcmp(const void *ptr1, const void *ptr2, size_t num);

/**
 * @brief Copy @p num bytes of data from @p src memory region to @p dst region
 *
 * @param dst destination pointer
 * @param src source pointer
 * @param num number of bytes to be copied
 *
 * @return destination pointer
 */
void * memcpy(void *dst, const void *src, size_t num);

/**
 * @brief Set the memory contents (byte-by-byte) of the memory region pointer by
 * @p ptr to value @p value
 *
 * @param ptr pointer to the destination memory region
 * @param value value to be written (byte)
 * @param num number of bytes to be set to @p value
 *
 * @return ptr address
 */
void * memset(void *ptr, int value, size_t num);

/**
 * @brief Get the length of the '\0'-terminated string
 *
 * @param ptr pointer to the string
 *
 * @return length of the string (excluding the '\0' termination)
 */
size_t strlen(const char *ptr);

/**
 * @brief Get the length of the '\0'-terminated string that is assumed to be no
 * longer than @p max_len bytes
 *
 * @param ptr pointer to the string
 * @param max_len maximal assumed length of string
 *
 * @return length of the string (excluding the '\0' termination) or max_len whichever
 * is smaller
 */
size_t strnlen(const char *ptr, size_t max_len);

/**
 * @brief Compare two '\0'-terminated strings.
 *
 * @param s1 pointer to the 1st string
 * @param s2 pointer to the 2nd string
 *
 * @return the difference of the lastly compared characters, so 0 is returned if both
 * strings are equal. Non-zero values means that the strings differ.
 */
int strcmp(const char *s1, const char *s2);

/**
 * @brief Compare two '\0'-terminated strings. Case insensitive version
 *
 * @param s1 pointer to the 1st string
 * @param s2 pointer to the 2nd string
 *
 * @return the difference of the lastly compared characters, so 0 is returned if both
 * strings are equal. Non-zero values means that the strings differ.
 */
int strcicmp(char const *s1, char const *s2);

/**
 * @brief Compare two '\0'-terminated strings. Case insensitive version. 
 * Size limited
 * 
 * @param s1 pointer to the 1st string
 * @param s2 pointer to the 2nd string
 * @param max_size maximal assumed length of string
 * 
 * @return the difference of the lastly compared characters, so 0 is returned if both
 * strings are equal. Non-zero values means that the strings differ.
 */
int strncicmp(char const *a, char const *b, size_t max_size);

/**
 * @brief Copy @p src string to the @p dst
 *
 * @param dst destination pointer, please ensure that it points to the memory region
 * that is long enough to hold the string pointed by the @p src
 * @param src source string pointer
 *
 * @return pointer to the destination string
 */
char * strcpy(char *dst, const char *src);

/**
 * @brief Copy @p src string to the @p dst. Number of bytes copied is never greater
 * than @p size. If the @p src string is shorter than @p size then the remaining space
 * in @p dst is filled with zeros
 *
 * @param dst destination pointer, please ensure that it points to the memory region
 * that is long enough to hold the string of length @p size
 * @param src source string pointer
 * @param size maximal number of bytes to copy
 *
 * @return pointer to the destination string
 */
char * strncpy(char *dst, const char *src, size_t size);

/**
 * @brief copy strings with limited space ensuring that we zero terminate the
 * destination
 *
 * @param dst destination buffer
 * @param src source string
 * @param size size of the destination buffer in bytes
 *
 * @return size_t length of the copied string
 */
size_t strlcpy(char *dst, const char *src, size_t size);

/**
 * @brief locate the occurence of s2 within s1 string within string
 * 
 * @param s1 string 1
 * @param s2 string 2
 * 
 * @return char* localization of s2 within s1 or null if not found 
 */
char * strstr(const char *s1, const char *s2);


/**
 * @brief locate the occurence of s2 within s1 string within string
 * (case insensitive)
 *
 * @param s1 haystack
 * @param s2 needle
 *
 * @return char * pointer to the haystack if needle is found :)
 */
char * strcistr(const char *s1, const char *s2);

#endif /* UTIL_STRING_H */
