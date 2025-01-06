/**
 * @file icmp.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-29
 * 
 * @brief TCP/IP ICMP protocol
 */

#ifndef NET_TCPIP_ICMP
#define NET_TCPIP_ICMP

#include "err.h"
#include "net/tcpip/tcpip.h"
#include "net/tcpip/ip_addr.h"
#include "sys/time.h"

/**
 * @brief Initialize ICMP layer
 * 
 * @return err_t error code
 */
err_t TCPIPIcmp_Init(void);

/**
 * @brief Input to the icmp frame processor
 * 
 * @param frame frame data structure
 *
 * @return err_t processing error code
 */
err_t TCPIPIcmp_Input(tcpip_frame_t *frame);

/**
 * @brief Send echo request to given destination address and waits for 
 * echo response
 * 
 * @param da destination address
 * @param timeout timeout in milliseconds
 * 
 * @return err_t error code: EOK when destination responded
 */
err_t TCPIPIcmp_Ping(tcpip_ip_addr_t da, dtime_t timeout);

#endif /* NET_TCPIP_ICMP */
