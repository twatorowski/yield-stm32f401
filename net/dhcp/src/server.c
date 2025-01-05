/**
 * @file dhcp_src.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-21
 * 
 * @copyright Copyright (c) 2024
 */

#include "assert.h"
#include "config.h"
#include "err.h"
#include "net/dhcp/frame.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/udp_sock.h"
#include "sys/yield.h"
#include "util/elems.h"

/* setup debug */
#define DEBUG DLVL_DEBUG
#include "debug.h"

#include "sys/sleep.h"



/** address association  */
typedef struct dhcp_record {
    /* current state */
    enum { STATE_FREE, STATE_OFFER, STATE_ASSIGNED } state;
    /* start of the lease timesamp or 0 to denote an empty entry  */
    dtime_t ts;
    /* mac address */
    tcpip_eth_addr_t mac;
    /* ip address */
    tcpip_ip_addr_t ip;
} dhcp_record_t;

/* database of records */
static dhcp_record_t records[DHCP_SRV_RECORDBOOK_CAPACITY];
/* ip address ranges that we are allowed to propose */
static const tcpip_ip_addr_t ip_range_start = DHCP_SRV_IP_RANGE_START,
    ip_range_end = DHCP_SRV_IP_RANGE_END;


/* find record by hw address or ip address */
static dhcp_record_t * DHCPSrv_FindRecord(tcpip_eth_addr_t *ha,
    tcpip_ip_addr_t *ip)
{
    /* go through all the records */
    for (dhcp_record_t *r = records; r - records != elems(records); r++) {
        /* looking for an empty record? */
        if (!ha && !ip) {
            if (r->state == STATE_FREE)
                return r;
        /* record found with matching address */
        } else if ((!ha || TCPIPEthAddr_AddressMatch(r->mac, *ha)) &&
            (!ip || TCPIPIpAddr_AddressMatch(r->ip, *ip))) {
            return r;
        }
    }

    /* no record was found */
    return 0;
}

/* try to reserve an address and update the recordbook */
static dhcp_record_t * DHCPSrv_ReserveAddress(tcpip_eth_addr_t hw,
    tcpip_ip_addr_t *reserved_ip)
{
    /* ip that we want to propose */
    tcpip_ip_addr_t ip = ip_range_start; int found = 0;
    /* get the dhcp record for that mac address */
    dhcp_record_t *rec = DHCPSrv_FindRecord(&hw, 0);

    /* use previously reserved address */
    if (rec && rec->ip.u32) {
        *reserved_ip = rec->ip; return EOK;
    }

    /* we did not find the record with this mac address, try to find anything
     * that is free */
    if (!rec && !(rec = DHCPSrv_FindRecord(0, 0)))
        return 0;

    /* scan across the ip range */
    for (; !found && !TCPIPIpAddr_AddressMatch(ip, ip_range_end);
        ip = TCPIPIpAddr_Next(ip)) {
        /* this ip is not found in our recordbook, so it's safe to use it */
        if (!DHCPSrv_FindRecord(0, &ip))
            found = 1;
    }
    /* could not reserve an address, sorry */
    if (!found)
        return 0;

    /* update the recordbook */
    rec->ip = ip; rec->mac = hw;
    /* caller wants to know the address? */
    if (reserved_ip)
        *reserved_ip = ip;

    /* return the pointer to the recordbook */
    return rec;
}

/* send a response frame with given addresses and option fields */
static err_t DHCPSrv_SendResponse(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port,
    uint32_t xid, dhcp_addrset_t *adrs, dhcp_optset_t *opts)
{
    /* work buffer: */
    uint8_t buf[512];

    /* place the frame on the buffer */
    dhcp_frame_t *frame = (void *)buf;
    /* options pointer */
    uint8_t *opt_ptr = frame->pld;

    /* zero out the frame and set the magic cookie value */
    DHCPFrame_Init(frame);
    /* setup basic frame fields */
    DHCPFrame_SetOP(frame, DHCP_OP_RESPONSE);
    DHCPFrame_SetHType(frame, DHCP_HTYPE_ETH);
    DHCPFrame_SetHLen(frame, 6);
    DHCPFrame_SetHops(frame, 0);
    /* clear flags */
    DHCPFrame_SetFlags(frame, 0x0000);
    /* set the transaction id */
    DHCPFrame_SetXID(frame, xid);

    /* store all requested address fields */
    if (DHCPFrame_RenderAddresses(frame, sizeof(buf), adrs) < EOK)
        return EFATAL;
    /* and store all the options */
    if (!(opt_ptr = DHCPFrame_RenderOptions(opt_ptr,
        sizeof(buf) - sizeof(*frame), opts)))
        return EFATAL;

    /* send the frame */
    return TCPIPUdpSock_SendTo(sock, ip, port, frame, opt_ptr - buf);
}

