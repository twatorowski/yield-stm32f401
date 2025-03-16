/**
 * @file forall.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-06-12
 * 
 * @brief For-All macro definition
 */
#ifndef UTIL_FORALL_H
#define UTIL_FORALL_H

#include "elems.h"

/* forall version that works with the variable profived from the outside */
#define __forall2(ptr, array)                     \
    for (ptr = array; ptr != array + elems(array); ptr++)

/* forall version that works with local pointer */
#define __forall3(type, ptr, array)                \
    for (type ptr = array; ptr != array + elems(array); ptr++)


/* macro for getting the correct forall macro */
#define __get_macro(_0, _1, _2, NAME, ...) NAME
#define forall(...) __get_macro(__VA_ARGS__, __forall3, __forall2)(__VA_ARGS__)




#endif /* UTIL_FORALL_H */
