/**
 * @file usb_vcp.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-08
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef DEV_USB_VCP_H
#define DEV_USB_VCP_H

#include "err.h"

/** VCP class requests */
/* set asynchronous line character formatting */
#define USB_VCP_REQ_SET_LINE_CODING				0x20
/* get asynchronous line character formatting */
#define USB_VCP_REQ_GET_LINE_CODING				0x21
/* This request generates RS-232/V.24 style control signals. */
#define USB_VCP_SET_CONTROL_LINE_STATE			0x22

/**
 * @brief initialize virtual com port logic
 *
 * @return err_t error code
 */
err_t USBVCP_Init(void);

/**
 * @brief send the data over the usb
 *
 * @param ptr pointer to the data buffer
 * @param size size of the data to be sent
 * @param timeout timeout
 *
 * @return err_t error code or the number of bytes that was sent
 */
err_t USBVCP_Send(const void *ptr, size_t size, dtime_t timeout);


/**
 * @brief receive the data over the usb
 *
 * @param ptr pointer to which to receive the data
 * @param size maximal size of the data
 * @param timeout reception timeout
 *
 * @return err_t error code or size of the data received
 */
err_t USBVCP_Recv(void *ptr, size_t size, dtime_t timeout);

#endif /* DEV_USB_VCP_H */
