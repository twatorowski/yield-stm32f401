/**
 * @file dhcp_frame.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-21
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef NET_DHCP_FRAME_H
#define NET_DHCP_FRAME_H


#include <stdint.h>

#include "err.h"
#include "compiler.h"
#include "net/tcpip/eth_addr.h"
#include "net/tcpip/eth_frame.h"
#include "net/tcpip/ip_addr.h"
#include "util/endian.h"
#include "util/string.h"
#include "util/bit.h"

/** operation type */
typedef enum dhcp_op {
    DHCP_OP_REQUEST = 0x01,
    DHCP_OP_RESPONSE = 0x02,
} dhcp_op_t;

/** hardware type  */
typedef enum dhcp_htype {
    DHCP_HTYPE_ETH = 0x01,
} dhcp_htype_t;

/** flags */
typedef enum dhcp_flags {
    DHCP_FLAGS_BROADCAST = 0x8000,
} dhcp_flags_t;

/** options */
typedef enum dhcp_option {
    DHCP_OPTION_SUBNET_MASK = 1,
    DHCP_OPTION_ROUTER = 3,
    DHCP_OPTION_DNS_SERVERS = 6,
    DHCP_OPTION_REQUESTED_IP = 50,
    DHCP_OPTION_LEASE_TIME = 51,
    DHCP_OPTION_MESSAGE_TYPE = 53,
    DHCP_OPTION_DHCP_SERVER = 54,
    DHCP_OPTION_PARAM_LIST = 55,
    DHCP_OPTION_END = 0xff,
} dhcp_option_t;

/** dhcp frame header */
typedef struct dhcp_frame {
    /* operation type */
    uint8_t op;
    /* hardware type */
    uint8_t htype;
    /* hardware address len */
    uint8_t hlen;
    /* number of network hops */
    uint8_t hops;
    /** transaction identifier */
    uint32_t xid;
    /* seconds count */
    uint16_t secs;
    /* flags field */
    uint16_t flags;

    /* clinets ip address */
    uint32_t ciaddr;
    /* address assigned to client */
    uint32_t yiaddr;
    /* server ip address */
    uint32_t siaddr;
    /* Gateway IP address switched by relay */
    uint32_t giaddr;
    /* clinet hardware address */
    uint8_t chaddr[16];
    /* server name and start file (legacy) */
    uint8_t reserved[192];
    /* magic cookie */
    uint32_t magic_cookie;
    /* options */
    uint8_t pld[];
} PACKED dhcp_frame_t;

/** message type */
typedef enum dhcp_msg_type {
    DHCP_MSG_TYPE_DISCOVER = 0x01,
    DHCP_MSG_TYPE_OFFER = 0x02,
    DHCP_MSG_TYPE_REQUEST = 0x03,
    DHCP_MSG_TYPE_DECLINE = 0x04,
    DHCP_MSG_TYPE_ACK = 0x05,
    DHCP_MSG_TYPE_NAK = 0x06,
    DHCP_MSG_TYPE_RELEASE = 0x07,
    DHCP_MSG_TYPE_INFORM = 0x08,
} dhcp_msg_type_t;

/** dhcp option header */
typedef struct dhcp_opt_hdr {
    /* option type size of the option payload */
    uint8_t option, size;
    // /* payload of varying size */
    // uint8_t pld[];
} PACKED dhcp_opt_hdr_t;

/** parameter request list elements */
typedef enum dhcp_param_req {
    DHCP_PARAM_REQ_SUBNET_MASK = 1,
    DHCP_PARAM_REQ_ROUTER = 3,
    DHCP_PARAM_REQ_DOMAIN_NAME = 15,
    DHCP_PARAM_REQ_DOMAIN_NAME_SERVER = 6,
} dhcp_param_req_t;


