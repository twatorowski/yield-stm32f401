/**
 * @file server.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-23
 * 
 * @copyright Copyright (c) 2024
 */

#include "assert.h"
#include "config.h"
#include "err.h"

#include "net/mdns/frame.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/udp_sock.h"
#include "sys/yield.h"
#include "util/elems.h"

#define DEBUG DLVL_DEBUG
#include "debug.h"

/* send a response frame //TODO: add payload */
static err_t MDNSSrv_SendResponse(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port,
    uint16_t trandaction_id, uint16_t rcode)
{
    /* work buffer */
    uint8_t buf[512];
    /* map the frame pointer to the work buffer */
    mdns_frame_t *frame = (mdns_frame_t *)buf;

    MDNSFrame_SetTransactionID(frame, trandaction_id);
    MDNSFrame_SetFlags(frame, MDNS_FRAME_HDR_FLAGS_QR_RESP |
        MDNS_FRAME_HDR_FLAGS_OPCODE_STD_QUERY | rcode);
    /* set the counts // TODO: we shall probably render some answers here xD */
    MDNSFrame_SetQuestionsCount(frame, 0);
    MDNSFrame_SetAnswersCount(frame, 0);
    MDNSFrame_SetAuthorityRRCount(frame, 0);
    MDNSFrame_SetAdditionalRRCount(frame, 0);

    /* send the frame back */
    return TCPIPUdpSock_SendTo(sock, ip, port, frame, sizeof(frame));
}

/* process one question from the buffer and return the pointer to another one */
static const void * MDNSSrv_ProcessQuestion(const void *ptr,
    const mdns_frame_t *frame, size_t frame_size)
{
    /* name in question */
    char qname[256]; const uint8_t *p = ptr;
    /* fields that follow the qname */
    const mdns_question_fields_t *fields;

    /* try to decode the name */
    err_t ec = MDNSFrame_DecodeName(p, (uintptr_t)ptr - (uintptr_t)frame,
        frame_size, qname, sizeof(qname) - 1);
    /* error */
    if (ec < EOK)
        return 0;
    /* map the fields */
    fields = (const mdns_question_fields_t *)(p + ec);

    /* show the extracted name */
    dprintf_d("Question: name %s, length = %d, class  %x, type = %x\n",
        qname, ec, MDNSFrameQuestion_GetClass(fields),
        MDNSFrameQuestion_GetType(fields));
    /* return the pointer to the next record */
    return p + ec + sizeof(*fields);
}

/* input routine for the mdns frames */
static err_t MDNSSrv_Input(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port, const void *ptr, size_t size)
{
    /* map the date to mdns pointer */
    const mdns_frame_t *frame = ptr; const uint8_t *p = frame->pld, *ps = ptr;

    /* get the flags */
    uint16_t flags = MDNSFrame_GetFlags(frame);
    /* we only support queries */
    if ((flags & MDNS_FRAME_HDR_FLAGS_QR) != MDNS_FRAME_HDR_FLAGS_QR_QUERY)
        return EFATAL;
    /* we only support standard queries */
    if ((flags & MDNS_FRAME_HDR_FLAGS_OPCODE) !=
        MDNS_FRAME_HDR_FLAGS_OPCODE_STD_QUERY)
        return EFATAL;

    /* get the number of questions */
    uint16_t qnum = MDNSFrame_GetQuestionsCount(frame);

    dprintf_d("flags = %x\n", flags);
    dprintf_d("qnum = %x\n", qnum);

    /* process the questions */
    for (uint16_t q = 0; p && q < qnum; q++) {
        if (!(p = MDNSSrv_ProcessQuestion(p, frame, size))) {
            dprintf_d("error at question %d\n", q);
            return EFATAL;
        }
        dprintf_d("processed %d of %d\n", p-ps, size);
    }

    /* return parsing status */
    return EOK;
}

/* server task */
static void MDNSSrv_Task(void *arg)
{
    /* buffer for holding the received frame */
    static uint8_t rx_buf[512];

    /* create the socket */
    tcpip_udp_sock_t *sock = TCPIPUdpSock_CreateSocket(MDNS_SRV_PORT, 512);
    /* unable to allocate memory for the socket */
    assert(sock, "unable to create the socket for mdns server");

    /* main serving loop */
    for (;; Yield()) {
        /* sender port and sender's ip address */
        tcpip_ip_addr_t ip; tcpip_udp_port_t port;
        /* receive data from the socket */
        err_t ec = TCPIPUdpSock_RecvFrom(sock, &ip, &port, rx_buf,
            sizeof(rx_buf), 0);
        /* error during reception */
        if (ec < EOK)
            continue;


        /* process frame */
        MDNSSrv_Input(sock, ip, port, rx_buf, ec);
    }
}

/* initialize server logic */
err_t MDNSSrv_Init(void)
{
    /* create the task for serving mdns requests */
    if (Yield_Task(MDNSSrv_Task, 0, 2048) < EOK)
        return EFATAL;
    /* return status of the initialization */
    return EOK;
}