/**
 * @file usb_desc.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-04
 *
 * @copyright Copyright (c) 2024
 */

#ifndef DEV_USB_DESC_H
#define DEV_USB_DESC_H

#include <stddef.h>

/* usb descriptor set entry */
typedef struct usb_desc {
    /* pointer to the descriptor and the size of the descriptor */
    const void *ptr; size_t size;
} usb_desc_t;

/** descriptor set */
typedef struct usb_descset {
    /** device descriptor */
    usb_desc_t device;
    /** qualifier descriptor */
    usb_desc_t qualifier;
    /** configuration descriptors */
    usb_desc_t *configs;
    /* number of configuration descritptors */
    int configs_num;
    /** string descritpros */
    usb_desc_t *strings;
    /* number of string descritptors */
    int strings_num;

    /** number of interfaces within the device */
    int ifaces_num;
    /** number of endpoints within the device */
    int endpoints_num;
} usb_descset_t;

/** set of usb descriptors */
extern usb_descset_t usb_descriptors;


#endif /* DEV_USB_DESC_H */
