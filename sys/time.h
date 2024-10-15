/**
 * @file time.h
 *
 * @date 2020-03-29
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief Basic system time routines
 */

#ifndef SYS_TIME_H
#define SYS_TIME_H

#include <stdint.h>

#include "err.h"
#include "compiler.h"

/* default time type */
typedef uint32_t time_t;
/* default time difference type */
typedef int32_t dtime_t;


/** @brief systick overflow exception handler */
void Time_TickHander(void);

/**
 * @brief intialize system timer circuitry
 * 
 * @return err_t error code
 */
err_t Time_Init(void);

/**
 * @brief return the absolute time in ms 
 * 
 * @return uint32_t timestamp in ms
 */
uint32_t Time_GetTime(void);

/**
 * @brief return microseconds counter value in range of 0-9999us
 * 
 * @return uint32_t microseconds counter value
 */
uint32_t Time_GetUS(void);


/**
 * @brief simple delay function. Keep in mind that it stalls the execution 
 * completely for the time of the delay
 * 
 * @param us number of microseconds to stall for
 */
void Time_DelayUS(uint32_t us);

/**
 * @brief Get current system time timer value in ms;
 *
 * @param t
 *
 * @return system time value
 */
static inline ALWAYS_INLINE time_t time(time_t *t)
{
    /* get current system timer value */
    time_t ms = Time_GetTime();
    /* store within the pointer value */
    if (t)
        *t = ms;
    /* return the value */
    return ms;
}

/**
 * @brief Get the time difference between two timestamps: a - b
 *
 * @param a 1st timestamp (must be greater than b to get the positive result)
 * @param b 2nd timestamp (must be lower than a to get the positive result)
 *
 * @return difference in milliseconds
 */
static inline ALWAYS_INLINE dtime_t dtime(time_t a, time_t b)
{
    return (dtime_t)(a - b);
}

/**
 * @brief Get the time difference between the timestamp and now: now() - b
 *
 * @param t timestamp
 *
 * @return difference in milliseconds
 */
static inline ALWAYS_INLINE dtime_t dtime_now(time_t t)
{
    return (dtime_t)(time(0) - t);
}

/**
 * @brief Get the time difference between two timestamps: a - b but ensure
 * monotonicity: i.e if 'a' happened before 'b' (and so a-b < 0) return the
 * 0.
 *
 * @param a 1st timestamp
 * @param b 2nd timestamp
 *
 * @return difference in milliseconds of max value representable if a happened
 * after b
 */
static inline ALWAYS_INLINE dtime_t dtime_m(time_t a, time_t b)
{
    /* get the time differential */
    dtime_t diff = dtime(a, b);
    /* never return negative numbers to ensure monotonicity */
    return diff < 0 ? 0 : diff;
}


#endif /* SYS_TIME_H */