/* all the options extracted from options field */
typedef struct dhcp_optset {
    /* options that are present */
    enum dhcp_optflags {
        DHCP_OPTFLAGS_MSGTYPE = BIT_VAL(0),
        DHCP_OPTFLAGS_SUBNET = BIT_VAL(1),
        DHCP_OPTFLAGS_ROUTER = BIT_VAL(2),
        DHCP_OPTFLAGS_DNS_SERVERS = BIT_VAL(3),
        DHCP_OPTFLAGS_REQUESTED_IP = BIT_VAL(4),
        DHCP_OPTFLAGS_LEASE_TIME = BIT_VAL(5),
        DHCP_OPTFLAGS_DHCP_SERVER = BIT_VAL(6),
        DHCP_OPTFLAGS_REQ_PARAM_LIST = BIT_VAL(7),
        /* termination */
        DHCP_OPTFLAGS_END = BIT_VAL(8),
    } optflags;
    /* message type */
    dhcp_msg_type_t msg_type;
    /* subnet mask, router ip, dhcp server ip, */
    tcpip_ip_addr_t subnet, router, server;
    /* domain name server ip and count */
    tcpip_ip_addr_t dns[3]; int dns_cnt;
    /* lease time */
    uint32_t lease_time;
    /* parameter request list */
    dhcp_param_req_t req_list[32]; int req_list_cnt;
    /* requested ip */
    tcpip_ip_addr_t req_ip;
} dhcp_optset_t;

/* all the addresses extracted from the frame */
typedef struct dhcp_addrset {
    /* addresses that are present */
    enum dhcp_addrflags {
        DHCP_ADDRFLAGS_CIADDR = BIT_VAL(0),
        DHCP_ADDRFLAGS_YIADDR = BIT_VAL(1),
        DHCP_ADDRFLAGS_SIADDR = BIT_VAL(2),
        DHCP_ADDRFLAGS_GIADDR = BIT_VAL(3),
        DHCP_ADDRFLAGS_CHADDR = BIT_VAL(4),
    } addrflags;
    /* ip addresses */
    tcpip_ip_addr_t ci, yi, si, gi;
    /* hardware address */
    tcpip_eth_addr_t ch;
} dhcp_addrset_t;


/**
 * @brief Render the address fields based on the address set
 *
 * @param ptr dhcp frame pointer
 * @param size size of the buffer that stores the frame that we work with
 * @param adrs address set record
 *
 * @return err_t EOK in case of success
 */
err_t DHCPFrame_RenderAddresses(void *ptr, size_t size,
    dhcp_addrset_t *adrs);

/**
 * @brief parse the addresses from the frame
 *
 * @param ptr pointer to the frame
 * @param size size of the frame
 * @param adrs address set that we are about to populate with the data from
 * the frame
 *
 * @return err_t error code
 */
err_t DHCPFrame_ParseAddresses(const void *ptr, size_t size,
    dhcp_addrset_t *adrs);

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
void * DHCPFrame_RenderOptions(void *ptr, size_t size,
    dhcp_optset_t *opts);

/**
 * @brief Parse all the options from the frame
 *
 * @param ptr pointer to the frame payload from which we read the info
 * @param size size of the payload
 * @param opts resulting options record
 *
 * @return err_t parsing error code
 */
const void * DHCPFrame_ParseOptions(const void *ptr, size_t size,
    dhcp_optset_t *opts);

/**
 * @brief Initialize the dhcp frame header. Clear all fields etc..
 *
 * @param dhcp frame header pointer
 */
static inline ALWAYS_INLINE void DHCPFrame_Init(dhcp_frame_t *dhcp)
{
    /* clear all fields */
    memset(dhcp, 0, sizeof(*dhcp));
    /* set the magic cookie value */
    dhcp->magic_cookie = HTOBE32(0x63825363);
}

