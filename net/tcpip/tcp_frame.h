/**
 * @file tcp_frame.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-31
 * 
 * @brief TCP/IP Stack: Transmission Control Protocol
 */

#ifndef NET_TCPIP_TCP_FRAME
#define NET_TCPIP_TCP_FRAME

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "net/tcpip/ip_frame.h"
#include "util/bit.h"
#include "util/endian.h"

/** port typedef */
typedef uint16_t tcpip_tcp_port_t;


/** tcp flags & data offset bits */
typedef enum tcpip_tcp_flags_doffs {
    /** all flags mask */
    TCPIP_TCP_FLAGS_DOFFS_FLAGS     = 0x03FF,
    /** data offset bits */
    TCPIP_TCP_FLAGS_DOFFS           = 0xF000,
} tcpip_tcp_flags_doffs_t;

/** tcp flags */
typedef enum tcpip_tcp_flags {
    /** last packet from sender */
    TCPIP_TCP_FLAGS_FIN             = 0x0001,
    /** Synchronize sequence numbers. Only the first packet 
     * sent from each end should have this flag set. */
    TCPIP_TCP_FLAGS_SYN             = 0x0002,
    /** Reset the connection */
    TCPIP_TCP_FLAGS_RST             = 0x0004,
    /** Push function. Asks to push the buffered data to the receiving 
     * application */
    TCPIP_TCP_FLAGS_PSH             = 0x0008,
    /** Indicates that the Acknowledgment field is significant. All packets 
     * after the initial SYN packet sent by the client should have this flag 
     * set */
    TCPIP_TCP_FLAGS_ACK             = 0x0010,
    /** Indicates that the Urgent pointer field is significant */
    TCPIP_TCP_FLAGS_URG             = 0x0020,
    /** Explicit congestion notification */
    TCPIP_TCP_FLAGS_ECE             = 0x0040,
    /** Congestion window reduced */
    TCPIP_TCP_FLAGS_CWR             = 0x0080,
    /** ECN-nonce - concealment protection */
    TCPIP_TCP_FLAGS_NS              = 0x0100,
} tcpip_tcp_flags_t;

/** tcp frame header */
typedef struct tcpip_tcp_frame_t {
    /** source port */
    uint16_t src_port;
    /** destination port */
    uint16_t dst_port;
    /** sequence number */
    uint32_t seq;
    /** acknowledgment number (if ACK is set) */
    uint32_t ack;

    /** flags and data offset */
    uint16_t flags_data_offs;
    /** window size */
    uint16_t win;

    /** flags and data offset */
    uint16_t checksum;
    /** urgent pointer */
    uint16_t urgent_pointer;

    /** payload/options */
    uint8_t pld[];
} PACKED tcpip_tcp_frame_t;

/**
 * @brief Returns the source port field value
 * 
 * @param tcp tcp frame pointer
 *  
 * @return source port
 */
static inline ALWAYS_INLINE tcpip_tcp_port_t TCPIPTcpFrame_GetSrcPort(
    const tcpip_tcp_frame_t *tcp)
{
    return BETOH16(tcp->src_port);
}

/**
 * @brief Sets the source port in the tcp frame
 * 
 * @param tcp tcp frame pointer
 * @param src_port source port
 */
static inline ALWAYS_INLINE void TCPIPTcpFrame_SetSrcPort(
    tcpip_tcp_frame_t *tcp, tcpip_tcp_port_t src_port)
{
    tcp->src_port = HTOBE16(src_port);
}

/**
 * @brief Returns the destination port field value
 * 
 * @param tcp tcp frame pointer
 *  
 * @return destination port
 */
static inline ALWAYS_INLINE tcpip_tcp_port_t TCPIPTcpFrame_GetDstPort(
    const tcpip_tcp_frame_t *tcp)
{
    return BETOH16(tcp->dst_port);
}

/**
 * @brief Sets the destination port in the tcp frame
 * 
 * @param tcp tcp frame pointer
 * @param src_port source port
 */
static inline ALWAYS_INLINE void TCPIPTcpFrame_SetDstPort(
    tcpip_tcp_frame_t *tcp, tcpip_tcp_port_t dst_port)
{
    tcp->dst_port = HTOBE16(dst_port);
}

/**
 * @brief Returns the sequence number field value
 * 
 * @param tcp tcp frame pointer
 *  
 * @return uint32_t sequence number
 */
static inline ALWAYS_INLINE uint32_t TCPIPTcpFrame_GetSeq(
    const tcpip_tcp_frame_t *tcp)
{
    return BETOH32(tcp->seq);
}

/**
 * @brief Sets the sequence number in the tcp frame
 * 
 * @param tcp tcp frame pointer
 * @param seq source port
 */
static inline ALWAYS_INLINE void TCPIPTcpFrame_SetSeq(tcpip_tcp_frame_t *tcp, 
    uint32_t seq)
{
    tcp->seq = HTOBE32(seq);
}

/**
 * @brief Returns the ack number field value
 * 
 * @param tcp tcp frame pointer
 *  
 * @return uint32_t ack number
 */
