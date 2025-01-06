/**
 * @file vectors.c
 * 
 * @date 2019-09-19
 * @author twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief Interrupt/Exception vector table
 */

#include <stdint.h>
#include <stddef.h>

#include "config.h"
#include "compiler.h"
#include "defhndl.h"
#include "linker.h"
#include "startup.h"
#include "vectors.h"
#include "stm32f401/stm32f401.h"
#include "sys/time.h"
#include "sys/yield.h"

// #include "dev/usb.h"

/* shorthands so that the vector table looks neat! */
#define SET_SP(sp)                  [STM32_VECTOR_STACK_PTR_BASE].v = sp
#define SET_EXC_VEC(index, func)    [STM32_VECTOR_EXC_BASE + index].f = &func
#define SET_INT_VEC(index, func)    [STM32_VECTOR_INT_BASE + index].f = &func

/* vectors */
SECTION(".flash_vectors") vector_entry_t flash_vectors[] = {
    /* stack pointer */
    SET_SP(&__stack),

    /* exception vectors */
    /* reset vector */
    SET_EXC_VEC(STM32_EXC_RESET, Startup_ResetHandler),
    /* hard-fault */
    SET_EXC_VEC(STM32_EXC_HARDFAULT, DefHndl_DefaultHandler),

    /* pending service */
    SET_EXC_VEC(STM32_EXC_SYSTICK, Time_TickHander),
    SET_EXC_VEC(STM32_EXC_PENDSV, Yield_PendSVHandler),

    /* interrupts */
    /* usb otg */
    // SET_INT_VEC(STM32_INT_OTG_FS, USB_OTGFSIsr),
};
