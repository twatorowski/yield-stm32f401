/**
 * @file heap.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-02
 * 
 * @brief Dynamic memory allocation
 */

#ifndef SYS_HEAP_H
#define SYS_HEAP_H

#include <stddef.h>

#include "err.h"

/**
 * @brief Initialize dynamic memory allocation
 * 
 * @return err_t error code
 */
err_t Heap_Init(void);

/**
 * @brief  Allocate a block of memory
 * 
 * @param size size of the allocated block
 * 
 * @return void * pointer to allocated memory
 */
void * Heap_Malloc(size_t size);

/**
 * @brief Free up previously allocated block of memory
 * 
 * @param ptr pointer to memory block
 */
void Heap_Free(void *ptr);

/**
 * @brief check heap integrity 
 * 
 */
err_t Heap_CheckIntegrity(void);

#endif /* SYS_HEAP_H */
