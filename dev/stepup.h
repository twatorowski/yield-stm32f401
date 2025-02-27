/**
 * @file stepup.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_STEPUP_H
#define DEV_STEPUP_H

#include "err.h"

/**
 * @brief initialize step up converter control
 *
 * @return err_t error code
 */
err_t StepUp_Init(void);

/**
 * @brief enable/disable step up converter
 *
 * @param enable 1 - enable, 0 - disable
 * @return err_t error code
 */
err_t StepUp_Enable(int enable);



#endif /* DEV_STEPUP_H */
