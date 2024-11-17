/**
 * @file tcp_checksum.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-05-15
 * 
 * @brief TCP/IP Stack: TCP Checksum
 */

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "net/tcpip/checksum.h"
#include "net/tcpip/ip_frame.h"
#include "net/tcpip/tcp_frame.h"

/* compute udp frame checksum */
uint16_t TCPIPTcpChecksum_Compute(tcpip_ip_frame_t *ip,
    tcpip_tcp_frame_t *udp)
{
    /* checksum accumulator */
    uint32_t sum = 0;
    /* organize pseudoheader */
    tcpip_ip_pseudo_hdr_t phdr = TCPIPIpFrame_GetPseudoHdr(ip);

    /* first of all compute the checksum over the ip pseudoheader */
    sum = TCPIPChecksum_Checksum(sum, &phdr, sizeof(phdr));
    /* then, on top of it comute the checksum for the udp frame */
    sum = TCPIPChecksum_Checksum(sum, udp,
        TCPIPIpFrame_GetLength(ip) - TCPIPIpFrame_GetHdrLen(ip));

    /* return sum, negated, as per standard */
    return ~sum;
}

/* set checksum in outgoing frames */
void TCPIPTcpChecksum_Set(tcpip_ip_frame_t *ip, tcpip_tcp_frame_t *udp)
{
    /* setup checksum with iv set to 0 */
    TCPIPTcpFrame_SetChecksum(udp, 0);
    TCPIPTcpFrame_SetChecksum(udp, TCPIPTcpChecksum_Compute(ip, udp));
}

/* validate checksum */
int TCPIPTcpChecksum_IsValid(tcpip_ip_frame_t *ip, tcpip_tcp_frame_t *udp)
{
    /* computing checksum over frame with valid checksum field value shall
     * result in zero being returned */
    return TCPIPTcpChecksum_Compute(ip, udp) == 0;
}