/* send the NAK response */
static err_t DHCPSrv_SendResponseNAK(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port,
    uint32_t xid, tcpip_eth_addr_t ch)
{

    /* adresses that we need to put into the response */
    dhcp_addrset_t adrs = {
        /* set the flags */
        .addrflags = DHCP_ADDRFLAGS_SIADDR | DHCP_ADDRFLAGS_CHADDR,
        /* fill in addresses */
        .si = TCPIPIpAddr_GetIP(),
        .ch = ch
    };

    /* options that we need to put into the response */
    dhcp_optset_t opts = {
        /* set the options */
        .optflags = DHCP_OPTFLAGS_MSGTYPE | DHCP_OPTFLAGS_END,
        /* fill in type of message */
        .msg_type = DHCP_MSG_TYPE_NAK,
    };
    /* send the response */
    return DHCPSrv_SendResponse(sock, ip, port, xid, &adrs, &opts);
}

/* process discovery request issued by the client */
static err_t DHCPSrv_ProcessDiscover(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port,
    const dhcp_frame_t *frame, dhcp_addrset_t *adrs, dhcp_optset_t *opts)
{
    /* placeholder for the reserved ip */
    tcpip_ip_addr_t reserved_ip; dhcp_record_t *rec;

    /* get the transaction identifier */
    uint32_t xid = DHCPFrame_GetXID(frame);
    /* transaction id and mac address must be specified */
    if (!xid || !(adrs->addrflags & DHCP_ADDRFLAGS_CHADDR))
        return EFATAL;
    /* try to reserve an ip address */
    if (!(rec = DHCPSrv_ReserveAddress(adrs->ch, &reserved_ip)))
        return EFATAL;

    /* update the record */
    rec->ts = time(0), rec->state = STATE_OFFER;

    /* adresses that we need to put into the response */
    dhcp_addrset_t resp_adrs = {
        /* set the flags */
        .addrflags = DHCP_ADDRFLAGS_YIADDR | DHCP_ADDRFLAGS_SIADDR |
            DHCP_ADDRFLAGS_CHADDR | DHCP_ADDRFLAGS_GIADDR,
        /* fill in addresses */
        .yi = reserved_ip,
        .si = TCPIPIpAddr_GetIP(),
        .gi = TCPIPIpAddr_GetGatewayIP(),
        .ch = adrs->ch,
    };

    /* options that we need to put into the response */
    dhcp_optset_t resp_opts = {
        /* set the options */
        .optflags = DHCP_OPTFLAGS_MSGTYPE | DHCP_OPTFLAGS_SUBNET |
            DHCP_OPTFLAGS_ROUTER | DHCP_OPTFLAGS_LEASE_TIME |
            DHCP_OPTFLAGS_DHCP_SERVER | DHCP_OPTFLAGS_END,
        /* fill in type of message */
        .msg_type = DHCP_MSG_TYPE_OFFER,
        /* setup addresses */
        .subnet = TCPIPIpAddr_GetSubnetMask(),
        .router = TCPIPIpAddr_GetGatewayIP(),
        /* time of the lease */
        .lease_time = 1 * 60,
        /* server ip */
        .server = TCPIPIpAddr_GetIP(),
    };

    /* send the response */
    return DHCPSrv_SendResponse(sock, (tcpip_ip_addr_t)TCPIP_IP_ADDR_BCAST,
        port, xid, &resp_adrs, &resp_opts);
}

/* process address request */
static err_t DHCPSrv_ProcessRequest(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port,
    const dhcp_frame_t *frame, dhcp_addrset_t *adrs, dhcp_optset_t *opts)
{
    /* get the transaction identifier */
    uint32_t xid = DHCPFrame_GetXID(frame);
    /* requested ip address */
    tcpip_ip_addr_t req_addr;
    /* transaction id and mac address must be specified */
    if (!xid || !(adrs->addrflags & DHCP_ADDRFLAGS_CHADDR))
        return EFATAL;

    /* client has received the ip address once in the past */
    if (adrs->addrflags & DHCP_ADDRFLAGS_CIADDR) {
        req_addr = adrs->ci;
    /* this is the request after the offer */
    } else if (opts->optflags & DHCP_OPTFLAGS_REQUESTED_IP) {
        req_addr = opts->req_ip;
    /* no ip address - not a valid request */
    } else {
        return EFATAL;
    }

    /* find the record */
    dhcp_record_t *rec = DHCPSrv_FindRecord(&adrs->ch, &req_addr);
    /* no record for that ip/mac combination or the record is in the
     * wrong state */
    if (!rec || !(rec->state == STATE_OFFER || rec->state == STATE_ASSIGNED))
        return DHCPSrv_SendResponseNAK(sock, ip, port, xid, adrs->ch);

    /* adresses that we need to put into the response */
    dhcp_addrset_t resp_adrs = {
        /* set the flags */
        .addrflags = DHCP_ADDRFLAGS_YIADDR | DHCP_ADDRFLAGS_SIADDR |
            DHCP_ADDRFLAGS_CHADDR | DHCP_ADDRFLAGS_GIADDR,
        /* fill in addresses */
        .yi = rec->ip,
        .si = TCPIPIpAddr_GetIP(),
        .gi = TCPIPIpAddr_GetGatewayIP(),
        .ch = adrs->ch,
    };

    /* options that we need to put into the response */
    dhcp_optset_t resp_opts = {
        /* set the options */
        .optflags = DHCP_OPTFLAGS_MSGTYPE | DHCP_OPTFLAGS_SUBNET |
            DHCP_OPTFLAGS_ROUTER | DHCP_OPTFLAGS_LEASE_TIME |
            DHCP_OPTFLAGS_DHCP_SERVER | DHCP_OPTFLAGS_END,
        /* fill in type of message */
        .msg_type = DHCP_MSG_TYPE_ACK,
        /* setup addresses */
        .subnet = TCPIPIpAddr_GetSubnetMask(),
        .router = TCPIPIpAddr_GetGatewayIP(),
        /* time of the lease */
        .lease_time = 15 * 60,
        /* server ip */
        .server = TCPIPIpAddr_GetIP(),
    };

    /* send the response */
    err_t ec = DHCPSrv_SendResponse(sock,  (tcpip_ip_addr_t)TCPIP_IP_ADDR_BCAST,
        port, xid, &resp_adrs, &resp_opts);
    /* unable to send the frame */
    if (ec < EOK)
        return EFATAL;
    /* update the record */
    rec->state = STATE_ASSIGNED; rec->ts = time(0);
    /* report success */
    return ec;
}