/**
 * @brief Sets headers OP field value
 *
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetOP(dhcp_frame_t *dhcp,
    dhcp_op_t op)
{
    dhcp->op = op;
}

/**
 * @brief Sets headers HTYPE field value
 *
 * @param dhcp frame header pointer
 * @param htype value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetHType(
    dhcp_frame_t *dhcp, dhcp_htype_t htype)
{
    dhcp->htype = htype;
}

/**
 * @brief Sets headers HLEN field value
 *
 * @param dhcp frame header pointer
 * @param len value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetHLen(
    dhcp_frame_t *dhcp, int hlen)
{
    dhcp->hlen = hlen;
}

/**
 * @brief Sets headers HOPS field value
 *
 * @param dhcp frame header pointer
 * @param ops value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetHops(
    dhcp_frame_t *dhcp, int hops)
{
    dhcp->hops = hops;
}

/**
 * @brief Sets headers XID field value
 *
 * @param dhcp frame header pointer
 * @param xid value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetXID(
    dhcp_frame_t *dhcp, uint32_t xid)
{
    dhcp->xid = HTOBE32(xid);
}

/**
 * @brief Sets headers flags field value
 *
 * @param dhcp frame header pointer
 * @param flags value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetFlags(
    dhcp_frame_t *dhcp, dhcp_flags_t flags)
{
    dhcp->flags = HTOBE16(flags);
}

/**
 * @brief Sets headers CIADDR field value
 *
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetCIAddr(
    dhcp_frame_t *dhcp, tcpip_ip_addr_t ciaddr)
{
    dhcp->ciaddr = HTOBE32(ciaddr.u32);
}

/**
 * @brief Sets headers YIADDR field value
 *
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetYIAddr(
    dhcp_frame_t *dhcp, tcpip_ip_addr_t yiaddr)
{
    dhcp->yiaddr = HTOBE32(yiaddr.u32);
}

/**
 * @brief Sets headers OSIADDR field value
 *
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetSIAddr(
    dhcp_frame_t *dhcp, tcpip_ip_addr_t siaddr)
{
    dhcp->siaddr = HTOBE32(siaddr.u32);
}

/**
 * @brief Sets headers GIADDR field value
 *
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetGIAddr(
    dhcp_frame_t *dhcp, tcpip_ip_addr_t giaddr)
{
    dhcp->giaddr = HTOBE32(giaddr.u32);
}

/**
 * @brief Sets headers CHADDR field value
 *
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void DHCPFrame_SetCHAddr(
    dhcp_frame_t *dhcp, tcpip_eth_addr_t chaddr)
{
    memcpy(dhcp->chaddr, chaddr.mac, sizeof(chaddr.mac));
}

/**
 * @brief Returns the OP field value
 *
 * @param dhcp frame header pointer
 * @return dhcp_op_t field value
 */
static inline ALWAYS_INLINE dhcp_op_t DHCPFrame_GetOP(
    const dhcp_frame_t *dhcp)
{
    return (dhcp_op_t)dhcp->op;
}

/**
 * @brief Returns the HTYPE field value
 *
 * @param dhcp frame header pointer
 * @return dhcp_htype_t field value
 */
static inline ALWAYS_INLINE dhcp_htype_t DHCPFrame_GetHType(
    const dhcp_frame_t *dhcp)
{
    return (dhcp_htype_t)dhcp->htype;
}

/**
 * @brief Returns the HLEN field value
 *
 * @param dhcp frame header pointer
 * @return int field value
 */
static inline ALWAYS_INLINE int DHCPFrame_GetHLen(
    const dhcp_frame_t *dhcp)
{
    return (int)dhcp->hlen;
}

/**
 * @brief Returns the HOPS field value
 *
 * @param dhcp frame header pointer
 * @return int field value
 */
static inline ALWAYS_INLINE int DHCPFrame_GetHops(
    const dhcp_frame_t *dhcp)
{
    return (int)dhcp->hops;
}

/**
 * @brief Returns the XID field value
 *
 * @param dhcp frame header pointer
 * @return uint32_t field value
 */
static inline ALWAYS_INLINE uint32_t DHCPFrame_GetXID(
    const dhcp_frame_t *dhcp)
{
    return BETOH32(dhcp->xid);
}

/**
 * @brief Returns the CIADDR field value
 *
 * @param dhcp frame header pointer
 * @return tcpip_ip_addr_t field value
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t DHCPFrame_GetCIAddr(
    const dhcp_frame_t *dhcp)
{
    return (tcpip_ip_addr_t) { .u32 = BETOH32(dhcp->ciaddr) };
}

/**
 * @brief Returns the YIADDR field value
 *
 * @param dhcp frame header pointer
 * @return tcpip_ip_addr_t field value
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t DHCPFrame_GetYIAddr(
    const dhcp_frame_t *dhcp)
{
    return (tcpip_ip_addr_t) { .u32 = BETOH32(dhcp->yiaddr) };
}

/**
 * @brief Returns the SIADDR field value
 *
 * @param dhcp frame header pointer
 * @return tcpip_ip_addr_t field value
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t DHCPFrame_GetSIAddr(
    const dhcp_frame_t *dhcp)
{
    return (tcpip_ip_addr_t) { .u32 = BETOH32(dhcp->siaddr) };
}

/**
 * @brief Returns the GIADDR field value
 *
 * @param dhcp frame header pointer
 * @return tcpip_ip_addr_t field value
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t DHCPFrame_GetGIAddr(
    const dhcp_frame_t *dhcp)
{
    return (tcpip_ip_addr_t) { .u32 = BETOH32(dhcp->giaddr) };
}

/**
 * @brief Returns the CHADDR field value
 *
 * @param dhcp frame header pointer
 * @return tcpip_eth_addr_t field value
 */
