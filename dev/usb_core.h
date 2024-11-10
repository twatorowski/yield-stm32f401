/**
 * @file usb_core.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-01
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef DEV_USB_CORE_H
#define DEV_USB_CORE_H

#include "err.h"
#include "compiler.h"

#include "err.h"

/* Mask to get recipient */
#define USBCORE_SETUP_REQTYPE_RECIPIENT			0x1F
/* Recipient device */
#define USBCORE_SETUP_REQTYPE_RECIPIENT_DEVICE	0x00
/* Recipient interface */
#define USBCORE_SETUP_REQTYPE_RECIPIENT_IFACE	0x01
/* Recipient endpoint */
#define USBCORE_SETUP_REQTYPE_RECIPIENT_EP		0x02
/* Other recipient */
#define USBCORE_SETUP_REQTYPE_RECIPIENT_OTHER	0x03

/* type mask */
#define USBCORE_SETUP_REQTYPE_TYPE				0x60
/* standard request */
#define USBCORE_SETUP_REQTYPE_TYPE_STANDARD		0x00
/* class specific request */
#define USBCORE_SETUP_REQTYPE_TYPE_CLASS		0x20
/* vendor specific request */
#define USBCORE_SETUP_REQTYPE_TYPE_VENDOR		0x40
/* reserved */
#define USBCORE_SETUP_REQTYPE_TYPE_RESERVED		0x60

/* direction bit */
#define USBCORE_SETUP_REQTYPE_DIR				0x80
/* from host to device */
#define USBCORE_SETUP_REQTYPE_DIR_OUT			0x00
/* from device to host */
#define USBCORE_SETUP_REQTYPE_DIR_IN			0x80

/* request field  */
/* get status */
#define USBCORE_SETUP_REQ_GET_STATUS			0x00
/* clear feature */
#define USBCORE_SETUP_REQ_CLEAR_FEATURE			0x01
/* set feature */
#define USBCORE_SETUP_REQ_SET_FEATURE			0x03
/* set device address */
#define USBCORE_SETUP_REQ_SET_ADDRESS			0x05
/* get descriptor */
#define USBCORE_SETUP_REQ_GET_DESCRIPTOR		0x06
/* set descriptor */
#define USBCORE_SETUP_REQ_SET_DESCRIPTOR		0x07
/* get status */
#define USBCORE_SETUP_REQ_GET_CONFIGURATION		0x08
/* set configuration */
#define USBCORE_SETUP_REQ_SET_CONFIGURATION		0x09
/* get interface */
#define USBCORE_SETUP_REQ_GET_INTERFACE			0x0A
/* set interface */
#define USBCORE_SETUP_REQ_SET_INTERFACE			0x0B
/* synch frame */
#define USBCORE_SETUP_REQ_SYNCH_FRAME			0x0C

/* descriptor types */
/* device descriptor */
#define USBCORE_SETUP_DESCTYPE_DEVICE			0x01
/* configuration descriptor */
#define USBCORE_SETUP_DESCTYPE_CONFIGURATION	0x02
/* string descriptor */
#define USBCORE_SETUP_DESCTYPE_STRING			0x03
/* interface descriptor */
#define USBCORE_SETUP_DESCTYPE_INTERFACE		0x04
/* enpoint descritpro */
#define USBCORE_SETUP_DESCTYPE_ENDPOINT			0x05
/* device qualifier descriptor */
#define USBCORE_SETUP_DESCTYPE_QUALIFIER		0x06

/* standard features */
/* halt endpoint */
#define USBCORE_SETUP_FEATURE_ENDPOINT_HALT		0x00
/* device remote wakeup */
#define USBCORE_SETUP_FEATURE_DEV_REMOTE_WKUP	0x01
/* test mode */
#define USBCORE_SETUP_FEATURE_TEST_MODE			0x02

/* status bits */
/* self powered device */
#define USBCORE_SETUP_STATUS_DEV_SELF_POW		0x01
/* remote wakeup */
#define USBCORE_SETUP_STATUS_DEV_REMOTE_WKUP	0x02
/* endpoint halted */
#define USBCORE_SETUP_STATUS_EP_HALT			0x01

/* device states */
typedef enum usbcore_state {
	USBCORE_STATE_DEFAULT,
	USBCORE_STATE_ADDRESS,
	USBCORE_STATE_CONFIGURED,
} usbcore_state_t;

/* setup frame */
typedef struct usb_setup {
	/* bmRequestType */
	uint8_t	request_type;
	/* bRequest */
	uint8_t	request;
	/* wValue */
	uint16_t value;
	/* wIndex */
	uint16_t index;
	/* wLength */
	uint16_t length;
} PACKED usb_setup_t;


/* class/interface specific request */
typedef struct {
	/* setup frame that caused the request */
	usb_setup_t *setup;
	/* request processing error code */
	err_t ec;
	/* pointer to the data buffer */
	void *ptr;
	/* size of data */
	size_t size;
} usbcore_req_evarg_t;

/* core request event */
extern ev_t usbcore_req_ev;


/**
 * @brief initialize usb core logic
 *
 * @return err_t error code
 */
err_t USBCore_Init(void);

/**
 * @brief perform the data out transfer (recv data from the host)
 *
 * @param ep_num endpoint number
 * @param ptr pointer to the data buffer
 * @param size size of the data
 * @param timeout timeout in ms
 *
 * @return err_t error code or the number of bytes recvd
 */
err_t USBCore_DataOUT(usb_epnum_t ep_num, void *ptr, size_t size,
	dtime_t timeout);

/**
 * @brief perform the data in transfer (send data to the host)
 *
 * @param ep_num encpoint number
 * @param ptr pointer to the data to be sent to host
 * @param size size of the data
 * @param timeout timeout value in ms
 *
 * @return err_t error code or the number of bytes sent to the host
 */
err_t USBCore_DataIN(usb_epnum_t ep_num, const void *ptr, size_t size,
	dtime_t timeout);

#endif /* DEV_USB_CORE_H */
