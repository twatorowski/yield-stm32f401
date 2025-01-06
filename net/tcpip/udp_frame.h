/**
 * @file udp_frame.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-02
 * 
 * @brief TCP/IP Stack: User Datagram Protocol
 */

#ifndef NET_TCPIP_UDP_FRAME
#define NET_TCPIP_UDP_FRAME

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "util/endian.h"

/** udp port */
typedef uint16_t tcpip_udp_port_t;

/** udp frame */
typedef struct tcpip_udp_frame {
    /** source port */
    uint16_t src_port;
    /** destination port */
    uint16_t dst_port;
    /** length of the entire packet */
    uint16_t length;
    /** checksum of entire packet (or zeros if unused) */
    uint16_t checksum;
    /** payload */
    uint8_t pld[];
} PACKED tcpip_udp_frame_t;

/**
 * @brief Returns the source port from the frame data
 * 
 * @param udp udp frame
 * 
 * @return  tcpip_udp_port_t source port
 */
static inline ALWAYS_INLINE tcpip_udp_port_t TCPIPUdpFrame_GetSrcPort(
    const tcpip_udp_frame_t *udp)
{
    return BETOH16(udp->src_port);
}

/**
 * @brief Writes the source port into the frame data
 * 
 * @param udp udp frame
 * @param src_port source port
 */
static inline ALWAYS_INLINE void TCPIPUdpFrame_SetSrcPort(
    tcpip_udp_frame_t *udp, tcpip_udp_port_t src_port)
{
    udp->src_port = HTOBE16(src_port);
}

/**
 * @brief Returns the destination port from the frame data
 * 
 * @param udp udp frame
 * 
 * @return  tcpip_udp_port_t source port
 */
static inline ALWAYS_INLINE tcpip_udp_port_t TCPIPUdpFrame_GetDstPort(
    const tcpip_udp_frame_t *udp)
{
    return BETOH16(udp->dst_port);
}

/**
 * @brief Writes the destination port into the frame data
 * 
 * @param udp udp frame
 * @param dst_port destination port
 */
static inline ALWAYS_INLINE void TCPIPUdpFrame_SetDstPort(
    tcpip_udp_frame_t *udp, tcpip_udp_port_t dst_port)
{
    udp->dst_port = HTOBE16(dst_port);
}

/**
 * @brief Read the length field from the udp header
 * 
 * @param udp udp frame 
 * @return uint16_t frame length field value 
 */
static inline ALWAYS_INLINE uint16_t TCPIPUdpFrame_GetLength(
    const tcpip_udp_frame_t *udp)
{
    return BETOH16(udp->length);
}

/**
 * @brief Write length field value into the udp frame header
 * 
 * @param udp udp frame
 * @param length length value
 */
static inline ALWAYS_INLINE void TCPIPUdpFrame_SetLength(
    tcpip_udp_frame_t *udp, uint16_t length)
{
    udp->length = HTOBE16(length);
}

/**
 * @brief Read the checksum field from the udp header
 * 
 * @param udp udp frame 
 * @return uint16_t frame checksum field value 
 */
static inline ALWAYS_INLINE uint16_t TCPIPUdpFrame_GetChecksum(
    const tcpip_udp_frame_t *udp)
{
    return BETOH16(udp->checksum);
}

/**
 * @brief Write checksum field value into the udp frame header
 * 
 * @param udp udp frame
 * @param checksum checksum value
 */
static inline ALWAYS_INLINE void TCPIPUdpFrame_SetChecksum(
    tcpip_udp_frame_t *udp, uint16_t checksum)
{
    /* read the source port from the frame data */
    udp->checksum = HTOBE16(checksum);
}


#endif /* NET_TCPIP_UDP_FRAME */
