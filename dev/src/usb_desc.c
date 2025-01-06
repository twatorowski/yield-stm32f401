/**
 * @file usb_desc.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024
 */

#include <stdint.h>
#include <stdarg.h>

#include "err.h"
#include "config.h"
#include "dev/usb_desc.h"
#include "util/elems.h"

/* usb standard device descriptor */
static const char device_desc[18] = {
    0x12,                   /* bLength */
    0x01,                   /* bDescriptorType */
    0x00, 0x02,             /* bcdUSB */
    0xEF,                   /* bDeviceClass: Composite */
    0x02,                   /* bDeviceSubClass: Composite */
    0x01,                   /* bDeviceProtocol */
    USB_CTRLEP_SIZE,        /* bMaxPacketSize0 */
    0x83, 0x04,             /* idVendor (0x0483) */
    0x50, 0x57,             /* idProduct = (0x5740) */
    0x00, 0x02,             /* bcdDevice rel. 2.00 */
    0x01,                   /* Index of string descriptor describing
                             * manufacturer */
    0x02,                   /* Index of string descriptor describing
                             * product */
    0x03,                   /* Index of string descriptor describing the
                             * device serial number */
    0x01                    /* bNumConfigurations */
};

/* qualifier descriptor */
const char qualifier_desc[10] = {
    10,                     /* bLength: Configuration Descriptor size */
    0x06,                   /* bDescriptorType: Qualifier */
    0x00, 0x02,             /* bcdUSB: USB Specification Release Number */
    0xef,                   /* bDeviceClass */
    0x02,                   /* bDeviceSubClass */
    0x01,                   /* bDeviceProtocol */
    USB_CTRLEP_SIZE,        /* bMaxPacketSize40 */
    0x01,                   /* bNumConfigurations */
    0x00                    /* bReserved */
};

