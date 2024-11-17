/**
 * @file udp_sock.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-02
 * 
 * @brief TCP/IP Stack: User Datagram Protocol
 */

#ifndef NET_TCPIP_UDP_SOCK
#define NET_TCPIP_UDP_SOCK

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "net/tcpip/tcpip.h"
#include "sys/queue.h"
#include "sys/time.h"

/** udp socket */
typedef struct tcpip_udp_sock_t {
    /** local port/ remote port */
    tcpip_udp_port_t loc_port;
    /** received data queue */
    queue_t *rxq;
    /** current frame data */
    size_t rx_offs, rx_size;
    /** ip address that we are receiiving from */
    tcpip_ip_addr_t rx_ip;
    /** port that we are receiving from */
    tcpip_udp_port_t rx_port;
} tcpip_udp_sock_t;


/* input routine to the socketization layer */
err_t TCPIPUdpSock_Input(tcpip_frame_t *frame);
/* create udp socket */
tcpip_udp_sock_t * TCPIPUdpSock_CreateSocket(tcpip_udp_port_t port, 
    size_t rx_size);
/* destroy previously created socket */
void TCPIPUdpSock_DestroySocket(tcpip_udp_sock_t *s);
/* receive udp data from socket */
err_t TCPIPUdpSock_RecvFrom(tcpip_udp_sock_t *sock, tcpip_ip_addr_t *addr, 
    tcpip_udp_port_t *port, void *ptr, size_t size, dtime_t timeout);
/* send to remote party */
err_t TCPIPUdpSock_SendTo(tcpip_udp_sock_t *sock, tcpip_ip_addr_t addr, 
    tcpip_udp_port_t port, const void *ptr, size_t size);


#endif /* NET_TCPIP_UDP_SOCK */