static inline ALWAYS_INLINE tcpip_eth_addr_t DHCPFrame_GetCHAddr(
     const dhcp_frame_t *dhcp)
{
    return (tcpip_eth_addr_t) { .mac = {dhcp->chaddr[0], dhcp->chaddr[1],
        dhcp->chaddr[2], dhcp->chaddr[3], dhcp->chaddr[4],
        dhcp->chaddr[5] } };
}


/**
 * @brief Write the end of options field
 *
 * @param ptr pointer to where to write to
 * @param size remaining size
 *
 * @return void * pointer after writing
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_SetOptionsEnd(void *ptr,
    size_t size)
{
    /* option record type */
    struct pld {
        uint8_t option;
    } PACKED *pld = ptr;

    /* insufficient space */
    if (size < sizeof(*pld))
        return 0;

    /* set marker */
    pld->option = DHCP_OPTION_END;
    /* return the address */
    return (void *)((uintptr_t)ptr + sizeof(*pld));
}

/**
 * @brief Write Message Type option
 *
 * @param ptr pointer to where to write to
 * @param size remaining size
 * @param msg_type message type to be stored
 *
 * @return void * pointer after writing
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_SetMessageType(void *ptr,
    size_t size, dhcp_msg_type_t msg_type)
{
    /* option record type */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* message type field */
        uint8_t msg_type;
    } PACKED *pld = ptr;

    /* insufficient space */
    if (size < sizeof(*pld))
        return 0;

    /* write the option */
    pld->hdr.option = DHCP_OPTION_MESSAGE_TYPE;
    pld->hdr.size = sizeof(*pld) - sizeof(pld->hdr);
    pld->msg_type = msg_type;
    /* return the address */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Write Requested IP option
 *
 * @param ptr payload pointer
 * @param size remaining size
 * @param ip ip to be written
 *
 * @return  void * pointer after writing
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_SetRequestedIP(void *ptr,
    size_t size, tcpip_ip_addr_t ip)
{
    /* option record type */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* requested ip */
        uint32_t ip;
    } PACKED *pld = ptr;

    /* insufficient space */
    if (size < sizeof(*pld))
        return 0;

    /* write the option */
    pld->hdr.option = DHCP_OPTION_REQUESTED_IP;
    pld->hdr.size = sizeof(*pld) - sizeof(pld->hdr);
    pld->ip = HTOBE32(ip.u32);

    /* return the address */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Writes parameter request list
 *
 * @param ptr payload pointer
 * @param size remaining size
 * @param req_list request list
 * @param req_list_size number of elements in the request list
 *
 * @return void * pointer after writing
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_SetParameterRequestList(
    void *ptr, size_t size, dhcp_param_req_t req_list[],
    int req_list_size)
{
    /* option record */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* list of requrested options */
        uint8_t list[];
    } PACKED *pld = ptr;

    /* insufficient space */
    if (size < sizeof(*pld) + req_list_size)
        return 0;

    /* write the option */
    pld->hdr.option = DHCP_OPTION_PARAM_LIST;
    pld->hdr.size = req_list_size;
    /* copy the request list */
    for (int i = 0; i < req_list_size; i++)
        pld->list[i] = req_list[i];

    /* return the address */
    return (void *)((uintptr_t)ptr + sizeof(*pld) + req_list_size);
}