/* USB Configuration Descriptor */
static const uint8_t config0_desc[98] = {
    /* Configuration Descriptor */
    0x09,                   /* bLength: Configuration Descriptor size */
    0x02,  	                /* bDescriptorType: Configuration */
    0x62, 0x00,             /* wTotalLength: no of returned bytes */
    0x03,                   /* bNumInterfaces: 2 interfaces */
    0x01,                   /* bConfigurationValue: Configuration value */
    0x00,                   /* iConfiguration: Index of string descriptor describing
                             * the configuration */
    0xC0,                   /* bmAttributes: self powered */
    0x32,                   /* MaxPower 100 mA */


    /*
     * FUNCTION 1
     */

    /* Interface Association Descriptor */
    0x08,                   /* bLength: Interface Descriptor size */
    0x0B,                   /* bDescriptorType: IAD */
    0x00,                   /* bFirstInterface */
    0x02,                   /* bInterfaceCount */
    0x02,                   /* bFunctionClass: CDC */
    0x02,                   /* bFunctionSubClass */
    0x01,                   /* bFunctionProtocol */
    0x00,                   /* iFunction */

    /* INTERFACE 0: Abstract Control Model */
    0x09,                   /* bLength: Interface Descriptor size */
    0x04,                   /* bDescriptorType: Interface descriptor type  */
    0x00,                   /* bInterfaceNumber: Number of Interface */
    0x00,                   /* bAlternateSetting: Alternate setting */
    0x01,                   /* bNumEndpoints: One endpoints used */
    0x02,                   /* bInterfaceClass: Communication Interface Class */
    0x02,                   /* bInterfaceSubClass: Abstract Control Model */
    0x01,                   /* bInterfaceProtocol: Common AT commands */
    0x00,                   /* iInterface: */

    /* Header Functional Descriptor */
    0x05,                   /* bLength: Endpoint Descriptor size */
    0x24,                   /* bDescriptorType: CS_INTERFACE */
    0x00,                   /* bDescriptorSubtype: Header Func Desc */
    0x10, 0x01,             /* bcdCDC: spec release number */

    /* Union Functional Descriptor */
    0x05,                   /* bFunctionLength */
    0x24,                   /* bDescriptorType: CS_INTERFACE */
    0x06,                   /* bDescriptorSubtype: Union func desc */
    0x00,                   /* bMasterInterface: Communication class interface */
    0x01,                   /* bSlaveInterface0: Data Class Interface */

    /* Call Management Functional Descriptor */
    0x05,                   /* bFunctionLength */
    0x24,                   /* bDescriptorType: CS_INTERFACE */
    0x01,                   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,                   /* bmCapabilities: D0+D1 */
    0x01,                   /* bDataInterface: 3 */

    /* ACM Functional Descriptor */
    0x04,                   /* bFunctionLength */
    0x24,                   /* bDescriptorType: CS_INTERFACE */
    0x02,                   /* bDescriptorSubtype: Abstract Control Management
                             * desc */
    0x02,                   /* bmCapabilities */

    /* ENDPOINT 1 IN Descriptor */
    0x07,                   /* bLength: Endpoint Descriptor size */
    0x05,                   /* bDescriptorType: Endpoint */
    0x81,                   /* bEndpointAddress: (IN1) */
    0x03,                   /* bmAttributes: Interrupt */
    /* wMaxPacketSize: */
    USB_VCP_INT_SIZE & 0xff, USB_VCP_INT_SIZE >> 8,
    0xFF,                   /* bInterval: */

    /* INTERFACE 1: Data class interface descriptor */
    0x09,                   /* bLength: Endpoint Descriptor size */
    0x04,                   /* bDescriptorType: */
    0x01,                   /* bInterfaceNumber: Number of Interface */
    0x00,                   /* bAlternateSetting: Alternate setting */
    0x02,                   /* bNumEndpoints: Two endpoints used */
    0x0A,                   /* bInterfaceClass: Data Interface Class */
    0x00,                   /* bInterfaceSubClass: */
    0x00,                   /* bInterfaceProtocol: */
    0x00,                   /* iInterface: */

    /* ENDPOINT 2 IN Descriptor */
    0x07,                   /* bLength: Endpoint Descriptor size */
    0x05,                   /* bDescriptorType: Endpoint */
    0x82,                   /* bEndpointAddress: (IN3) */
    0x02,                   /* bmAttributes: Bulk */
    /* wMaxPacketSize: */
    USB_VCP_TX_SIZE & 0xff, USB_VCP_TX_SIZE >> 8,
    0x00,                   /* bInterval: ignore for Bulk transfer */

    /* ENDPOINT 2 OUT Descriptor */
    0x07,                   /* bLength: Endpoint Descriptor size */
    0x05,                   /* bDescriptorType: Endpoint */
    0x02,                   /* bEndpointAddress: (OUT2) */
    0x02,                   /* bmAttributes: Bulk */
    /* wMaxPacketSize: */
    USB_VCP_RX_SIZE & 0xff, USB_VCP_RX_SIZE >> 8,
    0x00,


    /*
     * FUNCTION 2
     */



    /* INTERFACE 2: Data class interface descriptor */
    0x09,                   /* bLength: Endpoint Descriptor size */
    0x04,                   /* bDescriptorType: */
    0x02,                   /* bInterfaceNumber: Number of Interface */
    0x00,                   /* bAlternateSetting: Alternate setting */
    0x02,                   /* bNumEndpoints: Two endpoints used */
    0x02,                   /* bInterfaceClass: Communications Interface Class */
    0x0C,                   /* bInterfaceSubClass: EEM */
    0x07,                   /* bInterfaceProtocol: EEM */
    0x00,                   /* iInterface: */

    /* ENDPOINT 3 IN Descriptor */
    0x07,                   /* bLength: Endpoint Descriptor size */
    0x05,                   /* bDescriptorType: Endpoint */
    0x83,                   /* bEndpointAddress: (IN3) */
    0x02,                   /* bmAttributes: Bulk */
    /* wMaxPacketSize: */
    USB_EEM_TX_SIZE & 0xff, USB_EEM_TX_SIZE >> 8,
    0x00,                   /* bInterval: ignore for Bulk transfer */

    /* ENDPOINT 3 OUT Descriptor */
    0x07,                   /* bLength: Endpoint Descriptor size */
    0x05,                   /* bDescriptorType: Endpoint */
    0x03,                   /* bEndpointAddress: (OUT2) */
    0x02,                   /* bmAttributes: Bulk */
    /* wMaxPacketSize: */
    USB_EEM_RX_SIZE & 0xff, USB_EEM_RX_SIZE >> 8,
    0x00,





};

/* manufacturer string */
static const uint8_t string0_desc[12] = {
    12,                     /* bLength */
    0x03,                   /* bDescriptorType */
    'Y', 0, 'i', 0,
    'e', 0, 'l', 0,
    'd', 0,
};

/* product string */
static const uint8_t string1_desc[12] = {
    12,                     /* bLength */
    0x03,                   /* bDescriptorType */
    'Y', 0, 'i', 0,
    'e', 0, 'l', 0,
    'd', 0,
};

/* serial number string */
static const uint8_t string2_desc[10] = {
    10,                     /* bLength */
    0x03,                   /* bDescriptorType */
    '0', 0, '1', 0,
    '2', 0, '3', 0,
};

/* set of usb descriptors */
usb_descset_t usb_descriptors = {
    /* device descritptor */
    .device = { .ptr = device_desc, .size = sizeof(device_desc) },
    /* qualifier descriptor */
    .qualifier = { .ptr = qualifier_desc, .size = sizeof(qualifier_desc) },
    /* set of configuration descriptors */
    .configs = (usb_desc_t []) {
        [0] = { .ptr = config0_desc, .size = sizeof(config0_desc) },
    },
    /* number of configuration descriptors */
    .configs_num = 1,
    /* set of string descriptors */
    .strings = (usb_desc_t []) {
        [0] = { .ptr = string0_desc, .size = sizeof(string0_desc) },
        [1] = { .ptr = string1_desc, .size = sizeof(string1_desc) },
        [2] = { .ptr = string2_desc, .size = sizeof(string2_desc) },
    },
    /* number of string descriptors */
    .strings_num = 3,

    /* total number of interfaces */
    .ifaces_num = 3,
    /* total number of endpoints (here we use the endpoints from 0 to 3) */
    .endpoints_num = 4,
};