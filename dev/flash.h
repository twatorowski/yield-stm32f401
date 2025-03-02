/**
 * @file flash.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-02
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_FLASH_H
#define DEV_FLASH_H

#include <stddef.h>
#include "err.h"

/* flash page record */
typedef struct flash_sector {
    /* address and size of the flash page */
    uint32_t addr, size;
} flash_sector_t;


/**
 * @brief initialize flash memory driver
 *
 * @return err_t error code
 */
err_t Flash_Init(void);


/**
 * @brief get the sector id for given address
 *
 * @param addr address within the sector
 * @return err_t error code or the id of the sector if the address is correct
 */
err_t Flash_GetSectorIDForAddr(uint32_t addr);

/**
 * @brief return the information about memory sector
 *
 * @param sector_id sector id
 * @param sector placeholder for the information
 *
 * @return err_t error code
 */
err_t Flash_GetSectorInfo(int sector_id, flash_sector_t *sector);

/**
 * @brief erases a single sector
 *
 * @param sector_id sector identifier
 * @return err_t error code
 */
err_t Flash_EraseSector(int sector_id);

/**
 * @brief erases all sectors that are located within the specified address range
 *
 * @param ptr range start address
 * @param size size of the memory range
 *
 * @return err_t error code
 */
err_t Flash_EraseSectorsForAddressRange(const void *ptr, size_t size);

/**
 * @brief read the memory contents
 *
 * @param dst destination pointer
 * @param src flash memory pointer
 * @param size size of the memory to be read
 *
 * @return err_t error code
 */
err_t Flash_Read(void *dst, const void *src, size_t size);

/**
 * @brief write data to flash memory
 *
 * @param dst destination address
 * @param src source address
 * @param size data size
 *
 * @return err_t error code
 */
err_t Flash_Write(void *dst, const void *src, size_t size);

/**
 * @brief returns EOK when contents of the flash match the contents of the ram
 *
 * @param flash flash data pointer
 * @param ram ram data pointer
 * @param size size of the areas to compare
 * @return err_t EOK if contents match
 */
err_t Flash_Verify(const void *flash, const void *ram, size_t size);


#endif /* DEV_FLASH_H */
