/**
 * @file usb.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-10-26
 *
 * @copyright Copyright (c) 2024
 */

#ifndef DEV_USB_H
#define DEV_USB_H

#include <stddef.h>

#include "err.h"
#include "sys/ev.h"


/** endpoint numbers */
typedef enum usb_epnum {
    USB_EP0 = 0x00,
    USB_EP1 = 0x01,
    USB_EP2 = 0x02,
    USB_EP3 = 0x03,
    USB_EP4 = 0x04,
    USB_EP5 = 0x05,
    USB_EP6 = 0x06,
    USB_EP7 = 0x07
} usb_epnum_t;

/** endpoint types */
typedef enum usb_eptype {
    USB_EPTYPE_CTL = 0x00,
    USB_EPTYPE_ISO = 0x01,
    USB_EPTYPE_BULK	= 0x02,
    USB_EPTYPE_INT = 0x03
} usb_eptype_t;

/** event argument type */
typedef enum usb_evarg_type {
    USB_EVARG_TYPE_RESET,
    USB_EVARG_TYPE_ADDRESS,
    USB_EVARG_TYPE_ISOINC
} usb_evarg_type_t;

/** event argument for usb_ev */
typedef struct usb_evarg {
    /** event type */
    int type;
} usb_evarg_t;

/** @brief callback argument for StartXXXTransfer functions */
typedef struct usb_cbarg {
    /**< error code */
    err_t error;
    /**< transfer size */
    size_t size;
} usb_cbarg_t;

/** usb bus system event */
extern ev_t usb_ev;

/**
 * @brief initialize usb support
 *
 * @return err_t status
 */
err_t USB_Init(void);

/**
 * @brief Attach/Remove the USB device. To be called when VUSB is
 * applied/removed
 *
 * @param enable 1 - enable USB Device, 0 - disable USB Device
 *
 * @return status
 */
err_t USB_Connect(void);

/**
 * @brief kill usb connection
 *
 * @return err_t status
 */
err_t USB_Disconnect(void);

/**
 * @brief Sets the RX Fifo size which determines the maximal size of the frames
 * being received. This shall be configured according to implemented device
 * class requirements. Keep in mind that the hardware uses one RX fifo that is
 * shared along all OUT endpoints so it should be set to handle biggest transfer
 * possible.
 *
 * @param size size expressed in 32-bit words.
 */
void USB_SetRxFifoSize(size_t size);

/**
 * @brief Sets the TX Fifo size. Every IN endpoint has it's own tx fifo (nothing
 * is shared like in RX Fifo case).
 *
 * @param ep_num endpoint number
 * @param size fifo size expressed in 32-bit words
 */
void USB_SetTxFifoSize(usb_epnum_t ep_num, size_t size);

/**
 * @brief Flushes the TX Fifo associated with given endpoint.
 *
 * @param ep_num endpoint number
 */
void USB_FlushTxFifo(usb_epnum_t ep_num);

/**
 * @brief Perform IN transfer (from device to host). Send data from 'ptr' of
 * size 'size'
 *
 * @param ep_num endpoint number
 * @param ptr data to be sent
 * @param size size of the data
 * @param timeout optional timeout
 *
 * @return err_t error code or the number of bytes sent
 */
err_t USB_INTransfer(usb_epnum_t ep_num, const void *ptr, size_t size,
    dtime_t timeout);


/**
 * @brief Accept data from host (OUT transfer on any given endpoint). Data will
 * be stored under the ptr. Max size of the data being accepted is indicated by
 * the size.
 *
 * @param ep_num number of the endpoint
 * @param ptr buffer to which the data will be stored
 * @param size maximal size of the data
 * @param timeout transfer timeout
 *
 * @return err_t transfer error code of the size of the data received from the
 * host
 */
err_t USB_OUTTransfer(usb_epnum_t ep_num, void *ptr, size_t size,
    dtime_t timeout);

/**
 * @brief Start the setup transfer (as often done on the control endpoint 0).
 *
 * @param ep_num endpoint number
 * @param ptr pointer the buffer to where to store the data from host
 * @param size size of the buffer
 * @param timeout transfer timeout
 *
 * @return err_t transfer error code of the size of the data received from the
 * host
 */
err_t USB_SETUPTransfer(usb_epnum_t ep_num, void *ptr, size_t size,
    dtime_t timeout);

/**
 * @brief Configure IN endpoint. To be called after USB reset event to
 * re-initialize the endpoint.
 *
 * @param ep_num endpoint number
 * @param type type of endpoint (see USB_EPTYPE_ defines)
 * @param mp_size maximal packet size
 */
void USB_ConfigureINEndpoint(usb_epnum_t ep_num, usb_eptype_t type,
    size_t mp_size);

/**
 * @brief Configure OUT endpoint. To be called after USB reset event to
 * re-initialize the endpoint.
 *
 * @param ep_num endpoint number
 * @param type type of endpoint (see USB_EPTYPE_ defines)
 * @param mp_size maximal packet size
 */
void USB_ConfigureOUTEndpoint(usb_epnum_t ep_num, usb_eptype_t type,
    size_t mp_size);

/**
 * @brief Set the device addres. To be called during enumeration when host
 * assigns the address to the device.
 *
 * @param addr address value
 */
void USB_SetDeviceAddress(uint8_t addr);

/**
 * @brief Stall OUT endpoint. Halt all the transfers.
 *
 * @param ep_num endpoint number
 */
void USB_StallOUTEndpoint(usb_epnum_t ep_num);

/**
 * @brief Stall IN endpoint. Halt all the transfers.
 *
 * @param ep_num ednpoint number
 */
void USB_StallINEndpoint(usb_epnum_t ep_num);

/**
 * @brief Disable IN endpoint.
 *
 * @param ep_num endpoint number
 */
void USB_DisableINEndpoint(usb_epnum_t ep_num);

/**
 * @brief Disable OUT endpoint.
 *
 * @param ep_num endpoint number
 */
void USB_DisableOUTEndpoint(usb_epnum_t ep_num);


#endif /* DEV_USB_H */
