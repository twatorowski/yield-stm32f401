/**
 * @file icmp_checksum.c
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-05-15
 *
 * @brief TCP/IP: ICMP Checksum
 */

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "net/tcpip/checksum.h"
#include "net/tcpip/icmp_frame.h"

/* calculate checksum */
uint16_t TCPIPIcmpChecksum_Compute(tcpip_icmp_frame_t *icmp, size_t size)
{
    return ~TCPIPChecksum_Checksum(0, icmp, size);
}

/* set checksum in outgoing frames */
void TCPIPIcmpChecksum_Set(tcpip_icmp_frame_t *icmp, size_t size)
{
    /* setup checksum with iv set to 0 */
    TCPIPIcmpFrame_SetChecksum(icmp, 0);
    TCPIPIcmpFrame_SetChecksum(icmp, TCPIPIcmpChecksum_Compute(icmp, size));
}

/* validate checksum */
int TCPIPIcmpChecksum_IsValid(tcpip_icmp_frame_t *icmp, size_t size)
{
    /* computing checksum over frame with valid checksum field value shall
     * result in zero being returned */
    return TCPIPIcmpChecksum_Compute(icmp, size) == 0;
}