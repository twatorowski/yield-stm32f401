/**
 * @file usb_vcp.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-12
 * 
 * @copyright Copyright (c) 2024
 */

/**
 * @file usb_vcp.c
 *
 * @date 2019-12-07
 * @author twatorowski
 *
 * @brief Minimalistic Virtual Com Port implementation that works over the
 * usbcore.c
 */

#include "compiler.h"
#include "config.h"
#include "err.h"
#include "dev/usb.h"
#include "dev/usb_core.h"
#include "dev/usb_desc.h"
#include "dev/usb_vcp.h"
#include "sys/sem.h"
#include "sys/sleep.h"
#include "sys/queue.h"
#include "util/minmax.h"
#include "util/string.h"

#define DEBUG
#include "debug.h"


/* line encoding */
typedef struct {
	/* current baudrate */
	uint32_t bauds;
	/* stop bits */
	uint8_t stop_bits;
	/* parity type */
	uint8_t parity_type;
	/* data bits */
	uint8_t data_bits;
} PACKED le_t;
/* current line encoding: 115200bps, 1 stop bit, no parity, 8 bits */
static le_t le = {115200, 1, 0, 8};
/* queue for reception and transmission */
static queue_t *rxq, *txq;

/* request callback: handle all special requests */
static void USBVCP_RequestCallback(void *arg)
{
	/* event argument */
	usbcore_req_evarg_t *a = arg;
	/* setup frame that cauesd this event */
	usb_setup_t *s = a->setup;

	/* some debug */
	dprintf("index = 0x%x, len = %d, r = 0x%x, r_type = 0x%x, val = 0x%x\n",
	    s->index, s->length, s->request, s->request_type, s->value);

	/* switch on request type */
	switch (s->request) {
	/* set line encoding */
	case USB_VCP_REQ_SET_LINE_CODING : {
		/* still waiting for the data stage */
		if (a->ptr == 0) {
			/* set where to store data to */
			a->ptr = (void *)&le, a->size = sizeof(le);
		/* got data */
		} else	{
			/* TODO: apply line parameters */
		}
		/* set status */
		a->status = EOK;
	} break;
	/* get line encoding */
	case USB_VCP_REQ_GET_LINE_CODING : {
		/* set returned data */
		a->ptr = (void *)&le, a->size = sizeof(le);
		a->status = EOK;
	} break;
	/* set control line state */
	case USB_VCP_SET_CONTROL_LINE_STATE : {
		a->status = EOK;
	} break;
	}
}

// static uint8_t buf[64];

// static void INCB(usb_cbarg_t *arg);
// static void OUTCB(usb_cbarg_t *arg);

// /* tx callback */
// static void INCB(usb_cbarg_t *arg)
// {
// 	if (arg->error)
// 		return;
// 	size_t size = arg->size;
// 	dprintf("TX! %d\n", size);
// 	USB_StartOUTTransfer(USB_EP3, buf, sizeof(buf)-1, OUTCB);
// }

// /* rx callback */
// static void OUTCB(usb_cbarg_t *arg)
// {
// 	if (arg->error)
// 		return;

// 	size_t size = arg->size;
// 	buf[size] = 0;
// 	dprintf("RX! %d %s\n", size, buf);
// 	USB_StartINTransfer(USB_EP3, buf, size, INCB);
// }

/* usb reset callback */
static void USBVCP_ResetCallback(void *arg)
{
	/* prepare fifos */
    /* interrupt transfers */
	USB_SetTxFifoSize(USB_EP2, USB_VCP_INT_SIZE / 4);
    /* Bulk IN (used for data transfers from device to host) */
	USB_SetTxFifoSize(USB_EP3, USB_VCP_TX_SIZE / 4);
	/* flush fifos */
	USB_FlushTxFifo(USB_EP2);
	USB_FlushTxFifo(USB_EP3);
	/* configure endpoints */
	USB_ConfigureINEndpoint(USB_EP2, USB_EPTYPE_INT, USB_VCP_INT_SIZE);
	USB_ConfigureINEndpoint(USB_EP3, USB_EPTYPE_BULK, USB_VCP_TX_SIZE);
	USB_ConfigureOUTEndpoint(USB_EP3, USB_EPTYPE_BULK, USB_VCP_RX_SIZE);

	// USB_StartOUTTransfer(USB_EP3, buf, sizeof(buf)-1, OUTCB);
}

/* usb callback */
static void USBVCP_USBCallback(void *arg)
{
    /* cast event argument */
    usb_evarg_t *ea = arg;
    /* processing according to event type */
    switch (ea->type) {
    case USB_EVARG_TYPE_RESET : USBVCP_ResetCallback(arg); break;
    }
}

/* reception task */
static void USBVCP_RxTask(void *arg)
{
	/* buffer for receiving transfers from the endpoint, we use static to use
	 * less memory from the task's stack */
	uint8_t buf[USB_VCP_RX_SIZE];
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
static void USBVCP_TxTask(void *arg)
{
	/* buffer for receiving transfers from the endpoint, we use static to use
	 * less memory from the task's stack */
	uint8_t buf[USB_VCP_TX_SIZE];
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
		err_t ec = USBVCP_Recv(buf, sizeof(buf) - 1, 5);
		/* reception did not work */
		if (ec <= EOK)
			continue;
		/* zero terminate */
		buf[ec] = 0;
		/* show what was received */
		dprintf("RX received data: %d %s\n", ec, buf);

		/* send the data back to the usb */
		ec = USBVCP_Send(buf, ec, 0);
		/* show that we responded back */
		dprintf("TX data echoed back: %d\n", ec);
	}
}

/* initialize virtual com port logic */
err_t USBVCP_Init(void)
{
	/* allocate space for both queues */
	rxq = Queue_Create(1, 128);
	txq = Queue_Create(1, 128);
	/* complain */
	assert(rxq && txq, "unable to allocate space for vcp queues");

	/* start tasks */
	Yield_Task(USBVCP_RxTask, 0, 1024);
	Yield_Task(USBVCP_TxTask, 0, 1024);

	/* listen to usb reset events */
	Ev_Subscribe(&usb_ev, USBVCP_USBCallback);
	/* listen to control transfers */
	Ev_Subscribe(&usbcore_req_ev, USBVCP_RequestCallback);

	// TODO: test task
	Yield_Task(Test, 0, 2048);

	/* report status */
	return EOK;
}

/* send data to virtual com port */
err_t USBVCP_Send(const void *ptr, size_t size, dtime_t timeout)
{
	/* put data to the queue */
	return Queue_PutWait(txq, ptr, size, timeout);
}

/* receive data from virtual com port */
err_t USBVCP_Recv(void *ptr, size_t size, dtime_t timeout)
{
	/* get data from the queue */
	return Queue_GetWait(rxq, ptr, size, timeout);
}