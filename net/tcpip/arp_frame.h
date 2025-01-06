/**
 * @file arp_frame.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief TCP/IP Stack: Address Resolution Protocol Frame
 */

#ifndef NET_TCPIP_ARP_FRAME_H
#define NET_TCPIP_ARP_FRAME_H

#include <stdint.h>

#include "compiler.h"
#include "net/tcpip/eth_addr.h"
#include "net/tcpip/eth_frame.h"
#include "net/tcpip/ip_addr.h"
#include "util/endian.h"

/** arp hardware type */
typedef enum tcpip_arp_htype {
    /** ethernet */
    TCPIP_ARP_HTYPE_ETHERNET = 0x1,
} tcpip_arp_htype_t;

/** arp hardware type */
typedef enum tcpip_arp_ptype {
    /** ethernet */
    TCPIP_ARP_PTYPE_IP = TCPIP_ETH_ETHTYPE_IP,
} tcpip_arp_ptype_t;

/** arp operation types */
typedef enum tcpip_arp_oper {
    /** arp request */
    TCPIP_ARP_OPER_REQUEST = 0x1,
    /** arp reply */
    TCPIP_ARP_OPER_REPLY = 0x2,
} tcpip_arp_oper_t;

/** address resolution protocol frame */
typedef struct tcpip_arp_frame {
    /** hardware type */
    uint16_t htype;
    /** protocol type */
    uint16_t ptype;
    /** hardware address length */
    uint8_t hlen;
    /** protocol address length */
    uint8_t plen;
    /** operation */
    uint16_t oper;
    /* rest of the payload */
    uint8_t pld[];
} PACKED tcpip_arp_frame_t;

/** payload for ip over ethernet combination */
typedef struct tcpip_arp_pld_eth_ip {
    /* sender hardware address */
    uint8_t sha[6];
    /* sender protocol address */
    uint32_t spa;
    /* target hardware address */
    uint8_t tha[6];
    /* target protocol address */
    uint32_t tpa;
} PACKED tcpip_arp_frame_pld_eth_ip_t;

/**
 * @brief Sets the hardware type field value
 * 
 * @param arp header
 * @param htype hardware type
 */
static inline ALWAYS_INLINE void TCPIPArpFrame_SetHType(
    tcpip_arp_frame_t *arp, tcpip_arp_htype_t htype)
{
    arp->htype = HTOBE16(htype);
}

/**
 * @brief Returns the hardware type field value
 * 
 * @param arp header
 * 
 * @return tcpip_arp_htype_t hardware type value 
 */
static inline ALWAYS_INLINE tcpip_arp_htype_t TCPIPArpFrame_GetHType(
    const tcpip_arp_frame_t *arp)
{
    return BETOH16(arp->htype);
}

/**
 * @brief Sets the protocol field value
 * 
 * @param arp header
 * @param ptype protocol type
 */
static inline ALWAYS_INLINE void TCPIPArpFrame_SetPType(
    tcpip_arp_frame_t *arp, tcpip_arp_ptype_t ptype)
{
    arp->ptype = HTOBE16(ptype);
}

/**
 * @brief Returns the protocol type field value
 * 
 * @param arp header
 * 
 * @return tcpip_arp_ptype_t protocol type value 
 */
static inline ALWAYS_INLINE tcpip_arp_ptype_t TCPIPArpFrame_GetPType(
    const tcpip_arp_frame_t *arp)
{
    return BETOH16(arp->ptype);
}

/**
 * @brief Sets the hardware address length field value
 * 
 * @param arp header
 * @param hlen hardware address length
 */
static inline ALWAYS_INLINE void TCPIPArpFrame_SetHLen(
    tcpip_arp_frame_t *arp, size_t hlen)
{
    arp->hlen = hlen;
}

/**
 * @brief Returns the hardware address length type field value
 * 
 * @param arp header
 * 
 * @return size_t hardware address length
 */
static inline ALWAYS_INLINE size_t TCPIPArpFrame_GetHLen(
    const tcpip_arp_frame_t *arp)
{
    return arp->hlen;
}

/**
 * @brief Sets the protocol address length field value
 * 
 * @param arp header
 * @param hlen protocol address length
 */
static inline ALWAYS_INLINE void TCPIPArpFrame_SetPLen(
    tcpip_arp_frame_t *arp, size_t plen)
{
    arp->plen = plen;
}

/**
 * @brief Returns the protocol address length type field value
 * 
 * @param arp header
 * 
 * @return size_t protocol address length
 */
static inline ALWAYS_INLINE size_t TCPIPArpFrame_GetPLen(
    const tcpip_arp_frame_t *arp)
{
    return arp->plen;
}

/**
 * @brief Sets the operation code field value
 * 
 * @param arp header
 * @param oper operation code
 */
static inline ALWAYS_INLINE void TCPIPArpFrame_SetOper(
    tcpip_arp_frame_t *arp, tcpip_arp_oper_t oper)
{
    arp->oper = HTOBE16(oper);
}

