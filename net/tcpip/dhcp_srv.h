/**
 * @file dhcp_srv.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-18
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef NET_TCPIP_DHCP_SRV_H
#define NET_TCPIP_DHCP_SRV_H

#include "err.h"



/**
 * @brief initialize dhcp server
 *
 * @return err_t error code
 */
err_t TCPIPDhcpSrv_Init(void);

#endif /* NET_TCPIP_DHCP_SRV_H */
