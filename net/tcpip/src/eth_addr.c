/**
 * @file eth_addr.c
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 *
 * @brief TCP/IP Stack: Ethernet Addressing
 */

#include <stdint.h>
#include <stddef.h>

#include "assert.h"
#include "config.h"
#include "err.h"
// #include "dev/eth.h"
#include "net/tcpip/eth_addr.h"
#include "net/tcpip/rxtx.h"


/* get current value of the mac address */
tcpip_eth_addr_t TCPIPEthAddr_GetMAC(void)
{
    /* return the mac address */
    return (tcpip_eth_addr_t) TCPIP_ETH_ADDRESS;
}

/* compare two mac addresses */
int TCPIPEthAddr_AddressMatch(tcpip_eth_addr_t a, tcpip_eth_addr_t b)
{
    return (a.mac16[0] == b.mac16[0]) && (a.mac16[1] == b.mac16[1]) &&
        (a.mac16[2] == b.mac16[2]);
}

/* is the address our address? */
int TCPIPEthAddr_IsMatchingUnicast(tcpip_eth_addr_t a)
{
    return TCPIPEthAddr_AddressMatch(a, TCPIPEthAddr_GetMAC());
}