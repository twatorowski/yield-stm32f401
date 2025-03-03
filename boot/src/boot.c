/**
 * @file boot.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-02
 * 
 * @copyright Copyright (c) 2025
 */

#include "compiler.h"
#include "startup.h"
#include "coredump.h"

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
        /* reset the address */
        wr_addr = BOOT_START_ADDRESS; erased = 0;
        /* listen to the socket */
        if ((ec = WebSocket_Listen(ws, 6969, 0, 10 * 1000)) < EOK) {
            ec = ENOCONNECT; goto end;
        }

        /* reset the pointers */
        dprintf_i("we are now connected\n", 0);

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
                Flash_EraseSectorsForAddressRange((void *)BOOT_START_ADDRESS,
                    BOOT_MEM_SIZE); erased = 1;
            }

            /* show activity */
            dprintf_i("received %d bytes, putting at %#x\n", ec, wr_addr);
            /* write data to flash */
            Flash_Write((void *)wr_addr, buf, ec);
            /* write did not succeed */
            if (Flash_Verify((void *)wr_addr, buf, ec) != EOK) {
                ec = EFATAL; break;
            }

            /* update the write address */
            wr_addr += ec;
        }
        /* we didn't finish with disconnect, so something must not be right,
        * do not boot to firmware */
        end: if (ec != ENOCONNECT) {
            /* terminate connection ourselves */
            WebSocket_Close(ws);
        /* do the check and reboot */
        } else {
            /* reboot! */
            dprintf_i("all fine, total size = %d, would bang!\n",
                wr_addr - BOOT_START_ADDRESS);
            /* reboot into firmware */
            // Startup_ResetAndJump(BOOT_START_ADDRESS);
        }
    }

}

/* initialize bootloader logic */
err_t Boot_Init(void)
{
    dprintf_i("did we crash = %d\n", CoreDump_DidWeCrash());
    /* start the websocket server */
    return Yield_Task(Boot_ServeTask, 0, 3 * 1024);
}