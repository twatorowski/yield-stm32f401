/**
 * @file ffs.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef FFS_FFS_H
#define FFS_FFS_H

#include <stddef.h>

#include "err.h"
#include "util/bit.h"

/** modes of the file opening  */
typedef enum ffs_mode {
    FFS_MODE_R = BIT_VAL(0),
    FFS_MODE_W = BIT_VAL(1),
    FFS_MODE_RW = FFS_MODE_R | FFS_MODE_W,
} ffs_mode_t;

/** modes of moving file pointer offset  */
typedef enum ffs_seek_mode {
    FFS_SEEK_SET,
    FFS_SEEK_CUR,
    FFS_SEEK_END,
} ffs_seek_mode_t;

/** file descriptor */
typedef struct ffs_file_desc {
    /* name of the file (complete path) */
    const char *name;
    /* allowed modes */
    ffs_mode_t mode;
    /* size of the file */
    size_t size;
    /* data pointer */
    const void *ptr;
} ffs_file_desc_t;

/** file access struct */
typedef struct ffs_file {
    /* flag that indicates whether this entry is used */
    int used, task_id;
    /* file descriptor */
    ffs_file_desc_t *fd;
    /* mode of file open */
    ffs_mode_t mode;
    /* current position within the file */
    size_t position;
} ffs_file_t;


/**
 * @brief initialize the flash file system 
 * 
 * @return err_t error code
 */
err_t FFS_Init(void);

/**
 * @brief open a file
 * 
 * @param name name of the file
 * @param mode mode of opening
 * 
 * @return ffs_file_t * file pointer
 */
ffs_file_t * FFS_Open(const char *name, ffs_mode_t mode);

/**
 * @brief read a chunk of a file
 * 
 * @param fp file pointer
 * @param ptr pointer to a place where we store the data
 * @param size size of the data
 * 
 * @return err_t error code or the numbers of bytes read
 */
err_t FFS_Read(ffs_file_t *fp, void *ptr, size_t size);

/**
 * @brief write to a file 
 * 
 * @param fp @param fp file pointer
 * @param ptr pointer to source the data from
 * @param size size of the data to be written
 * 
 * @return err_t error code or the number of bytes that were written
 */
err_t FFS_Write(ffs_file_t *fp, const void *ptr, size_t size);

/**
 * @brief move the cursor within the file
 * 
 * @param fp file pointer
 * @param offset cursor offset
 * @param mode mode of readjusting the cursor
 * 
 * @return err_t error code
 */
err_t FFS_Seek(ffs_file_t *fp, size_t offset, ffs_seek_mode_t mode);

/**
 * @brief tell the position within the file 
 * 
 * @param fp file pointer
 * @param pos position within a file
 * 
 * @return err_t error code
 */
err_t FFS_Tell(ffs_file_t *fp, size_t *pos);

/**
 * @brief get the file size
 * 
 * @param fp file pointer
 * @param size file size
 * 
 * @return err_t error code
 */
err_t FFS_Size(ffs_file_t *fp, size_t *size);

/**
 * @brief close a file
 * 
 * @param fp file pointer
 * 
 * @return err_t error code
 */
err_t FFS_Close(ffs_file_t *fp);

#endif /* FFS_FFS_H */