/**
 * @brief Gets the operation code field value
 * 
 * @param arp header
 * @param oper operation code
 */
static inline ALWAYS_INLINE tcpip_arp_oper_t TCPIPArpFrame_GetOper(
    const tcpip_arp_frame_t *arp)
{
    return HTOBE16(arp->oper);
}


/**
 * @brief Sets the sender hardware address
 * 
 * @param arp_ethip eth ip request/response payload pointer
 * @param sha sender hardware address
 */
static inline ALWAYS_INLINE void TCPIPArpFrameETHIP_SetSHA(
    tcpip_arp_frame_pld_eth_ip_t *arp_ethip, tcpip_eth_addr_t sha)
{
    arp_ethip->sha[0] = sha.mac[0]; arp_ethip->sha[1] = sha.mac[1];
    arp_ethip->sha[2] = sha.mac[2]; arp_ethip->sha[3] = sha.mac[3];
    arp_ethip->sha[4] = sha.mac[4]; arp_ethip->sha[5] = sha.mac[5];
}

/**
 * @brief Returns the sender hardware address
 * 
 * @param arp_ethip eth ip request/response payload pointer
 * 
 * @return tcpip_eth_addr_t sender hardware address 
 */
static inline ALWAYS_INLINE tcpip_eth_addr_t TCPIPArpFrameETHIP_GetSHA(
    const tcpip_arp_frame_pld_eth_ip_t *arp_ethip)
{
    return (tcpip_eth_addr_t) { .mac = {arp_ethip->sha[0], arp_ethip->sha[1], 
        arp_ethip->sha[2], arp_ethip->sha[3], arp_ethip->sha[4], 
        arp_ethip->sha[5] } };
}

/**
 * @brief Sets the sender protocol address
 * 
 * @param arp_ethip eth ip request/response payload pointer
 * @param spa sender protocol address
 */
static inline ALWAYS_INLINE void TCPIPArpFrameETHIP_SetSPA(
    tcpip_arp_frame_pld_eth_ip_t *arp_ethip, tcpip_ip_addr_t spa)
{
    arp_ethip->spa = HTOBE32(spa.u32);
}

/**
 * @brief Returns the senders protocol address
 * 
 * @param arp_ethip eth ip request/response payload pointer
 * 
 * @return tcpip_ip_addr_t senders protocol address 
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t TCPIPArpFrameETHIP_GetSPA(
    const tcpip_arp_frame_pld_eth_ip_t *arp_ethip)
{
    return (tcpip_ip_addr_t) { .u32 = BETOH32(arp_ethip->spa) };
}


/**
 * @brief Sets the target hardware address
 * 
 * @param arp_ethip eth ip request/response payload pointer
 * @param tha target hardware address
 */
static inline ALWAYS_INLINE void TCPIPArpFrameETHIP_SetTHA(
    tcpip_arp_frame_pld_eth_ip_t *arp_ethip, tcpip_eth_addr_t tha)
{
    arp_ethip->tha[0] = tha.mac[0]; arp_ethip->tha[1] = tha.mac[1];
    arp_ethip->tha[2] = tha.mac[2]; arp_ethip->tha[3] = tha.mac[3];
    arp_ethip->tha[4] = tha.mac[4]; arp_ethip->tha[5] = tha.mac[5];
}

/**
 * @brief Returns the target hardware address
 * 
 * @param arp_ethip eth ip request/response payload pointer
 * 
 * @return tcpip_eth_addr_t target hardware address 
 */
static inline ALWAYS_INLINE tcpip_eth_addr_t TCPIPArpFrameETHIP_GetTHA(
    const tcpip_arp_frame_pld_eth_ip_t *arp_ethip)
{
    return (tcpip_eth_addr_t) { .mac = {arp_ethip->tha[0], arp_ethip->tha[1], 
        arp_ethip->tha[2], arp_ethip->tha[3], arp_ethip->tha[4], 
        arp_ethip->tha[5] } };
}

/**
 * @brief Sets the target protocol address
 * 
 * @param arp_ethip eth ip request/response payload pointer
 * @param spa target protocol address
 */
static inline ALWAYS_INLINE void TCPIPArpFrameETHIP_SetTPA(
    tcpip_arp_frame_pld_eth_ip_t *arp_ethip, tcpip_ip_addr_t tpa)
{
    arp_ethip->tpa = HTOBE32(tpa.u32);
}

/**
 * @brief Returns the target protocol address
 * 
 * @param arp_ethip eth ip request/response payload pointer
 * 
 * @return tcpip_ip_addr_t target protocol address 
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t TCPIPArpFrameETHIP_GetTPA(
    const tcpip_arp_frame_pld_eth_ip_t *arp_ethip)
{
    return (tcpip_ip_addr_t) { .u32 = BETOH32(arp_ethip->tpa) };
}


#endif /* NET_TCPIP_ARP_FRAME_H */