/**
 * @brief Sets the subnet mask record from the configuration
 *
 * @param ptr record start pointer
 * @param size remaining size
 * @param mask subnet mask to be set
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_SetSubnetMask(
    void *ptr, size_t size, tcpip_ip_addr_t mask)
{
    /* option record */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* subnet mask */
        uint32_t mask;
    } PACKED *pld = ptr;

    /* insufficient space */
    if (size < sizeof(*pld))
        return 0;

    /* fill in all the fields */
    pld->hdr.option = DHCP_OPTION_SUBNET_MASK;
    pld->hdr.size = sizeof(pld->mask);
    pld->mask = HTOBE32(mask.u32);

    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Sets the router ip record from the configuration
 *
 * @param ptr record start pointer
 * @param size remaining size
 * @param ip router ip
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_SetRouterIP(
    void *ptr, size_t size, tcpip_ip_addr_t ip)
{
    /* option record */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* subnet mask */
        uint32_t ip;
    } PACKED *pld = ptr;

    /* insufficient space */
    if (size < sizeof(*pld))
        return 0;

    /* fill in all the fields */
    pld->hdr.option = DHCP_OPTION_ROUTER;
    pld->hdr.size = sizeof(pld->ip);
    pld->ip = HTOBE32(ip.u32);

    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Sets the lease time record from the configuration
 *
 * @param ptr record start pointer
 * @param seconds lease time in seconds
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_SetLeaseTime(
    void *ptr, size_t size, uint32_t seconds)
{
     /* option record */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* lease time expressed in seconds */
        uint32_t seconds;
    } PACKED *pld = ptr;

    /* insufficient space */
    if (size < sizeof(*pld))
        return 0;

    /* fill in all the fields */
    pld->hdr.option = DHCP_OPTION_LEASE_TIME;
    pld->hdr.size = sizeof(pld->seconds);
    pld->seconds = HTOBE32(seconds);

    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Sets the DNS servers ip addresses
 *
 * @param ptr record start pointer
 * @param size remaining size
 * @param ips array of dns addresses
 * @param count number of ip addresses in 'ips' array
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_SetDNSServerIP(
    void *ptr, size_t size, tcpip_ip_addr_t *ips, int count)
{
    /* option record */
    struct pld {
        /* record header */
        dhcp_opt_hdr_t hdr;
        /* list of dns server ip addresses */
        uint32_t ip[];
    } PACKED *pld = ptr;

    /* insufficient space */
    if (size < sizeof(*pld))
        return 0;

    /* prepare the header */
    pld->hdr.option = DHCP_OPTION_DNS_SERVERS;
    pld->hdr.size = count * sizeof(pld->ip[0]);
    /* store the addresses */
    for (int i = 0; i < count; i++)
        pld->ip[i] = HTOBE32(ips[i].u32);

    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Writes DHCP server IP address option
 *
 * @param ptr potinter to where to write to
 * @param size remaining size
 * @param ip ip address of the server
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_SetDHCPServerIP(void *ptr,
    size_t size, tcpip_ip_addr_t ip)
{
        /* option record */
    struct pld {
        /* record header */
        uint8_t option, size;
        /* list of dns server ip addresses */
        uint32_t ip;
    } PACKED *pld = ptr;

    /* insufficient space */
    if (size < sizeof(*pld))
        return 0;

    /* setup record */
    pld->option = DHCP_OPTION_DHCP_SERVER;
    pld->size = sizeof(pld->ip);
    pld->ip = HTOBE32(ip.u32);

    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(*pld));
}

/**
 * @brief Read the option specifics
 *
 * @param ptr option record pointer
 * @param option extracted option type
 * @param size size of the option
 *
 * @return pointer to the next option record
 */
static inline ALWAYS_INLINE const void * DHCPFrameOpt_GetOptionHdr(
    const void *ptr, size_t size, dhcp_option_t *option,
    size_t *option_size)
{
    /* header pointer */
    const dhcp_opt_hdr_t *hdr = ptr;

    /* check if the header can even fit */
    if (size < sizeof(*hdr) || size < sizeof(*hdr) + hdr->size)
        return 0;

    /* obtain the information from the option header */
    *option = hdr->option;
    *option_size = hdr->size;

    /* end of the option list */
    if (*option == DHCP_OPTION_END)
        return 0;

    /* return the address of the next option record */
    return (void *)((uintptr_t)hdr + sizeof(*hdr) + hdr->size);
}

/**
 * @brief Reads message type from the option field
 *
 * @param ptr pointer to option field
 * @param msg_type message type placeholder
 *
 * @return pointer to next option record
 */
