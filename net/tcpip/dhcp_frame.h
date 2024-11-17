/**
 * @file dhcp_frame.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2022-04-13
 * 
 * @brief TCP/IP: DHCP frame specification
 */

#ifndef NET_TCPIP_DHCP_FRAME_H
#define NET_TCPIP_DHCP_FRAME_H

#include <stdint.h>

#include "err.h"
#include "compiler.h"
#include "net/tcpip/eth_addr.h"
#include "net/tcpip/eth_frame.h"
#include "net/tcpip/ip_addr.h"
#include "util/endian.h"
#include "util/string.h"

/** operation type */
typedef enum tcpip_dhcp_op {
    TCPIP_DHCP_OP_REQUEST = 0x01,
    TCPIP_DHCP_OP_RESPONSE = 0x02,
} tcpip_dhcp_op_t;

/** hardware type  */
typedef enum tcpip_dhcp_htype {
    TCPIP_DHCP_HTYPE_ETH = 0x01,
} tcpip_dhcp_htype_t;

/** flags */
typedef enum tcpip_dhcp_flags {
    TCPIP_DHCP_FLAGS_BROADCAST = 0x8000,
} tcpip_dhcp_flags_t;

/** options */
typedef enum tcpip_dhcp_option {
    TCPIP_DHCP_OPTION_SUBNET_MASK = 1,
    TCPIP_DHCP_OPTION_ROUTER = 3,
    TCPIP_DHCP_OPTION_DNS_SERVERS = 6, 
    TCPIP_DHCP_OPTION_REQUESTED_IP = 50,
    TCPIP_DHCP_OPTION_LEASE_TIME = 51,
    TCPIP_DHCP_OPTION_MESSAGE_TYPE = 53,
    TCPIP_DHCP_OPTION_DHCP_SERVER = 54,
    TCPIP_DHCP_OPTION_PARAM_LIST = 55,
    TCPIP_DHCP_OPTION_END = 0xff,
} tcpip_dhcp_option_t;

/** dhcp frame header */
typedef struct tcpip_dhcp_frame {
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
} PACKED tcpip_dhcp_frame_t;

/** message type */
typedef enum tcpip_dhcp_msg_type {
    TCPIP_DHCP_MSG_TYPE_DISCOVER = 0x01,
    TCPIP_DHCP_MSG_TYPE_OFFER = 0x02,
    TCPIP_DHCP_MSG_TYPE_REQUEST = 0x03,
    TCPIP_DHCP_MSG_TYPE_DECLINE = 0x04,
    TCPIP_DHCP_MSG_TYPE_ACK = 0x05,
    TCPIP_DHCP_MSG_TYPE_NAK = 0x06,
    TCPIP_DHCP_MSG_TYPE_RELEASE = 0x07,
    TCPIP_DHCP_MSG_TYPE_INFORM = 0x08,
} tcpip_dhcp_msg_type_t;

/** dhcp option header */
typedef struct tcpip_dhcp_opt_hdr {
    /* option type size of the option payload */
    uint8_t option, size;
    // /* payload of varying size */
    // uint8_t pld[];
} PACKED tcpip_dhcp_opt_hdr_t;

/** parameter request list elements */
typedef enum tcpip_dhcp_param_req {
    TCPIP_DHCP_PARAM_REQ_SUBNET_MASK = 1,
    TCPIP_DHCP_PARAM_REQ_ROUTER = 3,
    TCPIP_DHCP_PARAM_REQ_DOMAIN_NAME = 15,
    TCPIP_DHCP_PARAM_REQ_DOMAIN_NAME_SERVER = 6,
} tcpip_dhcp_param_req_t;


