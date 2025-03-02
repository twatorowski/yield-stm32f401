/**
 * @file boot.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-02
 * 
 * @copyright Copyright (c) 2025
 */

#include "compiler.h"
#include "boot/boot.h"
#include "net/websocket/websocket.h"
#include "sys/yield.h"
#include "util/string.h"
#include "dev/flash.h"

#define DEBUG DLVL_INFO
#include "debug.h"

/* starting address for the bootloader to put the data to */
#define BOOT_START_ADDRESS              (0x08020000)
/* memory size */
#define BOOT_MEM_SIZE                   (128 * 1024)


/* set stack pointer */
static void Boot_Jump(uint32_t addr)
{
	/* assembly jump routine */
	ASM volatile (
		/* disable interrupts */
		"cpsid i							\n"
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
}

/* receive data from the socket */
static err_t Boot_Recv(websocket_t *ws, void *ptr, size_t size)
{
    /* error code and the data type */
    err_t ec; websocket_data_type_t dtype;

    /* read data from the socket */
    if ((ec = WebSocket_Recv(ws, &dtype, ptr, size, 0)) < EOK)
        return ec;
    /* we only support binary transfers */
    if (dtype != WS_DATA_TYPE_BIN)
        return EFATAL;

    /* otherwise return the number of bytes received */
    return ec;
}

/* serve the api */
static void Boot_ServeTask(void *arg)
{
    /* create the socket */
    websocket_t *ws = WebSocket_Create();
    /* current write address */
    uint32_t wr_addr, erased;

    /* protocol buffer */
    uint8_t buf[128];
    /* error code */
    err_t ec;

    /* poll the websocket */
    for (;; Yield()) {
        /* listen to the socket */
        if ((ec = WebSocket_Listen(ws, 6969, 0)) < EOK)
            continue;

        /* reset the pointers */
        dprintf_i("we are now connected\n", 0);
        /* reset the address */
        wr_addr = BOOT_START_ADDRESS; erased = 0;

        /* listen to incoming frames */
        for (;; Yield()) {
            /* break the reception on errors or disconnects */
            if ((ec = Boot_Recv(ws, buf, sizeof(buf))) <= EOK)
                break;

            /* sanitize */
            if (wr_addr + ec >= BOOT_START_ADDRESS + BOOT_MEM_SIZE) {
                dprintf_i("write address out of bounds", 0);
                ec = EFATAL; break;
            }

            /* erase the memory making space for new firmware */
            if (!erased) {
                dprintf_i("erasing\n", 0);
                Flash_EraseSectorsForAddressRange((void *)BOOT_START_ADDRESS,
                BOOT_MEM_SIZE); erased = 1;
                dprintf_i("erased!\n", 0);
            }

            /* show activity */
            dprintf_i("received %d bytes, putting at %x\n", ec, wr_addr);
            /* write data to flash */
            Flash_Write((void *)wr_addr, buf, ec);
            /* write did not succeed */
            if (Flash_Verify((void *)wr_addr, buf, ec) != EOK) {
                dprintf_i("verify failed\n", 0);
                ec = EFATAL; break;
            }

            /* update the write address */
            wr_addr += ec;
        }
        /* we didn't finish with disconnect, so something must not be right,
        * do not boot to firmware */
        if (ec != ENOCONNECT) {
            /* terminate connection ourselves */
            WebSocket_Close(ws);
        /* do the check and reboot */
        } else {
            // TODO: reboot!
            dprintf_i("all fine, would bang!\n", 0);
        }
    }

}

/* initialize bootloader logic */
err_t Boot_Init(void)
{
    /* start the websocket server */
    return Yield_Task(Boot_ServeTask, 0, 3 * 1024);
}