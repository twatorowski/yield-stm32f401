/**
 * @file checksum.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-31
 * 
 * @brief TCP/IP Stack: Inet checksum algorithm
 */

#ifndef NET_TCPIP_CHECKSUM
#define NET_TCPIP_CHECKSUM

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Calculate checksum over byte buffer
 * 
 * @param sum initial value of the sum
 * @param ptr pointer to byte buffer
 * @param size size of the byte buffer
 *
 * @return uint16_t checksum value
 */
uint16_t TCPIPChecksum_Checksum(uint16_t sum, const void *ptr, size_t size);


#endif /* NET_TCPIP_CHECKSUM */