/**
 * @brief Initialize the dhcp frame header. Clear all fields etc..
 * 
 * @param dhcp frame header pointer
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_Init(tcpip_dhcp_frame_t *dhcp)
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
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetOP(tcpip_dhcp_frame_t *dhcp, 
    tcpip_dhcp_op_t op)
{
    dhcp->op = op;
}

/**
 * @brief Sets headers HTYPE field value
 * 
 * @param dhcp frame header pointer
 * @param htype value to be set
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetHType(
    tcpip_dhcp_frame_t *dhcp, tcpip_dhcp_htype_t htype)
{
    dhcp->htype = htype;
}

/**
 * @brief Sets headers HLEN field value
 * 
 * @param dhcp frame header pointer
 * @param len value to be set
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetHLen(
    tcpip_dhcp_frame_t *dhcp, int hlen)
{
    dhcp->hlen = hlen;
}

/**
 * @brief Sets headers HOPS field value
 * 
 * @param dhcp frame header pointer
 * @param ops value to be set
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetHops(
    tcpip_dhcp_frame_t *dhcp, int hops)
{
    dhcp->hops = hops;
}

/**
 * @brief Sets headers XID field value
 * 
 * @param dhcp frame header pointer
 * @param xid value to be set
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetXID(
    tcpip_dhcp_frame_t *dhcp, uint32_t xid)
{
    dhcp->xid = HTOBE32(xid);
}

/**
 * @brief Sets headers flags field value
 * 
 * @param dhcp frame header pointer
 * @param flags value to be set
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetFlags(
    tcpip_dhcp_frame_t *dhcp, tcpip_dhcp_flags_t flags)
{
    dhcp->flags = HTOBE16(flags);
}

/**
 * @brief Sets headers CIADDR field value
 * 
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetCIAddr(
    tcpip_dhcp_frame_t *dhcp, tcpip_ip_addr_t ciaddr)
{
    dhcp->ciaddr = HTOBE32(ciaddr.u32);
}

/**
 * @brief Sets headers YIADDR field value
 * 
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetYIAddr(
    tcpip_dhcp_frame_t *dhcp, tcpip_ip_addr_t yiaddr)
{
    dhcp->yiaddr = HTOBE32(yiaddr.u32);
}

/**
 * @brief Sets headers OSIADDR field value
 * 
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetSIAddr(
    tcpip_dhcp_frame_t *dhcp, tcpip_ip_addr_t siaddr)
{
    dhcp->siaddr = HTOBE32(siaddr.u32);
}

/**
 * @brief Sets headers GIADDR field value
 * 
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetGIAddr(
    tcpip_dhcp_frame_t *dhcp, tcpip_ip_addr_t giaddr)
{
    dhcp->giaddr = HTOBE32(giaddr.u32);
}

/**
 * @brief Sets headers CHADDR field value
 * 
 * @param dhcp frame header pointer
 * @param op value to be set
 */
static inline ALWAYS_INLINE void TCPIPDhcpFrame_SetCHAddr(
    tcpip_dhcp_frame_t *dhcp, tcpip_eth_addr_t chaddr)
{
    memcpy(dhcp->chaddr, chaddr.mac, sizeof(chaddr.mac));
}

/**
 * @brief Returns the OP field value
 * 
 * @param dhcp frame header pointer
 * @return tcpip_dhcp_op_t field value
 */
static inline ALWAYS_INLINE tcpip_dhcp_op_t TCPIPDhcpFrame_GetOP(
    const tcpip_dhcp_frame_t *dhcp)
{
    return (tcpip_dhcp_op_t)dhcp->op;
}

/**
 * @brief Returns the HTYPE field value
 * 
 * @param dhcp frame header pointer
 * @return tcpip_dhcp_htype_t field value
 */
static inline ALWAYS_INLINE tcpip_dhcp_htype_t TCPIPDhcpFrame_GetHType(
    const tcpip_dhcp_frame_t *dhcp)
{
    return (tcpip_dhcp_htype_t)dhcp->htype;
}

/**
 * @brief Returns the HLEN field value
 * 
 * @param dhcp frame header pointer
 * @return int field value
 */
static inline ALWAYS_INLINE int TCPIPDhcpFrame_GetHLen(
    const tcpip_dhcp_frame_t *dhcp)
{
    return (int)dhcp->hlen;
}

/**
 * @brief Returns the HOPS field value
 * 
 * @param dhcp frame header pointer
 * @return int field value
 */
static inline ALWAYS_INLINE int TCPIPDhcpFrame_GetHops(
    const tcpip_dhcp_frame_t *dhcp)
{
    return (int)dhcp->hops;
}

/**
 * @brief Returns the XID field value
 * 
 * @param dhcp frame header pointer
 * @return uint32_t field value
 */
static inline ALWAYS_INLINE uint32_t TCPIPDhcpFrame_GetXID(
    const tcpip_dhcp_frame_t *dhcp)
{
    return BETOH32(dhcp->xid);
}

