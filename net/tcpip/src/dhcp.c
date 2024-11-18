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

#define DEBUG DLVL_DEBUG
#include "debug.h"

/* render the options into the buffer */
err_t TCPIPDhcp_RenderOptions(void *ptr, size_t size,
    tcpip_dhcp_optset_t *opts)
{
    /* error code */
    uint8_t *p8 = ptr;

    // TODO: these shall propably check size
    /* message type */
    if (p8 && (opts->optflags & TCPIP_DHCP_OPTFLAGS_MSGTYPE))
        p8 = TCPIPDhcpFrameOpt_SetMessageType(p8, opts->msg_type);
    /* set subnet mask */
    if (p8 && (opts->optflags & TCPIP_DHCP_OPTFLAGS_SUBNET))
        p8 = TCPIPDhcpFrameOpt_SetSubnetMask(p8, opts->subnet);
    /* set router address */
    if (p8 && (opts->optflags & TCPIP_DHCP_OPTFLAGS_ROUTER))
        p8 = TCPIPDhcpFrameOpt_SetRouterIP(p8, opts->router);
    /* dns server addresses */
    if (p8 && (opts->optflags & TCPIP_DHCP_OPTFLAGS_DNS_SERVERS))
        p8 = TCPIPDhcpFrameOpt_SetDNSServerIP(p8, opts->dns, opts->dns_cnt);
    /* set the requested ip */
    if (p8 && (opts->optflags & TCPIP_DHCP_OPTFLAGS_REQUESTED_IP))
        p8 = TCPIPDhcpFrameOpt_SetRequestedIP(p8, opts->req_ip);
    /* set the lease time */
    if (p8 && (opts->optflags & TCPIP_DHCP_OPTFLAGS_LEASE_TIME))
        p8 = TCPIPDhcpFrameOpt_SetLeaseTime(p8, opts->lease_time);
    /* set the dhcp server ip */
    if (p8 && (opts->optflags & TCPIP_DHCP_OPTFLAGS_DHCP_SERVER))
        p8 = TCPIPDhcpFrameOpt_SetDHCPServerIP(p8, opts->dhcp_ip);
    /* store parameter list */
    if (p8 && (opts->optflags & TCPIP_DHCP_OPTFLAGS_REQ_PARAM_LIST))
        p8 = TCPIPDhcpFrameOpt_SetParameterRequestList(p8, opts->req_list,
            opts->req_list_cnt);

    /* terminate the options? */
    if (p8 && opts->optflags & TCPIP_DHCP_OPTFLAGS_END)
        p8 = TCPIPDhcpFrameOpt_SetOptionsEnd(p8);

    /* unable to render elements? */
    return !p8 ? EFATAL : p8 - (uint8_t *)ptr;
}

/* read the options field, extraxt the meaningful information */
err_t TCPIPDhcp_ParseOptions(const void *ptr, size_t size,
    tcpip_dhcp_optset_t *opts)
{
        /* error code */
    err_t ec = EOK;
    /* clear the options field */
    opts->optflags = 0;

    dprintf_d("OPTIONS START, size = %d\n", size);

    /* parse as long as there are records present */
    while (1) {
        /* extracter option code and option size */
        tcpip_dhcp_option_t opt_code; size_t opt_size;
        /* get the information about the current option */
        uint8_t *next_opt = TCPIPDhcpFrameOpt_GetOptionHdr(ptr, &opt_code,
            &opt_size);

        /* no more options */
        if (opt_code == TCPIP_DHCP_OPTION_END) {
            opts->optflags |= TCPIP_DHCP_OPTFLAGS_END;
            dprintf_d("OPTIONS END\n", 0);
            break;
        }
        /* option wont fit */
        if ((uintptr_t)next_opt - (uintptr_t)ptr > size) {
            return EFATAL;
        }

        /* switch on extractable options */
        switch (opt_code) {
        /* extract the message type */
        case TCPIP_DHCP_OPTION_MESSAGE_TYPE: {
            if (TCPIPDhcpFrameOpt_GetMessageType(ptr, &opts->msg_type)) {
                opts->optflags |= TCPIP_DHCP_OPTFLAGS_MSGTYPE;
                /* show debug */
                dprintf_d("message type = %x\n", opts->msg_type);
            }
        } break;
        /* extract subnet mask */
        case TCPIP_DHCP_OPTION_SUBNET_MASK: {
            if (TCPIPDhcpFrameOpt_GetSubnetMask(ptr, &opts->subnet)) {
                opts->optflags |= TCPIP_DHCP_OPTFLAGS_SUBNET;
                /* show debug */
                dprintf_d("subnet = %x\n", opts->subnet.u32);
            }
        } break;
        /* extract router ip */
        case TCPIP_DHCP_OPTION_ROUTER: {
            if (TCPIPDhcpFrameOpt_GetRouterIP(ptr, &opts->router)) {
                opts->optflags |= TCPIP_DHCP_OPTFLAGS_ROUTER;
                /* show debug */
                dprintf_d("router = %x\n", opts->router.u32);
            }
        } break;
        /* lease time in seconds */
        case TCPIP_DHCP_OPTION_LEASE_TIME: {
            if (TCPIPDhcpFrameOpt_GetLeaseTime(ptr, &opts->lease_time)) {
                opts->optflags |= TCPIP_DHCP_OPTFLAGS_LEASE_TIME;
                /* show debug */
                dprintf_d("lease time = %x\n", opts->lease_time);
            }
        } break;
        /* dns server ip address */
        case TCPIP_DHCP_OPTION_DNS_SERVERS: {
            /* clear the counter */
            opts->dns_cnt = 0;
            /* read all the records */
            for (int i = 0; i < elems(opts->dns); i++)
                if (TCPIPDhcpFrameOpt_GetDNSServerIP(ptr, &opts->dns[i], i)) {
                    opts->dns_cnt++;
                    /* show debug */
                    dprintf_d("dns: id = %d, addr = %x\n", i, opts->dns[i].u32);
                }
            /* got at least one valid dns record */
            if (opts->dns_cnt)
                opts->optflags |= TCPIP_DHCP_OPTFLAGS_DNS_SERVERS;
        } break;
        /* dhcp server itself */
        case TCPIP_DHCP_OPTION_DHCP_SERVER: {
            if (TCPIPDhcpFrameOpt_GetDHCPServerIP(ptr, &opts->server)) {
                opts->optflags |= TCPIP_DHCP_OPTFLAGS_DHCP_SERVER;
                /* show debug */
                dprintf_d("server: %x\n", opts->server.u32);
            }
        } break;
        /* parse parameter request list */
        case TCPIP_DHCP_OPTION_PARAM_LIST: {
            /* clear the counter */
            opts->req_list_cnt = 0;
            /* parse the list */
            for (int i = 0; i < elems(opts->req_list); i++) {
                /* valid entry found at that index? */
                if (TCPIPDhcpFrameOpt_GetParameterRequestList(ptr,
                    &opts->req_list[i], i)) {
                    opts->req_list_cnt++;
                    /* show debug */
                    dprintf_d("requested parameter id: %d, type = %d\n", i,
                        opts->req_list[i]);
                }
            }
            /* at least one element was read */
            if (opts->req_list_cnt)
                opts->optflags |= TCPIP_DHCP_OPTFLAGS_REQ_PARAM_LIST;
        } break;
        /* move forward */
        default: {
            dprintf_d("unsupported option: type = %d, size = %d\n",
                opt_code, opt_size);
        } break;
        }

        /* move to next option */
        ptr = next_opt;
    }

    dprintf_d("END OF PROCESSING\n", 0);
    /* return error code */
    return ec;
}