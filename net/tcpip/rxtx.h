/**
 * @file rxtx.h
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-28
 *
 * @brief TCP/IP Stack: transmission and reception routines
 */

#ifndef NET_TCPIP_RXTX_H
#define NET_TCPIP_RXTX_H

#include <stddef.h>

#include "err.h"
#include "net/tcpip/tcpip.h"

/**
 * @brief initialize underlying physical interface
 *
 * @return err_t error code
 */
err_t TCPIPRxTx_Init(void);

/**
 * @brief Allocate memory for packet build-up
 *
 * @param frame structure that describes packet memory, to be filled with
 * upper layers data
 *
 * @return err_t error code
 */
err_t TCPIPRxTx_Alloc(tcpip_frame_t *frame);


/**
 * @brief drop the given frame from the buffers
 * 
 * @param frame frame to be dropped
 * 
 * @return err_t error code
 */
err_t TCPIPRxTx_Drop(tcpip_frame_t *frame);

/**
 * @brief Sends (previously allocated) frame via underlying interface
 *
 * @param frame frame descriptor data pointer
 *
 * @return err_t error code
 */
err_t TCPIPRxTx_Send(tcpip_frame_t *frame);

#endif /* NET_TCPIP_RXTX_H */
