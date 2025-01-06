/**
 * @file eth.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-28
 * 
 * @brief TCP/IP Stack: Ethernet II layer
 */

#include <stdint.h>
#include <stddef.h>

#include "config.h"
#include "err.h"
#include "net/tcpip/arp.h"
#include "net/tcpip/eth.h"
#include "net/tcpip/eth_addr.h"
#include "net/tcpip/eth_frame.h"
#include "net/tcpip/ip.h"
#include "net/tcpip/rxtx.h"
#include "net/tcpip/tcpip.h"
#include "util/endian.h"

/* initialize ethernet layer */
err_t TCPIPEth_Init(void)
{
    /* nothing to do */
    return EOK;
}

/* main input routine to the ethernet layer */
err_t TCPIPEth_Input(tcpip_frame_t *frame)
{
    /* error code */
    err_t rc;

    /* ethernet header is located at the beginning of the frame */
    frame->eth = frame->ptr;
    /* setup frame descriptor for upper layers */
    frame->flags |= TCPIP_FRAME_FLAGS_ETH;
    frame->size -= sizeof(tcpip_eth_frame_t);
    frame->ptr = frame->eth->pld;

    /* switch on the payload type */
    switch (TCPIPEthFrame_GetEthType(frame->eth)) {
    /* ip carrying frame */
    case TCPIP_ETH_ETHTYPE_IP : {
        rc = TCPIPIp_Input(frame);
    } break;
    /* address resolution protocol frame */
    case TCPIP_ETH_ETHTYPE_ARP : {
        rc = TCPIPArp_Input(frame);
    } break;
    /* unknown protocol */
    default : rc = EUNKPROT; break;
    }

    /* return the error code */
    return rc;
}

/* allocate space for ethernet frame in output buffers */
err_t TCPIPEth_Alloc(tcpip_frame_t *frame)
{
    /* error code */
    err_t ec;
    /* try to allocate frame in output buffers */
    if ((ec = TCPIPRxTx_Alloc(frame)) != EOK)
        return ec;
    
    /* setup ethernet part */
    frame->flags |= TCPIP_FRAME_FLAGS_ETH;
    frame->eth = frame->ptr;
    frame->ptr = frame->eth->pld;

    /* report success */
    return EOK;
}

/* drop the corresponding frame */
err_t TCPIPEth_Drop(tcpip_frame_t *frame)
{
    /* no additional activities */
    return TCPIPRxTx_Drop(frame);
}

/* send allocated data */
err_t TCPIPEth_Send(tcpip_frame_t *frame, tcpip_eth_addr_t da, 
    tcpip_eth_frame_ethtype_t ethtype)
{
    /* pointer to allocated ethernet frame header */
    tcpip_eth_frame_t *eth = frame->eth;
    /* setup frame fields */
    TCPIPEthFrame_SetDA(eth, da);
    TCPIPEthFrame_SetSA(eth, TCPIPEthAddr_GetMAC());
    TCPIPEthFrame_SetEthType(eth, ethtype);

    /* setup frame structure for underlying layer */
    frame->flags |= TCPIP_FRAME_FLAGS_ETH;
    frame->ptr = frame->eth;
    frame->size += sizeof(tcpip_eth_frame_t);

    /* send the data */
    return TCPIPRxTx_Send(frame);
}