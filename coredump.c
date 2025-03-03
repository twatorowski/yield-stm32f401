/**
 * @file coredump.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-05
 * 
 * @copyright Copyright (c) 2025
 */


#include "err.h"
#include "compiler.h"
#include "coredump.h"

#include "arch/arch.h"
#include "dev/seed.h"
#include "stm32f401/scb.h"

#define DEBUG DLVL_WARN
#include "debug.h"

/* store the coredump in the ram area that is not initialized (meaning not
 * erased at reset)*/
static coredump_t SECTION(".core_dump") coredump;
/* will be non zero if the coredump was valid before invalidation */
static uint32_t coredump_was_valid;
// Usage fault status register (UFSR)

// Bits 31:26 Reserved, must be kept cleared
// Bit 25 DIVBYZERO: Divide by zero usage fault. When the processor sets this bit to 1, the PC value
// stacked for the exception return points to the instruction that performed the divide by zero.
// Enable trapping of divide by zero by setting the DIV_0_TRP bit in the CCR to 1, see
// Configuration and control register (CCR) on page 231.
// 0: No divide by zero fault, or divide by zero trapping not enabled
// 1: The processor has executed an SDIV or UDIV instruction with a divisor of 0.

// Bit 24 UNALIGNED: Unaligned access usage fault. Enable trapping of unaligned accesses by
// setting the UNALIGN_TRP bit in the CCR to 1, see Configuration and control register (CCR)
// on page 231.
// Unaligned LDM, STM, LDRD, and STRD instructions always fault irrespective of the setting of
// UNALIGN_TRP.
// 0: No unaligned access fault, or unaligned access trapping not enabled
// 1: the processor has made an unaligned memory access.

// Bits 23:20 Reserved, must be kept cleared
// Bit 19 NOCP: No coprocessor usage fault. The processor does not support coprocessor instructions:
// 0: No usage fault caused by attempting to access a coprocessor
// 1: the processor has attempted to access a coprocessor.

// Bit 18 INVPC: Invalid PC load usage fault, caused by an invalid PC load by EXC_RETURN:
// When this bit is set to 1, the PC value stacked for the exception return points to the instruction
// that tried to perform the illegal load of the PC.
// 0: No invalid PC load usage fault
// 1: The processor has attempted an illegal load of EXC_RETURN to the PC, as a result of an
// invalid context, or an invalid EXC_RETURN value.

// Bit 17 INVSTATE: Invalid state usage fault. When this bit is set to 1, the PC value stacked for the
// exception return points to the instruction that attempted the illegal use of the EPSR.
// This bit is not set to 1 if an undefined instruction uses the EPSR.
// 0: No invalid state usage fault
// 1: The processor has attempted to execute an instruction that makes illegal use of the
// EPSR.

// Bit 16 UNDEFINSTR: Undefined instruction usage fault. When this bit is set to 1, the PC value
// stacked for the exception return points to the undefined instruction.
// An undefined instruction is an instruction that the processor cannot decode.
// 0: No undefined instruction usage fault
// 1: The processor has attempted to execute an undefined instruction.



// Bus fault status register (BFSR)

// Bit 15 BFARVALID: Bus Fault Address Register (BFAR) valid flag. The processor sets this bit to 1
// after a bus fault where the address is known. Other faults can set this bit to 0, such as a
// memory management fault occurring later.
// If a bus fault occurs and is escalated to a hard fault because of priority, the hard fault handler
// must set this bit to 0. This prevents problems if returning to a stacked active bus fault handler
// whose BFAR value is overwritten.
// 0: Value in BFAR is not a valid fault address
// 1: BFAR holds a valid fault address.

// Bit 14 Reserved, must be kept cleared
// Bit 13 LSPERR: Bus fault on floating-point lazy state preservation.
// 0: No bus fault occurred during floating-point lazy state preservation.
// 1: A bus fault occurred during floating-point lazy state preservation

// Bit 12 STKERR: Bus fault on stacking for exception entry. When the processor sets this bit to 1, the
// SP is still adjusted but the values in the context area on the stack might be incorrect. The
// processor does not write a fault address to the BFAR.
// 0: No stacking fault
// 1: Stacking for an exception entry has caused one or more bus faults.

// Bit 11 UNSTKERR: Bus fault on unstacking for a return from exception. This fault is chained to the
// handler. This means that when the processor sets this bit to 1, the original return stack is still
// present. The processor does not adjust the SP from the failing return, does not performed a
// new save, and does not write a fault address to the BFAR.
// 0: No unstacking fault
// 1: Unstack for an exception return has caused one or more bus faults.

// Bit 10 IMPRECISERR: Imprecise data bus error. When the processor sets this bit to 1, it does not
// write a fault address to the BFAR. This is an asynchronous fault. Therefore, if it is detected
// when the priority of the current process is higher than the bus fault priority, the bus fault
// becomes pending and becomes active only when the processor returns from all higher priority
// processes. If a precise fault occurs before the processor enters the handler for the imprecise
// bus fault, the handler detects both IMPRECISERR set to 1 and one of the precise fault status
// bits set to 1.
// 0: No imprecise data bus error
// 1: A data bus error has occurred, but the return address in the stack frame is not related to
// the instruction that caused the error.

// Bit 9 PRECISERR: Precise data bus error. When the processor sets this bit is 1, it writes the
// faulting address to the BFAR.
// 0: No precise data bus error
// 1: A data bus error has occurred, and the PC value stacked for the exception return points to
// the instruction that caused the fault.

