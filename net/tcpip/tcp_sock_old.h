/**
 * @file tcp_sock.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-31
 * 
 * @brief TCP/IP Stack: TCP Sockets
 */

#ifndef NET_TCPIP_TCP_SOCK_OLD_H
#define NET_TCPIP_TCP_SOCK_OLD_H

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "net/tcpip/tcpip.h"
#include "sys/queue.h"
#include "sys/time.h"

/** tcp socket */
typedef struct tcpip_tcp_sock {
    /* socket state */
    enum tcpip_tcp_sock_state {
        TCPIP_TCP_SOCK_STATE_FREE,
        TCPIP_TCP_SOCK_STATE_LISTEN,
        TCPIP_TCP_SOCK_STATE_CONNECT,
        TCPIP_TCP_SOCK_STATE_ESTABLISHING,
        TCPIP_TCP_SOCK_STATE_ESTABLISHED,
        TCPIP_TCP_SOCK_STATE_CLOSING,
        TCPIP_TCP_SOCK_STATE_CLOSED,
        TCPIP_TCP_SOCK_STATE_RESET,
    } state;

    /** link side state one for out rx one for remotes rx channel */
    enum tcp_sock_link_state {
        TCPIP_TCP_LINK_STATE_CLOSED,
        TCPIP_TCP_LINK_STATE_OPEN,
        TCPIP_TCP_LINK_STATE_CLOSING,
    } loc_link, rem_link;

    /** local port/remote port */
    tcpip_tcp_port_t loc_port, rem_port;
    /** remote address */
    tcpip_ip_addr_t addr;

    /** buffer strctures */
    /** queues for the rx/tx channels */
    queue_t *rxq, *txq;

    /** initial value of the sequence number */
    uint32_t rx_seq_init;
    /** sequence number of the data that we received  */
    uint32_t rx_seq_recvd;
    /** sequence number of the data that we acked */
    uint32_t rx_seq_acked;
    /** received flags */
    tcpip_tcp_flags_t rx_flags;

    /** initial value of the transmit sequence */
    uint32_t tx_seq_init;
    /** next sequence number to send */
    uint32_t tx_seq_start, tx_seq_end;
    /** transmission window */
    uint32_t tx_win;
    /** retransmission counter */
    uint32_t tx_retr;
    /* last transmission timestamp */
    time_t tx_ts;
    /** transmitted flags */
    tcpip_tcp_flags_t tx_flags;
} tcpip_tcp_sock_t;

/**
 * @brief Initialize socket layer
 *
 * @return err_t error code
 */
err_t TCPIPTcpSock_Init(void);


/**
 * @brief Main input routine to the socket layer
 *
 * @param frame incoming frame descriptor
 *
 * @return err_t processing error code
 */
err_t TCPIPTcpSock_Input(tcpip_frame_t *frame);

/**
 * @brief Create the tcp socket
 *
 * @param rx_size size of the reception queue
 * @param tx_size size of the transmission queue
 *
 * @return tcpip_tcp_sock_t * socket descriptor
 */
tcpip_tcp_sock_t * TCPIPTcpSock_Create(size_t rx_size, size_t tx_size);

/**
 * @brief Starts listening on selected port. Will exit when connection is made
 *
 * @param sock socket descriptor
 * @param port port number to listen onto
 *
 * @return err_t error code
 */
err_t TCPIPTcpSock_Listen(tcpip_tcp_sock_t *sock, tcpip_tcp_port_t port);

/**
 * @brief Initiate the connection establishment procedure to selected remote
 * ip/port
 *
 * @param sock socket descriptor
 * @param ip remotes ip address
 * @param port remotes port
 * @param timeout timeout
 *
 * @return err_t error code
 */
err_t TCPIPTcpSock_Connect(tcpip_tcp_sock_t *sock, tcpip_ip_addr_t ip,
    tcpip_tcp_port_t port, dtime_t timeout);

/**
 * @brief Receive data from the socket
 *
 * @param sock socket descriptor
 * @param ptr pointer to where to store the data
 * @param size maximal size of the data received
 * @param timeout reception timeout
 *
 * @return err_t error code or the actual size of the data
 */
err_t TCPIPTcpSock_Recv(tcpip_tcp_sock_t *sock, void *ptr, size_t size, 
    dtime_t timeout);

/**
 * @brief Sends data to the remote site of the connection. This will buffer the
 * data within the output queue. If the queue was full then this will stall
 * unitl more space becomes available.
 *
 * @param sock socket descriptor
 * @param ptr data pointer
 * @param size size of the data
 * @param timeout send timeout
 *
 * @return err_t error code or the number of the data bytes sent
 */
err_t TCPIPTcpSock_Send(tcpip_tcp_sock_t *sock, const void *ptr, size_t size, 
    dtime_t timeout);

/**
 * @brief Close the connection
 *
 * @param sock socket descriptor
 * @param timeout timeout
 *
 * @return err_t error code
 */
err_t TCPIPTcpSock_Close(tcpip_tcp_sock_t *sock, dtime_t timeout);

#endif /* NET_TCPIP_TCP_SOCK_OLD_H */
