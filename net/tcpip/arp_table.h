/**
 * @file arp_table.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief TCP/IP Stack: ARP Table
 */

#ifndef NET_TCPIP_ARP_TABLE_H
#define NET_TCPIP_ARP_TABLE_H

#include "err.h"
#include "net/tcpip/eth_addr.h"
#include "net/tcpip/ip_addr.h"

/**
 * @brief reset the arp table
 *
 * @return err_t error code
 */
err_t TCPIPArpTable_ResetTable(void);

/**
 * @brief Update protocol address with hardware address: here we update the 
 * existing entry or overwrite the oldest one or use the free one .
 * 
 * @param ha hardware address to assign  
 * @param pa protocol address for which we upate the hardware address
 *
 * @return err_t error code
 */
err_t TCPIPArpTable_UpdateTable(tcpip_eth_addr_t ha, tcpip_ip_addr_t pa);

/**
 * @brief Look for the hadrware address (mac) that is associated with given 
 * protocol address (ip). If none is found then EUNKADDR is returned
 * 
 * @param pa protocol address
 * @param ha placeholder for hardware address
 *
 * @return err_t error code
 */
err_t TCPIPArpTable_GetHWAddr(tcpip_ip_addr_t pa, tcpip_eth_addr_t *ha);


#endif /* NET_TCPIP_ARP_TABLE_H */
