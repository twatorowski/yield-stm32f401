/**
 * @file eth.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-28
 * 
 * @brief TCP/IP Stack: Ethernet II layer
 */

#ifndef NET_TCPIP_ETH_H
#define NET_TCPIP_ETH_H

#include "err.h"
#include "net/tcpip/eth_addr.h"
#include "net/tcpip/tcpip.h"

/**
 * @brief initialize ethernet layer
 * 
 * @return err_t error code
 */
err_t TCPIPEth_Init(void);

/**
 * @brief input routine to the ethernet layer
 * 
 * @param frame frame information pointer
 *
 * @return err_t error code
 */
err_t TCPIPEth_Input(struct tcpip_frame *frame);

/**
 * @brief Allocate frame buffer for the ethernet frame
 * 
 * @return tcpip_frame_t allocated frame descriptor
 */
err_t TCPIPEth_Alloc(tcpip_frame_t *frame);

/**
 * @brief Drop given frame instead of sending it. This will release the 
 * transmission buffer.
 * 
 * @param frame frame to be dropped
 *
 * @return err_t error code
 */
err_t TCPIPEth_Drop(tcpip_frame_t *frame);

/**
 * @brief Sends previously allocated frame
 * 
 * @param frame frame descritor pointer
 * @param da destination address
 * @param ethtype upper layer protocol type
 * 
 * @return err_t send error code
 */
err_t TCPIPEth_Send(tcpip_frame_t *frame, tcpip_eth_addr_t da, 
    tcpip_eth_frame_ethtype_t ethtype);


#endif /* NET_TCPIP_ETH_H */
