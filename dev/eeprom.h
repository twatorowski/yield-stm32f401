/**
 * @file eeprom.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-01
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_EEPROM_H
#define DEV_EEPROM_H

#include "err.h"
#include "dev/swi2c.h"
#include "dev/gpio_signals.h"
#include "sys/sem.h"

/** device descriptor */
typedef struct eeprom_dev {
    /* underlyins i2c device */
    swi2c_dev_t *swi2c;
    /* gpio pin for the write protect signal */
    gpio_signal_t wp;
    /* memory capacity in bytes */
    uint32_t capacity, page_size;

    /* three least-significant address bits (configured by
     * hardwiring the pins) */
    int a2a1a0;
    /* access semaphore */
    sem_t sem;
    /* current memory addres (for sequential reads)*/
    uint32_t mem_addr, mem_addr_valid;
} eeprom_dev_t;


/**
 * @brief initialize eeprom device driver (common parts)
 *
 * @return err_t error codes
 */
err_t EEPROM_Init(void);

/**
 * @brief initialize device
 *
 * @param dev device descritptor
 *
 * @return err_t error code
 */
err_t EEPROM_DevInit(eeprom_dev_t *dev);

/**
 * @brief read the mem contents
 *
 * @param dev device descriptor
 * @param offset address within the memory
 * @param ptr pointer to where to store the data
 * @param size size of the data to be read
 *
 * @return err_t error code or number of bytes read
 */
err_t EEPROM_Read(eeprom_dev_t *dev, size_t offset, void *ptr, size_t size);


/**
 * @brief write data in memory under given offset
 *
 * @param dev device descriptor
 * @param offset memory address
 * @param ptr data pointer
 * @param size size of the data to be written
 *
 * @return err_t error code
 */
err_t EEPROM_Write(eeprom_dev_t *dev, size_t offset, const void *ptr,
    size_t size);

#endif /* DEV_EEPROM_H */
