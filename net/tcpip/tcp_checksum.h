/**
 * @file tcp_checksum.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-05-15
 * 
 * @brief TCP/IP Stack: TCP Checksum
 */

#ifndef _NET_TCPIP_TCP_CHECKSUM_H
#define _NET_TCPIP_TCP_CHECKSUM_H

/**
 * @brief Calculate the checksum over the ip header
 * 
 * @param ip ip header
 * @param tcp udp frame
 * 
 * @return uint16_t checksum value
 */
uint16_t TCPIPTcpChecksum_Compute(tcpip_ip_frame_t *ip,
    tcpip_tcp_frame_t *tcp);

/**
 * @brief Sets the checksum field to a valid value that is derived from the 
 * contents of the ip header
 * 
 * @param ip ip header
 * @param tcp tcp frame
 */
void TCPIPTcpChecksum_Set(tcpip_ip_frame_t *ip, tcpip_tcp_frame_t *tcp);

/**
 * @brief Validates the checksum of the provided tcp header
 * 
 * @param ip ip header
 * @param tcp tcp frame
 * 
 * @return int 1 - checksum is valid, 0 - checksum is invalid
 */
int TCPIPTcpChecksum_IsValid(tcpip_ip_frame_t *ip, tcpip_tcp_frame_t *tcp);


#endif /* _NET_TCPIP_TCP_CHECKSUM_H */
