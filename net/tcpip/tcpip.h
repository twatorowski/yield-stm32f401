/**
 * @file tcpip.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-28
 * 
 * @brief TCP/IP Stack
 */

#ifndef NET_TCPIP_TCPIP
#define NET_TCPIP_TCPIP

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "net/tcpip/arp_frame.h"
#include "net/tcpip/eth_frame.h"
#include "net/tcpip/ip_frame.h"
#include "net/tcpip/icmp_frame.h"
#include "net/tcpip/tcp_frame.h"
#include "net/tcpip/udp_frame.h"
#include "util/bit.h"
#include "sys/ev.h"

/* tpcip stack event */
extern ev_t tcpip_ev;

/** flags that describe the frame being processed */
typedef enum tcpip_frame_flags {
    TCPIP_FRAME_FLAGS_ETH   = BIT_VAL(0),
    TCPIP_FRAME_FLAGS_IP    = BIT_VAL(1),
    TCPIP_FRAME_FLAGS_ARP   = BIT_VAL(2),
    TCPIP_FRAME_FLAGS_ICMP  = BIT_VAL(2),
    TCPIP_FRAME_FLAGS_TCP   = BIT_VAL(3),
    TCPIP_FRAME_FLAGS_UDP   = BIT_VAL(4),
} tcpip_frame_flags_t;

/* single layer descriptor */
typedef struct tcpip_frame_layer {
    void *hdr; size_t size;
} tcpip_frame_layer_t;

/** frame descriptor passed up/down the stack */
typedef struct tcpip_frame {
    /* protocol presence flags */
    tcpip_frame_flags_t flags;
    /* current layer pointer */
    void *ptr; size_t size;
    /* buffer id */
    int bufid;
    /* data link layer */
    union { tcpip_eth_frame_t *eth; };
    /* inet layer */
    union { tcpip_ip_frame_t *ip; tcpip_arp_frame_t *arp; };
    /* transport layer */
    union { tcpip_icmp_frame_t *icmp; 
        tcpip_tcp_frame_t *tcp; tcpip_udp_frame_t *udp; };
} tcpip_frame_t;

/**
 * @brief initialize tcp/ip stack
 * 
 * @return err_t error code
 */
err_t TCPIP_Init(void);


/**
 * @brief reset all stack components for example on interface disconnect
 *
 * @return err_t error code
 */
err_t TCPIP_Reset(void);


#endif /* NET_TCPIP_TCPIP */
