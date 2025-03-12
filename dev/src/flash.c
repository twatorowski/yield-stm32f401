/**
 * @file flash.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-02
 * 
 * @copyright Copyright (c) 2025
 */


#include "compiler.h"
#include "err.h"
#include "arch/arch.h"
#include "dev/flash.h"
#include "dev/watchdog.h"
#include "stm32f401/flash.h"
#include "stm32f401/wwdg.h"
#include "arch/arch.h"
#include "util/msblsb.h"
#include "util/elems.h"
#include "util/forall.h"
#include "sys/yield.h"
#include "util/string.h"
#include "sys/critical.h"

/* flash memory map for given */
static const flash_sector_t sectors[] = {
    /* 4 * 16 kbyte pages */
    [0] = { .addr = 0x08000000, .size =  16 * 1024 },
    [1] = { .addr = 0x08004000, .size =  16 * 1024 },
    [2] = { .addr = 0x08008000, .size =  16 * 1024 },
    [3] = { .addr = 0x0800c000, .size =  16 * 1024 },
    /* 1 * 64kbyte page */
    [4] = { .addr = 0x08010000, .size =  64 * 1024 },
    /* 1 * 128kbyte page */
    [5] = { .addr = 0x08020000, .size = 128 * 1024 },
};


/* unlock the access to the flash mem */
static void Flash_Unlock(void)
{
    /* we are already unlocked */
    if (!(FLASH->CR & FLASH_CR_LOCK))
        return;

    /* apply the sequence that unlocks*/
    FLASH->KEYR = FLASH_KEYR_KEY1;
    FLASH->KEYR = FLASH_KEYR_KEY2;
}

/* lock the flash memory */
static void Flash_Lock(void)
{
    /* set the lock bit to 1 */
    FLASH->CR |= FLASH_CR_LOCK;
}

/* core function of the erase procedure. this is done this way so that the
 * compiler does not have to have to generate too many veneers for jumping
 * back-and-forth from ram to flash code */
static RAM_CODE void Flash_EraseCore(int sector_id)
{
    /* during the erase cycle the access to the bus will be stalled if anything
     * tries to read from flash. Disable all interrupts that may try to get
     executed during the erase cycle */
     STM32_DISABLEINTS();
     /* put a memory barrier to ensure that instruction was executed and is
      * now applied */
     Arch_ISB(); Arch_DSB();

     /* perform the operation */
     FLASH->CR = sector_id << LSB(FLASH_CR_SNB) |
         FLASH_CR_SER | FLASH_CR_STRT;

     /* put a memory barrier to ensure that register was updated */
     Arch_ISB(); Arch_DSB();
     /* wait till flash mem becomes ready */
     while (FLASH->SR & FLASH_SR_BSY) {
         /* watchdog may be running - make sure we kick it during the waiting
          * game, erase operation can take up to several seconds! */
         WWDG->CR = WWDG_CR_T;
     }

     /* re-enable the interrupts */
     STM32_ENABLEINTS();
}

/* initialize flash memory driver */
err_t Flash_Init(void)
{
    /* report status */
    return EOK;
}

/* get the sector id for given address */
err_t Flash_GetSectorIDForAddr(uint32_t addr)
{
    /* pointer into sector array */
    const flash_sector_t *s;
    /* check where the address lies */
    forall (s, sectors) {
        if (addr >= s->addr && addr < s->addr + s->size)
            return s - sectors;
    }

    /* unknown sector */
    return EFATAL;
}

/* return the information about memory sector */
err_t Flash_GetSectorInfo(int sector_id, flash_sector_t *sector)
{
    /* validate arguments */
    if (sector_id < 0 || sector_id >= elems(sectors))
        return EARGVAL;

    /* copy data */
    if (sector) *sector = sectors[sector_id];
    /* return status */
    return EOK;
}

/* erases a single sector */
err_t Flash_EraseSector(int sector_id)
{
    /* validate arguments */
    if (sector_id < 0 || sector_id >= elems(sectors))
        return EARGVAL;

    /* is the flash mem still busy? */
    while (FLASH->SR & FLASH_SR_BSY);
    /* Clear any errors */
    FLASH->SR = FLASH_SR_PGSERR | FLASH_SR_PGPERR | FLASH_SR_PGAERR |
        FLASH_SR_WRPERR;

    /* unlock the flash access */
    Flash_Unlock();
    /* do the erase */
    Flash_EraseCore(sector_id);
    /* lock the interface */
    Flash_Lock();
    /* return status */
    return EOK;
}

/* erases all sectors that are located within the specified address range */
err_t Flash_EraseSectorsForAddressRange(const void *ptr, size_t size)
{
    /* numeric representation of the address */
    uintptr_t p = (uintptr_t)ptr;

    /* erase all the sectors */
    for (size_t size_left = size;; Yield()) {
        /* get the sector id for given address */
        int sector_id = Flash_GetSectorIDForAddr(p);
        /* invalid address */
        if (sector_id < 0)
            return EFATAL;

        /* perform the erase operation */
        Flash_EraseSector(sector_id);
        /* get the size for the sector that was just erased */
        uint32_t sector_size = sectors[sector_id].size;
        /* job done */
        if (sector_size >= size_left)
            return EOK;

        /* update the pointer */
        p += sector_size; size_left -= sector_size;
    }

    /* report status */
    return EOK;
}

/* reading from flash can be done using regular memory copy */
err_t Flash_Read(void *dst, const void *src, size_t size)
{
    /* read the memory */
    memcpy(dst, src, size);
    /* return the size of the memory being read */
    return size;
}

/* returns EOK when contents of the flash match the contents of the ram */
err_t Flash_Verify(const void *flash, const void *ram, size_t size)
{
    /* use the memcmp for the comparison */
    return memcmp(flash, ram, size) == 0 ? EOK : EFATAL;
}

/* write data to flash memory */
err_t Flash_Write(void *dst, const void *src, size_t size)
{
    /* convert to u8 pointer */
    uint8_t *d8 = dst; const uint8_t *s8 = src;
    /* error code */
    err_t ec = EOK;
    /* since we don't want to spend too much time in a single place we are
     * going to yield from time to time  :-) */
    time_t ts = time(0);

    /* unlock the interface */
    Flash_Unlock();

    /* is the flash mem still busy? */
    while (FLASH->SR & FLASH_SR_BSY);
    /* Clear any errors */
    FLASH->SR = FLASH_SR_PGSERR | FLASH_SR_PGPERR | FLASH_SR_PGAERR |
        FLASH_SR_WRPERR;

    /* write the data */
    for (uint32_t b_stored = 0; b_stored < size; b_stored++) {
        /* set the chip into programming mode */
        FLASH->CR |= FLASH_CR_PG;
        /* store the data */
        *(d8 + b_stored) = *(s8 + b_stored);
        /* is the flash mem still busy? */
        while (FLASH->SR & FLASH_SR_BSY);
        /* error during programming */
        if (FLASH->SR & (FLASH_SR_PGSERR | FLASH_SR_PGPERR | FLASH_SR_PGAERR |
            FLASH_SR_WRPERR)) {
            ec = EFATAL; break;
        }

        /* yield from time to time */
        if (dtime_now(ts) > 10) {
            Yield(); ts = time(0);
        }
    }

    /* lock the interface */
    Flash_Lock();
    /* return the status */
    return ec < EOK ? ec : size;
}