/**
 * @file minmax.h
 *
 * @date 28.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief Macro helper for min/max
 */

#ifndef UTIL_MINMAX_H
#define UTIL_MINMAX_H

/** @brief get the maximal value out of two. Not 'side-effect' proof */
#define max(a, b)                               (((a) > (b)) ? (a) : (b))
/** @brief get the minimal value out of two. Not 'side-effect' proof */
#define min(a, b)                               (((a) < (b)) ? (a) : (b))

#endif /* UTIL_MINMAX_H */
