/**
 * @file startup.c
 *
 * @date 23.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief stm32l433 startup file
 */

#include <stdint.h>
#include <stddef.h>

#include "linker.h"
#include "compiler.h"
#include "reset.h"
#include "dev/watchdog.h"

/* initialization routine */
extern void Init(void);
/* main program routine */
extern void Main(void);

/* application jump address */
volatile uint32_t SECTION(".startup_jump_address") startup_jump_address;

/* copy a memory section */
static void Startup_CopySection(void *dst, const void *src, size_t size)
{
    /* destination and source pointer */
    uint8_t *d = dst; const uint8_t *s = src;

    /* some unrolling */
    for (; size >= 4; size -= 4)
        *d++ = *s++, *d++ = *s++, *d++ = *s++, *d++ = *s++;

    /* final touch */
    switch (size & 3) {
    case 3 : *d++ = *s++;
    case 2 : *d++ = *s++;
    case 1 : *d++ = *s++;
    }
}

/* clear a memory section */
static void Startup_ZeroSection(void *ptr, size_t size)
{
    /* zeroing pointer */
    uint8_t *p = ptr;

    /* some unrolling */
    for (; size >= 4; size -= 4)
        *p++ = 0, *p++ = 0, *p++ = 0, *p++ = 0;

    /* final touch */
    switch (size & 3) {
    case 3 : *p++ = 0;
    case 2 : *p++ = 0;
    case 1 : *p++ = 0;
    }
}

/* set stack pointer */
static void NORETURN Startup_JumpToCode(uint32_t addr)
{
	/* assembly jump routine */
	ASM volatile (
		/* load stack pointer address */
		"ldr r1,  [%[addr]]					\n"
		/* set stack pointer */
		"msr msp,  r1						\n"

		/* load jump address to link register */
		"ldr lr,  [%[addr], #4]			    \n"
		/* perform a jump */
		"bx lr								\n"
		:
		: [addr] "r" (addr)
	);

    /* stop complaining about NORETURN returning, ok? */
    while (1);
}

/* first function to be executed after the reset. Shall initialize the chip
 * to it's default state */
void SECTION(".flash_code") Startup_ResetHandler(void)
{
    /* start the watchdog to protect us from ending up in shitty code that
     * does not enable watchdog by itself */
    Watchdog_Init();

    /* user wants to jump to application code? */
    if (startup_jump_address != 0) {
        /* store the jump address in a local variable and clean the global one */
        uint32_t addr = startup_jump_address; startup_jump_address = 0;
        /* jump to application code */
        Startup_JumpToCode(addr);
    }

    /* initialize ram functions */
    Startup_CopySection(&__ram_code_addr, &__flash_sram_init_src_addr,
        (size_t)&__ram_code_size);
    /* initialize data */
    Startup_CopySection(&__data_addr,  (uint8_t *)&__flash_sram_init_src_addr +
        (size_t)&__ram_code_size, (size_t)&__data_size);
    /* zero out the bss */
    Startup_ZeroSection(&__bss_addr, (size_t)&__bss_size);

    /* kick the dog before jumping to main functions */
    Watchdog_Kick();

    /* do the initialization */
    Init();
}

/* reset the mcu and execute application at given address */
void Startup_ResetAndJump(uint32_t addr)
{
    /* store the address */
    startup_jump_address = addr;
    /* reset the mcu */
    Reset_ResetMCU();
}