/**
 * @file arch.h
 *
 * @date 23.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief architecture dependent instructions
 */

#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

#include <stdint.h>
#include "compiler.h"

/**
 * @brief Do nothing
 */
static inline ALWAYS_INLINE void Arch_NOP(void)
{
    /* instruction itself */
    ASM volatile ("nop\n");
}

/**
 * @brief wait for interrupt
 */
static inline ALWAYS_INLINE void Arch_WFI(void)
{
    /* instruction itself */
    ASM volatile ("wfi\n");
}

/**
 * @brief wait for event
 */
static inline ALWAYS_INLINE void Arch_WFE(void)
{
    /* instruction itself */
    ASM volatile ("wfe\n");
}

/**
 * @brief the LDREX instruction loads a word from memory, initializing the
 * state of the exclusive monitor(s) to track the synchronization operation
 *
 * @param src source address to load from. must be 32-bit aligned
 * @return 32-bit value present at address @p ptr
 */
static inline ALWAYS_INLINE uint32_t Arch_LDREX(volatile void *src)
{
    /* result */
    uint32_t result;
    /* instruction itself */
    ASM volatile (
        "ldrex    %[result], [%[src]] \n"
        : [result] "=r" (result)
        : [src] "r" (src)
    );
    /* report result */
    return result;
}

/**
 * @brief The STREX instruction performs a conditional store of a word to memory.
 * If the exclusive monitor(s) permit the store, the operation updates the
 * memory location and returns the value 0 in the destination register,
 * indicating that the operation succeeded. If the exclusive monitor(s) do not
 * permit the store, the operation does not update the memory location and
 * returns the value 1 in the destination register.
 *
 * @param dst destination address to store to. must be 32-bit aligned.
 * @param value value to be stored
 * @return 0 in case of success, 1 otherwise
 */
static inline ALWAYS_INLINE int Arch_STREX(volatile void *dst, uint32_t value)
{
    /* storage result */
    int result;
    /* instruction itself */
    ASM volatile (
        "strex %[result], %[value], [%[dst]] \n"
        : [result] "=r" (result)
        : [value] "r" (value), [dst] "r" (dst)
    );
    /* report result */
    return result;
}

/**
 * @brief The DSB instruction completes when all explicit memory accesses
 * before it complete.
 */
static inline ALWAYS_INLINE void Arch_DSB(void)
{
    /* this will stop the execution until all memory accesses are complete */
    ASM volatile ("dsb \n");
}

/**
 * @brief It flushes the pipeline of the processor, so that all instructions
 * following the ISB are fetched from cache or memory again, after the ISB
 * instruction has been completed.
 */
static inline ALWAYS_INLINE void Arch_ISB(void)
{
    /* flush pipeline */
    ASM volatile ("isb \n");
}

/**
 * @brief The BASEPRI register defines the minimum priority for exception
 * processing. When BASEPRI is set to a nonzero value, it prevents the
 * activation of all exceptions with the same or lower priority level as the
 * BASEPRI value.
 *
 * @param x value to be written
 */
static inline ALWAYS_INLINE void Arch_WriteBASEPRI(int x)
{
    /* assembly code */
    ASM volatile (
        "msr basepri, %[x] \n"
        :
        : [x] "r" (x)
    );
}

/**
 * @brief read the BASEPRI register value.
 *
 * @return value that was in the BASEPRI register.
 */
static inline ALWAYS_INLINE uint32_t Arch_ReadBASEPRI(void)
{
    /* result */
    uint32_t result;
    /* assembly code */
    ASM volatile (
        "mrs %[result], basepri \n"
        : [result] "=r" (result)
        :
    );

    /* report result */
    return result;
}

/**
 * @brief read the PRIMASK register value.
 *
 * @return value that was in the PRIMASK register.
 */
static inline ALWAYS_INLINE uint32_t Arch_ReadPRIMASK(void)
{
    /* result */
    uint32_t result;
    /* assembly code */
    ASM volatile (
        "mrs %[result], primask \n"
        : [result] "=r" (result)
        :
    );

    /* report result */
    return result;
}

/**
 * @brief Returns the value of the main stack pointer
 *
 * @return stack pointer value
 */
static inline ALWAYS_INLINE void * Arch_ReadMSP(void)
{
    /* result */
    void * result;
    /* some assembly magic */
    ASM volatile (
        "mrs    %[result], msp \n"
        : [result] "=r" (result)
        :
    );

    /* report result */
    return result;
}

/**
 * @brief Writes the main stack pointer value
 *
 * @param msp stack pointer value
 */
static inline ALWAYS_INLINE void Arch_WriteMSP(void *msp)
{
    /* some assembly magic */
    ASM volatile (
        "msr    msp, %[msp] \n"
        :
        : [msp] "r" (msp)
    );
}

/**
 * @brief Returns the value of the program stack pointer
 *
 * @return stack pointer value
 */
static inline ALWAYS_INLINE void * Arch_ReadPSP(void)
{
    /* result */
    void * result;
    /* some assembly magic */
    ASM volatile (
        "mrs    %[result], psp \n"
        : [result] "=r" (result)
    );

    /* report result */
    return result;
}

/**
 * @brief Writes the program stack pointer value
 *
 * @param msp stack pointer value
 */
static inline ALWAYS_INLINE void Arch_WritePSP(void *psp)
{
    /* some assembly magic */
    ASM volatile (
        "msr    psp, %[psp] \n"
        :
        : [psp] "r" (psp)
    );
}

/**
 * @brief Writes the CONTROL register value
 *
 * @param value value to be written
 */
static inline ALWAYS_INLINE void Arch_WriteCONTROL(uint32_t value)
{
    /* some assembly magic */
    ASM volatile (
        "msr    control, %[value]\n"
        :
        : [value] "r" (value)
    );
}

/**
 * @brief Returns the value of the interrupt program status register
 *
 * @return interrupt program status register value
 */
static inline ALWAYS_INLINE uint32_t Arch_ReadIPSR(void)
{
    /* result */
    uint32_t result;
    /* some assembly magic */
    ASM volatile (
        "mrs    %[result], ipsr \n"
        : [result] "=r" (result)
        :
    );

    /* report result */
    return result;
}

/**
 * @brief signed saturate the 'x' to be representable in 'bit' bits wide
 * signed word
 *
 * @param x value
 * @param bit number of bits that the x value shall be contained within, needs
 * to be a compile time constant
 *
 * @return uint32_t signed-saturated version of the word
 */
static inline ALWAYS_INLINE int32_t Arch_SSAT(int32_t x, const int bit)
{
    /* result */
    uint32_t result;
    /* some assembly magic */
    ASM volatile (
        "ssat    %[result], %[bit], %[x] \n"
        : [result] "=r" (result)
        : [bit] "M" (bit), [x] "r" (x)
    );
    /* report result */
    return result;
}

#endif /* ARCH_ARCH_H */
