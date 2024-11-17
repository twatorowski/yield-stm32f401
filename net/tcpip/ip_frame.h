/**
 * @file ip_frame.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief TCP/IP Stack: Internet Protocol Frame
 */

#ifndef NET_TCPIP_IP_FRAME
#define NET_TCPIP_IP_FRAME

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "compiler.h"
#include "net/tcpip/ip_addr.h"
#include "util/bit.h"
#include "util/endian.h"

/** ip header length/version field bit meaning */
typedef enum tcpip_ip_hdr_len_version {
    /** don't fragment */
    TCPIP_IP_HDR_LEN_VER_VER = 0xF0,
    /** more fragments */
    TCPIP_IP_HDR_LEN_VER_HDRLEN = 0x0F,
} tcpip_ip_hdr_len_version_t;

/** ip header flags */
typedef enum tcpip_ip_flags {
    /** don't fragment */
    TCPIP_IP_FLAGS_DF = 0x4000,
    /** more fragments */
    TCPIP_IP_FLAGS_MF = 0x8000,
    /** fragment offset */
    TCPIP_IP_FLAGS_FOFFS = 0x03FF,
    /** all flags mask */
    TCPIP_IP_FLAGS_ALL = TCPIP_IP_FLAGS_DF | TCPIP_IP_FLAGS_MF,
} tcpip_ip_flags_t;

/** ip header flags */
typedef enum tcpip_ip_protocol {
    /** don't fragment */
    TCPIP_IP_PROTOCOL_ICMP = 1,
    /** more fragments */
    TCPIP_IP_PROTOCOL_TCP = 6,
    /** more fragments */
    TCPIP_IP_PROTOCOL_UDP = 17,
} tcpip_ip_protocol_t;

/** ip packet header */
typedef struct tcpip_ip_frame_t {
    /** header length in 32-bit words + version */
    uint8_t hdr_len_version;
    
    /** type of service */
    uint8_t tos;
    
    /** total packet data (with header) */
    uint16_t total_length;
    /** identification (fragmentation) */
    uint16_t identification;

    /** fragment offset (fragmentation) */
    uint16_t flags_fragment_offset;

    /** time to live */
    uint8_t ttl;
    /** transported protocol */
    uint8_t protocol;
    /** header checksum */
    uint16_t header_checksum;
    /** source address */
    uint32_t src_addr;
    /** destination address */
    uint32_t dst_addr;
    /** payload with options (optional) */
    uint8_t pld[];
} PACKED tcpip_ip_frame_t;

/** ip pseudo header - used for checksum calculation for layers that 
 * reside on top of ip */
typedef struct tcpip_ip_pseudo_hdr {
    /** source address */
    uint32_t src_addr;
    /** destination address */
    uint32_t dst_addr;
    /** zero padding */
    uint8_t zeros;
    /** protocol */
    uint8_t protocol;
    /** payload length */
    uint16_t pld_length;
} PACKED tcpip_ip_pseudo_hdr_t;

/**
 * @brief Read the header length from the ip packet
 * 
 * @param ip ip header pointer
 *
 * @return int header length in bytes
 */
static inline ALWAYS_INLINE int TCPIPIpFrame_GetHdrLen(
    const tcpip_ip_frame_t *ip)
{
    /* read the bits from the hdr_len_version field */
    return BITS_RD(ip->hdr_len_version, TCPIP_IP_HDR_LEN_VER_HDRLEN) * 4;
}

/**
 * @brief Set the header length in the ip packet
 * 
 * @param ip ip header pointer
 * @param hdr_len headear length in bytes
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetHdrLen(tcpip_ip_frame_t *ip,
    size_t hdr_len)
{
    /* update the bits in hdr_len_version field */
    ip->hdr_len_version = BITS_WR(ip->hdr_len_version, 
        TCPIP_IP_HDR_LEN_VER_HDRLEN, hdr_len / 4);
}

/**
 * @brief Read the version from the ip packet
 * 
 * @param ip ip header pointer
 *
 * @return int version number
 */
