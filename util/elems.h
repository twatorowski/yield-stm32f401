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

#endif /* UTIL_ELEMS */
