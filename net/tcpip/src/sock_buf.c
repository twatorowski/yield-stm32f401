/**
 * @file sock_buf.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-10
 * 
 * @brief TCPIP Stack: Socket utilities
 */

#include "assert.h"
#include "err.h"
#include "util/minmax.h"
#include "util/string.h"

/* return the number of bytes used */
static err_t TCPIPSock_BufUsed(tcpip_sock_buf_t *buf)
{
    return buf->head - buf->tail;
}

/* return the number of bytes free */
static err_t TCPIPSock_BufFree(tcpip_sock_buf_t *buf)
{
    return buf->size - TCPIPTcpSock_BufUsed(buf);
}

/* write data into output buffer */
static err_t TCPIPSock_BufWrite(tcpip_sock_buf_t *buf, const void *src, 
    size_t size)
{
    /* byte-wise data pointer */
    const uint8_t *p8 = src;

    /* get the free space and head element pointer */
    size_t b_free = buf->size - (buf->head - buf->tail);
    size_t b_head = buf->head % buf->size;
    /* limit the number of bytes to be written before/after wrapping */
    size_t b_write = min(size, b_free);
    size_t b_wrap = min(b_write, buf->size - b_head);

    /* store data */
    memcpy(buf->ptr + b_head, p8, b_wrap);
    memcpy(buf->ptr, p8 + b_wrap, b_write - b_wrap);

    /* return the number of bytes write */
    return b_write;
}

/* commit data that was previously written into buffer */
static err_t TCPIPTcpSock_BufCommit(tcpip_sock_buf_t *buf, size_t size)
{
    /* size sanity check */
    assert(buf->size - (buf->head - buf->tail) >= size, 
        "buffer size mismatch", 0);
    /* advance the tail pointer */
    buf->tail += size;
    /* return the number of bytes dropped */
    return size;
}

/* read data from buffer */
static err_t TCPIPTcpSock_BufRead(tcpip_sock_buf_t *buf, void *dst, 
    size_t size)
{
    /* byte-wise data pointer */
    uint8_t *p8 = dst;
    
    /* get the number of data bytes that are buffered and the index of the 
     * tail pointer */
    size_t b_buff = buf->head - buf->tail;
    size_t b_tail = buf->tail % buf->size;
    /* limit the number of bytes to read before and after the buffer wraps */
    size_t b_read = min(size, b_buff);
    size_t b_wrap = min(b_read, buf->size - b_tail);

    /* fill in the data */
    memcpy(p8, buf->ptr + b_tail, b_wrap);
    memcpy(p8 + b_wrap, buf->ptr, b_read - b_wrap);

    /* return the number of bytes read */
    return b_read;
}