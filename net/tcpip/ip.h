/**
 * @file ip.h
 * 
 * @date 2021-02-02
 * @author twatorowski 
 * 
 * @brief TCP/IP Stack: Internet Protocol
 */

#ifndef NET_TCPIP_IP_H
#define NET_TCPIP_IP_H

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "net/tcpip/tcpip.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/ip_frame.h"

/**
 * @brief initialize IP layer
 * 
 * @return err_t status
 */
err_t TCPIPIp_Init(void);

/**
 * @brief input routine to the ip layer
 * 
 * @param frame frame to be processed
 *
 * @return err_t error code
 */
err_t TCPIPIp_Input(tcpip_frame_t *frame);


/**
 * @brief Allocate frame buffer within ip layer
 * 
 * @param frame frame descriptor
 *
 * @return err_t allocation error code
 */
err_t TCPIPIp_Alloc(tcpip_frame_t *frame);

/**
 * @brief Drop allocated frame
 * 
 * @param frame frame to be dropped
 *
 * @return err_t error code
 */
err_t TCPIPIp_Drop(tcpip_frame_t *frame);

/**
 * @brief Sends previously allocated ip frame
 *
 * @param frame frame descriptor
 * @param da destination address
 * @param proto carried protocol
 *
 * @return err_t send error code
 */
err_t TCPIPIp_Send(tcpip_frame_t *frame, tcpip_ip_addr_t da, 
    tcpip_ip_protocol_t proto);


#endif /* NET_TCPIP_IP_H */
