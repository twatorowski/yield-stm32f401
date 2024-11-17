/**
 * @file dhcp.c
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2022-04-14
 *
 * @brief TCP/IP: DHCP Client implementation
 */

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "net/tcpip/ip.h"
#include "net/tcpip/udp.h"
#include "net/tcpip/udp_sock.h"
#include "net/tcpip/dhcp.h"
#include "net/tcpip/dhcp_frame.h"
#include "sys/sem.h"
#include "sys/yield.h"
#include "util/elems.h"

#define DEBUG
#include "debug.h"

/* udp socket for the dhcp client */
static tcpip_udp_sock_t *sock;

/* current state */
static enum state {
    UTM_STATE_IDLE,
    STATE_RESET,
    STATE_DISCOVER,
    STATE_OFFER_WAIT,
    STATE_REQUEST,
    STATE_REPLY_WAIT,
    STATE_BOUND,
} state;
/* state change timestamp */
static time_t state_ts;

/* current lease state */
static enum lease_state {
    LEASE_INVALID,
    LEASE_VALID,
} lease_state;

/* clinet enabled flag*/
static int enabled = TCPIP_DHCP_ENABLED;
/* transaction id */
static uint32_t xid;
/* lease timer */
static time_t lease_start;
/* time remaining before lease needs renewal */
static dtime_t lease_duration;
/* aquired network data */
static tcpip_ip_addr_t ip, subnet, gateway, server;
/* dns servers */
static tcpip_ip_addr_t dns[2];

/* all the options extracted from options field */
typedef struct dhcp_options {
    /* options that are present */
    enum options {
        OPTION_MSGTYPE = BIT_VAL(0),
        OPTION_SUBNET = BIT_VAL(1),
        OPTION_ROUTER = BIT_VAL(2),
        OPTION_SERVER = BIT_VAL(3),
        OPTION_DNS = BIT_VAL(4),
        OPTION_LEASE_TIME = BIT_VAL(5),
    } options;
    /* message type */
    tcpip_dhcp_msg_type_t msg_type;
    /* subnet mask, router ip, dhcp server ip, */
    tcpip_ip_addr_t subnet, router, server;
    /* domain name server ip and count */
    tcpip_ip_addr_t dns[3]; int dns_cnt;
    /* lease time */
    uint32_t lease_time;
} dhcp_options_t;

/* read the options field, extraxt the meaningful information */
static err_t TCPIPDhcp_ParseOptions(const void *ptr, size_t size,
    dhcp_options_t *opts)
{
    /* error code */
    err_t ec = EOK;
    /* clear the options field */
    opts->options = 0;
    /* parse as long as there are records present */
    while (1) {
        /* extracter option code and option size */
        tcpip_dhcp_option_t opt_code; size_t opt_size;
        /* get the information about the current option */
        uint8_t *next_opt = TCPIPDhcpFrameOpt_GetOptionHdr(ptr, &opt_code,
            &opt_size);

        /* no more options */
        if (opt_code == TCPIP_DHCP_OPTION_END)
            break;
        /* option wont fit */
        if ((uintptr_t)next_opt - (uintptr_t)ptr > size)
            return EFATAL;

        /* switch on extractable options */
        switch (opt_code) {
        /* extract the message type */
        case TCPIP_DHCP_OPTION_MESSAGE_TYPE: {
            if (TCPIPDhcpFrameOpt_GetMessageType(ptr, &opts->msg_type))
                opts->options |= OPTION_MSGTYPE;
        } break;
        /* extract subnet mask */
        case TCPIP_DHCP_OPTION_SUBNET_MASK: {
            if (TCPIPDhcpFrameOpt_GetSubnetMask(ptr, &opts->subnet))
                opts->options |= OPTION_SUBNET;
        } break;
        /* extract router ip */
        case TCPIP_DHCP_OPTION_ROUTER: {
            if (TCPIPDhcpFrameOpt_GetRouterIP(ptr, &opts->router))
                opts->options |= OPTION_ROUTER;
        } break;
        /* lease time in seconds */
        case TCPIP_DHCP_OPTION_LEASE_TIME: {
            if (TCPIPDhcpFrameOpt_GetLeaseTime(ptr, &opts->lease_time))
                opts->options |= OPTION_LEASE_TIME;
        } break;
        /* dns server ip address */
        case TCPIP_DHCP_OPTION_DNS_SERVERS: {
            /* read all the records */
            for (int i = (opts->dns_cnt = 0); i < elems(opts->dns); i++)
                if (TCPIPDhcpFrameOpt_GetDNSServerIP(ptr, &opts->dns[i], i))
                    opts->dns_cnt = i + 1;
            /* got at least one valid dns record */
            if (opts->dns_cnt)
                opts->options |= OPTION_DNS;
        } break;
        /* dhcp server itself */
        case TCPIP_DHCP_OPTION_DHCP_SERVER: {
            if (TCPIPDhcpFrameOpt_GetDHCPServerIP(ptr, &opts->server))
                opts->options |= OPTION_SERVER;
        } break;
        /* move forward */
        default: break;
        }

        /* move to next option */
        ptr = next_opt;
    }

    /* return error code */
    return ec;
}

