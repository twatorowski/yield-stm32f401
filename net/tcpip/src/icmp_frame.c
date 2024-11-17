/**
 * @file icmp_frame.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief TCP/IP Stack: Internet Control Message Protocol
 */

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "net/tcpip/checksum.h"
#include "net/tcpip/icmp_frame.h"
#include "util/endian.h"

/* calculate checksum */
uint16_t TCPIPIcmpFrame_Checksum(tcpip_icmp_frame_t *icmp, size_t size)
{
    return ~TCPIPChecksum_Checksum(0, icmp, size);
}
