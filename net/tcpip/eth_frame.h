/**
 * @file eth_frame.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief TCP/IP Stack: Ethernet Frame
 */

#ifndef NET_TCPIP_ETH_FRAME
#define NET_TCPIP_ETH_FRAME

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "net/tcpip/eth_addr.h"
#include "util/endian.h"

/** encapsulated frame types */
typedef enum tcpip_eth_frame_ethtype {
    /** internet protocol */
    TCPIP_ETH_ETHTYPE_IP = 0x0800,
    /** address resolution protocol */
    TCPIP_ETH_ETHTYPE_ARP = 0x0806,
} tcpip_eth_frame_ethtype_t;

/** ethernet frame */
typedef struct tcpip_eth_frame {
    /** destination mac address */
    uint8_t da[6];
    /** source mac address */
    uint8_t sa[6];
    /** ether-type */
    uint16_t ethtype;
    /** encapsulated payload */
    uint8_t pld[];
} PACKED tcpip_eth_frame_t;

/**
 * @brief Sets the destination address field value
 * 
 * @param eth header
 * @param da destination address
 */
static inline ALWAYS_INLINE void TCPIPEthFrame_SetDA(
    tcpip_eth_frame_t *eth, tcpip_eth_addr_t da)
{
    eth->da[0] = da.mac[0]; eth->da[3] = da.mac[3];
    eth->da[1] = da.mac[1]; eth->da[4] = da.mac[4];
    eth->da[2] = da.mac[2]; eth->da[5] = da.mac[5];
}

/**
 * @brief Returns the destination address field value
 * 
 * @param eth header
 * 
 * @return tcpip_eth_addr_t destination address
 */
static inline ALWAYS_INLINE tcpip_eth_addr_t TCPIPEthFrame_GetDA(
    const tcpip_eth_frame_t *eth)
{
    return (tcpip_eth_addr_t) { .mac = {eth->da[0], eth->da[1], eth->da[2],
        eth->da[3], eth->da[4], eth->da[5] } };
}

/**
 * @brief Sets the source address field value
 * 
 * @param eth header
 * @param sa source address
 */
static inline ALWAYS_INLINE void TCPIPEthFrame_SetSA(
    tcpip_eth_frame_t *eth, tcpip_eth_addr_t sa)
{
    eth->sa[0] = sa.mac[0]; eth->sa[3] = sa.mac[3];
    eth->sa[1] = sa.mac[1]; eth->sa[4] = sa.mac[4];
    eth->sa[2] = sa.mac[2]; eth->sa[5] = sa.mac[5];
}

/**
 * @brief Returns the ource address field value
 * 
 * @param eth header
 * 
 * @return tcpip_eth_addr_t source adddress
 */
static inline ALWAYS_INLINE tcpip_eth_addr_t TCPIPEthFrame_GetSA(
    const tcpip_eth_frame_t *eth)
{
    return (tcpip_eth_addr_t) { .mac = {eth->sa[0], eth->sa[1], eth->sa[2],
        eth->sa[3], eth->sa[4], eth->sa[5] } };
}

/**
 * @brief Sets the underlying frame type
 * 
 * @param eth header
 * @param ethtype frame type
 */
static inline ALWAYS_INLINE void TCPIPEthFrame_SetEthType(
    tcpip_eth_frame_t *eth, tcpip_eth_frame_ethtype_t ethtype)
{
    eth->ethtype = HTOBE16(ethtype);
}

/**
 * @brief Returns the frame type field value
 * 
 * @param eth header
 * 
 * @return tcpip_eth_frame_ethtype_t frame type value 
 */
static inline ALWAYS_INLINE tcpip_eth_frame_ethtype_t TCPIPEthFrame_GetEthType(
    const tcpip_eth_frame_t *eth)
{
    return BETOH16(eth->ethtype);
}

#endif /* NET_TCPIP_ETH_FRAME */
