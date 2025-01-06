/**
 * @file usb_ecm.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-09
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef DEV_USB_ECM_H
#define DEV_USB_ECM_H



/** ecm request types */
#define USBECM_REQ_SEND_ENCAPSULATED_COMMAND                       0x00
#define USBECM_REQ_GET_ENCAPSULATED_RESPONSE                       0x01
#define USBECM_REQ_SET_ETH_MULTICAST_FILTERS                       0x40
#define USBECM_REQ_SET_ETH_PWRM_PATTERN_FILTER                     0x41
#define USBECM_REQ_GET_ETH_PWRM_PATTERN_FILTER                     0x42
#define USBECM_REQ_SET_ETH_PACKET_FILTER                           0x43
#define USBECM_REQ_GET_ETH_STATISTIC                               0x44

#define USBECM_NET_DISCONNECTED                                    0x00
#define USBECM_NET_CONNECTED                                       0x01


/* Ethernet statistics definitions */
#define USBECM_XMIT_OK_VAL                                     0xff
#define USBECM_XMIT_OK                                         0x01
#define USBECM_RVC_OK                                          0x02
#define USBECM_XMIT_ERROR                                      0x04
#define USBECM_RCV_ERROR                                       0x08
#define USBECM_RCV_NO_BUFFER                                   0x10
#define USBECM_DIRECTED_BYTES_XMIT                             0x20
#define USBECM_DIRECTED_FRAMES_XMIT                            0x40
#define USBECM_MULTICAST_BYTES_XMIT                            0x80

#define USBECM_MULTICAST_FRAMES_XMIT                           0x01
#define USBECM_BROADCAST_BYTES_XMIT                            0x02
#define USBECM_BROADCAST_FRAMES_XMIT                           0x04
#define USBECM_DIRECTED_BYTES_RCV                              0x08
#define USBECM_DIRECTED_FRAMES_RCV                             0x10
#define USBECM_MULTICAST_BYTES_RCV                             0x20
#define USBECM_MULTICAST_FRAMES_RCV                            0x40
#define USBECM_BROADCAST_BYTES_RCV                             0x80

#define USBECM_BROADCAST_FRAMES_RCV                            0x01
#define USBECM_RCV_CRC_ERROR                                   0x02
#define USBECM_TRANSMIT_QUEUE_LENGTH                           0x04
#define USBECM_RCV_ERROR_ALIGNMENT                             0x08
#define USBECM_XMIT_ONE_COLLISION                              0x10
#define USBECM_XMIT_MORE_COLLISIONS                            0x20
#define USBECM_XMIT_DEFERRED                                   0x40
#define USBECM_XMIT_MAX_COLLISIONS                             0x80

#define USBECM_RCV_OVERRUN                                     0x40
#define USBECM_XMIT_UNDERRUN                                   0x40
#define USBECM_XMIT_HEARTBEAT_FAILURE                          0x40
#define USBECM_XMIT_TIMES_CRS_LOST                             0x40
#define USBECM_XMIT_LATE_COLLISIONS                            0x40

#define USBECM_ETH_STATS_RESERVED                              0xE0
#define USBECM_BMREQUEST_TYPE_ECM                              0xA1


#endif /* DEV_USB_ECM_H */
