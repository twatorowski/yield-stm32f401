/**
 * @file dhcp_srv.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-18
 *
 * @copyright Copyright (c) 2024
 */

#include "assert.h"
#include "err.h"
#include "net/tcpip/dhcp.h"
#include "net/tcpip/dhcp_frame.h"
#include "net/tcpip/udp_sock.h"
#include "sys/yield.h"
#include "util/elems.h"

/* setup debug */
#define DEBUG DLVL_DEBUG
#include "debug.h"

/** address association  */
typedef struct dhcp_record {
    /* start of the lease timesamp or 0 to denote an empty entry  */
    dtime_t ts;
    /* mac address */
    tcpip_eth_addr_t mac;
    /* ip address */
    tcpip_ip_addr_t ip;
} dhcp_record_t;

/* database of records */
static dhcp_record_t records[4];

/* find record by hw address */
static dhcp_record_t * TCPIPDhcpSrv_FindByHWAddr(tcpip_eth_addr_t ha)
{
    /* go through all the records */
    for (dhcp_record_t *r = records; r - records != elems(records); r++) {
        /* an empty record */
        if (!r->ts)
            continue;
        /* record found */
        if (TCPIPEthAddr_AddressMatch(r->mac, ha))
            return r;
    }

    /* no record was found */
    return 0;
}

/* find an empty record */
static dhcp_record_t * TCPIPDhcpSrv_FindFree(void)
{
    /* go through all the records */
    for (dhcp_record_t *r = records; r - records != elems(records); r++) {
        /* an empty record was found */
        if (!r->ts)
            return r;
    }
    /* no record was found */
    return 0;
}

/* send offer */
static err_t TCPIPDhcpSrv_SendResponseOffer(tcpip_udp_sock_t *sock,
    uint32_t xid, uint32_t lease_time)
{
    /* work buffer */
    uint8_t buf[512];
    /* place the frame on the buffer */
    tcpip_dhcp_frame_t *dhcp = (void *)buf;
    /* options pointer */
    uint8_t *opts = dhcp->pld;

    /* initialize frame */
    TCPIPDhcpFrame_Init(dhcp);
    /* setup frame fields */
    TCPIPDhcpFrame_SetOP(dhcp, TCPIP_DHCP_OP_RESPONSE);
    TCPIPDhcpFrame_SetHType(dhcp, TCPIP_DHCP_HTYPE_ETH);
    TCPIPDhcpFrame_SetHLen(dhcp, 6);
    TCPIPDhcpFrame_SetHops(dhcp, 0);

    /* clear flags */
    TCPIPDhcpFrame_SetFlags(dhcp, 0);
    /* set the transaction id */
    TCPIPDhcpFrame_SetXID(dhcp, xid);
    /* set mac */
    TCPIPDhcpFrame_SetCHAddr(dhcp, TCPIPEthAddr_GetMAC());

    /* fill in all the options */
    opts = TCPIPDhcpFrameOpt_SetMessageType(opts, TCPIP_DHCP_MSG_TYPE_OFFER);
    opts = TCPIPDhcpFrameOpt_SetSubnetMask(opts, TCPIPIpAddr_GetSubnetMask());
    opts = TCPIPDhcpFrameOpt_SetLeaseTime(opts, lease_time);
    opts = TCPIPDhcpFrameOpt_SetOptionsEnd(opts);

    /* send the frame */
    return TCPIPUdpSock_SendTo(sock, (tcpip_ip_addr_t) TCPIP_IP_ADDR_BCAST,
        TCPIP_DHCP_CLT_PORT, dhcp, opts - buf);
}

