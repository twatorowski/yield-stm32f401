/**
 * @file string.c
 *
 * @date 28.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief String manipulation functions
 */


#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "util/stdio.h"

/* memory compare */
int OPTIMIZE("Os") memcmp(const void *ptr1, const void *ptr2, size_t num)
{
    const uint8_t *p1 = ptr1;
    const uint8_t *p2 = ptr2;

    /* memory compare */
    while (num > 0) {
        /* greater than */
        if (*p1 > *p2) {
            return 1;
        /* less than */
        } else if (*p1 < *p2) {
            return -1;
        }
        /* next byte */
        p1++; p2++; num--;
    }

    /* all equal */
    return 0;
}

/* memory area copy */
void * OPTIMIZE("03") memcpy(void * restrict dst, const void * restrict src, 
    size_t size)
{
    /* pointers */
    uint8_t *d = dst; const uint8_t *s = src;
    /* meet the famous Duff's device! */
    register size_t n = (size + 7) / 8;
    /* 0 size given = 0 fucks given */
    if (!size)
        return dst;

    /* switch on the offset */
    switch (size % 8) {
    case 0:	do {    *d++ = *s++;
    case 7:	        *d++ = *s++;
    case 6:         *d++ = *s++;
    case 5:         *d++ = *s++;
    case 4:		    *d++ = *s++;
    case 3:		    *d++ = *s++;
    case 2:		    *d++ = *s++;
    case 1:		    *d++ = *s++; } while (--n > 0);
    }

    /* return destination */
    return dst;
}

/* memory area set */
void * OPTIMIZE("Os") memset(void *ptr, int value, size_t size)
{
    /* pointers */
    uint8_t *p = ptr;

    /* fill memory */
    while (size-- > 0)
        *(p++) = value;

    /* return pointer */
    return ptr;
}

/* return string length */
size_t OPTIMIZE("Os") strlen(const char *ptr)
{
    /* data pointer */
    const char *p = ptr;

    /* loop till end of string is found */
    while (*p)
        p++;

    return p - ptr;
}

/* return string length */
size_t OPTIMIZE("Os") strnlen(const char *ptr, size_t max_len)
{
    /* data pointer */
    const char *p = ptr;
    /* size */
    size_t size = 0;
    /* loop till end of string is found */
    while (*p++ && size < max_len)
        size++;
    /* return size */
    return size;
}

/* compare two strings */
int OPTIMIZE("Os") strcmp(const char *s1, const char *s2)
{
    /* go byte by byte */
    while (*s1 && (*s1 == *s2))
        s1++, s2++;

    /* report difference of lastly compared character */
    return *(const uint8_t *)s1 - *(const uint8_t *)s2;
}

/* case insensitive string compare */
int OPTIMIZE("Os") strcicmp(char const *a, char const *b)
{
    /* go byte by byte */
    for (;; a++, b++) {
        /* get the difference between the chars */
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        /* there is a difference or we've reached the end of the string */
        if (d != 0 || !*a)
            return d;
    }
}

/* case insensitive string compare (size limited)*/
int OPTIMIZE("O0") strncicmp(char const *a, char const *b, size_t max_size)
{
    /* difference between the characters */
    int d;
    /* go byte by byte */
    for (;max_size; a++, b++, max_size--) {
        /* get the difference between the chars */
        d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        /* there is a difference or we've reached the end of the string */
        if (d != 0 || !*a)
            return d;
    }
    /* loop may exit due to max size being reached */
    return d;
}

/* copy strings */
char * OPTIMIZE("Os") strcpy(char *dst, const char *src)
{
    char *d = dst;
    /* copy byte by byte */
    while((*dst++ = *src++));
    /* return initial destination pointer */
    return d;
}

/* copy strings */
char * OPTIMIZE("Os") strncpy(char *dst, const char *src, size_t size)
{
    size_t i;
    /* copy data */
    for (i = 0; i < size && src[i] != '\0'; i++)
        dst[i] = src[i];
    /* fill the rest with zeros */
    for ( ; i < size; i++)
        dst[i] = '\0';

    /* return initial destination pointer */
    return dst;
}

/* copy strings with limited space ensuring that we zero terminate the
 * destination */
size_t OPTIMIZE("Os") strlcpy(char *dst, const char *src, size_t size)
{
    size_t i;
    /* how can we copy anything if there is no space in output buffer */
    if (size == 0)
        return 0;
    /* copy bytes */
    for (i = 0; src[i] && (i < size - 1); i++)
        dst[i] = src[i];
    /* zero terminate */
    dst[i] = '\0';

    /* return the length of the string */
    return i;
}

/* locate the occurence of s2 within s1 string within string */
char * OPTIMIZE("Os") strstr(const char *s1, const char *s2)
{
    /* get the length of the s2 */
    size_t n = strlen(s2);

    /* try to match the s1 */
    while (*s1)
        /* memory compare shows a match? */
        if (!memcmp(s1++, s2, n))
            return (char *) (s1 - 1);
    /* no match was found */
    return 0;
}


/* locate the occurence of s2 within s1 string within string (case insensitive) */
const char * OPTIMIZE("O0") strcistr(const char *s1, const char *s2)
{
    /* difference holder */
    int d;

    /* handle edge cases */
    if (!s2)
        return s1;

    /* do actual comparison */
    for (; *s1; s1++) {
        /* get the difference between first two chars */
        d = tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        /* sadly characters do not match */
        if (d)
            continue;

        /* compare the rest of the string */
        for (const char *a = s1 + 1, *b = s2 + 1; *b; a++, b++)
            d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        /* no difference */
        if (!d)
            break;
    }

    /* no match was found */
    return d == 0 ? s1 : 0;
}