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

/* iteration macro definition */
#define forall(elem, array)                     \
    for (elem = array; elem != array + elems(array); elem++)


#endif /* UTIL_FORALL_H */
