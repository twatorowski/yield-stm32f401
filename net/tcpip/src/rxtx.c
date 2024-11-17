/**
 * @file rxtx.c
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-28
 *
 * @brief TCP/IP Stack: transmission and reception routines
 */

#include <stddef.h>

#include "assert.h"
#include "compiler.h"
#include "config.h"
#include "err.h"

#include "net/tcpip/eth.h"
#include "net/tcpip/tcpip.h"
#include "sys/yield.h"
#include "util/elems.h"

#define DEBUG
#include "debug.h"

/* underlying device */
#include "dev/usb_eem.h"

/* buffer for reception */
static uint8_t rx_buf[TCPIP_RXTX_BUF_SIZE];
/* size of tx buffer */
static int rx_size;

/* transmission buffers */
static struct tx_buf {
    /* frame size */
    size_t size;
    /* element is allocated */
    int allocated;
    /* buffer */
    uint8_t ALIGNED(4) buf[TCPIP_RXTX_BUF_SIZE];
} tx_buf[4];

/* reception task for the tcp/ip stack */
void TCPIPRxTx_RxTask(void *arg)
{
    /* frame descriptor */
    static tcpip_frame_t frame;
    /* infinite loop */
    for (;; Yield()) {
        /* receive frame from the ethernet interface */
        rx_size = USBEEM_Recv(rx_buf, sizeof(rx_buf), 1);

        /* valid reception with no errors? */
        if (rx_size > 0) {
            /* setup frame descriptor */
            frame.flags = 0;
            frame.bufid = 0;
            frame.ptr = rx_buf;
            frame.size = rx_size;
            /* put the frame on the stack */
            TCPIPEth_Input(&frame);
        }
    }
}

/* sender task */
void TCPIPRxTx_TxTask(void *arg)
{
    /* endless loop */
    for (;; Yield()) {
        /* transmission buffer to be sent */
        struct tx_buf *t;
        
        /* go through all allocated elements */
        for (t = tx_buf; t != tx_buf + elems(tx_buf); t++) {
            /* buffer is unallocated or not filled */
            if (!t->allocated || !t->size)
                continue;
            /* send frame over ethernet interface */
            USBEEM_Send(t->buf, t->size, 0);
            /* clear the buffer */
            t->size = 0; t->allocated = 0;
        }
    }
}

/* initialize underlying physical interface */
err_t TCPIPRxTx_Init(void)
{
    /* create reception task didas */
    Yield_Task(TCPIPRxTx_RxTask, 0, 2048);
    Yield_Task(TCPIPRxTx_TxTask, 0, 1024);
    /* report status */
    return EOK;
}

/* allocate buffer for sending data */
err_t TCPIPRxTx_Alloc(tcpip_frame_t *frame)
{
    /* transmission buffer to be assigned to the caller */
    struct tx_buf *t = tx_buf + elems(tx_buf);

    /* loop until free transmission buffer is found */
    for (; t == tx_buf + elems(tx_buf); Yield()) {
        /* look for an element that is not occupied */
        for (t = tx_buf; t != tx_buf + elems(tx_buf) && t->allocated; t++);
    }

    /* mark as occupied */
    t->allocated = 1;
    /* setup the frame descriptor structure */
    frame->flags = 0;
    frame->ptr = t->buf;
    frame->size = sizeof(t->buf);
    frame->bufid = t - tx_buf;

    /* report success */
    return EOK;
}

/* drop the given frame */
err_t TCPIPRxTx_Drop(tcpip_frame_t *frame)
{
    struct tx_buf *t = &tx_buf[frame->bufid];
    /* clear the buffer */
    t->size = 0;
    t->allocated = 0;
    /* nothing can fail here ;-) */
    return EOK;
}

/* stack's output routine */
err_t TCPIPRxTx_Send(tcpip_frame_t *frame)
{
    /* result code */
    err_t rc = EOK;

    /* mark as ready to be sent */
    tx_buf[frame->bufid].size = frame->size;

    /* return the status code */
    return rc;
}
