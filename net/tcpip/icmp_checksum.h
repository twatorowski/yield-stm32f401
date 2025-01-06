/**
 * @file icmp_checksum.h
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-05-15
 *
 * @brief TCP/IP: ICMP Checksum computation
 */

#ifndef _NET_TCPIP_ICMP_CHECKSUM_H
#define _NET_TCPIP_ICMP_CHECKSUM_H

#include <stdint.h>
#include <stddef.h>

#include "net/tcpip/icmp_frame.h"

/**
 * @brief Compute checksum over the icmp frame
 *
 * @param icmp icmp frame header
 * @param size size of icmp frame
 *
 * @return uint16_t statu
 */
uint16_t TCPIPIcmpFrame_Checksum(tcpip_icmp_frame_t *icmp, size_t size);

/**
 * @brief Sets the checksum field of the icmp frame to proper value
 *
 * @param icmp icmp frame
 * @param size size of the icmp frame
 */
void TCPIPIcmpChecksum_Set(tcpip_icmp_frame_t *icmp, size_t size);

/**
 * @brief Validates the checksum of the icmp frame
 *
 * @param icmp icmp frame pointer
 * @param size size of the icmp frame
 *
 * @return int 1 - checksum is valie, 0 - ohterwise
 */
int TCPIPIcmpChecksum_IsValid(tcpip_icmp_frame_t *icmp, size_t size);

#endif /* _NET_TCPIP_ICMP_CHECKSUM_H */
