/**
 * @file ws.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-05
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef WWW_WS_H
#define WWW_WS_H

#include "err.h"

/**
 * @brief initialize the websocket based api
 *
 * @return err_t error code
 */
err_t WebSocketSrv_Init(void);

#endif /* WWW_WS_H */
