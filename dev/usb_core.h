/**
 * @file usb_core.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-10
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef DEV_USB_CORE_H
#define DEV_USB_CORE_H



/* Mask to get recipient */
#define USB_SETUP_REQTYPE_RECIPIENT			0x1F
/* Recipient device */
#define USB_SETUP_REQTYPE_RECIPIENT_DEVICE	0x00
/* Recipient interface */
#define USB_SETUP_REQTYPE_RECIPIENT_IFACE	0x01
/* Recipient endpoint */
#define USB_SETUP_REQTYPE_RECIPIENT_EP		0x02
/* Other recipient */
#define USB_SETUP_REQTYPE_RECIPIENT_OTHER	0x03

/* type mask */
#define USB_SETUP_REQTYPE_TYPE				0x60
/* standard request */
#define USB_SETUP_REQTYPE_TYPE_STANDARD		0x00
/* class specific request */
#define USB_SETUP_REQTYPE_TYPE_CLASS		0x20
/* vendor specific request */
#define USB_SETUP_REQTYPE_TYPE_VENDOR		0x40
/* reserved */
#define USB_SETUP_REQTYPE_TYPE_RESERVED		0x60

/* direction bit */
#define USB_SETUP_REQTYPE_DIR				0x80
/* from host to device */
#define USB_SETUP_REQTYPE_DIR_OUT			0x00
/* from device to host */
#define USB_SETUP_REQTYPE_DIR_IN			0x80

/* request field  */
/* get status */
#define USB_SETUP_REQ_GET_STATUS			0x00
/* clear feature */
#define USB_SETUP_REQ_CLEAR_FEATURE			0x01
/* set feature */
#define USB_SETUP_REQ_SET_FEATURE			0x03
/* set device address */
#define USB_SETUP_REQ_SET_ADDRESS			0x05
/* get descriptor */
#define USB_SETUP_REQ_GET_DESCRIPTOR		0x06
/* set descriptor */
#define USB_SETUP_REQ_SET_DESCRIPTOR		0x07
/* get status */
#define USB_SETUP_REQ_GET_CONFIGURATION		0x08
/* set configuration */
#define USB_SETUP_REQ_SET_CONFIGURATION		0x09
/* get interface */
#define USB_SETUP_REQ_GET_INTERFACE			0x0A
/* set interface */
#define USB_SETUP_REQ_SET_INTERFACE			0x0B
/* synch frame */
#define USB_SETUP_REQ_SYNCH_FRAME			0x0C

/* descriptor types */
/* device descriptor */
#define USB_SETUP_DESCTYPE_DEVICE			0x01
/* configuration descriptor */
#define USB_SETUP_DESCTYPE_CONFIGURATION	0x02
/* string descriptor */
#define USB_SETUP_DESCTYPE_STRING			0x03
/* interface descriptor */
#define USB_SETUP_DESCTYPE_INTERFACE		0x04
/* enpoint descritpro */
#define USB_SETUP_DESCTYPE_ENDPOINT			0x05
/* device qualifier descriptor */
#define USB_SETUP_DESCTYPE_QUALIFIER		0x06

/* standard features */
/* halt endpoint */
#define USB_SETUP_FEATURE_ENDPOINT_HALT		0x00
/* device remote wakeup */
#define USB_SETUP_FEATURE_DEV_REMOTE_WKUP	0x01
/* test mode */
#define USB_SETUP_FEATURE_TEST_MODE			0x02

/* status bits */
/* self powered device */
#define USB_SETUP_STATUS_DEV_SELF_POW		0x01
/* remote wakeup */
#define USB_SETUP_STATUS_DEV_REMOTE_WKUP	0x02
/* endpoint halted */
#define USB_SETUP_STATUS_EP_HALT			0x01

/* device states */
/* defaul (after-reset) state */
#define USB_DEV_DEFAULT						0x00
/* addressing state */
#define USB_DEV_ADDRESS						0x01
/* device is fully configured and functional */
#define USB_DEV_CONFIGURED					0x02

/* setup frame */
typedef struct {
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
} __attribute__((packed)) usb_setup_t;

/* class/interface specific request */
typedef struct {
	/* setup frame that caused the request */
	usb_setup_t *setup;
	/* request status */
	int status;
	/* data address returned */
	void *ptr;
	/* size of data */
	size_t size;
} usbcore_req_evarg_t;

/* core request event */
extern ev_t usbcore_req_ev;

/* initialize usb core */
err_t USBCore_Init(void);

/**
 * @brief is the device configured
 *
 * @return int 1 - device is configured, 0 - device is not configured
 */
int USBCore_IsConfigured(void);

#endif /* DEV_USB_CORE_H */
