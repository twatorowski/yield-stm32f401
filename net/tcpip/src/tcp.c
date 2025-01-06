/**
 * @file tcp.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-31
 * 
 * @brief TCP/IP Stack: Transmission Control Protocol
 */

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "compiler.h"
#include "net/tcpip/ip.h"
#include "net/tcpip/tcp_frame.h"
#include "net/tcpip/tcp_sock.h"
#include "sys/time.h"
#include "util/elems.h"
#include "util/endian.h"
#include "util/msblsb.h"

/* initialize tcp layer */
err_t TCPIPTcp_Init(void)
{
    /* report status */
    return EOK;
}

/* main input routine to the tcp layer */
err_t TCPIPTcp_Input(tcpip_frame_t *frame)
{
    /* tcp header pointer */
    frame->tcp = frame->ptr;
    
    /* process flags and data offset field */
    int doffs = TCPIPTcpFrame_GetDataOffs(frame->tcp);
    /* setup payload pointer and size */
    frame->ptr = (void *)((uintptr_t)frame->ptr + doffs);
    frame->size -= doffs;
    /* mark as parsed */
    frame->flags |= TCPIP_FRAME_FLAGS_TCP;

    /* pass it into the socketization layer */
    return TCPIPTcpSock_Input(frame);
}

/* allocate space for tcp frame in output buffers */
err_t TCPIPTcp_Alloc(tcpip_frame_t *frame)
{
    /* error code */
    err_t ec;
    /* try to allocate frame in output buffers */
    if ((ec = TCPIPIp_Alloc(frame)) != EOK)
        return ec;
    
    /* setup for upper layer */
    frame->flags |= TCPIP_FRAME_FLAGS_TCP;
    frame->tcp = frame->ptr;
    frame->ptr = frame->tcp->pld;

    /* report success */
    return EOK;
}

/* drop the frame */
err_t TCPIPTcp_Drop(tcpip_frame_t *frame)
{
    /* no additional activities */
    return TCPIPIp_Drop(frame);
}

/* send allocated data */
err_t TCPIPTcp_Send(tcpip_frame_t *frame, tcpip_ip_addr_t dst_addr, 
    tcpip_tcp_port_t src_port, tcpip_tcp_port_t dst_port,
    uint32_t seq, uint32_t ack, uint16_t win, tcpip_tcp_flags_t flags)
{
    /* tcp header pointer */
    tcpip_tcp_frame_t *tcp = frame->tcp;

    /* setup bitfields */
    TCPIPTcpFrame_SetFlags(tcp, flags);
    TCPIPTcpFrame_SetDataOffs(tcp, sizeof(*tcp));
    /* setup ports */
    TCPIPTcpFrame_SetDstPort(tcp, dst_port);
    TCPIPTcpFrame_SetSrcPort(tcp, src_port);

    /* setup data sequencing */
    TCPIPTcpFrame_SetSeq(tcp, seq);
    TCPIPTcpFrame_SetAck(tcp, ack);
    TCPIPTcpFrame_SetWindow(tcp, win);

    /* urgent poitner is not supported */
    TCPIPTcpFrame_SetUrgentPtr(tcp, 0);

    /* prepare for underlying layers */
    frame->ptr = tcp;
    frame->size += sizeof(tcpip_tcp_frame_t);

    /* bombs away! */
    return TCPIPIp_Send(frame, dst_addr, TCPIP_IP_PROTOCOL_TCP);
}
