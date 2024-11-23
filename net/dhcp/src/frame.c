/**
 * @file frame.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-21
 * 
 * @copyright Copyright (c) 2024
 */



#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "sys/sem.h"
#include "net/dhcp/frame.h"
#include "sys/yield.h"
#include "util/elems.h"

#define DEBUG DLVL_DEBUG
#include "debug.h"


/* store all the addresses into the frame */
err_t DHCPFrame_RenderAddresses(void *ptr, size_t size,
    dhcp_addrset_t *adrs)
{
    /* default values for the fields that are not set*/
    tcpip_ip_addr_t def_ip = TCPIP_IP_ADDR_ZERO;
    tcpip_eth_addr_t def_hw = TCPIP_ETH_ADDR_ZERO;
    /* frame pointer */
    dhcp_frame_t *frame = ptr;
    /* no space to store the addresses */
    if (size < sizeof(dhcp_frame_t))
        return EFATAL;

    /* store ci address */
    DHCPFrame_SetCIAddr(frame,
        adrs->addrflags & DHCP_ADDRFLAGS_CIADDR ? adrs->ci : def_ip);
    /* store yi address */
    DHCPFrame_SetYIAddr(frame,
        adrs->addrflags & DHCP_ADDRFLAGS_YIADDR ? adrs->yi : def_ip);
    /* store si address */
    DHCPFrame_SetSIAddr(frame,
        adrs->addrflags & DHCP_ADDRFLAGS_SIADDR ? adrs->si : def_ip);
    /* store gi address */
    DHCPFrame_SetGIAddr(frame,
        adrs->addrflags & DHCP_ADDRFLAGS_GIADDR ? adrs->gi : def_ip);
    /* store ch address */
    DHCPFrame_SetCHAddr(frame,
        adrs->addrflags & DHCP_ADDRFLAGS_CHADDR ? adrs->ch : def_hw);

    /* report status */
    return EOK;
}

/* parse the addresses from the frame */
err_t DHCPFrame_ParseAddresses(const void *ptr, size_t size,
    dhcp_addrset_t *adrs)
{
    /* default values for the fields that are not set*/
    tcpip_ip_addr_t def_ip = TCPIP_IP_ADDR_ZERO;
    tcpip_eth_addr_t def_hw = TCPIP_ETH_ADDR_ZERO;

    /* frame pointer */
    const dhcp_frame_t *frame = ptr;
    /* no space to store the addresses */
    if (size < sizeof(dhcp_frame_t))
        return EFATAL;

    /* read ci address */
    adrs->ci = DHCPFrame_GetCIAddr(frame);
    /* set the flag */
    if (!TCPIPIpAddr_AddressMatch(adrs->ci, def_ip))
        adrs->addrflags |= DHCP_ADDRFLAGS_CIADDR;

    /* read yi address */
    adrs->yi = DHCPFrame_GetYIAddr(frame);
    /* set the flag */
    if (!TCPIPIpAddr_AddressMatch(adrs->yi, def_ip))
        adrs->addrflags |= DHCP_ADDRFLAGS_YIADDR;

    /* read si address */
    adrs->si = DHCPFrame_GetSIAddr(frame);
    /* set the flag */
    if (!TCPIPIpAddr_AddressMatch(adrs->si, def_ip))
        adrs->addrflags |= DHCP_ADDRFLAGS_SIADDR;

    /* read gi address */
    adrs->gi = DHCPFrame_GetGIAddr(frame);
    /* set the flag */
    if (!TCPIPIpAddr_AddressMatch(adrs->gi, def_ip))
        adrs->addrflags |= DHCP_ADDRFLAGS_GIADDR;

    /* read ch address */
    adrs->ch = DHCPFrame_GetCHAddr(frame);
    /* set the flag */
    if (!TCPIPEthAddr_AddressMatch(adrs->ch, def_hw))
        adrs->addrflags |= DHCP_ADDRFLAGS_CHADDR;

    /* report status */
    return EOK;
}

/* render the options into the buffer */
void * DHCPFrame_RenderOptions(void *ptr, size_t size, dhcp_optset_t *opts)
{
    /* error code */
    uint8_t *pcurr = ptr, *pstart = ptr;

    /* message type */
    if (pcurr && (opts->optflags & DHCP_OPTFLAGS_MSGTYPE))
        pcurr = DHCPFrameOpt_SetMessageType(pcurr, size - (pcurr - pstart),
            opts->msg_type);
    /* set subnet mask */
    if (pcurr && (opts->optflags & DHCP_OPTFLAGS_SUBNET))
        pcurr = DHCPFrameOpt_SetSubnetMask(pcurr, size - (pcurr - pstart),
            opts->subnet);
    /* set router address */
    if (pcurr && (opts->optflags & DHCP_OPTFLAGS_ROUTER))
        pcurr = DHCPFrameOpt_SetRouterIP(pcurr, size - (pcurr - pstart),
            opts->router);
    /* dns server addresses */
    if (pcurr && (opts->optflags & DHCP_OPTFLAGS_DNS_SERVERS))
        pcurr = DHCPFrameOpt_SetDNSServerIP(pcurr, size - (pcurr - pstart),
            opts->dns, opts->dns_cnt);
    /* set the requested ip */
    if (pcurr && (opts->optflags & DHCP_OPTFLAGS_REQUESTED_IP))
        pcurr = DHCPFrameOpt_SetRequestedIP(pcurr, size - (pcurr - pstart),
            opts->req_ip);
    /* set the lease time */
    if (pcurr && (opts->optflags & DHCP_OPTFLAGS_LEASE_TIME))
        pcurr = DHCPFrameOpt_SetLeaseTime(pcurr, size - (pcurr - pstart),
            opts->lease_time);
    /* set the dhcp server ip */
    if (pcurr && (opts->optflags & DHCP_OPTFLAGS_DHCP_SERVER))
        pcurr = DHCPFrameOpt_SetDHCPServerIP(pcurr, size - (pcurr - pstart),
            opts->server);
    /* store parameter list */
    if (pcurr && (opts->optflags & DHCP_OPTFLAGS_REQ_PARAM_LIST))
        pcurr = DHCPFrameOpt_SetParameterRequestList(pcurr,
            size - (pcurr - pstart), opts->req_list, opts->req_list_cnt);

    /* terminate the options? */
    if (pcurr && opts->optflags & DHCP_OPTFLAGS_END)
        pcurr = DHCPFrameOpt_SetOptionsEnd(pcurr, size - (pcurr - pstart));

    /* return null pointer in case of an error */
    return pcurr;
}



