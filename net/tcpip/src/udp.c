/**
 * @file udp.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-02
 * 
 * @brief TCP/IP Stack: User Datagram Protocol
 */

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "net/tcpip/ip.h"
#include "net/tcpip/udp.h"
#include "net/tcpip/udp_frame.h"
#include "net/tcpip/udp_sock.h"

/* initialize udp layer */
err_t TCPIPUdp_Init(void)
{
    /* report status */
    return EOK;
}

/* main input routine to the udp layer */
err_t TCPIPUdp_Input(tcpip_frame_t *frame)
{
    /* tcp header pointer */
    frame->udp = frame->ptr;
    
    /* setup payload pointer and size */
    frame->ptr = frame->udp->pld;
    frame->size = TCPIPUdpFrame_GetLength(frame->udp) - sizeof(tcpip_udp_frame_t);
    /* mark as parsed */
    frame->flags |= TCPIP_FRAME_FLAGS_UDP;

    /* pass it into the socketization layer */
    return TCPIPUdpSock_Input(frame);
}

/* allocate space for udp frame in output buffers */
err_t TCPIPUdp_Alloc(tcpip_frame_t *frame)
{
    /* error code */
    err_t ec;
    /* try to allocate frame in output buffers */
    if ((ec = TCPIPIp_Alloc(frame)) != EOK)
        return ec;
    
    /* setup for upper layer */
    frame->flags |= TCPIP_FRAME_FLAGS_UDP;
    frame->udp = frame->ptr;
    frame->ptr = frame->udp->pld;

    /* report success */
    return EOK;
}

/* drop the frame */
err_t TCPIPUdp_Drop(tcpip_frame_t *frame)
{
    /* no additional activities */
    return TCPIPIp_Drop(frame);
}

/* send allocated data */
err_t TCPIPUdp_Send(tcpip_frame_t *frame, tcpip_ip_addr_t dst_addr, 
    tcpip_udp_port_t src_port, tcpip_udp_port_t dst_port)
{
    /* tcp header pointer */
    tcpip_udp_frame_t *udp = frame->udp;

    /* setup ports */
    TCPIPUdpFrame_SetSrcPort(udp, src_port);
    TCPIPUdpFrame_SetDstPort(udp, dst_port);
    /* setup data sequencing */
    TCPIPUdpFrame_SetLength(udp, frame->size + sizeof(*udp));

    /* prepare for underlying layers */
    frame->ptr = udp;
    frame->size += sizeof(tcpip_udp_frame_t);

    /* bombs away! */
    return TCPIPIp_Send(frame, dst_addr, TCPIP_IP_PROTOCOL_UDP);
}