/* send discover frame */
static err_t TCPIPDhcp_SendDiscover(tcpip_udp_sock_t *sock)
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
    TCPIPDhcpFrame_SetOP(dhcp, TCPIP_DHCP_OP_REQUEST);
    TCPIPDhcpFrame_SetHType(dhcp, TCPIP_DHCP_HTYPE_ETH);
    TCPIPDhcpFrame_SetHLen(dhcp, 6);
    TCPIPDhcpFrame_SetHops(dhcp, 0);

    /* force the server to respond with broadcast frame */
    TCPIPDhcpFrame_SetFlags(dhcp, TCPIP_DHCP_FLAGS_BROADCAST);

    /* set the transaction id */
    TCPIPDhcpFrame_SetXID(dhcp, xid);
    /* set mac */
    TCPIPDhcpFrame_SetCHAddr(dhcp, TCPIPEthAddr_GetMAC());

    /* now let's setup the options */
    opts = TCPIPDhcpFrameOpt_SetMessageType(opts, TCPIP_DHCP_MSG_TYPE_DISCOVER);
    opts = TCPIPDhcpFrameOpt_SetParameterRequestList(opts,
        (tcpip_dhcp_param_req_t []) {
            TCPIP_DHCP_PARAM_REQ_SUBNET_MASK,
            TCPIP_DHCP_PARAM_REQ_ROUTER,
            TCPIP_DHCP_PARAM_REQ_DOMAIN_NAME_SERVER,
        }, 3);
    opts = TCPIPDhcpFrameOpt_SetOptionsEnd(opts);

    /* send the frame */
    return TCPIPUdpSock_SendTo(sock, (tcpip_ip_addr_t) TCPIP_IP_ADDR_BCAST,
        TCPIP_DHCP_SRV_PORT, dhcp, opts - buf);
}

/* send dhcp request */
static err_t TCPIPDhcp_SendRequest(tcpip_udp_sock_t *sock)
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
    TCPIPDhcpFrame_SetOP(dhcp, TCPIP_DHCP_OP_REQUEST);
    TCPIPDhcpFrame_SetHType(dhcp, TCPIP_DHCP_HTYPE_ETH);
    TCPIPDhcpFrame_SetHLen(dhcp, 6);
    TCPIPDhcpFrame_SetHops(dhcp, 0);

    /* force the server to respond with broadcast frame */
    TCPIPDhcpFrame_SetFlags(dhcp, TCPIP_DHCP_FLAGS_BROADCAST);

    /* set the transaction id */
    TCPIPDhcpFrame_SetXID(dhcp, xid);
    /* set mac & ip addresses */
    TCPIPDhcpFrame_SetCHAddr(dhcp, TCPIPEthAddr_GetMAC());
    TCPIPDhcpFrame_SetCIAddr(dhcp, ip);
    TCPIPDhcpFrame_SetSIAddr(dhcp, server);

    /* now let's setup the options */
    opts = TCPIPDhcpFrameOpt_SetMessageType(opts, TCPIP_DHCP_MSG_TYPE_REQUEST);
    opts = TCPIPDhcpFrameOpt_SetRequestedIP(opts, ip);
    opts = TCPIPDhcpFrameOpt_SetDHCPServerIP(opts, server);
    opts = TCPIPDhcpFrameOpt_SetOptionsEnd(opts);

    /* send the frame */
    return TCPIPUdpSock_SendTo(sock, (tcpip_ip_addr_t) TCPIP_IP_ADDR_BCAST,
        TCPIP_DHCP_SRV_PORT, dhcp, opts - buf);
}

