/**
 * @file eth_addr.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief 
 */

#ifndef NET_TCPIP_ETH_ADDR
#define NET_TCPIP_ETH_ADDR

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"

/** mac address generating macro */
#define TCPIP_ETH_ADDR(a, b, c, d, e, f)                    \
    { .mac = {a, b, c, d, e, f} }

/** broadcast address */
#define TCPIP_ETH_ADDR_BCAST                                \
    TCPIP_ETH_ADDR(0xff, 0xff, 0xff, 0xff, 0xff, 0xff)

/** zero address */
#define TCPIP_ETH_ADDR_ZERO                                 \
    TCPIP_ETH_ADDR(0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

/** ethernet mac address */
typedef union tcpip_eth_addr {
    /** device address */
    uint8_t mac[6];
    /** device address 16 bit words format */
    uint16_t mac16[3];
} PACKED tcpip_eth_addr_t;

/**
 * @brief get current value of the mac address
 * 
 * @return tcpip_eth_addr_t ethernet address
 */
tcpip_eth_addr_t TCPIPEthAddr_GetMAC(void);

/**
 * @brief get current value of the mac address
 * 
 * @param mac mac address
 * 
 * @return err_t mac address update status code
 */
err_t TCPIPEthAddr_SetMAC(tcpip_eth_addr_t mac);

/**
 * @brief Compare two mac addresses. return non zero if they match
 * 
 * @param a address A
 * @param b address B
 * 
 * @return int comparison result: non zero - address is equal
 */
int TCPIPEthAddr_AddressMatch(const tcpip_eth_addr_t a, const tcpip_eth_addr_t b);

/**
 * @brief Is the address our address?
 * 
 * @param a address to check
 *
 * @return int non-zero - yes, it is. 0 - no it is not.
 */
int TCPIPEthAddr_IsMatchingUnicast(tcpip_eth_addr_t a);

#endif /* NET_TCPIP_ETH_ADDR */
