/**
 * @file defhndl.c
 * 
 * @date 2019-09-19
 * @author twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief Default Exception/Interrupt Handler
 */

#include "compiler.h"
#include "reset.h"

#include "stm32f401/scb.h"

/* default interrupt/exception handler */
void NAKED DefHndl_DefaultHandler(void)
{
    /* reset the system */
    Reset_ResetMCU();
}