/**
 * @brief Returns the CIADDR field value
 * 
 * @param dhcp frame header pointer
 * @return tcpip_ip_addr_t field value
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t TCPIPDhcpFrame_GetCIAddr(
    const tcpip_dhcp_frame_t *dhcp)
{
    return (tcpip_ip_addr_t) { .u32 = BETOH32(dhcp->ciaddr) };
}

/**
 * @brief Returns the YIADDR field value
 * 
 * @param dhcp frame header pointer
 * @return tcpip_ip_addr_t field value
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t TCPIPDhcpFrame_GetYIAddr(
    const tcpip_dhcp_frame_t *dhcp)
{
    return (tcpip_ip_addr_t) { .u32 = BETOH32(dhcp->yiaddr) };
}

/**
 * @brief Returns the SIADDR field value
 * 
 * @param dhcp frame header pointer
 * @return tcpip_ip_addr_t field value
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t TCPIPDhcpFrame_GetSIAddr(
    const tcpip_dhcp_frame_t *dhcp)
{
    return (tcpip_ip_addr_t) { .u32 = BETOH32(dhcp->siaddr) };
}

/**
 * @brief Returns the GIADDR field value
 * 
 * @param dhcp frame header pointer
 * @return tcpip_ip_addr_t field value
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t TCPIPDhcpFrame_GetGIAddr(
    const tcpip_dhcp_frame_t *dhcp)
{
    return (tcpip_ip_addr_t) { .u32 = BETOH32(dhcp->giaddr) };
}

/**
 * @brief Returns the CHADDR field value
 * 
 * @param dhcp frame header pointer
 * @return tcpip_eth_addr_t field value
 */
