/**
 * @file arp_table.c
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief TCP/IP Stack: ARP Table
 */

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "config.h"
#include "err.h"
#include "net/tcpip/eth.h"
#include "net/tcpip/eth_addr.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/tcpip.h"
#include "sys/sleep.h"
#include "sys/time.h"
#include "util/elems.h"
#include "util/endian.h"
#include "util/forall.h"


/* address array */
struct arp_entry {
    /* hardware address */
    tcpip_eth_addr_t ha;
    /* protocol address */
    tcpip_ip_addr_t pa;
    /* update timestamp, 0 marks empty entry */
    time_t ts;
} arp_table[TCPIP_ARP_TABLE_SIZE];

/* reset the arp table */
err_t TCPIPArpTable_ResetTable(void)
{
    /* arp entry pointer */
    struct arp_entry *e;
    /* reset all entries */
    forall (e, arp_table)
        *e = (struct arp_entry){0};
    /* return status */
    return EOK;
}

/* update protocol address with hardware address: here we update the existing
 * entry or overwrite the oldest one or use the free one  */
err_t TCPIPArpTable_UpdateTable(tcpip_eth_addr_t ha, tcpip_ip_addr_t pa)
{
    /* appropriate array entry */
    int oldest_id = -1, ha_id = -1; dtime_t oldest_dtime = 0;
    /* look for entry to be updated */
    for (struct arp_entry *e = arp_table; e != arp_table + elems(arp_table); e++) {
        /* address found */
        if (TCPIPEthAddr_AddressMatch(ha, e->ha))
            ha_id = e - arp_table;
        /* timestamp check */
        if (oldest_dtime < dtime(time(0), e->ts))
            oldest_id = e - arp_table, oldest_dtime = dtime(time(0), e->ts);
    }

    /* get the arp entry that we wish to update */
    struct arp_entry *e = &arp_table[ha_id >= 0 ? ha_id : oldest_id];
    /* update entry */
    e->ha = ha; e->pa = pa; e->ts = time(0);

    /* this routine never fails */
    return EOK;
}

/* find the ethernet address associated with ip address */
err_t TCPIPArpTable_GetHWAddr(tcpip_ip_addr_t pa, tcpip_eth_addr_t *ha)
{
    /* entry pointer */
    struct arp_entry *e;

    /* look for the entry with given hardware address */
    for (e = arp_table; e != arp_table + elems(arp_table); e++)
        if (TCPIPIpAddr_AddressMatch(e->pa, pa)) {
            *ha = e->ha; e->ts = time(0); return EOK;
        }
    
    /* entry nof found */
    return EUNKADDR;
}