/* process requests */
static err_t DHCPSrv_Process(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port,
    const dhcp_frame_t *frame, dhcp_addrset_t *adrs, dhcp_optset_t *opts)
{
    /* processing error code */
    err_t ec;

    dprintf_d("req: type = %x\n", opts->msg_type);

    /* switch on message type */
    switch (opts->msg_type) {
    /* client wants to discover the dhcp server */
    case DHCP_MSG_TYPE_DISCOVER: {
        ec = DHCPSrv_ProcessDiscover(sock, ip, port, frame, adrs, opts);
    } break;
    /* client want's to lease the ip address */
    case DHCP_MSG_TYPE_REQUEST: {
        ec = DHCPSrv_ProcessRequest(sock, ip, port, frame, adrs, opts);
    } break;
    /* client wants to let go of the address */
    case DHCP_MSG_TYPE_RELEASE: {
    } break;

    /* TODO: implement more pls ;-) */
    default: ec = EFATAL;
    }

    /* unsupported requests go here */
    return ec;
}

/* process the frame that was received */
static err_t DHCPSrv_Input(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port, const void *ptr, size_t size)
{
    /* map to the pointer */
    const dhcp_frame_t *frame = ptr;
    /* data extracted from the frame */
    dhcp_addrset_t adrs = { 0 }; dhcp_optset_t opts = { 0 };

    /* display info about the frame */
    dprintf_d("dhcp rx: %d, %x\n", DHCPFrame_GetOP(frame),
        DHCPFrame_GetXID(frame));

    /* unsupported medium */
    if (DHCPFrame_GetHType(frame) != DHCP_HTYPE_ETH ||
        DHCPFrame_GetHLen(frame) != 6) {
        return EFATAL;
    }

    /* parse addresses from the frame */
    if (DHCPFrame_ParseAddresses(frame, size, &adrs) < EOK)
        return EFATAL;
    /* parse options */
    if (!DHCPFrame_ParseOptions(frame->pld, size - sizeof(*frame), &opts))
        return EFATAL;

    /* no message type set */
    if (!(opts.optflags & DHCP_OPTFLAGS_MSGTYPE))
        return EFATAL;

    /* process based on the op field value */
    switch (DHCPFrame_GetOP(frame)) {
    /* process requests */
    case DHCP_OP_REQUEST: {
        return DHCPSrv_Process(sock, ip, port, frame, &adrs, &opts);
    } break;
    /* unsupported type of frame */
    default: return EFATAL;
    }

    /* unsupported type of frame */
    return EFATAL;
}

/* dhcp server task */
static void DHCPSrv_Task(void *arg)
{
    /* buffer for holding the received frame */
    static uint8_t rx_buf[512];

    /* create the socket */
    tcpip_udp_sock_t *sock = TCPIPUdpSock_CreateSocket(DHCP_SRV_PORT, 512);
    /* unable to allocate memory for the socket */
    assert(sock, "unable to create the socket for dhcp server");

    // Sleep(5000);

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
        DHCPSrv_Input(sock, ip, port, rx_buf, ec);
    }
}

/* initialize dhcp server */
err_t DHCPSrv_Init(void)
{
    /* create the task for serving dhcp service */
    if (Yield_Task(DHCPSrv_Task, 0, 2048) < EOK)
        return EFATAL;
    /* return status */
    return EOK;
}