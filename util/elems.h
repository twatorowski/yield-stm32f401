/**
 * @file elems.h
 *
 * @date 28.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief Macro helper for determining the number of array elements
 */

#ifndef UTIL_ELEMS
#define UTIL_ELEMS

/** @brief number of array elems */
#define elems(x)                                (sizeof(x) / (sizeof(x[0])))

/** get the pointer to the element after the array  */
#define arrend(x)                              ((x) + elems((x)))

#endif /* UTIL_ELEMS */