/* read the options field, extraxt the meaningful information */
const void * DHCPFrame_ParseOptions(const void *ptr, size_t size,
    dhcp_optset_t *opts)
{
    /* data pointers */
    const uint8_t *pcurr = ptr, *pstart = ptr;
    /* clear the options field */
    opts->optflags = 0;

    /* parse as long as there are records present */
    while (1) {
        /* extracter option code and option size */
        dhcp_option_t opt_code; size_t opt_size;
        /* no data to process */
        if (!size)
            return 0;
        /* end of options */
        if (*pcurr == DHCP_OPTION_END) {
            /* show that we have reached the end */
            opts->optflags |= DHCP_OPTFLAGS_END; return ++pcurr;
        }

        /* get the information about the current option */
        const void *pnext = DHCPFrameOpt_GetOptionHdr(pcurr,
            size - (pcurr - pstart), &opt_code, &opt_size);
        /* not enough space for the header of next option record */
        if (!pnext)
            return pnext;

        /* switch on extractable options */
        switch (opt_code) {
        /* extract the message type */
        case DHCP_OPTION_MESSAGE_TYPE: {
            if (DHCPFrameOpt_GetMessageType(pcurr, &opts->msg_type))
                opts->optflags |= DHCP_OPTFLAGS_MSGTYPE;
        } break;
        /* requested ip */
        case DHCP_OPTION_REQUESTED_IP: {
            if (DHCPFrameOpt_GetRequestedIP(pcurr, &opts->req_ip))
                opts->optflags |= DHCP_OPTFLAGS_REQUESTED_IP;
        } break;
        /* extract subnet mask */
        case DHCP_OPTION_SUBNET_MASK: {
            if (DHCPFrameOpt_GetSubnetMask(pcurr, &opts->subnet))
                opts->optflags |= DHCP_OPTFLAGS_SUBNET;
        } break;
        /* extract router ip */
        case DHCP_OPTION_ROUTER: {
            if (DHCPFrameOpt_GetRouterIP(pcurr, &opts->router))
                opts->optflags |= DHCP_OPTFLAGS_ROUTER;
        } break;
        /* lease time in seconds */
        case DHCP_OPTION_LEASE_TIME: {
            if (DHCPFrameOpt_GetLeaseTime(pcurr, &opts->lease_time))
                opts->optflags |= DHCP_OPTFLAGS_LEASE_TIME;
        } break;

        /* dns server ip address */
        case DHCP_OPTION_DNS_SERVERS: {
            /* clear the counter */
            opts->dns_cnt = 0;
            /* read all the records */
            for (int i = 0; i < elems(opts->dns); i++)
                if (DHCPFrameOpt_GetDNSServerIP(pcurr, &opts->dns[i], i))
                    opts->dns_cnt++;
            /* got at least one valid dns record */
            if (opts->dns_cnt)
                opts->optflags |= DHCP_OPTFLAGS_DNS_SERVERS;
        } break;
        /* dhcp server itself */
        case DHCP_OPTION_DHCP_SERVER: {
            if (DHCPFrameOpt_GetDHCPServerIP(pcurr, &opts->server))
                opts->optflags |= DHCP_OPTFLAGS_DHCP_SERVER;
        } break;

        /* parse parameter request list */
        case DHCP_OPTION_PARAM_LIST: {
            /* clear the counter */
            opts->req_list_cnt = 0;
            /* parse the list */
            for (int i = 0; i < elems(opts->req_list); i++) {
                if (DHCPFrameOpt_GetParameterRequestList(pcurr,
                    &opts->req_list[i], i))
                    opts->req_list_cnt++;
            }
            /* at least one element was read */
            if (opts->req_list_cnt)
                opts->optflags |= DHCP_OPTFLAGS_REQ_PARAM_LIST;
        } break;

        /* move forward */
        default: {
            dprintf_w("unsupported option: type = %d, size = %d\n",
                opt_code, opt_size);
        } break;
        }

        /* move to next option */
        pcurr = pnext;
    }

    /* return error code */
    return pcurr;
}
