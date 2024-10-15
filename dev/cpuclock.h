/**
 * @file cpuclock.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-09-02
 * 
 * @brief CPU Clock configuration
 */

#ifndef DEV_CPUCLOCK_H
#define DEV_CPUCLOCK_H

#include "err.h"

/**
 * @brief initialize cpu clock to 256MHz (HSE + PLL). On the
 * nucleo board make sure that you have the solder jumpers set to correct
 * values: Close the SB50.
 *
 * @return int initialization status error code @ref ERR_ERROR_CODES
 */
err_t CpuClock_Init(void);

#endif /* DEV_CPUCLOCK_H */
