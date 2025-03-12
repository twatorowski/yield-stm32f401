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
#include "util/string.h"

#define DEBUG DLVL_DEBUG
#include "debug.h"

typedef struct mdns_record {
    /* type of record */
    uint16_t type;

    /* ip address for the 'A' records */
    tcpip_ip_addr_t ip;
    /* name for the 'cname' records */
    const char *name;
} mdns_record_t;

/* list of dns records */
static mdns_record_t records[] = {
    /* main A record for converting name into ip address */
    [0] = { .type = MDNS_RECORD_TYPE_A,
        /* we use zero to make the record dynamic */
        .ip = TCPIP_IP_ADDR_ZERO, .name = MDNS_SRV_DEVICE_NAME  }
};

/* find the record within the record base */
static mdns_record_t * MDNSSrv_FindRecord(uint16_t type, const char *name)
{
    /* look into the database */
    for (mdns_record_t *r = records; r != records + elems(records); r++)
        if (r->type == type && strcmp(name, r->name) == 0) {
            return r;
        }

    /* no record was found */
    return 0;
}

/* send a response frame */
static err_t MDNSSrv_SendResponse(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port,
    uint16_t transaction_id, uint16_t rcode, uint16_t answers_cnt,
    void *rsps, size_t rsps_size)
{
    /* work buffer */
    uint8_t buf[512];
    /* map the frame pointer to the work buffer */
    mdns_frame_t *frame = (mdns_frame_t *)buf;

    /* not enough space to build up the response frame */
    if (rsps_size + sizeof(*frame) > sizeof(buf))
        return EFATAL;

    /* set basic header stuff */
    MDNSFrame_SetTransactionID(frame, transaction_id);
    MDNSFrame_SetFlags(frame, MDNS_FRAME_HDR_FLAGS_QR_RESP |
        MDNS_FRAME_HDR_FLAGS_OPCODE_STD_QUERY | rcode);

    /* set the counts */
    MDNSFrame_SetQuestionsCount(frame, 0);
    MDNSFrame_SetAnswersCount(frame, answers_cnt);
    MDNSFrame_SetAuthorityRRCount(frame, 0);
    MDNSFrame_SetAdditionalRRCount(frame, 0);

    /* copy the responses */
    memcpy(frame->pld, rsps, rsps_size);

    /* send the frame back */
    return TCPIPUdpSock_SendTo(sock, ip, port, frame,
        sizeof(*frame) + rsps_size);
}

/* produce an answer */
static void * MDNSSrv_RenderAnswerA(void *ptr, size_t size, mdns_record_t *rec)
{
    /* pointers to write stuff down */
    uint8_t *p = ptr, *ps = ptr; err_t ec;

    /* unable to encode the name into the buffer */
    if ((ec = MDNSFrame_EncodeName(rec->name, p, size)) < EOK)
        return 0;
    /* mission sucessfull */
    p += ec;

    /* let's continue with the fields that go after the name */
    mdns_answer_fields_t *fields = (mdns_answer_fields_t *)p;
    /* let's continue with rdata field */
    mdns_answer_rdata_a_t *rdata = (mdns_answer_rdata_a_t *)fields->rdata;

    /* not enough space in buffer to fit all the data */
    if (size < (p - ps) + sizeof(mdns_answer_fields_t) +
        sizeof(mdns_answer_rdata_a_t))
        return 0;

    /* set the basic fields */
    MDNSFrameAnswer_SetType(fields, MDNS_RECORD_TYPE_A);
    MDNSFrameAnswer_SetClass(fields, MDNS_RECORD_CLASS_IA);
    MDNSFrameAnswer_SetTTL(fields, 60);
    MDNSFrameAnswer_SetRDLength(fields, sizeof(*rdata));

    /* ip address to be returned */
    tcpip_ip_addr_t ip = rec->ip;
    /* get our current ip address */
    if (TCPIPIpAddr_AddressMatch(ip, (tcpip_ip_addr_t)TCPIP_IP_ADDR_ZERO))
        ip = TCPIPIpAddr_GetIP();

    /* set the rdata field */
    MDNSFrameRDataA_SetIP(rdata, ip);

    /* return the pointer to after the record */
    return p + sizeof(*fields) +  sizeof(*rdata);
}

/* process one question from the buffer and return the pointer to another one */
static const void * MDNSSrv_ProcessQuestion(const void *ptr,
    const mdns_frame_t *frame, size_t frame_size,
    uint8_t **rsp, size_t rsp_size, size_t *rsp_count)
{
    /* name in question */
    char qname[256]; const uint8_t *p = ptr; uint8_t *r = *rsp, *rs = *rsp;
    /* fields that follow the qname */
    const mdns_question_fields_t *fields;

    /* initialize response counter */
    *rsp_count = 0;

    /* try to decode the name */
    err_t ec = MDNSFrame_DecodeName(p, frame, frame_size, qname,
        sizeof(qname) - 1);
    /* error */
    if (ec < EOK)
        return 0;
    /* update the read pointer */
    p += ec;

    /* map the fields */
    fields = (const mdns_question_fields_t *)p;
    /* unsupported class */
    if (MDNSFrameQuestion_GetClass(fields) != MDNS_RECORD_CLASS_IA)
        return 0;
    /* update the read pointer */
    p += sizeof(*fields);

    /* get the question type */
    uint16_t qtype = MDNSFrameQuestion_GetType(fields);
    /* find the record */
    mdns_record_t *rec = MDNSSrv_FindRecord(qtype, qname);
    /* if we dont find the record for that type then we do not respond */
    if (!rec)
        return p;


    /* switch on the type of the record */
    switch (qtype) {
    /* type A: convert name to ip address */
    case MDNS_RECORD_TYPE_A: {
        r = MDNSSrv_RenderAnswerA(r, rsp_size - (r - rs), rec);
    } break;
    /* no renderer for that type */
    default: {
        assert(0, "no renderer for this record type");
    } break;
    }

    /* store the updated pointer and the response count. it will be set to 0
     * when rendering error has occured, like not enough space in output
     * buffers */
    *rsp = r; *rsp_count = r ? 1 : 0;
    /* return the pointer to the next question record */
    return p;
}

/* input routine for the mdns frames */
static err_t MDNSSrv_Input(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port, const void *ptr, size_t size)
{
    /* map the date to mdns pointer */
    const mdns_frame_t *frame = ptr; const uint8_t *p = frame->pld;
    /* buffer for rendering responses */
    uint8_t rsps[256], *r = rsps;

    /* get the transaction_id and flags */
    uint16_t transaction_id = MDNSFrame_GetTransactionID(frame);
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
    /* placeholder for the  number of responses */
    size_t rnum, rtotal = 0;
    /* process the questions */
    for (uint16_t q = 0; p && q < qnum; q++) {
        /* try to process the question and render the responses */
        if (!(p = MDNSSrv_ProcessQuestion(p, frame, size,
                &r, sizeof(rsps) - (r - rsps), &rnum)) || !r)
            return EFATAL;
        /* sum up the total count of the responses */
        rtotal += rnum;
    }

    /* show how many records did we generate */
    dprintf_d("produced %d answers for %d questions\n", rtotal, qnum);
    /* no responses generated */
    if (!rtotal)
        return EFATAL;

    /* return parsing status */
    return MDNSSrv_SendResponse(sock, (tcpip_ip_addr_t)MDNS_SRV_MCAST_IP,
        port, transaction_id, MDNS_FRAME_HDR_FLAGS_RCODE_OK, rtotal,
        rsps, r - rsps);
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