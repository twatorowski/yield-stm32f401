/**
 * @file server
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-21
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef NET_DHCP_SERVER_H
#define NET_DHCP_SERVER_H

#include "err.h"

/**
 * @brief initialize dhcp server
 *
 * @return err_t status code
 */
err_t DHCPSrv_Init(void);


#endif /* NET_DHCP_SERVER_H */
