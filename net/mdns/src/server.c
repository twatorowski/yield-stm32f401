/**
 * @file server.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-23
 * 
 * @copyright Copyright (c) 2024
 */

#include "assert.h"
#include "config.h"
#include "err.h"

#include "net/mdns/frame.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/udp_sock.h"
#include "sys/yield.h"
#include "util/elems.h"

#define DEBUG DLVL_DEBUG
#include "debug.h"

/* input routine for the mdns frames */
static err_t MDNSSrv_Input(tcpip_udp_sock_t *sock,
    tcpip_ip_addr_t ip, tcpip_udp_port_t port, const void *ptr, size_t size)
{
    dprintf_d("got something!\n", 0);
    return EFATAL;
}

/* server task */
static void MDNSSrv_Task(void *arg)
{
    /* buffer for holding the received frame */
    static uint8_t rx_buf[512];

    /* create the socket */
    tcpip_udp_sock_t *sock = TCPIPUdpSock_CreateSocket(MDNS_SRV_PORT, 512);
    /* unable to allocate memory for the socket */
    assert(sock, "unable to create the socket for mdns server");

    /* main serving loop */
    for (;; Yield()) {
        /* sender port and sender's ip address */
        tcpip_ip_addr_t ip; tcpip_udp_port_t port;
        /* receive data from the socket */
        err_t ec = TCPIPUdpSock_RecvFrom(sock, &ip, &port, rx_buf,
            sizeof(rx_buf), 0);
        /* error during reception */
        if (ec < EOK)
            continue;

        /* process frame */
        MDNSSrv_Input(sock, ip, port, rx_buf, ec);
    }
}

/* initialize server logic */
err_t MDNSSrv_Init(void)
{
    /* create the task for serving mdns requests */
    if (Yield_Task(MDNSSrv_Task, 0, 2048) < EOK)
        return EFATAL;
    /* return status of the initialization */
    return EOK;
}