/**
 * @file ip_frame.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief TCP/IP Stack: Internet Protocol Checksum
 */

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "compiler.h"
#include "net/tcpip/ip_frame.h"
#include "net/tcpip/checksum.h"
#include "util/endian.h"

/* calculate checksum over the ip header */
uint16_t TCPIPIpChecksum_Compute(tcpip_ip_frame_t *ip)
{
    /* calculate the checksum over entire frame */
    return ~TCPIPChecksum_Checksum(0, ip, TCPIPIpFrame_GetHdrLen(ip));
}

/* calculate checksum over the ip header */
void TCPIPIpChecksum_Set(tcpip_ip_frame_t *ip)
{
    /* setup checksum with iv set to 0 */
    TCPIPIpFrame_SetChecksum(ip, 0);
    TCPIPIpFrame_SetChecksum(ip, TCPIPIpChecksum_Compute(ip));
}

/* validate checksum */
int TCPIPIpChecksum_IsValid(tcpip_ip_frame_t *ip)
{
    /* computing checksum over frame with valid checksum field value shall 
     * result in zero being returned */
    return TCPIPIpChecksum_Compute(ip) == 0;
}