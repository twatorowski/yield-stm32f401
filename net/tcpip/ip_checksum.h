/**
 * @file ip_checksum.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-05-15
 * 
 * @brief TCP/IP Stack: IP checksum computation
 */

#ifndef _NET_TCPIP_IP_CHECKSUM_H
#define _NET_TCPIP_IP_CHECKSUM_H

#include <stdint.h>

#include "net/tcpip/ip_frame.h"

/**
 * @brief Calculate the checksum over the ip header
 * 
 * @param ip ip header
 * 
 * @return uint16_t checksum value
 */
uint16_t TCPIPIpChecksum_Compute(tcpip_ip_frame_t *ip);

/**
 * @brief Sets the checksum field to a valid value that is derived from the 
 * contents of the ip header
 * 
 * @param ip ip header
 */
void TCPIPIpChecksum_Set(tcpip_ip_frame_t *ip);

/**
 * @brief Validates the checksum of the provided ip header
 * 
 * @param ip ip header
 * @return int 1 - checksum is valid, 0 - checksum is invalid
 */
int TCPIPIpChecksum_IsValid(tcpip_ip_frame_t *ip);


#endif /* _NET_TCPIP_IP_CHECKSUM_H */
