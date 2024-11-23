/**
 * @file frame.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-23
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef NET_MDNS_FRAME_H
#define NET_MDNS_FRAME_H

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "util/endian.h"

/* flags for the frame header */
#define MDNS_FRAME_HDR_FLAGS_QR                             0x0001
#define MDNS_FRAME_HDR_FLAGS_OPCODE                         0x001e
#define MDNS_FRAME_HDR_FLAGS_OPCODE_STD_QUERY               0x0000
#define MDNS_FRAME_HDR_FLAGS_AA                             0x0020
#define MDNS_FRAME_HDR_FLAGS_TC                             0x0040
#define MDNS_FRAME_HDR_FLAGS_RD                             0x0080
#define MDNS_FRAME_HDR_FLAGS_RA                             0x0100
#define MDNS_FRAME_HDR_FLAGS_Z                              0x0e00
#define MDNS_FRAME_HDR_FLAGS_RCODE                          0xf000
#define MDNS_FRAME_HDR_FLAGS_RCODE_OK                       0x0000
#define MDNS_FRAME_HDR_FLAGS_RCODE_FORMAT_ERROR             0x1000
#define MDNS_FRAME_HDR_FLAGS_RCODE_SERVER_FAIL              0x2000
#define MDNS_FRAME_HDR_FLAGS_RCODE_NAME_ERROR               0x3000
#define MDNS_FRAME_HDR_FLAGS_RCODE_NOT_IMPL                 0x4000
#define MDNS_FRAME_HDR_FLAGS_RCODE_REFUSED                  0x5000


/* mdns frame header structure */
typedef struct mdns_frame {
    /* id of the transaction */
    uint16_t transaction_id;
    /* flags as enumerated above*/
    uint16_t flags;
    /* number of questions that follow the header */
    uint16_t questions_count;
    /* number of aswers that follow the question section */
    uint16_t answers_count;
    /* number of authority resource records */
    uint16_t authority_rr_count;
    /* number of additional records */
    uint16_t additional_rr_count;

    /* query/response payload that follows the header */
    uint8_t pld[];
} mdns_frame_t;



/**
 * @brief Function that will extract the domain name from dns packet to a
 * standard form (like www.something.com). Supportrs both plain and compressed
 * names
 *
 * @param src pointer from which we start decoding
 * @param offset offset of the 'src' within the whole frame
 * @param size size of the whole frame
 * @param dst destination buffer pointer
 * @param dst_size size of the destination buffer (must accomodate trailing 0)
 *
 * @return err_t length of the string or EFATAL if src data is malformed or
 * EARGVAL if the output buffer was too short
 */
err_t MDNSFrame_DecodeName(const uint8_t *src, size_t offset, size_t size,
    uint8_t *dst, size_t dst_size);


/**
 * @brief Returns the value of the Transaction ID
 *
 * @param mdns pointer to the frame
 * @return uint16_t transaction id value
 */
static inline uint16_t MDNSFrame_GetTransactionID(const mdns_frame_t *mdns)
{
    /* return the transaction id */
    return BETOH16(mdns->transaction_id);
}

/**
 * @brief Returns the value of the flags
 *
 * @param mdns pointer to the frame
 * @return uint16_t flags value
 */
static inline uint16_t MDNSFrame_GetFlags(const mdns_frame_t *mdns)
{
    /* return the flags */
    return BETOH16(mdns->flags);
}

/**
 * @brief Returns the value of the questions count
 *
 * @param mdns pointer to the frame
 * @return uint16_t questions count value
 */
static inline uint16_t MDNSFrame_GetQuestionsCount(const mdns_frame_t *mdns)
{
    /* return the questions count */
    return BETOH16(mdns->questions_count);
}

/**
 * @brief Returns the value of the answers count
 *
 * @param mdns pointer to the frame
 * @return uint16_t answers count value
 */
static inline uint16_t MDNSFrame_GetAnswersCount(const mdns_frame_t *mdns)
{
    /* return the answers count */
    return BETOH16(mdns->answers_count);
}

/**
 * @brief Returns the value of the authority resource records count
 *
 * @param mdns pointer to the frame
 * @return uint16_t authority resource records count value
 */
static inline uint16_t MDNSFrame_GetAuthorityRRCount(const mdns_frame_t *mdns)
{
    /* return the authority resource records count */
    return BETOH16(mdns->authority_rr_count);
}

/**
 * @brief Returns the value of the additional resource records count
 *
 * @param mdns pointer to the frame
 * @return uint16_t additional resource records count value
 */
static inline uint16_t MDNSFrame_GetAdditionalRRCount(const mdns_frame_t *mdns)
{
    /* return the additional resource records count */
    return BETOH16(mdns->additional_rr_count);
}

/**
 * @brief Returns the value of the Transaction ID
 *
 * @param mdns pointer to the frame
 * @return uint16_t transaction id value
 */
static inline uint16_t MDNSFrame_SetTransactionID(mdns_frame_t *mdns,
    uint16_t transaction_id)
{
    /* return the transaction id */
    return mdns->transaction_id = HTOBE16(transaction_id);
}

/**
 * @brief Returns the value of the flags
 *
 * @param mdns pointer to the frame
 * @return uint16_t flags value
 */
static inline uint16_t MDNSFrame_SetFlags(mdns_frame_t *mdns,
    uint16_t flags)
{
    /* return the flags */
    return mdns->flags = HTOBE16(flags);
}

/**
 * @brief Returns the value of the questions count
 *
 * @param mdns pointer to the frame
 * @return uint16_t questions count value
 */
static inline uint16_t MDNSFrame_SetQuestionsCount(mdns_frame_t *mdns,
    uint16_t questions_count)
{
    /* return the questions count */
    return mdns->questions_count = HTOBE16(questions_count);
}

/**
 * @brief Returns the value of the answers count
 *
 * @param mdns pointer to the frame
 * @return uint16_t answers count value
 */
static inline uint16_t MDNSFrame_SetAnswersCount(mdns_frame_t *mdns,
    uint16_t answers_count)
{
    /* return the answers count */
    return mdns->answers_count = HTOBE16(answers_count);
}

/**
 * @brief Returns the value of the authority resource records count
 *
 * @param mdns pointer to the frame
 * @return uint16_t authority resource records count value
 */
static inline uint16_t MDNSFrame_SetAuthorityRRCount(mdns_frame_t *mdns,
    uint16_t authority_rr_count)
{
    /* return the authority resource records count */
    return mdns->authority_rr_count = HTOBE16(authority_rr_count);
}

/**
 * @brief Returns the value of the additional resource records count
 *
 * @param mdns pointer to the frame
 * @return uint16_t additional resource records count value
 */
static inline uint16_t MDNSFrame_SetAdditionalRRCount(mdns_frame_t *mdns,
    uint16_t additional_rr_count)
{
    /* return the additional resource records count */
    return mdns->additional_rr_count = HTOBE16(additional_rr_count);
}



#endif /* NET_MDNS_FRAME_H */
