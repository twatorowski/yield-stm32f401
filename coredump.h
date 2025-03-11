/**
 * @file coredump.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-05
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef COREDUMP_H
#define COREDUMP_H

#include <stdint.h>

#include "err.h"

/* core dump structure */
typedef struct coredump {
    /* two words that shall indicate that this dump is valid */
    volatile int32_t valid, valid_neg;

    /* stack pointer */
    uint32_t sp, ipsr;
    /* core registers */
    uint32_t r0, r1, r2, r3, r12, lr, pc, psr;
    /* scb registers */
    uint32_t cfsr, hfsr, mmar, bfar;
} coredump_t;

/**
 * @brief store the core dump within ram
 *
 * @param sp stack pointer
 * @param ipsr Interrupt Program Status Register value
 */
void CoreDump_StoreDump(uint32_t *sp, uint32_t ipsr);

/**
 * @brief prints the core dump to the debug interface
 *
 * @param invalidate_after_printing 1 - invalidate, 0 - do not invalidate
 * @return err_t error code
 */
err_t CoreDump_PrintDump(int invalidate_after_printing);

/**
 * @brief checks if the core dump is valid
 *
 * @return int 1 - it is valid, 0 - otherwise
 */
int CoreDump_IsValid(void);

/**
 * @brief invalidate the core dump
 *
 */
void CoreDump_Invalidate(void);

/**
 * @brief returns true if previous excution of the application caused
 * mcu to crash
 *
 * @return int true if we are running this app after a crash
 */
int CoreDump_DidWeCrash(void);

#endif /* COREDUMP_H */
