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

#define DEBUG DLVL_DEBUG
#include "debug.h"

/* queue for reception and transmission */
static queue_t *rxq, *txq;

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
	/* buffer for receiving transfers from the endpoint, we use static to use
	 * less memory from the task's stack */
	uint8_t buf[USB_EEM_RX_SIZE];
	/* error code */
	err_t ec;

	/* endless loop of listening on the endpoint */
	for (;; Yield()) {
		/* start the transfer */
		ec = USB_StartOUTTransfer(USB_EP3, buf, sizeof(buf), 0);
		/* transfer already started */
		if (ec < EOK && ec != EBUSY)
			continue;

		/* wait for the transfer to finish */
		ec = USB_WaitOUTTransfer(USB_EP3, 0);
		/* got some data? */
		if (ec > EOK)
			Queue_PutWait(rxq, buf, ec, 0);
	}
}

/* transmission task */
static void USBEEM_TxTask(void *arg)
{
	/* buffer for receiving transfers from the endpoint, we use static to use
	 * less memory from the task's stack */
	uint8_t buf[USB_EEM_TX_SIZE];
	/* error code */
	err_t ec;

	/* endless transmission loop */
	for (;; Yield()) {
		/* get data from transmission queue */
		size_t size = Queue_GetWait(txq, buf, sizeof(buf), 5);
		/* send frames */
		for (size_t offs = 0; offs != size; Yield()) {
			/* try to initiate the transfer */
			ec = USB_StartINTransfer(USB_EP3, buf + offs, size, 0);
			if (ec < EOK)
				continue;
			/* wait for the transfer to finish */
			ec = USB_WaitINTransfer(USB_EP3, 0);
			/* transfer completed */
			if (ec > EOK)
				offs += ec;
		}
	}
}

static void Test(void *arg)
{
	/* temporary buffer */
	uint8_t buf[10];

	/* endless test loop */
	for (;; Yield()) {
		/* wait for the data from the usb */
		err_t ec = USBEEM_Recv(buf, sizeof(buf) - 1, 5);
		/* reception did not work */
		if (ec <= EOK)
			continue;
		/* zero terminate */
		buf[ec] = 0;
		/* show what was received */
		dprintf_d("RX received data: %d %s\n", ec, buf);

		/* send the data back to the usb */
		ec = USBEEM_Send(buf, ec, 0);
		/* show that we responded back */
		dprintf_d("TX data echoed back: %d\n", ec);
	}
}

/* initialize virtual com port logic */
err_t USBEEM_Init(void)
{
	/* allocate space for both queues */
	rxq = Queue_Create(1, 128);
	txq = Queue_Create(1, 128);
	/* complain */
	assert(rxq && txq, "unable to allocate space for vcp queues");

	/* start tasks */
	Yield_Task(USBEEM_RxTask, 0, 1024);
	Yield_Task(USBEEM_TxTask, 0, 1024);

	/* listen to usb reset events */
	Ev_Subscribe(&usb_ev, USBEEM_USBCallback);

	// TODO: test task
	Yield_Task(Test, 0, 2048);

	/* report status */
	return EOK;
}

/* send data to virtual com port */
err_t USBEEM_Send(const void *ptr, size_t size, dtime_t timeout)
{
	/* put data to the queue */
	return Queue_PutWait(txq, ptr, size, timeout);
}

/* receive data from virtual com port */
err_t USBEEM_Recv(void *ptr, size_t size, dtime_t timeout)
{
	/* get data from the queue */
	return Queue_GetWait(rxq, ptr, size, timeout);
}