/* process offer */
static err_t TCPIPDhcp_ProcessOffer(tcpip_dhcp_frame_t *dhcp,
    dhcp_options_t *options)
{
    /* required fields */
    enum options req_fields = OPTION_SUBNET | OPTION_ROUTER | OPTION_LEASE_TIME |
        OPTION_SERVER | OPTION_DNS;

    /* not all required options were given */
    if ((options->options & req_fields) != req_fields)
        return EFATAL;

    /* extract ip addresses */
    ip = TCPIPDhcpFrame_GetYIAddr(dhcp);
    server = TCPIPDhcpFrame_GetSIAddr(dhcp);
    /* copy the subnet mask and router ip */
    subnet = options->subnet;
    gateway = options->router;
    /* copy dns server ip address */
    for (int i = 0; i < options->dns_cnt && i < elems(dns); i++)
        dns[i] = options->dns[i];

    /* move to the request state */
    state = STATE_REQUEST;

    /* report status */
    return EOK;
}

/* process the acknowledgement */
static err_t TCPIPDhcp_ProcessAck(tcpip_dhcp_frame_t *dhcp,
    dhcp_options_t *options)
{
    /* invalid ip was granted */
    if (!TCPIPIpAddr_AddressMatch(TCPIPDhcpFrame_GetYIAddr(dhcp), ip))
        return EFATAL;

    /* lease time not given */
    if ((options->options & OPTION_LEASE_TIME) == 0)
        return EFATAL;

    /* setup lease timer */
    lease_duration = dtime_from_sec(options->lease_time);
    /* mark the start of the lease */
    lease_start = time(0);

    /* reconfigure the ip layer */
    TCPIPIpAddr_SetIP(ip);
    TCPIPIpAddr_SetGatewayIP(gateway);
    TCPIPIpAddr_SetSubnetMask(subnet);

    /* mark the lease as valid */
    lease_state = LEASE_VALID;
    /* go back to */
    state = STATE_BOUND;

    /* only for debuggung */
    #ifdef DEBUG
        /* stringified ip addresses placeholders */
        tcpip_ip_addr_str_t ip_str, subnet_str, gateway_str;
        /* show to the world! */
        dprintf("got valid lease for ip: %s, subnet: %s, gw: %s\n",
            TCPIPIpAddr_ToStr(ip, ip_str), TCPIPIpAddr_ToStr(subnet, subnet_str),
            TCPIPIpAddr_ToStr(gateway, gateway_str));
        /* show the lease time as well */
        dprintf("lease time: %d (s)\n", options->lease_time);
    #endif

    /* report status */
    return EOK;
}

/* process denial */
static err_t TCPIPDhcp_ProcessNak(tcpip_dhcp_frame_t *dhcp,
    dhcp_options_t *options)
{
    /* go back to idle state */
    state = UTM_STATE_IDLE;
    /* nothing we can do */
    return EOK;
}

/* process state machine */
static void TCPIPDhcp_FSM(void)
{
    /* swotch on current state */
    switch (state) {
    /* we are in idle state */
    case UTM_STATE_IDLE: {
        /* no valid lease available? */
        if (lease_state == LEASE_INVALID)
            state = STATE_RESET, xid = time(0);
        /* invalidate the lease when it reaches the end of life */
        if (dtime(lease_start + lease_duration, time(0)) < 0)
            lease_state = LEASE_INVALID;
        /* dhcp client is disabled */
        if (!enabled)
            state = UTM_STATE_IDLE;
    } break;
    /* reset the ip layer */
    case STATE_RESET : {
        /* reset the ip layer */
        TCPIPIpAddr_SetIP((tcpip_ip_addr_t)TCPIP_IP_ADDR_ANY);
        TCPIPIpAddr_SetSubnetMask((tcpip_ip_addr_t)TCPIP_IP_ADDR_ANY);
        TCPIPIpAddr_SetGatewayIP((tcpip_ip_addr_t)TCPIP_IP_ADDR_ANY);
        /* advance */
        state = STATE_DISCOVER;
    } break;
    /* discover dhcp servers */
    case STATE_DISCOVER : {
        /* send the discover frame */
        if (TCPIPDhcp_SendDiscover(sock) >= EOK)
            state = STATE_OFFER_WAIT, state_ts = time(0);
    } break;
    /* wait for the dhcp server to issue an offer */
    case STATE_OFFER_WAIT : {
        /* no valid frame within the timeout window */
        if (dtime(time(0), state_ts) > 2000)
            state = STATE_DISCOVER, state_ts = time(0);
    } break;
    /* request the lease using the data collected during discovery-offer phase */
    case STATE_REQUEST : {
        if (TCPIPDhcp_SendRequest(sock) >= EOK)
            state = STATE_REPLY_WAIT, state_ts = time(0);
    } break;
    /* wait for the ack or nak */
    case STATE_REPLY_WAIT : {
        /* no valid frame within the timeout window */
        if (dtime(time(0), state_ts) > 2000)
            state = STATE_DISCOVER, state_ts = time(0);
    } break;
    /* ip is now bound */
    case STATE_BOUND: {
        /* invalidate the lease when it reaches the end of life */
        if (dtime(lease_start + lease_duration, time(0)) < lease_duration / 2)
            state = STATE_REQUEST;
        /* no more time */
        if (dtime(lease_start + lease_duration, time(0)) < 0)
            lease_state = LEASE_INVALID;
        /* no valid lease */
        if (lease_state == LEASE_INVALID)
            state = UTM_STATE_IDLE;
    } break;
    }
}

