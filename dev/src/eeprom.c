/**
 * @file eeprom.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-01
 *
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "dev/swi2c.h"
#include "dev/eeprom.h"
#include "sys/time.h"
#include "sys/yield.h"
#include "util/endian.h"
#include "util/minmax.h"


/* device access codes (these are the most significant i2c address bits) */
#define EEPROM_DEV_CODE_MEM                     0b1010
#define EEPROM_DEV_CODE_ID                      0b1011


/* set the state of the write protect pin */
static inline void EEPROM_SetWriteProtect(eeprom_dev_t *dev, int enable)
{
    /* drive the pin. WP 1 = protection on (no writes), 0 = off */
    GPIOSig_Set(dev->wp, enable ? 1 : 0);
}

/* poll as long as the chip is busy or timeout occurs */
static err_t EEPROM_PollBusy(eeprom_dev_t *dev, dtime_t timeout)
{
    /* status flag */
    err_t ec = EOK;

    /* device address for memory access */
    int addr = (EEPROM_DEV_CODE_MEM << 3) | (dev->a2a1a0 & 0x7);
    /* we are not actually planning to send data but the protocol expects us to
     * sent the address with the R/W bit set to 0 which means 'Writing' */
    swi2c_oper_t oper = SWI2C_OPER_ADDR | SWI2C_OPER_WR | SWI2C_OPER_STOP;

    /* poll until timeout occurs */
    for (time_t ts = time(0); !timeout || dtime_now(ts) < timeout; Yield()) {
        /* let's to a read on the bus */
        ec = SwI2C_Transfer(dev->swi2c, oper, addr, 0, 0);
        /* device responded? */
        if (ec >= EOK)
            break;
    }

    /* report status */
    return ec;
}

/* do a swi2c transfer within the memory page */
static err_t EEPROM_MemTransfer(eeprom_dev_t *dev, swi2c_oper_t oper,
    void *ptr, size_t size)
{
    /* device address for memory access */
    int addr = (EEPROM_DEV_CODE_MEM << 3) | (dev->a2a1a0 & 0x7);
    /* do the operation */
    return SwI2C_Transfer(dev->swi2c, oper, addr, ptr, size);
}

/* initialize eeprom device driver */
err_t EEPROM_Init(void)
{
    /* report status */
    return EOK;
}

/* initialize device */
err_t EEPROM_DevInit(eeprom_dev_t *dev)
{
    /* dummy word storage */
    uint32_t dummy;

    /* configure wp signal */
    GPIOSig_CfgOutput(dev->wp, GPIO_OTYPE_PP, 1);
    /* perform a dummy read */
    return EEPROM_Read(dev, 0, &dummy, sizeof(dummy));
}

/* read the mem contents */
err_t EEPROM_Read(eeprom_dev_t *dev, size_t offset, void *ptr, size_t size)
{
    /* error code, number of bytes written */
    err_t ec; uint8_t *p8 = ptr;

    /* sanitize parameters */
    if (offset + size >= dev->capacity)
        return EFATAL;

    /* do we need to write the address? */
    int write_addr = !dev->mem_addr_valid || dev->mem_addr != offset;

    /* wait for the chip to become ready*/
    if ((ec = EEPROM_PollBusy(dev, 10)) < EOK)
        goto end;

    /* need to re-address the memory */
    if (write_addr) {
        /* address word */
        uint16_t addr_word = HTOBE16(offset);
        /* do the addressing */
        if ((ec = EEPROM_MemTransfer(dev, SWI2C_OPER_ADDR | SWI2C_OPER_WR |
                SWI2C_OPER_STOP, &addr_word, sizeof(addr_word))) < EOK)
            goto end;
    }

    /* write the data */
    if ((ec = EEPROM_MemTransfer(dev, SWI2C_OPER_ADDR | SWI2C_OPER_RD |
        SWI2C_OPER_STOP, (uint8_t *)p8, size)) < EOK)
        goto end;

    /* update the address */
    dev->mem_addr = offset + size;
    dev->mem_addr_valid = 1;

    /* release the i2c device */
    end: if (ec < EOK) {
        dev->mem_addr_valid = 0; Sem_Release(&dev->swi2c->sem);
    }
    /* return the number of the bytes written */
    return ec < EOK ? ec : size;
}


/* write data in memory under given offset */
err_t EEPROM_Write(eeprom_dev_t *dev, size_t offset, const void *ptr,
    size_t size)
{
    /* error code, number of bytes written */
    err_t ec = EOK; const uint8_t *p8 = ptr; size_t b_written = 0;

    /* sanitize parameters */
    if (offset + size >= dev->capacity)
        return EFATAL;

    /* disable write protect */
    EEPROM_SetWriteProtect(dev, 0);

    /* writing loop */
    for (; b_written < size; Yield()) {
        /* lock the i2c device */
        Sem_Lock(&dev->swi2c->sem, 0);

        /* wait for the chip to become ready */
        if ((ec = EEPROM_PollBusy(dev, 10)) < EOK)
            goto end;

        /* address word */
        uint16_t addr_word = HTOBE16(offset);
        /* do the addressing */
        if ((ec = EEPROM_MemTransfer(dev, SWI2C_OPER_ADDR | SWI2C_OPER_WR,
            &addr_word, sizeof(addr_word))) < EOK)
            goto end;

        /* determine current transfer size */
        size_t t_size = min((size - b_written),
            dev->page_size - (offset % dev->page_size));
        /* write the data */
        if ((ec = EEPROM_MemTransfer(dev, SWI2C_OPER_WR | SWI2C_OPER_STOP,
            (uint8_t *)p8 + b_written, t_size)) < EOK)
            goto end;

        /* update counters */
        b_written += t_size; offset += t_size;
        /* allow others to use the bus */
        Sem_Release(&dev->swi2c->sem);
    }

    /* update the memory address */
    dev->mem_addr = offset;
    dev->mem_addr_valid = 1;
    /* release the i2c device */
    end: if (ec < EOK) {
        dev->mem_addr_valid = 0; Sem_Release(&dev->swi2c->sem);
    }
    /* enable write protect */
    EEPROM_SetWriteProtect(dev, 1);
    /* return the number of the bytes written */
    return ec < EOK ? ec : b_written;
}