static inline ALWAYS_INLINE tcpip_eth_addr_t TCPIPDhcpFrame_GetCHAddr(
     tcpip_dhcp_frame_t *dhcp)
{
    return (tcpip_eth_addr_t) { .mac = {dhcp->chaddr[0], dhcp->chaddr[1], 
        dhcp->chaddr[2], dhcp->chaddr[3], dhcp->chaddr[4], 
        dhcp->chaddr[5] } };
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
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_GetOptionHdr(const void *ptr, 
    tcpip_dhcp_option_t *option, size_t *size)
{
    /* header pointer */
    const tcpip_dhcp_opt_hdr_t *hdr = ptr;

    /* obtain the information from the option header */
    *option = hdr->option;
    *size = hdr->size;

    /* end of the option list */
    if (*option == TCPIP_DHCP_OPTION_END)
        return 0;
    
    /* return the address of the next option record */
    return (void *)((uintptr_t)hdr + sizeof(*hdr) + hdr->size);
}

/**
 * @brief Write the end of options field 
 * 
 * @param ptr pointer to where to write to
 * 
 * @return void * pointer after writing
 */
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_SetOptionsEnd(void *ptr)
{
    /* option record type */
    struct pld { 
        uint8_t option; 
    } PACKED *pld = ptr;

    /* set marker */
    pld->option = TCPIP_DHCP_OPTION_END;
    /* return the address */
    return (void *)((uintptr_t)ptr + sizeof(*pld));
}

/**
 * @brief Write Message Type option
 * 
 * @param ptr pointer to where to write to
 * @param msg_type message type to be stored
 * 
 * @return void * pointer after writing 
 */
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_SetMessageType(void *ptr, 
    tcpip_dhcp_msg_type_t msg_type)
{
    /* option record type */
    struct pld { 
        /* option header */
        tcpip_dhcp_opt_hdr_t hdr;
        /* message type field */
        uint8_t msg_type; 
    } PACKED *pld = ptr;
    
    /* write the option */
    pld->hdr.option = TCPIP_DHCP_OPTION_MESSAGE_TYPE;
    pld->hdr.size = sizeof(*pld) - sizeof(pld->hdr);
    pld->msg_type = msg_type;
    /* return the address */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Write Requested IP option
 * 
 * @param ptr payload pointer
 * @param ip ip to be written
 * 
 * @return  void * pointer after writing 
 */
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_SetRequestedIP(void *ptr, 
    tcpip_ip_addr_t ip)
{
    /* option record type */
    struct pld { 
        /* option header */
        tcpip_dhcp_opt_hdr_t hdr; 
        /* requested ip */
        uint32_t ip; 
    } PACKED *pld = ptr;

    /* write the option */
    pld->hdr.option = TCPIP_DHCP_OPTION_REQUESTED_IP;
    pld->hdr.size = sizeof(*pld) - sizeof(pld->hdr);
    pld->ip = HTOBE32(ip.u32);

    /* return the address */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Writes parameter request list
 * 
 * @param ptr payload pointer
 * @param req_list request list
 * @param req_list_size number of elements on request list
 * 
 * @return void * pointer after writing
 */
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_SetParameterRequestList(
    void *ptr, tcpip_dhcp_param_req_t req_list[], int req_list_size)
{
    /* option record */
    struct pld { 
        /* option header */
        tcpip_dhcp_opt_hdr_t hdr;
        /* list of requrested options */
        uint8_t list[];
    } PACKED *pld = ptr;

    /* write the option */
    pld->hdr.option = TCPIP_DHCP_OPTION_PARAM_LIST;
    pld->hdr.size = req_list_size;
    /* copy the request list */
    for (int i = 0; i < req_list_size; i++)
        pld->list[i] = req_list[i];

    /* return the address */
    return (void *)((uintptr_t)ptr + sizeof(*pld) + req_list_size);
}

/**
 * @brief Reads message type from the option field
 * 
 * @param ptr pointer to option field
 * @param msg_type message type placeholder
 * 
 * @return pointer to next option record
 */
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_GetMessageType(
    const void *ptr, tcpip_dhcp_msg_type_t *msg_type)
{
    /* option record */
    struct pld {
        /* option header */
        tcpip_dhcp_opt_hdr_t hdr;
        /* message_type */
        uint8_t msg_type;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != TCPIP_DHCP_OPTION_MESSAGE_TYPE || 
        pld->hdr.size != sizeof(*pld) - sizeof(pld->hdr))
        return 0;
    /* extract the mask */
    *msg_type = pld->msg_type;
    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(pld->hdr) + pld->hdr.size);
}

/**
 * @brief Reads the subnet mask record from the configuration
 * 
 * @param ptr record start pointer
 * @param mask value to be extracted
 * 
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_GetSubnetMask(
    const void *ptr, tcpip_ip_addr_t *mask)
{
    /* option record */
    struct pld {
        /* option header */
        tcpip_dhcp_opt_hdr_t hdr;
        /* subnet mask */
        uint32_t mask;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != TCPIP_DHCP_OPTION_SUBNET_MASK || 
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
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_GetRouterIP(
    const void *ptr, tcpip_ip_addr_t *ip)
{
    /* option record */
    struct pld {
        /* option header */
        tcpip_dhcp_opt_hdr_t hdr;
        /* router ip address */
        uint32_t ip;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != TCPIP_DHCP_OPTION_ROUTER || 
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
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_GetLeaseTime(
    const void *ptr, uint32_t *seconds)
{
     /* option record */
    struct pld {
        /* option header */
        tcpip_dhcp_opt_hdr_t hdr;
        /* lease time expressed in seconds */
        uint32_t seconds;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != TCPIP_DHCP_OPTION_LEASE_TIME ||
        pld->hdr.size != sizeof(*pld) - sizeof(pld->hdr))
        return 0;
    /* extract the mask */
    *seconds = BETOH32(pld->seconds);
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
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_GetDHCPServerIP(
    const void *ptr, tcpip_ip_addr_t *ip)
{
    /* option record */
    struct pld {
        /* record header */
        tcpip_dhcp_opt_hdr_t hdr;
        /* dhcp server ip */
        uint32_t ip;
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != TCPIP_DHCP_OPTION_DHCP_SERVER || 
        pld->hdr.size != sizeof(*pld) - sizeof(pld->hdr))
        return 0;
    /* extract the mask */
    ip->u32 = BETOH32(pld->ip);
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
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_GetDNSServerIP(
    const void *ptr, tcpip_ip_addr_t *ip, int index)
{
    /* option record */
    struct pld {
        /* record header */
        tcpip_dhcp_opt_hdr_t hdr;
        /* list of dns server ip addresses */
        uint32_t ip[];
    } PACKED const *pld = ptr;

    /* malformed frame */
    if (pld->hdr.option != TCPIP_DHCP_OPTION_DNS_SERVERS || 
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
 * @brief Writes DHCP server IP address option
 * 
 * @param ptr potinter to where to write to
 * @param ip ip address of the server
 *  
 * @return void * pointer to the beginning of the next record
 */
static inline ALWAYS_INLINE void * TCPIPDhcpFrameOpt_SetDHCPServerIP(void *ptr, 
    tcpip_ip_addr_t ip)
{
        /* option record */
    struct pld {
        /* record header */
        uint8_t option, size;
        /* list of dns server ip addresses */
        uint32_t ip;
    } PACKED *pld = ptr;

    /* setup record */
    pld->option = TCPIP_DHCP_OPTION_DHCP_SERVER;
    pld->size = 4;
    pld->ip = HTOBE32(ip.u32);

    /* return the pointer to next option field */
    return (void *)((uintptr_t)ptr + sizeof(*pld));
}


#endif /* NET_TCPIP_DHCP_FRAME_H */
