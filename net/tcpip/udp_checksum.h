/**
 * @file udp_checksum.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-05-15
 * 
 * @brief TCP/IP Stack: User Datagram Protocol Checksum
 */

#ifndef _NET_TCPIP_UDP_CHECKSUM_H
#define _NET_TCPIP_UDP_CHECKSUM_H

#include <stdint.h>

#include "net/tcpip/ip_frame.h"
#include "net/tcpip/udp_frame.h"

/**
 * @brief Calculate the checksum over the ip header
 * 
 * @param ip ip header
 * @param udp udp frame
 * 
 * @return uint16_t checksum value
 */
uint16_t TCPIPUdpChecksum_Compute(tcpip_ip_frame_t *ip, 
    tcpip_udp_frame_t *udp);

/**
 * @brief Sets the checksum field to a valid value that is derived from the 
 * contents of the ip header
 * 
 * @param ip ip header
 * @param udp udp frame
 */
void TCPIPUdpChecksum_Set(tcpip_ip_frame_t *ip, tcpip_udp_frame_t *udp);

/**
 * @brief Validates the checksum of the provided ip header
 * 
 * @param ip ip header
 * @param udp udp frame
 * 
 * @return int 1 - checksum is valid, 0 - checksum is invalid
 */
int TCPIPUdpChecksum_IsValid(tcpip_ip_frame_t *ip, tcpip_udp_frame_t *udp);

#endif /* _NET_TCPIP_UDP_CHECKSUM_H */
