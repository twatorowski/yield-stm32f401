/**
 * @file wssrv.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-12-26
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef NET_WEBSOCKET_WEBSOCKET_H
#define NET_WEBSOCKET_WEBSOCKET_H

#include "err.h"
#include "net/tcpip/tcp_sock.h"
#include "sys/sem.h"
#include "sys/time.h"


/** type of the data being received or sent */
typedef enum websocket_data_type {
    /* text data */
    WS_DATA_TYPE_TEXT,
    /* binary data */
    WS_DATA_TYPE_BIN
} websocket_data_type_t;

/* websocket structure */
typedef struct websocket {
    /* current socket state */
    enum websocket_state {
        WS_STATE_CLOSED,
        WS_STATE_LISTEN,
        WS_STATE_OPEN,
    } state;

    /* underlying tcp socket */
    tcpip_tcp_sock_t *sock;
    /* semaphores that guard the usage of the tcp socket */
    sem_t tx_sem, rx_sem;

    /* current frame data size and read offset */
    size_t rx_size, rx_offs;
    /* opcode of the frame being currently processed */
    uint16_t rx_opcode;
    /* socket mask */
    union { uint8_t u8[4]; uint32_t u32; } mask;

    /* socket's mode of operation */
    enum websocket_mode {
        WS_ROLE_SERVER,
        WS_ROLE_CLIENT,
    } role;
} websocket_t;

/**
 * @brief initialize websocket support
 *
 * @return err_t error code
 */
err_t WebSockSrv_Init(void);

/**
 * @brief create (allocate the memory) the socket
 *
 * @return websocket_t* websocket or null
 */
websocket_t * WebSocket_Create(void);

/**
 * @brief connect to the websocket server
 *
 * @param ws websocket
 * @param ip ip address of the server
 * @param port port of the server
 * @param url endpoint url or null if "/" should be used
 *
 * @return err_t error code
 */
err_t WebSocket_Connect(websocket_t *ws, tcpip_ip_addr_t ip,
    tcpip_tcp_port_t port, const char *url);

/**
 * @brief listen for incomming connections on given port and for given url
 *
 * @param ws websocket
 * @param port port on which we listen
 * @param url url (endpoint) on which we listen
 *
 * @return err_t error code
 */
err_t WebSocket_Listen(websocket_t *ws, tcpip_tcp_port_t port, const char *url);

/**
 * @brief receive data from the socket
 *
 * @param ws websocket
 * @param dtype data type placeholder (can be null if not interested)
 * @param ptr buffer to where to store the data to
 * @param size size of the data
 *
 * @return err_t error code or the number of bytes received
 */
err_t Websocket_Recv(websocket_t *ws, websocket_data_type_t *dtype,
    void *ptr, size_t size);


/**
 * @brief sends a message via websocket
 *
 * @param ws websocket
 * @param dtype data type
 * @param ptr pointer to the data buffer
 * @param size size of the data to be sent
 *
 * @return err_t error code or the number of bytes sent
 */
err_t WebSocket_Send(websocket_t *ws, websocket_data_type_t dtype,
    const void *ptr, size_t size);

/**
 * @brief close the websocket connection
 *
 * @param ws websocket
 *
 * @return err_t error code
 */
err_t WebSocket_Close(websocket_t *ws);


#endif /* NET_WEBSOCKET_WEBSOCKET_H */
