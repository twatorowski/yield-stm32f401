/**
 * @file ip_addr.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief TCP/IP Stack: Internet Protocol Addressing
 */

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "config.h"
#include "err.h"
#include "net/tcpip/ip_addr.h"
#include "util/stdio.h"

/* our ip address */
static tcpip_ip_addr_t tcpip_ip_addr = TCPIP_IP_ADDRESS;
/* sub-network mask */
static tcpip_ip_addr_t tcpip_ip_mask = TCPIP_IP_NETMASK;
/* gateway */
static tcpip_ip_addr_t tcpip_ip_gway = TCPIP_IP_GATEWAY;

/* obtain current ip */
tcpip_ip_addr_t TCPIPIpAddr_GetIP(void)
{
    return tcpip_ip_addr;
}

/* setup ip address */
tcpip_ip_addr_t TCPIPIpAddr_SetIP(tcpip_ip_addr_t ip)
{
    /* return the value after update */
    return tcpip_ip_addr = ip;
}

/* obtain current subnet mask */
tcpip_ip_addr_t TCPIPIpAddr_GetSubnetMask(void)
{
    return tcpip_ip_mask;
}

/* set current subnet mask value */
tcpip_ip_addr_t TCPIPIpAddr_SetSubnetMask(tcpip_ip_addr_t ip)
{
    /* return the value after update */
    return tcpip_ip_mask = ip;
}

/* obtain current router/gateway ip */
tcpip_ip_addr_t TCPIPIpAddr_GetGatewayIP(void)
{
    return tcpip_ip_gway;
}

/* set gateway ip */
tcpip_ip_addr_t TCPIPIpAddr_SetGatewayIP(tcpip_ip_addr_t ip)
{
    /* return the value after update */
    return tcpip_ip_gway = ip;
}

/* check if the addresses are the same */
int TCPIPIpAddr_AddressMatch(tcpip_ip_addr_t a, tcpip_ip_addr_t b)
{
    return a.u32 == b.u32;
}

/* build up the next address */
tcpip_ip_addr_t TCPIPIpAddr_Next(tcpip_ip_addr_t ip)
{
    /* copy the address from the initial value */
    tcpip_ip_addr_t next = ip;
    /* bump up */
    next.u32++;
    /* return next adress */
    return next;
}

/* check if the address is the same as our address */
int TCPIPIpAddr_IsMatchingUnicast(tcpip_ip_addr_t a)
{
    return (a.u32 == tcpip_ip_addr.u32);
}

/* is the address broadcast one with respect to sub-net mask */
int TCPIPIpAddr_IsMatchingBroadcast(tcpip_ip_addr_t a)
{
    /* check if masked part is the same as in our address and if the unmasked 
     * part is all ones */
    return  a.u32 == (((tcpip_ip_addr_t)TCPIP_IP_ADDR_BCAST).u32) ||
            ( ((a.u32 &  tcpip_ip_mask.u32) ==  tcpip_ip_addr.u32) &&
              ((a.u32 & ~tcpip_ip_mask.u32) == ~tcpip_ip_mask.u32) );
}

/* is this a multicast address? */
int TCPIPIpAddr_IsMatchingMulticast(tcpip_ip_addr_t a)
{
    /* address range */
    const tcpip_ip_addr_t start = TCPIP_IP_ADDR(224, 0, 0, 0);
    const tcpip_ip_addr_t end = TCPIP_IP_ADDR(239, 255, 255, 255);
    /* do the comparison */
    return (a.u32 >= start.u32) && (a.u32 <= end.u32);
}

/* is the address matching any ip address? */
int TCPIPIpAddr_IsMatchingAny(tcpip_ip_addr_t a)
{
    return a.u32 == (((tcpip_ip_addr_t)TCPIP_IP_ADDR_ANY).u32);
}

/* is the address within subnet? */
int TCPIPIpAddr_IsWithinSubnet(tcpip_ip_addr_t a)
{
    /* check if masked part is the same as in our address when masked */
    return ((a.u32 & tcpip_ip_mask.u32) == (tcpip_ip_addr.u32 & tcpip_ip_mask.u32));
}

/* prints the ip address */
char * TCPIPIpAddr_ToStr(tcpip_ip_addr_t a, tcpip_ip_addr_str_t str)
{
    /* */
    static tcpip_ip_addr_str_t __str;

    /* print the ip address */
    snprintf(str ? str: __str, sizeof(tcpip_ip_addr_str_t), "%d.%d.%d.%d", 
        a.u8[3], a.u8[2], a.u8[1], a.u8[0]);

    /* return the pointer to the string with ip address */
    return str ? str : __str;
}