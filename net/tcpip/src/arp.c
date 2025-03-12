/**
 * @file arp.c
 * 
 * @date 2021-02-02
 * @author twatorowski 
 * 
 * @brief TCP/IP Stack: Address Resolution Protocol 
 */

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "config.h"
#include "err.h"
#include "net/tcpip/arp.h"
#include "net/tcpip/arp_frame.h"
#include "net/tcpip/arp_table.h"
#include "net/tcpip/eth.h"
#include "net/tcpip/eth_addr.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/tcpip.h"
#include "sys/sleep.h"
#include "sys/time.h"
#include "util/elems.h"
#include "util/endian.h"

/* since both request and respose share the same structure i've created this 
 * helper function */
static err_t TCPIPArp_SendFrame(tcpip_eth_addr_t eth_da, tcpip_arp_oper_t oper, 
    tcpip_eth_addr_t sha, tcpip_ip_addr_t spa, tcpip_eth_addr_t tha, 
    tcpip_ip_addr_t tpa)
{
    /* error code */
    err_t ec; tcpip_frame_t frame;
    /* allocate space for underlying ethernet frame */
    if ((ec = TCPIPEth_Alloc(&frame)) != EOK)
        return ec;

    /* setup arp layer */
    frame.flags |= TCPIP_FRAME_FLAGS_ARP;
    frame.arp = frame.ptr;
    /* setup pointers to point into ethernet's frame payload field */
    tcpip_arp_frame_t *arp = frame.arp; 
    tcpip_arp_frame_pld_eth_ip_t *arp_eth_ip = 
        (tcpip_arp_frame_pld_eth_ip_t *)arp->pld;

    /* setup protocol type for both: link and internet layer */
    TCPIPArpFrame_SetOper(arp, oper);
    TCPIPArpFrame_SetHLen(arp, sizeof(arp_eth_ip->sha));
    TCPIPArpFrame_SetPLen(arp, sizeof(arp_eth_ip->spa));
    TCPIPArpFrame_SetHType(arp, TCPIP_ARP_HTYPE_ETHERNET);
    TCPIPArpFrame_SetPType(arp, TCPIP_ARP_PTYPE_IP);

    /* assign addresses */
    TCPIPArpFrameETHIP_SetSHA(arp_eth_ip, sha);
    TCPIPArpFrameETHIP_SetTHA(arp_eth_ip, tha);
    TCPIPArpFrameETHIP_SetSPA(arp_eth_ip, spa);
    TCPIPArpFrameETHIP_SetTPA(arp_eth_ip, tpa);

    /* prepare frame for underlying layers */
    frame.size = sizeof(*arp) + sizeof(*arp_eth_ip);
    /* finish up by sending */
    return TCPIPEth_Send(&frame, eth_da, TCPIP_ETH_ETHTYPE_ARP);
}

/* process arp request */
static err_t TCPIPArp_ProcessRequest(tcpip_frame_t *frame)
{
    /* arp header */
    tcpip_arp_frame_t *arp = frame->arp;
    tcpip_eth_frame_t *eth = frame->eth;
    /* we only work in eth/ip mode, so we can build a pointer right away */
    tcpip_arp_frame_pld_eth_ip_t *arp_eth_ip = 
        (tcpip_arp_frame_pld_eth_ip_t *)arp->pld;

    /* unknown hardware/protocol types - may not be an error */
    if (TCPIPArpFrame_GetHType(arp) != TCPIP_ARP_HTYPE_ETHERNET ||
        TCPIPArpFrame_GetPType(arp) != TCPIP_ARP_PTYPE_IP)
        return EUNKPROT;
    
    /* address lengths do not match */
    if (TCPIPArpFrame_GetHLen(arp) != sizeof(arp_eth_ip->sha) ||
        TCPIPArpFrame_GetPLen(arp) != sizeof(arp_eth_ip->spa))
        return EUNKPROT;

    /* get the protocol addresses */
    tcpip_ip_addr_t spa = TCPIPArpFrameETHIP_GetSPA(arp_eth_ip);
    tcpip_ip_addr_t tpa = TCPIPArpFrameETHIP_GetTPA(arp_eth_ip);
    
    /* are we the one being addressed by this request? */
    if (!TCPIPIpAddr_AddressMatch(tpa, TCPIPIpAddr_GetIP()))
        return EOK;

    /* get the addresses that will be used to generate the response */
    tcpip_eth_addr_t sha = TCPIPArpFrameETHIP_GetSHA(arp_eth_ip);
    tcpip_eth_addr_t sa = TCPIPEthFrame_GetSA(eth);
    
    /* update the arp look up table */
    TCPIPArpTable_UpdateTable(sha, spa);
    /* send the reply frame */
    TCPIPArp_SendFrame(sa, TCPIP_ARP_OPER_REPLY, TCPIPEthAddr_GetMAC(), 
        TCPIPIpAddr_GetIP(), sha, spa);

    /* report success */
    return EOK;
}

