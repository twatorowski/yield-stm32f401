/**
 * @file reset.h
 * 
 * @date 2019-11-13
 * twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief Routine to reset the MCU
 */
#ifndef RESET_H
#define RESET_H

#include "err.h"
#include "compiler.h"
#include "util/bit.h"

/** reset source mask */
typedef enum reset_src {
    RESET_SRC_LP = BIT_VAL(0),
    RESET_SRC_WWDG = BIT_VAL(1),
    RESET_SRC_IWDG = BIT_VAL(2),
    RESET_SRC_SW = BIT_VAL(3),
    RESET_SRC_POR = BIT_VAL(4),
    RESET_SRC_PIN = BIT_VAL(5),
    RESET_SRC_BOR = BIT_VAL(6),
    RESET_SRC_STANDBY = BIT_VAL(7),
} reset_src_t;

/**
 * @brief initialize reset support
 *
 * @return err_t error code
 */
err_t Reset_Init(void);

/**
 * @brief get reset cause
 *
 * @return reset_src_t reset source
 */
reset_src_t Reset_GetLastResetSource(void);

/**
 * @brief Resets the MCU
 */
void NORETURN Reset_ResetMCU(void);

#endif /* RESET_H */
