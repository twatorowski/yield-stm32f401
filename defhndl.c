/**
 * @file defhndl.c
 *
 * @date 2019-09-19
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief Default Exception/Interrupt Handler
 */

#include <stdint.h>

#include "compiler.h"
#include "reset.h"
#include "coredump.h"

#include "arch/arch.h"
#include "dev/seed.h"
#include "stm32f401/scb.h"


/* default interrupt/exception handler */
void NAKED DefHndl_DefaultHandler(void)
{
    /* read stack pointer */
	uint32_t *sp = (uint32_t *)Arch_ReadPSP();
	/* read ipsr */
	uint32_t ipsr = Arch_ReadIPSR();

    /* dump the info into the RAM */
    CoreDump_StoreDump(sp, ipsr);

    /* reset the system */
    Reset_ResetMCU();
}

