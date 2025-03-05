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
#include "reset.h"

#include "boot/boot.h"
#include "net/websocket/websocket.h"
#include "sys/yield.h"
#include "util/string.h"
#include "dev/flash.h"
#include "dev/keyboard.h"
#include "dev/display.h"
#include "dev/stepup.h"

#define DEBUG DLVL_INFO
#include "debug.h"

/* starting address for the bootloader to put the data to */
#define BOOT_START_ADDRESS              (0x08020000)
/* memory size */
#define BOOT_MEM_SIZE                   (128 * 1024)

/* are we connected */
static int connected;

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

/* task for monitoring the keyboard */
static void Boot_AcceleratedRebootTask(void *arg)
{
    /* key combination that forced boot into app */
    static const kbd_mask_t key_combination = KBD_MASK_LEFT | KBD_MASK_RIGHT;
    /* keypress timestamp */
    time_t key_ts = time(0);

    for (;; Yield()) {
        /* get current keyboard state */
        kbd_mask_t keys = Kbd_GetState();
        /* we are connected, do not allow keypress to reboot */
        if (connected)
            key_ts = time(0);
        /* wrong combination is pressed */
        if ((keys & key_combination) != key_combination)
            key_ts = time(0);

        /* reboot! */
        if (dtime_now(key_ts) > 5000)
            Startup_ResetAndJump(BOOT_START_ADDRESS);
    }
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

    /* get the reset cause */
    reset_src_t reset = Reset_GetLastResetSource();
    /* reset was not caused by the watchdog */
    if (!(reset & (RESET_SRC_WWDG | RESET_SRC_IWDG))) {
        /* we've exited the stanby mode */
        if (reset & RESET_SRC_STANDBY)
            Startup_ResetAndJump(BOOT_START_ADDRESS);
        /* power-on reset */
        if (reset & RESET_SRC_POR)
            Startup_ResetAndJump(BOOT_START_ADDRESS);
    }

    /* enable step up converter to power the display */
    StepUp_Enable(1);
    /* start the display */
    Display_Enable(1);
    /* write the text */
    Display_SetChars(0, "boot", 4);

    /* poll the websocket */
    for (;; Yield()) {
        /* reset the address */
        wr_addr = BOOT_START_ADDRESS; erased = 0;
        /* listen to the socket */
        if ((ec = WebSocket_Listen(ws, 6969, 0, 45 * 1000)) < EOK) {
            ec = ENOCONNECT; goto end;
        }

        dprintf_i("we are now connected\n", 0);
        /* set the flag  */
        connected = 1;

        /* listen to incoming frames */
        for (;; Yield()) {
            /* break the reception on errors or disconnects */
            if ((ec = Boot_Recv(ws, buf, sizeof(buf))) <= EOK)
                break;

            /* sanitize */
            if (wr_addr + ec >= BOOT_START_ADDRESS + BOOT_MEM_SIZE) {
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
        end:
        /* reset the flag */
        connected = 0;
        /* error during update? */
        if (ec != ENOCONNECT) {
            /* terminate connection ourselves */
            WebSocket_Close(ws);
        /* do the check and reboot */
        } else {
            /* reboot! */
            dprintf_i("all fine, total size = %d, would bang!\n",
                wr_addr - BOOT_START_ADDRESS);
            /* reboot into firmware */
            Startup_ResetAndJump(BOOT_START_ADDRESS);
        }
    }

}

/* initialize bootloader logic */
err_t Boot_Init(void)
{
    /* start the websocket server */
    Yield_Task(Boot_ServeTask, 0, 3 * 1024);
    Yield_Task(Boot_AcceleratedRebootTask, 0, 1024);
    /* report status */
    return EOK;
}