/**
 * @file vectors.h
 * 
 * @date 2019-09-19
 * @author twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief MCU interrupt/exception vector table
 */

#ifndef VECTORS_H
#define VECTORS_H

/* this typedef is used to represent a vector entry in array */
typedef union vector_entry {
    /* 'object' pointer */
    void *v;
    /* 'function' pointer */
    void (*f)(void);
} vector_entry_t;

/* flash vector table */
extern vector_entry_t flash_vectors[];

#endif /* VECTORS_H */
