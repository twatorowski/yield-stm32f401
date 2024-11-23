/**
 * @file ip_addr.h
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 *
 * @brief TCP/IP Stack: Internet Protocol Addressing
 */

#ifndef NET_TCPIP_IP_ADDR_H
#define NET_TCPIP_IP_ADDR_H

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"

/** macro for building ip address */
#define TCPIP_IP_ADDR(a, b, c, d)                           \
    { .u8 = { d, c, b, a }}

/** broadcast ip address */
#define TCPIP_IP_ADDR_BCAST                                 \
    TCPIP_IP_ADDR(255, 255, 255, 255)

/** any ip address */
#define TCPIP_IP_ADDR_ANY                                   \
    TCPIP_IP_ADDR(0, 0, 0, 0)

/** zeroed ip address */
#define TCPIP_IP_ADDR_ZERO                                  \
    TCPIP_IP_ADDR(0, 0, 0, 0)

/** ip address */
typedef union tcpip_ip_addr {
    /** four octets */
    uint8_t u8[4];
    /** 16-bit version */
    uint16_t u16[2];
    /** 32-bit version */
    uint32_t u32;
} PACKED tcpip_ip_addr_t;


/** type for the stringified ip address */
typedef char tcpip_ip_addr_str_t[sizeof("255.255.255.255")];

/**
 * @brief returns current ip value
 *
 * @return tcpip_ip_addr_t
 */
tcpip_ip_addr_t TCPIPIpAddr_GetIP(void);

/**
 * @brief Sets current ip address
 *
 * @param ip ip address
 *
 * @return tcpip_ip_addr_t current ip address
 */
tcpip_ip_addr_t TCPIPIpAddr_SetIP(tcpip_ip_addr_t ip);

/**
 * @brief obtain current subnet mask
 *
 * @return tcpip_ip_addr_t return subnet mask
 */
tcpip_ip_addr_t TCPIPIpAddr_GetSubnetMask(void);

/**
 * @brief set current subnet mask value
 *
 * @param ip ip address
 *
 * @return tcpip_ip_addr_t subnet mask value after update
 */
tcpip_ip_addr_t TCPIPIpAddr_SetSubnetMask(tcpip_ip_addr_t ip);

/**
 * @brief obtain current router/gateway ip
 *
 * @return tcpip_ip_addr_t current gateway ip
 */
tcpip_ip_addr_t TCPIPIpAddr_GetGatewayIP(void);

/**
 * @brief set gateway/router ip
 *
 * @param ip ip address
 *
 * @return tcpip_ip_addr_t gateway ip after update
 */
tcpip_ip_addr_t TCPIPIpAddr_SetGatewayIP(tcpip_ip_addr_t ip);

/**
 * @brief check if the addresses are the same
 *
 * @param a first ip address
 * @param b second ip address
 *
 * @return int non zero if the addresses are the same
 */
int TCPIPIpAddr_AddressMatch(tcpip_ip_addr_t a, tcpip_ip_addr_t b);

/**
 * @brief Get the next ip address. It does not check for overflows in any
 * way imaginable
 *
 * @param ip address
 * @return tcpip_ip_addr_t next ip address
 */
tcpip_ip_addr_t TCPIPIpAddr_Next(tcpip_ip_addr_t ip);

/**
 * @brief Checks id the address is an unicast address that matches our address
 *
 * @param a address to be checked
 *
 * @return int 1 - matches our address, 0 - no match
 */
int TCPIPIpAddr_IsMatchingUnicast(tcpip_ip_addr_t a);

/**
 * @brief Checks if the address is a valid broadcast address within out subbet.
 *
 * @param a address to check
 *
 * @return int check result: 0 - it's not broadcast, non-zero - broadcast
 */
int TCPIPIpAddr_IsMatchingBroadcast(tcpip_ip_addr_t a);

/**
 * @brief is this a multicast address?
 *
 * @param a address to check
 *
 * @return int check result: 0 - it's not broadcast, non-zero - broadcast
 */
int TCPIPIpAddr_IsMatchingMulticast(tcpip_ip_addr_t a);


/**
 * @brief is the address matching any (zero) ip address?
 *
 * @param a address
 * @return int 1 - match, 0 - no match
 */
int TCPIPIpAddr_IsMatchingAny(tcpip_ip_addr_t a);

/**
 * @brief Checks if given ip address is within the subnet
 *
 * @param a address to be checked
 *
 * @return int check result: 0 - it's not, non-zero - it is
 */
int TCPIPIpAddr_IsWithinSubnet(tcpip_ip_addr_t a);

/**
 * @brief Prints the ip address into the string provided or uses the static
 * buffer if null is given
 *
 * @param a ip address to be converted
 * @param str string placeholder
 *
 * @return tcpip_ip_addr_str_t ip string pointer
 */
char * TCPIPIpAddr_ToStr(tcpip_ip_addr_t a, tcpip_ip_addr_str_t str);

#endif /* NET_TCPIP_IP_ADDR_H */
