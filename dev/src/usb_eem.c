/**
 * @file usb_eem.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-09
 * 
 * @copyright Copyright (c) 2024
 */

#include "compiler.h"
#include "config.h"
#include "err.h"
#include "dev/usb.h"
#include "dev/usb_core.h"
#include "dev/usb_desc.h"
#include "dev/usb_eem.h"
#include "sys/sem.h"
#include "sys/sleep.h"
#include "sys/queue.h"
#include "util/minmax.h"
#include "util/string.h"
#include "util/elems.h"

#define DEBUG DLVL_DEBUG
#include "debug.h"

/* buffer element */
typedef struct buf {
	/* current size of the data being stored */
	size_t size, offs;
	/* payload size for the ethernet frame + the eem header */
	uint8_t pld[USBEEM_MAX_ETH_FRAME_LEN];
} buf_t;

/* buffers for rx frames and tx frames */
static buf_t rx[USBEEM_RX_BUF_CAPACITY], tx[USBEEM_TX_BUF_CAPACITY];
/* head and tail pointers ro the reception */
static uint32_t rx_head, rx_tail;
/* head and tail pointers for the transmission */
static uint32_t tx_head, tx_tail;


/* usb reset callback */
static void USBEEM_ResetCallback(void *arg)
{
	/* prepare fifos */
    /* Bulk IN (used for data transfers from device to host) */
	USB_SetTxFifoSize(USB_EP3, USB_EEM_TX_SIZE);
	/* flush fifo */
	USB_FlushTxFifo(USB_EP3);
	/* configure endpoints */
	USB_ConfigureINEndpoint(USB_EP3, USB_EPTYPE_BULK, USB_EEM_TX_SIZE);
	USB_ConfigureOUTEndpoint(USB_EP3, USB_EPTYPE_BULK, USB_EEM_RX_SIZE);
}

/* usb callback */
static void USBEEM_USBCallback(void *arg)
{
    /* cast event argument */
    usb_evarg_t *ea = arg;
    /* processing according to event type */
    switch (ea->type) {
    case USB_EVARG_TYPE_RESET : USBEEM_ResetCallback(arg); break;
    }
}

/* reception task */
static void USBEEM_RxTask(void *arg)
{
	/* buffer for receiving transfers. in eem synchronization takes place at
	 * transfer level, so you cannot accept less than one ethernet frame worth
	 * of data */
	static uint8_t transfer[1518 + 4 + sizeof(usbeem_hdr_t)];
	/* extracted fields from the header */
	int type, cmd; size_t pld_len;

	/* endless loop of listening on the endpoint */
	for (;; Yield()) {
		/* start the transfer */
		err_t ec = USB_StartOUTTransfer(USB_EP3, transfer, sizeof(transfer), 0);
		/* transfer already started */
		if (ec < EOK)
			continue;
		/* wait for the transfer to finish */
		ec = USB_WaitOUTTransfer(USB_EP3, 0);
		/* transfer finished but with an error */
		if (ec < EOK)
			continue;

		/* show that we have received a frame */
		dprintf_d("RX: processing transfer: size  %d\n", ec);
		/* process all the data from the transfer */
		for (size_t size = ec, offs = 0; offs < size;) {
			/* point to the beginning of eem frame */
			usbeem_frame_t *frame = (usbeem_frame_t *)(transfer + offs);

			/* extract the fields */
			type = frame->hdr & USBEEM_HDR_TYPE, cmd =
				frame->hdr & USBEEM_HDR_CMD;

			/* do we have a payload that follows? */
			if (type == USBEEM_HDR_TYPE_DATA) {
				pld_len = (frame->hdr & USBEEM_HDR_DATA_LENGTH) >>
					LSB(USBEEM_HDR_DATA_LENGTH);
			/* echo commands also carry payload */
			} else if (cmd == USBEEM_HDR_CMD_ECHO_REQ ||
				cmd == USBEEM_HDR_CMD_ECHO_RESP) {
				pld_len = (frame->hdr & USBEEM_HDR_ECHO_LENGTH) >>
					LSB(USBEEM_HDR_ECHO_LENGTH);
			/* no payload */
			} else {
				pld_len = 0;
			}

			/* we only support data frames c'mon bro */
			if (type != USBEEM_HDR_TYPE_DATA)
				goto next_frame;

			/* we need to have at least four bytes for the ethernet checksum,
			 * if not then this frame is malformed */
			if (pld_len < 4)
				break;

			/* transfer the ethernet data to the buffer */
			/* wait for the rx buffer slot to become empty */
			for (; rx_head - rx_tail == elems(rx); Yield());
			/* get the pointer to the buffer slot */
			buf_t *buf = &rx[rx_head % elems(rx)];

			/* limit the copy size so that we do not overflow, also skip the
			 * checksum */
			size_t copy_size = min(sizeof(buf->pld), pld_len - 4);
			/* copy the data */
			memcpy(buf->pld, frame->pld, copy_size);
			/* store the size */
			buf->size = copy_size;
			/* mark buffer as busy */
			rx_head++;

			/* display debug */
			dprintf_d("RX: size %d out of %d\n", copy_size, pld_len);
			/* update the offset */
			next_frame: offs += pld_len + sizeof(*frame);
		}
	}
}