static inline ALWAYS_INLINE uint32_t TCPIPTcpFrame_GetAck(
    const tcpip_tcp_frame_t *tcp)
{
    return BETOH32(tcp->ack);
}

/**
 * @brief Sets the acknoledgement number in the tcp frame
 * 
 * @param tcp tcp frame pointer
 * @param ack acknoledgement port
 */
static inline ALWAYS_INLINE void TCPIPTcpFrame_SetAck(tcpip_tcp_frame_t *tcp, 
    uint32_t ack)
{
    tcp->ack = HTOBE32(ack);
}

/**
 * @brief Read data offset from the tcp header
 * 
 * @param tcp tcp header pointer
 * 
 * @return data offset in bytes 
 */
static inline ALWAYS_INLINE size_t TCPIPTcpFrame_GetDataOffs(
    const tcpip_tcp_frame_t *tcp)
{
    /* data offset is given in 4-byte units */
    return BITS_RD(BETOH16(tcp->flags_data_offs), TCPIP_TCP_FLAGS_DOFFS) * 4;
}

/**
 * @brief Set data offset within tcp header
 * 
 * @param tcp tcp header
 */
static inline ALWAYS_INLINE void TCPIPTcpFrame_SetDataOffs(
    tcpip_tcp_frame_t *tcp, size_t doffs)
{
    /* translate to our endianess */
    uint16_t fdo = BETOH16(tcp->flags_data_offs);
    /* write bits and translate back to network order */
    tcp->flags_data_offs = HTOBE16(BITS_WR(fdo, TCPIP_TCP_FLAGS_DOFFS, 
        doffs / 4));
}

/**
 * @brief Read the flag field from the tcp header
 * 
 * @param tcp tcp header pointer
 *
 * @return tcpip_tcp_flags_doffs_t flags
 */
static inline ALWAYS_INLINE tcpip_tcp_flags_t TCPIPTcpFrame_GetFlags(
    const tcpip_tcp_frame_t *tcp)
{
    /* read the flags */
    return BITS_RD(BETOH16(tcp->flags_data_offs), TCPIP_TCP_FLAGS_DOFFS_FLAGS);
}

/**
 * @brief Set the flags within tcp header
 * 
 * @param tcp tcp header pointer
 * @param flags flags to be set
 */
static inline ALWAYS_INLINE void TCPIPTcpFrame_SetFlags(
    tcpip_tcp_frame_t *tcp, tcpip_tcp_flags_t flags)
{
    /* translate to our endianess */
    uint16_t fdo = BETOH16(tcp->flags_data_offs);
    /* update flags value */
    tcp->flags_data_offs = HTOBE16(BITS_WR(fdo, TCPIP_TCP_FLAGS_DOFFS_FLAGS, 
        flags));
}

/**
 * @brief Returns the window size field value
 * 
 * @param tcp tcp frame pointer
 *  
 * @return window size
 */
static inline ALWAYS_INLINE size_t TCPIPTcpFrame_GetWindow(
    const tcpip_tcp_frame_t *tcp)
{
    return BETOH16(tcp->win);
}

/**
 * @brief Sets the window size in the tcp frame
 * 
 * @param tcp tcp frame pointer
 * @param win window size
 */
static inline ALWAYS_INLINE void TCPIPTcpFrame_SetWindow(
    tcpip_tcp_frame_t *tcp, size_t win)
{
    tcp->win = HTOBE16(win);
}

/**
 * @brief Returns the checksum field value
 * 
 * @param tcp tcp frame pointer
 *  
 * @return checksum
 */
static inline ALWAYS_INLINE uint16_t TCPIPTcpFrame_GetChecksum(
    const tcpip_tcp_frame_t *tcp)
{
    return BETOH16(tcp->checksum);
}

/**
 * @brief Sets the checksum value in the tcp frame
 * 
 * @param tcp tcp frame pointer
 * @param checksum checksum
 */
static inline ALWAYS_INLINE void TCPIPTcpFrame_SetChecksum(
    tcpip_tcp_frame_t *tcp, uint16_t checksum)
{
    tcp->checksum = HTOBE16(checksum);
}

/**
 * @brief Returns the urgent pointer field value
 * 
 * @param tcp tcp frame pointer
 *  
 * @return urgent pointer
 */
static inline ALWAYS_INLINE uint16_t TCPIPTcpFrame_GetUrgentPtr(
    const tcpip_tcp_frame_t *tcp)
{
    return BETOH16(tcp->urgent_pointer);
}

/**
 * @brief Sets the urgent pointer value in the tcp frame
 * 
 * @param tcp tcp frame pointer
 * @param urgent_pointer urgent pointer
 */
static inline ALWAYS_INLINE void TCPIPTcpFrame_SetUrgentPtr(
    tcpip_tcp_frame_t *tcp, uint16_t urgent_pointer)
{
    tcp->urgent_pointer = HTOBE16(urgent_pointer);
}

#endif /* NET_TCPIP_TCP_FRAME */