static inline ALWAYS_INLINE const void * DHCPFrameOpt_GetMessageType(
    const void *ptr, dhcp_msg_type_t *msg_type)
{
    /* option record */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* message_type */
        uint8_t msg_type;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != DHCP_OPTION_MESSAGE_TYPE ||
        pld->hdr.size != sizeof(*pld) - sizeof(pld->hdr))
        return 0;
    /* extract the mask */
    *msg_type = pld->msg_type;
    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Reads the requested ip from the record
 *
 * @param ptr record start pointer
 * @param ip requested ip
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_GetRequestedIP(
    const void *ptr, tcpip_ip_addr_t *ip)
{
    /* option record */
    struct pld {
        /* record header */
        dhcp_opt_hdr_t hdr;
        /* list of dns server ip addresses */
        uint32_t ip;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != DHCP_OPTION_REQUESTED_IP ||
        pld->hdr.size != sizeof(pld->ip))
        return 0;
    /* extract the mask */
    ip->u32 = BETOH32(pld->ip);
    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Read the request parameter list from the frame options
 *
 * @param ptr pointer to the option data
 * @param req_list place to store the request list
 * @param req_list_size request list size that was read
 * @param req_list_max_size maximal size of the request list (to avoid overflows)
 *
 * @return const void * pointer to the next option
 */
static inline ALWAYS_INLINE const void * DHCPFrameOpt_GetParameterRequestList(
    const void *ptr, dhcp_param_req_t *req, int index)
{
    /* option record */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* list of requrested options */
        uint8_t list[];
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != DHCP_OPTION_PARAM_LIST ||
        index >= pld->hdr.size)
        return 0;

    /* store the request id  */
    *req = pld->list[index];
    /* return the pointer to next option field */
    return (const void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Reads the subnet mask record from the configuration
 *
 * @param ptr record start pointer
 * @param mask value to be extracted
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE const void * DHCPFrameOpt_GetSubnetMask(
    const void *ptr, tcpip_ip_addr_t *mask)
{
    /* option record */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* subnet mask */
        uint32_t mask;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != DHCP_OPTION_SUBNET_MASK ||
        pld->hdr.size != sizeof(*pld) - sizeof(pld->hdr))
        return 0;
    /* extract the mask */
    mask->u32 = BETOH32(pld->mask);
    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Reads the subnet mask record from the configuration
 *
 * @param ptr record start pointer
 * @param ip value to be extracted
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE const void * DHCPFrameOpt_GetRouterIP(
    const void *ptr, tcpip_ip_addr_t *ip)
{
    /* option record */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* router ip address */
        uint32_t ip;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != DHCP_OPTION_ROUTER ||
        pld->hdr.size != sizeof(*pld) - sizeof(pld->hdr))
        return 0;
    /* extract the mask */
    ip->u32 = BETOH32(pld->ip);
    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Reads the lease time record from the configuration
 *
 * @param ptr record start pointer
 * @param seconds number of seconds for the lease time
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_GetLeaseTime(
    const void *ptr, uint32_t *seconds)
{
     /* option record */
    struct pld {
        /* option header */
        dhcp_opt_hdr_t hdr;
        /* lease time expressed in seconds */
        uint32_t seconds;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != DHCP_OPTION_LEASE_TIME ||
        pld->hdr.size != sizeof(*pld) - sizeof(pld->hdr))
        return 0;
    /* extract the mask */
    *seconds = BETOH32(pld->seconds);
    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Reads the DNS servers ip address
 *
 * @param ptr record start pointer
 * @param ip dhcp server ip
 * @param index index of the dns ip (in case of multiple dns ips being given)
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_GetDNSServerIP(
    const void *ptr, tcpip_ip_addr_t *ip, int index)
{
    /* option record */
    struct pld {
        /* record header */
        dhcp_opt_hdr_t hdr;
        /* list of dns server ip addresses */
        uint32_t ip[];
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != DHCP_OPTION_DNS_SERVERS ||
        pld->hdr.size % sizeof(*pld->ip) != 0)
        return 0;
    /* dns server record number exceeded */
    if (pld->hdr.size <= index * sizeof(*pld->ip))
        return 0;
    /* extract the mask */
    ip->u32 = BETOH32(pld->ip[index]);
    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Reads the DHCP server ip
 *
 * @param ptr record start pointer
 * @param ip dhcp server ip
 *
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * DHCPFrameOpt_GetDHCPServerIP(
    const void *ptr, tcpip_ip_addr_t *ip)
{
    /* option record */
    struct pld {
        /* record header */
        dhcp_opt_hdr_t hdr;
        /* dhcp server ip */
        uint32_t ip;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != DHCP_OPTION_DHCP_SERVER ||
        pld->hdr.size != sizeof(*pld) - sizeof(pld->hdr))
        return 0;
    /* extract the mask */
    ip->u32 = BETOH32(pld->ip);
    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

#endif /* NET_DHCP_DHCP_FRAME_H */
