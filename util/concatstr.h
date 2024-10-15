/**
 * @file concatstr.h
 * 
 * @date 2019-11-03
 * @author twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief Macro for concatenating strings
 */

#ifndef UTIL_CONCATSTR_H
#define UTIL_CONCATSTR_H

/* this is a helper for strinc concatenation*/
#define _CONCATSTR(x)                       #x
#define CONCATSTR(x)                        _CONCATSTR(x)

#endif /* UTIL_CONCATSTR_H */
