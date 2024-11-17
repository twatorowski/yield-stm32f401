/**
 * @file ip.c
 * 
 * @date 2021-02-02
 * @author twatorowski 
 * 
 * @brief TCP/IP Stack: Internet Protocol 
 */

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "config.h"
#include "err.h"
#include "net/tcpip/arp.h"
#include "net/tcpip/eth.h"
#include "net/tcpip/icmp.h"
#include "net/tcpip/ip.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/ip_frame.h"
#include "net/tcpip/ip_checksum.h"
#include "net/tcpip/tcp.h"
#include "net/tcpip/tcp_checksum.h"
#include "net/tcpip/tcp_frame.h"
#include "net/tcpip/udp.h"
#include "net/tcpip/udp_checksum.h"
#include "net/tcpip/udp_frame.h"
#include "net/tcpip/tcpip.h"
#include "sys/time.h"
#include "util/endian.h"
#include "util/msblsb.h"

/* initialize IP layer */
err_t TCPIPIp_Init(void)
{
    /* return status */
    return EOK;
}

/* main input routine to the ip layer */
err_t TCPIPIp_Input(tcpip_frame_t *frame)
{
    /* result/error code */
    err_t rc = 0;
    /* ip header resides just after the ethernet frame header */
    tcpip_ip_frame_t *ip = frame->ip = frame->ptr;

    /* validate checksum */
    if (!TCPIPIpChecksum_IsValid(ip))
        return EFATAL;

    /* header length is expressed in 32-bit words */
    size_t hdr_len = TCPIPIpFrame_GetHdrLen(ip);
    size_t pld_len = TCPIPIpFrame_GetLength(ip) - hdr_len;
    /* extract the address */
    tcpip_ip_addr_t da = TCPIPIpFrame_GetDstAddr(ip); 
    /* we accept either broadcast packets or unicast ones */
    if (!TCPIPIpAddr_IsMatchingUnicast(da) &&
        !TCPIPIpAddr_IsMatchingBroadcast(da))
        return EOK;

    /* extract flags and fragment offset */
    uint16_t flags = TCPIPIpFrame_GetFlags(ip);
    uint16_t foffs = TCPIPIpFrame_GetFragmentOffset(ip);
    /* we do not support fragmentation */
    if ((flags & TCPIP_IP_FLAGS_MF) || foffs != 0)
        return EOK;

    /* ip is now processed */
    frame->flags |= TCPIP_FRAME_FLAGS_IP;
    frame->size = pld_len;
    frame->ptr = (void *)((uintptr_t)frame->ptr + hdr_len);

    /* switch on operation */
    switch (TCPIPIpFrame_GetProtocol(ip)) {
    /* Internet Control Message Protocol */
    case TCPIP_IP_PROTOCOL_ICMP : {
        rc = TCPIPIcmp_Input(frame);
    } break;
    /* transmission control protocol */
    case TCPIP_IP_PROTOCOL_TCP: {
        rc = TCPIPTcp_Input(frame);
    } break;
    /* user datagram protocol */
    case TCPIP_IP_PROTOCOL_UDP : {
        rc = TCPIPUdp_Input(frame);
    } break;
    /* unsupported */
    default: rc = EUNKPROT; break;
    }

    /* return error code */
    return rc;
}

/* allocate space for ip frame in output buffers */
err_t TCPIPIp_Alloc(tcpip_frame_t *frame)
{
    /* error code */
    err_t ec;
    /* try to allocate frame in output buffers */
    if ((ec = TCPIPEth_Alloc(frame)) != EOK)
        return ec;

    /* setup for upper layer */
    frame->flags |= TCPIP_FRAME_FLAGS_IP;
    frame->ip = frame->ptr;
    frame->ptr = frame->ip->pld;
    
    /* report success */
    return EOK;
}

/* drop the frame */
err_t TCPIPIp_Drop(tcpip_frame_t *frame)
{
    /* no additional activities */
    return TCPIPEth_Drop(frame);
}

/* send allocated data */
err_t TCPIPIp_Send(tcpip_frame_t *frame, tcpip_ip_addr_t dst_addr, 
    tcpip_ip_protocol_t proto)
{
    /* ip header */
    tcpip_ip_frame_t *ip = (tcpip_ip_frame_t *)frame->eth->pld;

    /* setup header size/version */
    TCPIPIpFrame_SetHdrLen(ip, sizeof(tcpip_ip_frame_t));
    TCPIPIpFrame_SetVersion(ip, 4);

    /* addresses & protcol information */
    TCPIPIpFrame_SetSrcAddr(ip, TCPIPIpAddr_GetIP());
    TCPIPIpFrame_SetDstAddr(ip, dst_addr);
    TCPIPIpFrame_SetProtocol(ip, proto);
    TCPIPIpFrame_SetTTL(ip, TCPIP_IP_TTL);
    TCPIPIpFrame_SetLength(ip, frame->size + sizeof(tcpip_ip_frame_t));

    /* fragmentation/congestion is not implemented */
    TCPIPIpFrame_SetTos(ip, 0);
    TCPIPIpFrame_SetIdentification(ip, 0);
    TCPIPIpFrame_SetFlags(ip, 0);
    TCPIPIpFrame_SetFragmentOffset(ip, 0);

    /* compute the header checksum */
    TCPIPIpChecksum_Set(ip);

    /* upper layers may need to compute checksums after the ip header was 
     * filled */
    switch (proto) {
    case TCPIP_IP_PROTOCOL_TCP: TCPIPTcpChecksum_Set(ip, frame->tcp); break;
    case TCPIP_IP_PROTOCOL_UDP: TCPIPUdpChecksum_Set(ip, frame->udp); break;
    default: break;
    }
    
    /* corresponding ethernet address */
    tcpip_eth_addr_t eth_da; err_t ec;
    /* try to obtain the hardware address */
    ec = TCPIPArp_GetHWAddr(TCPIPIpAddr_IsWithinSubnet(dst_addr) ? 
        dst_addr : TCPIPIpAddr_GetGatewayIP(), &eth_da);
    /* we are unable to resolve the address */
    if (ec != EOK) {
        TCPIPIp_Drop(frame); return ec;
    }

    /* prepare for underlying layers */
    frame->ptr = ip;
    frame->size += sizeof(tcpip_ip_frame_t);

    /* pass to underlying layer */
    return TCPIPEth_Send(frame, eth_da, TCPIP_ETH_ETHTYPE_IP);
}