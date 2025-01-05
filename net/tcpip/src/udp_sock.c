/**
 * @file udp_sock.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-02
 * 
 * @brief TCP/IP Stack: User Datagram Protocol
 */

#include <stdint.h>
#include <stddef.h>

#include "assert.h"
#include "compiler.h"
#include "config.h"
#include "err.h"
#include "net/tcpip/ip.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/udp.h"
#include "net/tcpip/udp_frame.h"
#include "net/tcpip/udp_sock.h"
#include "sys/heap.h"
#include "sys/queue.h"
#include "sys/time.h"
#include "sys/yield.h"
#include "util/elems.h"
#include "util/endian.h"
#include "util/minmax.h"
#include "util/string.h"


/* sockets */
static tcpip_udp_sock_t sockets[TCPIP_UDP_SOCK_NUM];
/* type for the information contained within the socket's rxq that preceeds any 
 * frame data */
typedef struct rxq_hdr {
    /* size of the frame contents */
    size_t size;
    /* source ip adddress */
    tcpip_ip_addr_t ip;
    /* source port */
    tcpip_udp_port_t port;
} PACKED rxq_hdr_t;

/* push the data into socket */
static err_t TCPIPUdpSock_ProcessIncoming(tcpip_frame_t *frame, 
    tcpip_udp_sock_t *sock)
{
    /* udp frame pointer */
    tcpip_udp_frame_t *udp = frame->udp;
    /* extract port information */
    tcpip_udp_port_t dst_port = TCPIPUdpFrame_GetDstPort(udp);
    tcpip_udp_port_t src_port = TCPIPUdpFrame_GetSrcPort(udp);
    /* source ip address */
    tcpip_ip_addr_t ip = TCPIPIpFrame_GetSrcAddr(frame->ip);

    /* not the destination socket */
    if (!sock->loc_port || sock->loc_port != dst_port)
        return EUNREACHABLE;
    
    /* determine how many bytes are we about to put in rxq */
    size_t b_to_put = sizeof(size_t) + sizeof(ip) + sizeof(src_port) +
        frame->size;
    /* unable to store incoming frame */
    if (Queue_GetFree(sock->rxq) < b_to_put)
        return EOK;
    
    /* store the meta-data */
    Queue_Put(sock->rxq, &frame->size, sizeof(frame->size));
    Queue_Put(sock->rxq, &ip, sizeof(ip));
    Queue_Put(sock->rxq, &src_port, sizeof(src_port));
    /* store the frame contents */
    Queue_Put(sock->rxq, frame->ptr, frame->size);

    /* report success */
    return EOK;
}

/* input routine to the socketization layer */
err_t TCPIPUdpSock_Input(tcpip_frame_t *frame)
{
    /* error code */
    err_t ec = EFATAL;

    /* process the frame with respect to all sockets */
    for (tcpip_udp_sock_t *s = sockets; s != sockets + elems(sockets); s++)
        if ((ec = TCPIPUdpSock_ProcessIncoming(frame, s)) == EOK)
            break;
    /* this will be set to ok if any of the sockets accepts incoming frame */
    return ec;
}

/* create udp socket */
tcpip_udp_sock_t * TCPIPUdpSock_CreateSocket(tcpip_udp_port_t port, 
    size_t rx_size)
{
    /* socket pointer */
    tcpip_udp_sock_t *sock;

    /* look for the free socket */
    for (sock = sockets; sock != sockets + elems(sockets); sock++)
        if (sock->loc_port == 0)
            break;
    
    /* none found? */
    if (sock == sockets + elems(sockets))
        return 0;
    
    /* allocate memory for the incoming frames */
    sock->rxq = Queue_Create(1, rx_size);
    sock->loc_port = port;
    /* sanity check */
    assert(sock->rxq, "unable to allocte memory for udp socket\n");
    /* return socket pointer */
    return sock;
}

/* destroy previously created socket */
void TCPIPUdpSock_DestroySocket(tcpip_udp_sock_t *sock)
{
    /* clear the port number */
    Queue_Destroy(sock->rxq);
    /* socket record is free when it's local port is set to 0 */
    sock->loc_port = 0;
}

/* receive udp data from socket */
err_t TCPIPUdpSock_RecvFrom(tcpip_udp_sock_t *sock, tcpip_ip_addr_t *addr, 
    tcpip_udp_port_t *port, void *ptr, size_t size, dtime_t timeout)
{
    /* previous frame was consumed. we need to wait for another one to arrive 
     * and get stored within our buffers */
    if (sock->rx_offs == sock->rx_size) {
        /* wait for the 1st data that constitutes the header. if it's present 
         * then we can be sure that the rest of the data is also present since
         * we use coop mutli-tasking mode */
        if (Queue_GetWait(sock->rxq, &sock->rx_size, sizeof(sock->rx_size), 
            timeout) == 0)
            return ETIMEOUT;
        /* continue with the rest of the meta-data */
        Queue_Get(sock->rxq, &sock->rx_ip, sizeof(sock->rx_ip));
        Queue_Get(sock->rxq, &sock->rx_port, sizeof(sock->rx_port));
        /* reset the offset counter */
        sock->rx_offs = 0;
    }

    /* setup the address and port information */
    *addr = sock->rx_ip;
    *port = sock->rx_port;

    /* read the data, but no more that what is left within current frame */
    size = Queue_Get(sock->rxq, ptr, min(size, sock->rx_size));
    sock->rx_offs += size;
    /* return the number of bytes received */
    return size;
}

/* send to remote party */
err_t TCPIPUdpSock_SendTo(tcpip_udp_sock_t *sock, tcpip_ip_addr_t addr, 
    tcpip_udp_port_t port, const void *ptr, size_t size)
{
    /* error code */
    err_t ec; tcpip_frame_t frame;
    /* allocate space for underlying ethernet frame */
    if ((ec = TCPIPUdp_Alloc(&frame)) != EOK)
        return ec;
    
    /* setup data */
    memcpy(frame.ptr, ptr, frame.size = size);
    /* send frame */
    return TCPIPUdp_Send(&frame, addr, sock->loc_port, port);
}