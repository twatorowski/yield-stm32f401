/**
 * @file tcp.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-31
 * 
 * @brief TCP/IP Stack: Transmission Control Protocol
 */

#ifndef NET_TCPIP_TCP_H
#define NET_TCPIP_TCP_H

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "net/tcpip/tcpip.h"

/**
 * @brief Initialize tcp layer
 * 
 * @return err_t error code
 */
err_t TCPIPTcp_Init(void);

/**
 * @brief main input routine to the tcp layer 
 * 
 * @param frame frame to be processed
 * 
 * @return err_t error code
 */
err_t TCPIPTcp_Input(tcpip_frame_t *frame);

/**
 * @brief allocate space for tcp frame in output buffers 
 * 
 * @param frame frame descriptor to be filled by allocator
 *
 * @return err_t error code
 */
err_t TCPIPTcp_Alloc(tcpip_frame_t *frame);

/**
 * @brief Drop previously allocated frame
 * 
 * @param frame frame descriptor
 *
 * @return err_t error code
 */
err_t TCPIPTcp_Drop(tcpip_frame_t *frame);

/**
 * @brief Sends pre-allocated tcp frame
 * 
 * @param frame allocated frame pointer
 * @param da destination ip address
 * @param src_port source port
 * @param dst_port destination port
 * @param seq sequence number
 * @param ack acknoledgement number
 * @param win window size
 * @param flags tcp flags
 * 
 * @return err_t send status
 */
err_t TCPIPTcp_Send(tcpip_frame_t *frame, tcpip_ip_addr_t da, 
    tcpip_tcp_port_t src_port, tcpip_tcp_port_t dst_port,
    uint32_t seq, uint32_t ack, uint16_t win, tcpip_tcp_flags_t flags);

#endif /* NET_TCPIP_TCP_H */