/* transmission task */
static void USBEEM_TxTask(void *arg)
{
	/* buffer for receiving transfers. in eem synchronization takes place at
	 * transfer level, so you cannot accept less than one ethernet frame worth
	 * of data */
	static uint8_t transfer[1518 + 4 + sizeof(usbeem_hdr_t)];


	/* endless transmission loop */
	for (;; Yield()) {
		/* offset within the transfer */
		size_t offs, cnt = 0;

		/* eem allows to pack as many frames as you want in a single transfer */
		for (offs = 0; tx_head != tx_tail; ) {
			/* get the buffer pointer */
			buf_t *buf = &tx[tx_tail % elems(tx)];
			/* compute the size of the usb data that we'll need to send because
			 * of this frame. additional 4 bytes come from ethernet checksum */
			size_t frame_size = buf->size + sizeof(usbeem_hdr_t) + 4;
			/* we cannot add this frame to the transfer */
			if (frame_size > sizeof(transfer) - offs)
				break;

			/* map the header onto the transfer buffer */
			usbeem_frame_t *frame = (usbeem_frame_t *)(transfer + offs);
			/* compose the header */
			frame->hdr = USBEEM_HDR_TYPE_DATA | USBEEM_HDR_DATA_CRC_DEADBEEF |
				((buf->size + 4) << LSB(USBEEM_HDR_DATA_LENGTH));
			/* copy the payload */
			memcpy(frame->pld, buf->pld, buf->size);
			/* add bogus checksum */
			frame->pld[buf->size + 0] = 0xde;
			frame->pld[buf->size + 1] = 0xad;
			frame->pld[buf->size + 2] = 0xbe;
			frame->pld[buf->size + 3] = 0xef;
			/* update the offset, consume the frame */
			offs += frame_size; tx_tail++; cnt++;

			if (cnt > 1)
				dprintf_d("sending frame no %d\n", cnt);
		}
		/* no frames to be sent */
		if (!offs)
			continue;

		/* try to send, if unsuccesfull, bail */
		if (USB_StartINTransfer(USB_EP3, transfer, offs, 0) < EOK)
			continue;
		/* wait for the transfer to finish */
		USB_WaitINTransfer(USB_EP3, 0);
	}
}

/* initialize virtual com port logic */
err_t USBEEM_Init(void)
{
	/* start tasks */
	Yield_Task(USBEEM_RxTask, 0, 1024);
	Yield_Task(USBEEM_TxTask, 0, 1024);

	/* listen to usb reset events */
	Ev_Subscribe(&usb_ev, USBEEM_USBCallback);

	/* report status */
	return EOK;
}

/* receive data from virtual com port */
err_t USBEEM_Recv(void *ptr, size_t size, dtime_t timeout)
{
	/* waiting loop */
	for (dtime_t ts = time(0); rx_head - rx_tail == 0; Yield()) {
		/* support for timoeut */
		if (timeout && dtime_now(ts) > timeout)
			return ETIMEOUT;
		/* support for cancellation */
		if (Yield_IsCancelled())
			return ECANCEL;
		/* interface is inactive */
		if (!USBCore_IsConfigured())
			return EUSB_INACTIVE;
	}

	/* get the buffer pointer */
	buf_t *buf = &rx[rx_tail % elems(rx)];

	/* limit the size */
	size = min(size, buf->size);
	/* copy data */
	memcpy(ptr, buf->pld, size);
	/* mark as free */
	rx_tail++;

	/* return the size of the data */
	return size;
}

/* send the data over the eem */
err_t USBEEM_Send(const void *ptr, size_t size, dtime_t timeout)
{
	/* frame to be sent is wayy to big */
	if (size > USBEEM_MAX_ETH_FRAME_LEN)
		return EARGVAL;

	/* wait for the transmission buffer to become empty */
	for (dtime_t ts = time(0); tx_head - tx_tail == elems(tx); Yield()) {
		/* support for timoeut */
		if (timeout && dtime_now(ts) > timeout)
			return ETIMEOUT;
		/* support for cancellation */
		if (Yield_IsCancelled())
			return ECANCEL;
		/* interface is inactive */
		if (!USBCore_IsConfigured())
			return EUSB_INACTIVE;
	}

	/* get the pointer to the buffer element that we are about to fill */
	buf_t *buf = &tx[tx_head % elems(tx)];
	/* store the payload */
	memcpy(buf->pld, ptr, buf->size = size);
	/* mark as ready to be sent */
	tx_head++;

	/* return the size of the data */
	return size;
}
