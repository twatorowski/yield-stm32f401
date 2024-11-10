/**
 * @file usb_vcp.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024
 */

#include "compiler.h"
#include "dev/usb.h"
#include "dev/usb_core.h"
#include "dev/usb_vcp.h"

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
static le_t le = { .bauds = 115200, .stop_bits = 1,
    .parity_type = 0, .data_bits = 8 };

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
			/* apply line parameters */
		}
		/* set status */
		a->ec = EOK;
	} break;
	/* get line encoding */
	case USB_VCP_REQ_GET_LINE_CODING : {
		/* set returned data */
		a->ptr = (void *)&le, a->size = sizeof(le), a->ec = EOK;
	} break;
	/* set control line state */
	case USB_VCP_SET_CONTROL_LINE_STATE : {
		a->ec = EOK;
	} break;
	}
}

/* usb reset callback */
static void USBVCP_ResetCallback(void *arg)
{
	/* prepare fifos */
    /* interrupt transfers */
	USB_SetTxFifoSize(USB_EP1, USB_VCP_INT_SIZE / 4);
    /* Bulk IN (used for data transfers from device to host) */
	USB_SetTxFifoSize(USB_EP2, USB_VCP_TX_SIZE / 4);
	/* flush fifos */
	USB_FlushTxFifo(USB_EP1);
	USB_FlushTxFifo(USB_EP2);
	/* configure endpoints */
	USB_ConfigureINEndpoint(USB_EP1, USB_EPTYPE_INT, USB_VCP_INT_SIZE);
	USB_ConfigureINEndpoint(USB_EP2, USB_EPTYPE_BULK, USB_VCP_TX_SIZE);
	USB_ConfigureOUTEndpoint(USB_EP2, USB_EPTYPE_BULK, USB_VCP_RX_SIZE);
}

/* usb device callback */
static void USBVCP_USBCallback(void *arg)
{
    /* cast event argument */
    usb_evarg_t *ea = arg;
    /* processing according to event type */
    switch (ea->type) {
    case USB_EVARG_TYPE_RESET : USBVCP_ResetCallback(arg); break;
    }
}

/* initialize virtual com port logic */
int USBVCP_Init(void)
{
	/* listen to usb reset events */
	Ev_Subscribe(&usb_ev, USBVCP_USBCallback);
	Ev_Subscribe(&usbcore_req_ev, USBVCP_RequestCallback);

	/* report status */
	return EOK;
}

/* send the data over the usb */
err_t USBVCP_Send(const void *ptr, size_t size, dtime_t timeout)
{
	/* do the in transfer on the endpoint */
	return USBCore_DataIN(USB_EP2, ptr, size, timeout);
}

/* receive the data over the usb */
err_t USBVCP_Recv(void *ptr, size_t size, dtime_t timeout)
{
	/* do the out transfer on the endpoint */
	return USBCore_DataOUT(USB_EP2, ptr, size, timeout);
}