static inline ALWAYS_INLINE int TCPIPIpFrame_GetVersion(
    const tcpip_ip_frame_t *ip)
{
    /* read the bits from the hdr_len_version field */
    return BITS_RD(ip->hdr_len_version, TCPIP_IP_HDR_LEN_VER_VER);
}

/**
 * @brief Set the version in the ip packet
 * 
 * @param ip ip header pointer
 * @param hdr_len headear length in bytes
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetVersion(tcpip_ip_frame_t *ip,
    int version)
{
    /* update the bits in hdr_len_version field */
    ip->hdr_len_version = BITS_WR(ip->hdr_len_version, 
        TCPIP_IP_HDR_LEN_VER_VER, version);
}

/**
 * @brief Returns the type of service field value
 * 
 * @param ip ip header
 * 
 * @return uint8_t type of service value 
 */
static inline ALWAYS_INLINE uint8_t TCPIPIpFrame_GetTos(
    const tcpip_ip_frame_t *ip)
{
    return ip->tos;
}

/**
 * @brief Sets the type of service field value
 * 
 * @param ip ip header
 * @param tos type of service
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetTos(tcpip_ip_frame_t *ip,
    uint8_t tos)
{
    ip->tos = tos;
}

/**
 * @brief Returns the length field value
 * 
 * @param ip ip header
 * 
 * @return uint16_t length
 */
static inline ALWAYS_INLINE uint16_t TCPIPIpFrame_GetLength(
    const tcpip_ip_frame_t *ip)
{
    return BETOH16(ip->total_length);
}

/**
 * @brief Sets the length field value
 * 
 * @param ip ip header
 * @param length length
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetLength(tcpip_ip_frame_t *ip,
    uint16_t length)
{
    ip->total_length = HTOBE16(length);
}

/**
 * @brief Returns the identification field value
 * 
 * @param ip ip header
 * 
 * @return uint16_t identification
 */
static inline ALWAYS_INLINE uint8_t TCPIPIpFrame_GetIdentification(
    const tcpip_ip_frame_t *ip)
{
    return BETOH16(ip->identification);
}

/**
 * @brief Sets the identification field value
 * 
 * @param ip ip header
 * @param identification identification
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetIdentification(
    tcpip_ip_frame_t *ip, uint16_t identification)
{
    ip->identification = HTOBE16(identification);
}

/**
 * @brief Returns the flags field value
 * 
 * @param ip ip header
 * 
 * @return tcpip_ip_flags_t flags
 */
static inline ALWAYS_INLINE tcpip_ip_flags_t TCPIPIpFrame_GetFlags(
    const tcpip_ip_frame_t *ip)
{
    return BETOH16(ip->flags_fragment_offset) & TCPIP_IP_FLAGS_ALL;
}

/**
 * @brief Sets the flags field value
 * 
 * @param ip ip header
 * @param identification identification
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetFlags(
    tcpip_ip_frame_t *ip, tcpip_ip_flags_t flags)
{
    /* read and convert to workable format */
    uint16_t flags_fragment_offset = BETOH16(ip->flags_fragment_offset);
    /* apply new flags */
    flags_fragment_offset &= ~TCPIP_IP_FLAGS_ALL;
    flags_fragment_offset |= flags;

    /* write back */
    ip->flags_fragment_offset = HTOBE16(flags_fragment_offset);
}

/**
 * @brief Returns the fragment offset field value
 * 
 * @param ip ip header
 * 
 * @return tcpip_ip_flags_t flags
 */
static inline ALWAYS_INLINE size_t TCPIPIpFrame_GetFragmentOffset(
    const tcpip_ip_frame_t *ip)
{
    return BETOH16(ip->flags_fragment_offset) & TCPIP_IP_FLAGS_FOFFS;
}

/**
 * @brief Sets the flags field value
 * 
 * @param ip ip header
 * @param identification identification
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetFragmentOffset(
    tcpip_ip_frame_t *ip, size_t offset)
{
    /* read and convert to workable format */
    uint16_t flags_fragment_offset = BETOH16(ip->flags_fragment_offset);
    /* apply new flags */
    flags_fragment_offset &= ~TCPIP_IP_FLAGS_FOFFS;
    flags_fragment_offset |= offset;

    /* write back */
    ip->flags_fragment_offset = HTOBE16(flags_fragment_offset);
}

