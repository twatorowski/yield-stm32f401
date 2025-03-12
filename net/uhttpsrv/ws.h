/**
 * @file ws.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-11
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef NET_UHTTPSRV_WS_H
#define NET_UHTTPSRV_WS_H

#include "sys/sem.h"

/** type of the data being received or sent */
typedef enum uhttp_ws_data_type {
    /* text data */
    HTTP_WS_DATA_TYPE_TEXT,
    /* binary data */
    HTTP_WS_DATA_TYPE_BIN
} uhttp_ws_data_type_t;



/* initialize common parts of the websocket support */
err_t UHTTPSrvWS_Init(void);

/**
 * @brief accept incoming connection request
 *
 * @param req uhttpsrv request
 *
 * @return err_t error code
 */
err_t UHTTPSrvWS_Accept(struct uhttp_request *req);

/**
 * @brief receive data from the socket
 *
 * @param req request which hosts the websocket connection
 * @param dtype data type placeholder
 * @param ptr pointer to where to store the data
 * @param size size of the data
 * @param timeout reception timeout
 *
 * @return err_t error code
 */
err_t UHTTPSrvWS_Recv(struct uhttp_request *req, uhttp_ws_data_type_t *dtype,
    void *ptr, size_t size, dtime_t timeout);

/**
 * @brief sends the data over the websocket
 *
 * @param req request which hosts the websocket connection
 * @param dtype type of data to be sent
 * @param ptr pointer to the data
 * @param size size of the data to be sent
 *
 * @return err_t error code
 */
err_t UHTTPSrvWS_Send(struct uhttp_request *req, uhttp_ws_data_type_t dtype,
    const void *ptr, size_t size);


/**
 * @brief Closes websocket connection
 *
 * @param req requests which hosts underlying connection
 * @return err_t error code
 */
err_t UHTTPSrvWS_Close(struct uhttp_request *req);

#endif /* NET_UHTTPSRV_WS_H */
