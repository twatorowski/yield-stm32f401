/**
 * @file usb_eem.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-14
 *
 * @copyright Copyright (c) 2024
 */

#ifndef DEV_USB_EEM_H
#define DEV_USB_EEM_H

#include "err.h"
#include "sys/time.h"

/* packet type */
/* type bitmask */
#define USBEEM_HDR_TYPE                                 0x8000
/* data carrying packet */
#define USBEEM_HDR_TYPE_DATA                            0x0000
/* command carrying packet */
#define USBEEM_HDR_TYPE_CMD                             0x8000

/* valid crc present mask */
#define USBEEM_HDR_DATA_CRC                             0x4000
/* valid crc present */
#define USBEEM_HDR_DATA_CRC_VALID                       0x4000
/* bogus crc */
#define USBEEM_HDR_DATA_CRC_DEADBEEF                    0x0000
/* length of the ethernet data that follows the header */
#define USBEEM_HDR_DATA_LENGTH                          0x3FFF

/* command type field */
#define USBEEM_HDR_CMD                                  0x3800
/* echo request command */
#define USBEEM_HDR_CMD_ECHO_REQ                         0x0000
/* echo response command */
#define USBEEM_HDR_CMD_ECHO_RESP                        0x0800
/* suspend hint command */
#define USBEEM_HDR_CMD_SUSPEND_HINT                     0x1000
/* response hint command */
#define USBEEM_HDR_CMD_RESP_HINT                        0x1800
/* response complete hint command */
#define USBEEM_HDR_CMD_RESP_COMPL_HINT                  0x2000
/* tickle command */
#define USBEEM_HDR_CMD_TICKLE                           0x2800


/* length of the ping data that follows the header */
#define USBEEM_HDR_ECHO_LENGTH                          0x07FF
/* suggested response interval in milliseconds */
#define USBEEM_HDR_RESP_HINT_IVAL                       0x07FF


/* special packet */
#define USBEEM_HDR_ZLP                                  0x0000

/** struct that holds the header  */
typedef struct usbeem_hdr {
    uint16_t hdr;
} PACKED usbeem_hdr_t;

/** eem frame format */
typedef struct usbeem_frame {
    /* frame header */
    uint16_t hdr;
    /* frame payload */
    uint8_t pld[];
} PACKED usbeem_frame_t;



/* initialize virtual com port logic */
err_t USBEEM_Init(void);
/* send data to virtual com port */
err_t USBEEM_Send(const void *ptr, size_t size, dtime_t timeout);
/* receive data from virtual com port */
err_t USBEEM_Recv(void *ptr, size_t size, dtime_t timeout);


#endif /* DEV_USB_EEM_H */
