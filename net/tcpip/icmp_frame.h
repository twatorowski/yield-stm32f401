/**
 * @file icmp_frame.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-30
 * 
 * @brief TCP/IP Stack: Internet Control Message Protocol
 */

#ifndef NET_TCPIP_ICMP_FRAME
#define NET_TCPIP_ICMP_FRAME

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "net/tcpip/icmp_frame.h"
#include "util/endian.h"

/** icmp control messages types  */
typedef enum tcpip_icmp_type {
    /** echo reply */
    TCPIP_ICMP_TYPE_ECHO_REPLY = 0,
    /** echo request */
    TCPIP_ICMP_TYPE_ECHO_REQUEST = 8,
} tcpip_icmp_type_t;

/** echo request/response payload */
typedef struct tcpip_icmp_pld_echo_req_res {
    /** identification number */
    uint16_t id;
    /** sequence number */
    uint16_t seqno;
    /** echo data */
    uint8_t data[];
} PACKED tcpip_icmp_pld_echo_req_res_t;

/** internet control message frame */
typedef struct tcpip_icmp_frame {
    /** control message type */
    uint8_t type;
    /** control message code, a.k.a sub-type */
    uint8_t code;
    /** checksum as in RFC 1071 */
    uint16_t checksum;
    /* raw format */
    uint8_t pld[];
} PACKED tcpip_icmp_frame_t;

/**
 * @brief Returns the type field value
 * 
 * @param ip ip header
 * 
 * @return uint8_t type of service value 
 */
static inline ALWAYS_INLINE tcpip_icmp_type_t TCPIPIcmpFrame_GetType(
    const tcpip_icmp_frame_t *icmp)
{
    return icmp->type;
}

/**
 * @brief Sets the type field value
 * 
 * @param ip ip header
 * @param type type value
 * 
 * @return uint8_t type of service value 
 */
static inline ALWAYS_INLINE void TCPIPIcmpFrame_SetType(
    tcpip_icmp_frame_t *icmp, tcpip_icmp_type_t type)
{
    icmp->type = type;
}

/**
 * @brief Returns the code field value
 * 
 * @param ip ip header
 * 
 * @return uint8_t type of service value 
 */
static inline ALWAYS_INLINE uint8_t TCPIPIcmpFrame_GetCode(
    const tcpip_icmp_frame_t *icmp)
{
    return icmp->code;
}

/**
 * @brief Sets the code field value
 * 
 * @param ip ip header
 * @param code code value
 */
static inline ALWAYS_INLINE void TCPIPIcmpFrame_SetCode(
    tcpip_icmp_frame_t *icmp, uint8_t code)
{
    icmp->code = code;
}

/**
 * @brief Returns the checksum field value
 * 
 * @param icmp header
 * 
 * @return uint16_t checksum value 
 */
static inline ALWAYS_INLINE uint16_t TCPIPIcmpFrame_GetChecksum(
    const tcpip_icmp_frame_t *icmp)
{
    return BETOH16(icmp->checksum);
}

/**
 * @brief Sets the code field value
 * 
 * @param icmp header
 * @param checksum checksum field value
 */
static inline ALWAYS_INLINE void TCPIPIcmpFrame_SetChecksum(
    tcpip_icmp_frame_t *icmp, uint16_t checksum)
{
    icmp->checksum = HTOBE16(checksum);
}

/**
 * @brief Sets the id of the echo request/response
 *
 * @param echo echop req/res payload pointer
 * @param id value of the id
 */
static inline ALWAYS_INLINE void TCPIPIcmpFrameEcho_SetID(
    tcpip_icmp_pld_echo_req_res_t *echo, uint16_t id)
{
    echo->id = HTOBE16(id);
}

/**
 * @brief Returns the id field value
 *
 * @param icmp header
 *
 * @return uint16_t id value
 */
static inline ALWAYS_INLINE uint16_t TCPIPIcmpFrameEcho_GetID(
    const tcpip_icmp_pld_echo_req_res_t *echo)
{
    return BETOH16(echo->id);
}

/**
 * @brief Sets the sequential number of the echo request/response
 *
 * @param echo echop req/res payload pointer
 * @param seqno sequential number
 */
static inline ALWAYS_INLINE void TCPIPIcmpFrameEcho_SetSeqNo(
    tcpip_icmp_pld_echo_req_res_t *echo, uint16_t seqno)
{
    echo->seqno = HTOBE16(seqno);
}

/**
 * @brief Returns the seqno field value
 *
 * @param icmp header
 *
 * @return uint16_t seqno value
 */
static inline ALWAYS_INLINE uint16_t TCPIPIcmpFrameEcho_GetSeqNo(
    const tcpip_icmp_pld_echo_req_res_t *echo)
{
    return BETOH16(echo->seqno);
}

#endif /* NET_TCPIP_ICMP_FRAME */
