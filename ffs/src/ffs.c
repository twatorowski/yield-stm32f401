/**
 * @file ffs.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "assert.h"
#include "err.h"
#include "ffs/ffs.h"
#include "util/bit.h"
#include "util/string.h"
#include "util/elems.h"
#include "util/minmax.h"
#include "sys/yield.h"


/* file descriptor array providers */
extern const ffs_file_desc_t *ffs_fda_www[];
/* all of the files */
static const ffs_file_desc_t **files[] = {
    ffs_fda_www,
    0,
};

/* pool of file access blocks */
static ffs_file_t fpool[32];

/* initialize the flash file system */
err_t FFS_Init(void)
{
    /* return status */
    return EOK;
}

/* open a file */
ffs_file_t * FFS_Open(const char *name, ffs_mode_t mode)
{
    /* file descriptor array lookup pointer */
    const ffs_file_desc_t ***fda = files, **fd;
    /* sanitize the args */
    if (!mode || !name)
        return 0;

    /* look for the file with given name */
    for (fda = files; *fda; fda++) {
        /* look within the array */
        for (fd = *fda; *fd && strcmp((*fd)->name, name); fd++);
        /* found the entry with matching name */
        if (*fd)
            break;
    }

    /* no such file exist or file cannot be opened in this mode */
    if (!(*fd) || (mode & ~(*fd)->mode))
        return 0;

    /* file pool pointer */
    ffs_file_t *fp = fpool;
    /* look for the unused file descriptor */
    for (; fp != fpool + elems(fpool) && fp->used; fp++);
    /* no such descriptor found */
    if (fp == fpool + elems(fpool))
        return 0;
    
    /* setup the data in the file pointer */
    *fp = (ffs_file_t) { .fd = (ffs_file_desc_t *)*fd, .mode = mode, 
        .position = 0, .used = 1, .task_id = Yield_GetTaskID() };
    /* return the prepared file pointer */
    return fp;
}

/* read a chunk of a file */
err_t FFS_Read(ffs_file_t *fp, void *ptr, size_t size)
{
    /* no valid file pointer */
    if (!fp || !(fp->mode & FFS_MODE_R))
        return EFATAL;
    /* limit the size */
    size = min(size, fp->fd->size - fp->position);
    /* do the read */
    memcpy(ptr, (uint8_t *)fp->fd->ptr + fp->position, size);
    /* move the position around */
    fp->position += size;
    /* return the size that was read */
    return size;
} 

/* write to a file */
err_t FFS_Write(ffs_file_t *fp, const void *ptr, size_t size)
{
    /* no valid file pointer */
    if (!fp || !(fp->mode & FFS_MODE_W))
        return EFATAL;
    
    /* do the write read */
    memcpy((uint8_t *)fp->fd->ptr + fp->position, ptr, size);
    /* move the offset around */
    fp->position += size;
    /* update the size if needed */
    if (fp->position > fp->fd->size)
        fp->fd->size = fp->position;
    /* return the size that was read */
    return size;
}


/* move the cursor within the file */
err_t FFS_Seek(ffs_file_t *fp, size_t offset, ffs_seek_mode_t mode)
{
    /* no valid file pointer */
    if (!fp)
        return EFATAL;
    
    /* we are doing everything in unsigned arithmetics. there are some caveats 
     * to that though :)*/
    size_t fsize = fp->fd->size, fpos = fp->position, new_pos;

    /* switch on the mode of seeking */
    switch (mode) {
    case FFS_SEEK_SET: new_pos = offset; break;
    case FFS_SEEK_CUR: new_pos = fpos + offset; break;
    case FFS_SEEK_END: new_pos = fsize + offset; break;
    default: assert(0, "unknown seek mode ");
    }
    /* since size_t is unsigned we do not have to worry about 0 comparison :)
     * i know it's a cheesy way to implement it */
    if (new_pos > fsize)
        return EFATAL;
    /* store the new position */
    fp->position = new_pos;
    /* return the status */
    return EOK;
}

/* tell the position within the file */
err_t FFS_Tell(ffs_file_t *fp, size_t *pos)
{
    /* no a valid file pointer */
    if (!fp) 
        return EFATAL;
    /* get the position of the file pointer */
    if (pos) 
        *pos = fp->position;
    /* report status */
    return EOK;
}

/* get the file size */
err_t FFS_Size(ffs_file_t *fp, size_t *size)
{
    /* no a valid file pointer */
    if (!fp) 
        return EFATAL;
    /* get the position of the file pointer */
    if (size) 
        *size = fp->fd->size;
    /* report status */
    return EOK;
}

/* close a file */
err_t FFS_Close(ffs_file_t *fp)
{
    /* not a valid file pointer */
    if (!fp || !fp->used)
        return EFATAL;

    /* mark the entry as being unused */
    fp->used = 0;
    /* return the status */
    return EOK;
}
