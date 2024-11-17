/**
 * @file dhcp.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2022-04-13
 * 
 * @brief TCP/IP DHCP client implementation
 */

#ifndef NET_TCPIP_DHCP_H
#define NET_TCPIP_DHCP_H

/** ports */
#define TCPIP_DHCP_SRV_PORT             67
#define TCPIP_DHCP_CLT_PORT             68

/**
 * @brief initialze common parts of the layer 
 * 
 * @return err_t error code
 */
err_t TCPIPDhcp_Init(void);

/**
 * @brief return the status of the lease
 * 
 * @return int 1 - lease valid, 0 - invalid
 */
int TCPIPDhcp_IsLeaseValid(void);

/**
 * @brief invalidate the lease, forces the renewal if the client is active
 * 
 * @return err_t error code
 */
err_t TCPIPDhcp_InvalidateLease(void);

/**
 * @brief enable or disable dhcp client 
 * 
 * @param enable 1 - enable, 0 - disable
 * 
 * @return err_t error code
 */
err_t TCPIPDhcp_SetEnable(int enable);

#endif /* NET_TCPIP_DHCP_H */