/* process the incoming frame */
static err_t TCPIPDhcp_Input(void *arg, size_t size)
{
    /* frame pointer */
    tcpip_dhcp_frame_t *dhcp = arg;

    /* we are the client side and we only support responses */
    if (TCPIPDhcpFrame_GetOP(dhcp) != TCPIP_DHCP_OP_RESPONSE)
        return EFATAL;
    /* we only do ethernet here */
    if (TCPIPDhcpFrame_GetHType(dhcp) != TCPIP_DHCP_HTYPE_ETH ||
        TCPIPDhcpFrame_GetHLen(dhcp) != 6)
        return EFATAL;
    /* extract the hw address */
    if (!TCPIPEthAddr_IsMatchingUnicast(TCPIPDhcpFrame_GetCHAddr(dhcp)))
        return EFATAL;

    /* check the transaction id */
    if (TCPIPDhcpFrame_GetXID(dhcp) != xid)
        return EFATAL;

    /* parse options TODO: for some reason gcc complains with
     * -Wmaybe-uninitialized warning when options do not have the message
     * type initialized to 0 */
    dhcp_options_t options = { .msg_type = 0 };
    /* unable to process options */
    if (TCPIPDhcp_ParseOptions(dhcp->pld, size - sizeof(*dhcp), &options) != EOK)
        return EFATAL;
    /* no message type set */
    if (!(options.options & OPTION_MSGTYPE))
        return EFATAL;

    /* switch on the message type */
    switch (options.msg_type) {
    case TCPIP_DHCP_MSG_TYPE_OFFER: return TCPIPDhcp_ProcessOffer(dhcp, &options);
    case TCPIP_DHCP_MSG_TYPE_ACK: return TCPIPDhcp_ProcessAck(dhcp, &options);
    case TCPIP_DHCP_MSG_TYPE_NAK: return TCPIPDhcp_ProcessNak(dhcp, &options);
    default: return EFATAL;
    }

    /* nothing to proces the frame with */
    return EFATAL;
}

/* tx and fsm task */
static void TCPIPDhcp_TxTask(void *arg)
{
    /* process the dhcp client state machine */
    for (;; Yield()) {
        /* process the state machine */
        TCPIPDhcp_FSM();
    }
}

/* listener task */
static void TCPIPDhcp_RxTask(void *arg)
{
    /* reception buffer */
    static uint8_t buf[1024];
    /* source credentials */
    tcpip_ip_addr_t src_ip; tcpip_udp_port_t src_port;

    /* process incoming data */
    for (;; Yield()) {
        /* receive data from client socket */
        err_t size = TCPIPUdpSock_RecvFrom(sock, &src_ip, &src_port, buf,
            sizeof(buf), 0);
        /* ignore errors */
        if (size <= EOK)
            continue;
        /* parse the frame */
        TCPIPDhcp_Input(buf, size);
    }
}

/* initialze common parts of the layer */
err_t TCPIPDhcp_Init(void)
{
    /* start the socket */
    sock = TCPIPUdpSock_CreateSocket(TCPIP_DHCP_CLT_PORT, 512);
    /* no mem for the socket? */
    if (!sock)
        return EFATAL;

    /* create reception task */
    if (Yield_CreateTask(TCPIPDhcp_TxTask, 0, 2048) != EOK)
        return EFATAL;
    /* create state machine task */
    if (Yield_CreateTask(TCPIPDhcp_RxTask, 0, 2048) != EOK)
        return EFATAL;

    /* report status  */
    return EOK;
}

/* return the status of the lease */
int TCPIPDhcp_IsLeaseValid(void)
{
    return lease_state == LEASE_VALID;
}

/* forces the lease renewal */
err_t TCPIPDhcp_InvalidateLease(void)
{
    /* reset the lease state */
    lease_state = LEASE_INVALID;
    /* return status */
    return EOK;
}

/* enable or disable dhcp client */
err_t TCPIPDhcp_SetEnable(int enable)
{
    /* set the flag state */
    enable = !!enable;
    /* return status */
    return EOK;
}