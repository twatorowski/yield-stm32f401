/**
 * @file dhcp.h
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2022-04-13
 *
 * @brief TCP/IP DHCP client implementation
 */

#ifndef NET_TCPIP_DHCP_H
#define NET_TCPIP_DHCP_H


#include "net/tcpip/dhcp_frame.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/eth_addr.h"
#include "util/bit.h"

/** ports */
#define TCPIP_DHCP_SRV_PORT             67
#define TCPIP_DHCP_CLT_PORT             68

/* all the options extracted from options field */
typedef struct tcpip_dhcp_optset {
    /* options that are present */
    enum tcpip_dhcp_optflags {
        TCPIP_DHCP_OPTFLAGS_MSGTYPE = BIT_VAL(0),
        TCPIP_DHCP_OPTFLAGS_SUBNET = BIT_VAL(1),
        TCPIP_DHCP_OPTFLAGS_ROUTER = BIT_VAL(2),
        TCPIP_DHCP_OPTFLAGS_DNS_SERVERS = BIT_VAL(3),
        TCPIP_DHCP_OPTFLAGS_REQUESTED_IP = BIT_VAL(4),
        TCPIP_DHCP_OPTFLAGS_LEASE_TIME = BIT_VAL(5),
        TCPIP_DHCP_OPTFLAGS_DHCP_SERVER = BIT_VAL(6),
        TCPIP_DHCP_OPTFLAGS_REQ_PARAM_LIST = BIT_VAL(7),
        /* termination */
        TCPIP_DHCP_OPTFLAGS_END = BIT_VAL(8),
    } optflags;
    /* message type */
    tcpip_dhcp_msg_type_t msg_type;
    /* subnet mask, router ip, dhcp server ip, */
    tcpip_ip_addr_t subnet, router, server;
    /* domain name server ip and count */
    tcpip_ip_addr_t dns[3]; int dns_cnt;
    /* lease time */
    uint32_t lease_time;
    /* parameter request list */
    tcpip_dhcp_param_req_t req_list[32]; int req_list_cnt;
    /* requested ip */
    tcpip_ip_addr_t req_ip;
    /* dhcp server ip */
    tcpip_ip_addr_t dhcp_ip;
} tcpip_dhcp_optset_t;


/**
 * @brief render dhcp options into a buffer
 *
 * @param ptr buffer pointer
 * @param size buffer size
 * @param opts options to be rendered
 *
 * @return void * pointer to the location after the options or 0 if an error
 * has occured
 */
void * TCPIPDhcp_RenderOptions(void *ptr, size_t size,
    tcpip_dhcp_optset_t *opts);

/**
 * @brief Parse all the options from the frame
 *
 * @param ptr pointer to the frame payload from which we read the info
 * @param size size of the payload
 * @param opts resulting options record
 *
 * @return err_t parsing error code
 */
err_t TCPIPDhcp_ParseOptions(const void *ptr, size_t size,
    tcpip_dhcp_optset_t *opts);





#endif /* NET_TCPIP_DHCP_H */
