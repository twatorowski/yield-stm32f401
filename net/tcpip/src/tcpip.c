/**
 * @file tcpip.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-28
 * 
 * @brief TCP/IP stack
 */

#include "err.h"
#include "net/tcpip/arp.h"
#include "net/tcpip/eth.h"
#include "net/tcpip/icmp.h"
#include "net/tcpip/ip.h"
#include "net/tcpip/udp.h"
#include "net/tcpip/tcp.h"
#include "net/tcpip/tcp_sock.h"
#include "net/tcpip/rxtx.h"

/* initialize tcp/ip stack */
err_t TCPIP_Init(void)
{
    /* initialize Ethernet II layer */
    TCPIPEth_Init();
    /* initialize arp layer */
    TCPIPArp_Init();
    /* initialize ip layer */
    TCPIPIp_Init();
    /* initialize icmp layer */
    TCPIPIcmp_Init();
    /* initialize udp layer */
    TCPIPUdp_Init();
    /* initialize tcp layer */
    TCPIPTcp_Init();
    /* initialize tcp socket layer */
    TCPIPTcpSock_Init();

    /* initialize talking to underlying interface */
    TCPIPRxTx_Init();

    /* report status */
    return EOK;
}
