/**
 * @file udp.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-02
 * 
 * @brief TCP/IP Stack: User Datagram Protocol
 */

#ifndef NET_TCPIP_UDP_H
#define NET_TCPIP_UDP_H

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "net/tcpip/tcpip.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/udp_frame.h"


/**
 * @brief Initialize udp layer
 * 
 * @return err_t error code
 */
err_t TCPIPUdp_Init(void);

/**
 * @brief main input routine to the udp layer 
 * 
 * @param frame frame pointer
 * 
 * @return err_t processing error code
 */
err_t TCPIPUdp_Input(tcpip_frame_t *frame);

/**
 * @brief Allocate space in the output buffer for the nea udp frame
 * 
 * @param frame allocated frame descriptor
 * 
 * @return err_t allocation error code
 */
err_t TCPIPUdp_Alloc(tcpip_frame_t *frame);

/**
 * @brief Drops the previously allocated frame
 * 
 * @param frame frame pointer
 * 
 * @return err_t error code
 */
err_t TCPIPUdp_Drop(tcpip_frame_t *frame);

/**
 * @brief Sends allocated frame
 * 
 * @param frame frame buffer
 * @param dst_addr destination address
 * @param scr_port source port
 * @param dst_port destination port
 * 
 * @return err_t send error code
 */
err_t TCPIPUdp_Send(tcpip_frame_t *frame, tcpip_ip_addr_t dst_addr, 
    tcpip_udp_port_t scr_port, tcpip_udp_port_t dst_port);


#endif /* NET_TCPIP_UDP_H */
