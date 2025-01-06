/**
 * @file arp.h
 * 
 * @date 2021-02-02
 * @author twatorowski 
 * 
 * @brief TCP/IP Stack: Address Resolution Protocol 
 */

#ifndef NET_TCPIP_ARP_H
#define NET_TCPIP_ARP_H

#include "err.h"
#include "net/tcpip/tcpip.h"
#include "net/tcpip/eth_addr.h"
#include "net/tcpip/ip_addr.h"

/**
 * @brief initialize Address Resolution Protocol layer 
 * 
 * @return err_t return status
 */
err_t TCPIPArp_Init(void);

/**
 * @brief Input routine to the arp protocol processor
 * 
 * @param frame frame descriptor
 *
 * @return err_t frame processing error code
 */
err_t TCPIPArp_Input(tcpip_frame_t *frame);

/**
 * @brief Look for the hadrware address (mac) that is associated with given 
 * protocol address (ip). If none is found then issue the arp request in hope 
 * that the other side will provide it's address.
 * 
 * @param pa protocol address
 * @param ha placeholder for hardware address
 *
 * @return err_t error code
 */
err_t TCPIPArp_GetHWAddr(tcpip_ip_addr_t pa, tcpip_eth_addr_t *ha);

#endif /* NET_TCPIP_ARP_H */