// Bit 8 IBUSERR: Instruction bus error. The processor detects the instruction bus error on prefetching
// an instruction, but it sets the IBUSERR flag to 1 only if it attempts to issue the faulting
// instruction.
// When the processor sets this bit is 1, it does not write a fault address to the BFAR.
// 0: No instruction bus error
// 1: Instruction bus error.


// Memory management fault address register (MMFSR)

// Bit 7 MMARVALID: Memory Management Fault Address Register (MMAR) valid flag. If a memory
// management fault occurs and is escalated to a hard fault because of priority, the hard fault
// handler must set this bit to 0. This prevents problems on return to a stacked active memory
// management fault handler whose MMAR value is overwritten.
// 0: Value in MMAR is not a valid fault address
// 1: MMAR holds a valid fault address.

// Bit 6 Reserved, must be kept cleared

// Bit 5 MLSPERR:
// 0: No MemManage fault occurred during floating-point lazy state preservation
// 1: A MemManage fault occurred during floating-point lazy state preservation

// Bit 4 MSTKERR: Memory manager fault on stacking for exception entry. When this bit is 1, the SP
// is still adjusted but the values in the context area on the stack might be incorrect. The
// processor has not written a fault address to the MMAR.
// 0: No stacking fault
// 1: Stacking for an exception entry has caused one or more access violations.

// Bit 3 MUNSTKERR: Memory manager fault on unstacking for a return from exception. This fault is
// chained to the handler. This means that when this bit is 1, the original return stack is still
// present. The processor has not adjusted the SP from the failing return, and has not performed
// a new save. The processor has not written a fault address to the MMAR.
// 0: No unstacking fault
// 1: Unstack for an exception return has caused one or more access violations.

// Bit 2 Reserved, must be kept cleared
// Bit 1 DACCVIOL: Data access violation flag. When this bit is 1, the PC value stacked for the
// exception return points to the faulting instruction. The processor has loaded the MMAR with
// the address of the attempted access.
// 0: No data access violation fault
// 1: The processor attempted a load or store at a location that does not permit the operation.

// Bit 1 IACCVIOL: Instruction access violation flag. This fault occurs on any access to an XN region,
// even the MPU is disabled or not present.
// When this bit is 1, the PC value stacked for the exception return points to the faulting
// instruction. The processor has not written a fault address to the MMAR.
// 0: No instruction access violation fault
// 1: The processor attempted an instruction fetch from a location that does not permit
// execution.


/* store the core dump */
void CoreDump_StoreDump(uint32_t *sp, uint32_t ipsr)
{
    /* get the random value */
    uint32_t valid = Seed_GetRand();

    /* store the data into the part of the RAM that will not get erased
     * after reset */
    coredump = (coredump_t) {
        /* store the flags */
        .valid = valid, .valid_neg = ~valid,
        /* store stack pointer */
        .sp = (uint32_t)sp, ipsr = ipsr,
        /* store registers from stack */
        .r0  = sp[0], .r1 = sp[1], .r2 = sp[2], .r3 = sp[3],
        .r12 = sp[4], .lr = sp[5], .pc = sp[6], .psr = sp[7],
        /* store scb registers */
        .cfsr = SCB->CFSR, .hfsr = SCB->HFSR,
        .mmar = SCB->MMFAR, .bfar = SCB->BFAR
    };
}

/* print the core dump */
err_t CoreDump_PrintDump(int invalidate_after_printing)
{
    /* are we compiled for debug */
    #ifdef DEBUG

    /* shorthand */
    coredump_t *cd = &coredump;

    /* no valid dump is stored */
    if (!CoreDump_IsValid())
        return EFATAL;

    dprintf_w("-------------------------------------------------------\n", 0);
    /* display stack pointer */
    dprintf_w("msp = %#010x, ipsr = %#010x\n", cd->sp, cd->ipsr);
    /* display registers */
    dprintf_w("r0  = %#010x, r1  = %#010x\n", cd->r0, cd->r1);
    dprintf_w("r2  = %#010x, r3  = %#010x\n", cd->r2, cd->r3);
    dprintf_w("r12 = %#010x, lr  = %#010x\n", cd->r12, cd->lr);
    dprintf_w("pc  = %#010x, psr = %#010x\n", cd->pc, cd->psr);
    /* fault registers */
    dprintf_w("cfsr = %#010x, hfsr = %#010x\n", cd->cfsr, cd->hfsr);
    dprintf_w("mmar = %#010x, bfar = %#010x\n", cd->mmar, cd->bfar);
    dprintf_w("-------------------------------------------------------\n", 0);

    #endif

    /* clear the coredump */
    if (invalidate_after_printing)
        CoreDump_Invalidate();

    /* report status */
    return EOK;
}

/* checks if the core dump is valid */
int CoreDump_IsValid(void)
{
    /* if these words are negated versions of each-other then core
     * dump is valid */
    return coredump.valid == ~coredump.valid_neg;
}

/* invalidate the core dump */
void CoreDump_Invalidate(void)
{
    /* core dump was valid before we invalidated, store this information */
    if (CoreDump_IsValid())
        coredump_was_valid = 1;

    /* invalidate the coredump */
    coredump.valid = 0; coredump.valid_neg = 1;
}

/* returns true if previous excution of the application caused mcu to crash */
int CoreDump_DidWeCrash(void)
{
    return CoreDump_IsValid() || coredump_was_valid;
}