/**
 * @brief Returns the time-to-live field value
 * 
 * @param ip ip header
 * 
 * @return uint8_t time to live
 */
static inline ALWAYS_INLINE uint8_t TCPIPIpFrame_GetTTL(
    const tcpip_ip_frame_t *ip)
{
    return ip->ttl;
}

/**
 * @brief Sets the identification field value
 * 
 * @param ip ip header
 * @param identification identification
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetTTL(tcpip_ip_frame_t *ip, 
    uint8_t ttl)
{
    ip->ttl = ttl;
}

/**
 * @brief Returns the protocolfield value
 * 
 * @param ip ip header
 * 
 * @return tcpip_ip_protocol_t protocol
 */
static inline ALWAYS_INLINE tcpip_ip_protocol_t TCPIPIpFrame_GetProtocol(
    const tcpip_ip_frame_t *ip)
{
    return ip->protocol;
}

/**
 * @brief Sets the identification field value
 * 
 * @param ip ip header
 * @param identification identification
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetProtocol(tcpip_ip_frame_t *ip, 
    tcpip_ip_protocol_t protocol)
{
    ip->protocol = protocol;
}

/**
 * @brief Returns the checksum field value
 * 
 * @param ip ip header
 * 
 * @return uint16_t checksum
 */
static inline ALWAYS_INLINE uint8_t TCPIPIpFrame_GetChecksum(
    const tcpip_ip_frame_t *ip)
{
    return BETOH16(ip->header_checksum);
}

/**
 * @brief Sets the checksum field value
 * 
 * @param ip ip header
 * @param checksum checksum
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetChecksum(
    tcpip_ip_frame_t *ip, uint16_t checksum)
{
    ip->header_checksum = HTOBE16(checksum);
}

/**
 * @brief Returns the source address field value
 * 
 * @param ip ip header
 * 
 * @return uint16_t checksum
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t TCPIPIpFrame_GetSrcAddr(
    const tcpip_ip_frame_t *ip)
{
    return (tcpip_ip_addr_t){ .u32 = BETOH32(ip->src_addr) };
}

/**
 * @brief Sets the source address field value
 * 
 * @param ip ip header
 * @param src_addr source address
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetSrcAddr(
    tcpip_ip_frame_t *ip, tcpip_ip_addr_t src_addr)
{
    ip->src_addr = HTOBE32(src_addr.u32);
}

/**
 * @brief Returns the destination address field value
 * 
 * @param ip ip header
 * 
 * @return tcpip_ip_addr_t destination address
 */
static inline ALWAYS_INLINE tcpip_ip_addr_t TCPIPIpFrame_GetDstAddr(
    const tcpip_ip_frame_t *ip)
{
    return (tcpip_ip_addr_t){ .u32 = BETOH32(ip->dst_addr) };
}

/**
 * @brief Sets the destination address field value
 * 
 * @param ip ip header
 * @param dst_addr destination address
 */
static inline ALWAYS_INLINE void TCPIPIpFrame_SetDstAddr(
    tcpip_ip_frame_t *ip, tcpip_ip_addr_t dst_addr)
{
    ip->dst_addr = HTOBE32(dst_addr.u32);
}

/**
 * @brief Gets the pseudoheader from the true ip header
 * 
 * @param ip ip header
 * 
 * * @return tcpip_ip_pseudo_hdr_t pseudo header
 */
static inline ALWAYS_INLINE tcpip_ip_pseudo_hdr_t TCPIPIpFrame_GetPseudoHdr(
    const tcpip_ip_frame_t *ip)
{
    /* compute payload length based on total length of the ip packet minus the 
     * size of the header */
    uint16_t pld_len = TCPIPIpFrame_GetLength(ip) - TCPIPIpFrame_GetHdrLen(ip);
    /* construct pseudo header */
    return (tcpip_ip_pseudo_hdr_t) {
        .src_addr = ip->src_addr,
        .dst_addr = ip->dst_addr,
        .zeros = 0,
        .protocol = ip->protocol,
        .pld_length = HTOBE16(pld_len)
    };
}

#endif /* NET_TCPIP_IP_FRAME */
