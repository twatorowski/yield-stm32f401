/**
 * @file server.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-23
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef NET_MDNS_SERVER_H
#define NET_MDNS_SERVER_H

#include "err.h"

/**
 * @brief initialize server logic
 *
 * @return err_t error code
 */
err_t MDNSSrv_Init(void);


#endif /* NET_MDNS_SERVER_H */