/* process incoming replies */
static err_t TCPIPArp_ProcessReply(tcpip_frame_t *frame)
{
    /* arp header */
    tcpip_arp_frame_t *arp = frame->arp;
    /* we only work in eth/ip mode, so we can build a pointer right away */
    tcpip_arp_frame_pld_eth_ip_t *arp_eth_ip = 
        (tcpip_arp_frame_pld_eth_ip_t *)arp->pld;
    
    /* unknown hardware/protocol types - may not be an error */
    if (TCPIPArpFrame_GetHType(arp) != TCPIP_ARP_HTYPE_ETHERNET ||
        TCPIPArpFrame_GetPType(arp) != TCPIP_ARP_PTYPE_IP)
        return EUNKPROT;
    
    /* address lengths do not match */
    if (TCPIPArpFrame_GetHLen(arp) != sizeof(arp_eth_ip->sha) ||
        TCPIPArpFrame_GetPLen(arp) != sizeof(arp_eth_ip->spa))
        return EUNKPROT;
    
    /* get the protocol & hardwre addresses */
    tcpip_ip_addr_t spa = TCPIPArpFrameETHIP_GetSPA(arp_eth_ip);
    tcpip_ip_addr_t tpa = TCPIPArpFrameETHIP_GetTPA(arp_eth_ip);
    tcpip_eth_addr_t sha = TCPIPArpFrameETHIP_GetSHA(arp_eth_ip);
    tcpip_eth_addr_t tha = TCPIPArpFrameETHIP_GetTHA(arp_eth_ip);

    /* are we the one being addressed by this reply? */
    if (!TCPIPIpAddr_IsMatchingUnicast(tpa) || 
        !TCPIPEthAddr_IsMatchingUnicast(tha))
        return EOK;

    /* update the table with the information provided */
    TCPIPArpTable_UpdateTable(sha, spa);
    /* report success */
    return EOK;
}

/* initialize Address Resolution Protocol layer */
err_t TCPIPArp_Init(void)
{
    /* return status */
    return EOK;
}

/* reset the arp table */
err_t TCPIPArp_Reset(void)
{
    return TCPIPArpTable_ResetTable();
}

/* main input routine to the ethernet layer */
err_t TCPIPArp_Input(tcpip_frame_t *frame)
{
    /* returned value */
    err_t rc = EOK;

    /* arp is just after the ethernet */
    frame->arp = frame->ptr;
    /* setup layer stuff */
    frame->flags |= TCPIP_FRAME_FLAGS_ARP;

    /* switch on operation code */
    switch (TCPIPArpFrame_GetOper(frame->arp)) {
    /* process requests */
    case TCPIP_ARP_OPER_REQUEST : {
        rc = TCPIPArp_ProcessRequest(frame);
    } break;
    /* process replies */
    case TCPIP_ARP_OPER_REPLY : {
        rc = TCPIPArp_ProcessReply(frame);
    } break;
    /* unknown */
    default: rc = EUNKREQ; break;
    }

    /* return status */
    return rc;
}

/* return the hardware address that corresponds with given protocol address */
err_t TCPIPArp_GetHWAddr(tcpip_ip_addr_t pa, tcpip_eth_addr_t *ha)
{
    /* deal with any address class */
    if (TCPIPIpAddr_IsMatchingAny(pa) || 
        TCPIPIpAddr_IsMatchingBroadcast(pa)) {
        /* in case of any return broadcast */
        *ha = (tcpip_eth_addr_t)TCPIP_ETH_ADDR_BCAST; return EOK;
    }

    /* this loop will poll the arp table for as long as the address is found */
    for (int i = 0; i < TCPIP_ARP_ATTEMPTS; i++) {
        /* look for the entry with given hardware address */
        if (TCPIPArpTable_GetHWAddr(pa, ha) == EOK)
            return EOK;
        /* no entry is found, need to issue the request */
        TCPIPArp_SendFrame((tcpip_eth_addr_t)TCPIP_ETH_ADDR_BCAST,
            TCPIP_ARP_OPER_REQUEST, TCPIPEthAddr_GetMAC(), 
            TCPIPIpAddr_GetIP(), (tcpip_eth_addr_t)TCPIP_ETH_ADDR_BCAST, pa);
        /* now use the cooldown timer to wait for a little bit */
        Sleep(300);
    }

    /* return broadcast address if not found */
    return EUNKADDR;
}

