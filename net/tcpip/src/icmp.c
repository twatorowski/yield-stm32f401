/**
 * @file icmp.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief TCP/IP Stack: Internet Control Message Protocol
 */

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "config.h"
#include "err.h"
#include "net/tcpip/icmp_checksum.h"
#include "net/tcpip/icmp_frame.h"
#include "net/tcpip/ip.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/ip_frame.h"
#include "net/tcpip/tcpip.h"
#include "sys/time.h"
#include "sys/yield.h"
#include "util/endian.h"
#include "util/msblsb.h"
#include "util/string.h"

/** echo sequence numbers */
static uint16_t echo_seqno, echo_id, echo_ackno;

/* send echo response frame */
static err_t TCPIPIcmp_SendEcho(tcpip_ip_addr_t da, tcpip_icmp_type_t type, 
    uint16_t id, uint16_t seqno, const void *data, size_t data_size)
{
    /* error code */
    err_t ec;
    /* frame descriptor */
    tcpip_frame_t frame;
    /* allocate space for underlying ethernet frame */
    if ((ec = TCPIPIp_Alloc(&frame)) != EOK)
        return ec;

    /* setup icmp layer */
    frame.flags |= TCPIP_FRAME_FLAGS_ICMP;
    frame.icmp = frame.ptr;
    /* cast to appropriate pointers */
    tcpip_icmp_frame_t *icmp = frame.icmp;
    tcpip_icmp_pld_echo_req_res_t *echo = 
        (tcpip_icmp_pld_echo_req_res_t *)icmp->pld;

    /* setup icmp header */
    TCPIPIcmpFrame_SetType(icmp, type);
    TCPIPIcmpFrame_SetCode(icmp, 0);
    /* setup echo payload */
    TCPIPIcmpFrameEcho_SetID(echo, id);
    TCPIPIcmpFrameEcho_SetSeqNo(echo, seqno);

    /* copy data section */
    memcpy(echo->data, data, data_size);
    /* derive total message size */
    size_t tot_size = data_size + sizeof(*icmp) + sizeof(*echo);
    /* compute the checksum */
    TCPIPIcmpChecksum_Set(icmp, tot_size);

    /* prepare frame for underlying layers */
    frame.size = tot_size;
    /* bombs away! */
    return TCPIPIp_Send(&frame, da, TCPIP_IP_PROTOCOL_ICMP);
}

/* process icmp request */
static err_t TCPIPIcmp_ProcessEchoRequest(tcpip_frame_t *frame)
{
    /* cast to appropriate pointers */
    tcpip_icmp_frame_t *icmp = (tcpip_icmp_frame_t *)frame->icmp;
    tcpip_icmp_pld_echo_req_res_t *echo = 
        (tcpip_icmp_pld_echo_req_res_t *)icmp->pld;

    /* decode identifier and sequential number */
    uint16_t id = TCPIPIcmpFrameEcho_GetID(echo);
    uint16_t seqno = TCPIPIcmpFrameEcho_GetSeqNo(echo);
    /* extract source address so that we know where to respond */
    tcpip_ip_addr_t sa = TCPIPIpFrame_GetSrcAddr(frame->ip);
    size_t data_size = frame->size - sizeof(*icmp) - sizeof(*echo);

    /* respond with echo response */
    return TCPIPIcmp_SendEcho(sa, TCPIP_ICMP_TYPE_ECHO_REPLY,
        id, seqno, echo->data, data_size);
}

/* process icmp echo response */
static err_t TCPIPIcmp_ProcessEchoResponse(tcpip_frame_t *frame)
{
    /* cast to appropriate pointers */
    tcpip_icmp_frame_t *icmp = frame->icmp;
    tcpip_icmp_pld_echo_req_res_t *echo = 
        (tcpip_icmp_pld_echo_req_res_t *)icmp->pld;
    /* extract source address so that we know where to respond */
    tcpip_ip_addr_t da = TCPIPIpFrame_GetSrcAddr(frame->ip);
    
    /* decode identifier and sequential number */
    uint16_t id = TCPIPIcmpFrameEcho_GetID(echo);
    uint16_t seqno = TCPIPIcmpFrameEcho_GetSeqNo(echo);

    /* echo responses come only from valid unicats addresses */
    if (TCPIPIpAddr_IsMatchingUnicast(da) && 
        echo_id == id && echo_ackno) {
        echo_ackno = seqno;
    }
    
    /* report success in frame processing */
    return EOK;
}

/* initialize ICMP layer */
err_t TCPIPIcmp_Init(void)
{
    /* return status */
    return EOK;
}

/* main input routine to the icmp layer */
err_t TCPIPIcmp_Input(tcpip_frame_t *frame)
{
    /* result/error code */
    tcpip_icmp_frame_t *icmp = frame->icmp; err_t ec = 0;
    /* icmp header is just after the ip */
    frame->icmp = frame->ptr;

    /* validate checksum */
    if (!TCPIPIcmpChecksum_IsValid(icmp, frame->size))
        return EFATAL;

    /* setup frame */
    frame->flags |= TCPIP_FRAME_FLAGS_ICMP;

    /* processing is based on the type of the request */
    switch (TCPIPIcmpFrame_GetType(icmp)) {
    /* process echo requests */
    case TCPIP_ICMP_TYPE_ECHO_REQUEST: {
        ec = TCPIPIcmp_ProcessEchoRequest(frame);
    } break;
    /* process echo responses */
    case TCPIP_ICMP_TYPE_ECHO_REPLY: {
        ec = TCPIPIcmp_ProcessEchoResponse(frame);
    } break;
    }

    /* report status */
    return ec;
}

/* ping given destination address */
err_t TCPIPIcmp_Ping(tcpip_ip_addr_t da, dtime_t timeout)
{
    /* timestamp */
    time_t ts = time(0);

    /* lock the echo mechanism */
    while (echo_ackno)
        Yield();
    
    /* reset flag */
    echo_ackno = 0xffff;
    /* do not use zeroed sequence number */
    if (++echo_seqno == 0)
        echo_seqno = 1;

    /* send echo request */
    err_t ec = TCPIPIcmp_SendEcho(da, TCPIP_ICMP_TYPE_ECHO_REQUEST, echo_id = 1, 
        echo_seqno, 0, 0);
    /* check if the send was successful, if so then set the ec value to eok */
    if ((ec = ec > EOK ? EOK : ec) < EOK)
        goto end;
    
    /* still no matching response? */
    while (echo_ackno != echo_seqno) {
        /* timeout has occured */
        if (timeout && dtime(time(0), ts) >= timeout) {
            ec = EUNREACHABLE; break;
        }
        /* no timeout, play the waiting game */
        Yield();
    }

    /* reset the ack number to indicate that the echo engine is now free */
    end: echo_ackno = 0;
    /* report success */
    return ec;
}