/* send ack */
static err_t TCPIPDhcpSrv_SendResponseAcknowledgement(tcpip_udp_sock_t *sock,
    uint32_t xid, tcpip_dhcp_optset_t *opts)
{
    /* work buffer */
    uint8_t buf[512];
    /* place the frame on the buffer */
    tcpip_dhcp_frame_t *dhcp = (void *)buf;
    /* options pointer */
    uint8_t *opt_ptr = dhcp->pld;

    /* initialize frame */
    TCPIPDhcpFrame_Init(dhcp);
    /* setup frame fields */
    TCPIPDhcpFrame_SetOP(dhcp, TCPIP_DHCP_OP_RESPONSE);
    TCPIPDhcpFrame_SetHType(dhcp, TCPIP_DHCP_HTYPE_ETH);
    TCPIPDhcpFrame_SetHLen(dhcp, 6);
    TCPIPDhcpFrame_SetHops(dhcp, 0);

    /* clear flags */
    TCPIPDhcpFrame_SetFlags(dhcp, 0);
    /* set the transaction id */
    TCPIPDhcpFrame_SetXID(dhcp, xid);
    /* set mac */
    TCPIPDhcpFrame_SetCHAddr(dhcp, TCPIPEthAddr_GetMAC()); // TODO: this is wrong

    /* fill in all the options */
    opt_ptr = TCPIPDhcp_RenderOptions(opt_ptr, sizeof(buf) - sizeof(*dhcp), opts);
    /* we were unable to render options */
    if (!opt_ptr)
        return EFATAL;

    /* send the frame */
    return TCPIPUdpSock_SendTo(sock, (tcpip_ip_addr_t) TCPIP_IP_ADDR_BCAST,
        TCPIP_DHCP_CLT_PORT, dhcp, opt_ptr - buf);
}


/* process discovery request issued by the client */
static err_t TCPIPDhcpSrv_ProcessRequestDiscover(
    const tcpip_dhcp_frame_t *frame, tcpip_dhcp_optset_t *opts)
{
    /* get the transaction identifier */
    uint32_t xid = TCPIPDhcpFrame_GetXID(frame);


    // TODO: we shall implement this shit
    return EFATAL;
}

/* process requests */
static err_t TCPIPDhcpSrv_ProcessRequest(const tcpip_dhcp_frame_t *frame,
    tcpip_dhcp_optset_t *opts)
{
    /* processing error code */
    err_t ec;

    /* switch on message type */
    switch (opts->msg_type) {
    /* client wants to discover the dhcp server */
    case TCPIP_DHCP_MSG_TYPE_DISCOVER: {
        ec = TCPIPDhcpSrv_ProcessRequestDiscover(frame, opts);
    } break;
    case TCPIP_DHCP_MSG_TYPE_REQUEST: {
    } break;
    case TCPIP_DHCP_MSG_TYPE_RELEASE: {
    } break;


    /* TODO: implement more pls ;-) */
    default: ec = EFATAL;
    }

    /* unsupported requests go here */
    return ec;
}

/* process the frame that was received */
static err_t TCPIPDhcpSrv_Input(tcpip_ip_addr_t ip, tcpip_udp_port_t port,
    const void *ptr, size_t size)
{
    /* map to the pointer */
    const tcpip_dhcp_frame_t *frame = ptr;
    /* display info about the frame */
    dprintf_d("dhcp rx: %d, %x\n", TCPIPDhcpFrame_GetOP(frame),
        TCPIPDhcpFrame_GetXID(frame));

    /* unsupported medium */
    if (TCPIPDhcpFrame_GetHType(frame) != TCPIP_DHCP_HTYPE_ETH ||
        TCPIPDhcpFrame_GetHLen(frame) != 6) {
        return EFATAL;
    }

    /* parse options  */
    tcpip_dhcp_optset_t opts = { 0 };

    /* unable to process options */
    if (TCPIPDhcp_ParseOptions(frame->pld, size - sizeof(*frame),
        &opts) != EOK)
        return EFATAL;
    /* no message type set */
    if (!(opts.optflags & TCPIP_DHCP_OPTFLAGS_MSGTYPE))
        return EFATAL;

    /* process based on the op field value */
    switch (TCPIPDhcpFrame_GetOP(frame)) {
    /* process requests */
    case TCPIP_DHCP_OP_REQUEST: {
        return TCPIPDhcpSrv_ProcessRequest(frame, &opts);
    } break;

    /* process responses */
    case TCPIP_DHCP_OP_RESPONSE: {
    } break;
    }

    /* unsupported type of frame */
    return EFATAL;
}

/* dhcp server task */
static void TCPIPDhcpSrv_Task(void *arg)
{
    /* buffer for holding the received frame */
    static uint8_t rx_buf[512];

    /* create the socket */
    tcpip_udp_sock_t *sock = TCPIPUdpSock_CreateSocket(67, 512);
    /* whops */
    assert(sock, "unable to create the socket for dhcp server");

    /* processing loop */
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
        TCPIPDhcpSrv_Input(ip, port, rx_buf, ec);
    }
}

/* initialize dhcp server */
err_t TCPIPDhcpSrv_Init(void)
{
    /* create the task for serving dhcp service */
    Yield_Task(TCPIPDhcpSrv_Task, 0, 1024);

    /* return status */
    return EOK;
}