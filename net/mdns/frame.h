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

#include "compiler.h"
#include "err.h"
#include "net/tcpip/ip_addr.h"
#include "util/endian.h"

/* flags for the frame header */
#define MDNS_FRAME_HDR_FLAGS_QR                             0x8000
#define MDNS_FRAME_HDR_FLAGS_QR_QUERY                       0x0000
#define MDNS_FRAME_HDR_FLAGS_QR_RESP                        0x8000
#define MDNS_FRAME_HDR_FLAGS_OPCODE                         0x7800
#define MDNS_FRAME_HDR_FLAGS_OPCODE_STD_QUERY               0x0000
#define MDNS_FRAME_HDR_FLAGS_AA                             0x0400
#define MDNS_FRAME_HDR_FLAGS_TC                             0x0200
#define MDNS_FRAME_HDR_FLAGS_RD                             0x0100
#define MDNS_FRAME_HDR_FLAGS_RA                             0x0080
#define MDNS_FRAME_HDR_FLAGS_Z                              0x0070
#define MDNS_FRAME_HDR_FLAGS_RCODE                          0x000f
#define MDNS_FRAME_HDR_FLAGS_RCODE_OK                       0x0000
#define MDNS_FRAME_HDR_FLAGS_RCODE_FORMAT_ERROR             0x0001
#define MDNS_FRAME_HDR_FLAGS_RCODE_SERVER_FAIL              0x0002
#define MDNS_FRAME_HDR_FLAGS_RCODE_NAME_ERROR               0x0003
#define MDNS_FRAME_HDR_FLAGS_RCODE_NOT_IMPL                 0x0004
#define MDNS_FRAME_HDR_FLAGS_RCODE_REFUSED                  0x0005


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
} PACKED mdns_frame_t;

/* type field values */
#define MDNS_RECORD_TYPE_A                                0x0001
#define MDNS_RECORD_TYPE_CNAME                            0x0005
#define MDNS_RECORD_TYPE_MX                               0x0002
/* class field values */
#define MDNS_RECORD_CLASS_IA                              0x0001

/* constant size fields that follow the qname in question record */
typedef struct mdns_question_fields {
    /* type of the querry */
    uint16_t type;
    /* class of the querry */
    uint16_t class;
} PACKED mdns_question_fields_t;


/**
 * @brief Function that will extract the domain name from dns packet to a
 * standard form (like www.something.com). Supportrs both plain and compressed
 * names
 *
 * @param src pointer from which we start decoding
 * @param frame pointer to the mdns frame
 * @param frame_size size of the whole frame
 * @param dst destination buffer pointer
 * @param dst_size size of the destination buffer (must accomodate trailing 0)
 *
 * @return err_t length of the string or EFATAL if src data is malformed or
 * EARGVAL if the output buffer was too short
 */
err_t MDNSFrame_DecodeName(const uint8_t *src, const mdns_frame_t *frame,
    size_t frame_size, char *dst, size_t dst_size);


/**
 * @brief encode domain name in mdns format
 *
 * @param name domain name to be encoded (eg. www.example.com)
 * @param dst destination buffer
 * @param dst_size size of the destination buffer
 *
 * @return err_t size of the data encoded or error code
 */
err_t MDNSFrame_EncodeName(const char *name, uint8_t *dst, size_t dst_size);


/**
 * @brief Returns the value of the Transaction ID
 *
 * @param  pointer to the frame
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
 * @param  pointer to the frame
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
 * @param  pointer to the frame
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
 * @param  pointer to the frame
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
 */
static inline void MDNSFrame_SetTransactionID(mdns_frame_t *mdns,
    uint16_t transaction_id)
{
    /* return the transaction id */
    mdns->transaction_id = HTOBE16(transaction_id);
}

/**
 * @brief Returns the value of the flags
 *
 * @param mdns pointer to the frame
 */
static inline void MDNSFrame_SetFlags(mdns_frame_t *mdns,
    uint16_t flags)
{
    /* return the flags */
    mdns->flags = HTOBE16(flags);
}

/**
 * @brief Returns the value of the questions count
 *
 * @param mdns pointer to the frame
 */
static inline void MDNSFrame_SetQuestionsCount(mdns_frame_t *mdns,
    uint16_t questions_count)
{
    /* return the questions count */
    mdns->questions_count = HTOBE16(questions_count);
}

/**
 * @brief Returns the value of the answers count
 *
 * @param mdns pointer to the frame
 */
static inline void MDNSFrame_SetAnswersCount(mdns_frame_t *mdns,
    uint16_t answers_count)
{
    /* return the answers count */
    mdns->answers_count = HTOBE16(answers_count);
}

/**
 * @brief Returns the value of the authority resource records count
 *
 * @param mdns pointer to the frame
 */
static inline void MDNSFrame_SetAuthorityRRCount(mdns_frame_t *mdns,
    uint16_t authority_rr_count)
{
    /* return the authority resource records count */
    mdns->authority_rr_count = HTOBE16(authority_rr_count);
}

/**
 * @brief Returns the value of the additional resource records count
 *
 * @param mdns pointer to the frame
 */
static inline void MDNSFrame_SetAdditionalRRCount(mdns_frame_t *mdns,
    uint16_t additional_rr_count)
{
    /* return the additional resource records count */
    mdns->additional_rr_count = HTOBE16(additional_rr_count);
}

/**
 * @brief Returns the value of type
 *
 * @param q pointer to the question record
 * @return uint16_t type value
 */
static inline uint16_t MDNSFrameQuestion_GetType(const mdns_question_fields_t *q)
{
    /* return the transaction id */
    return BETOH16(q->type);
}

/**
 * @brief Returns the value of class
 *
 * @param q pointer to the question record
 * @return uint16_t type value
 */
static inline uint16_t MDNSFrameQuestion_GetClass(const mdns_question_fields_t *q)
{
    /* return the class of the question */
    return BETOH16(q->class);
}

/**
 * @brief Sets the type value
 *
 * @param q question record fields pointer
 * @param type value to be se
 */
static inline void MDNSFrameQuestion_SetType(mdns_question_fields_t *q,
    uint16_t type)
{
    /* set the value */
    q->type = HTOBE16(type);
}

/**
 * @brief Sets the class value
 *
 * @param q question record fields pointer
 * @param class value to be se
 */
static inline void MDNSFrameQuestion_SetClass(mdns_question_fields_t *q,
    uint16_t class)
{
    /* set the value */
    q->class = HTOBE16(class);
}


/* constant size fields that follow the qname in answer record */
typedef struct mdns_answer_fields {
    /* type of the data*/
    uint16_t type;
    /* class of the rdata */
    uint16_t class;
    /* time to live in seconds */
    uint32_t ttl;
    /* length of the record data */
    uint16_t rdlength;
    /* record data */
    uint8_t rdata[];
} PACKED mdns_answer_fields_t;


/**
 * @brief Returns the value of type
 *
 * @param a pointer to the answer record
 * @return uint16_t type value
 */
static inline uint16_t MDNSFrameAnswer_GetType(const mdns_answer_fields_t *a)
{
    /* return the transaction id */
    return BETOH16(a->type);
}

/**
 * @brief Returns the value of class
 *
 * @param a pointer to the answer record
 * @return uint16_t class value
 */
static inline uint16_t MDNSFrameAnswer_GetClass(const mdns_answer_fields_t *a)
{
    /* return the class of the Answer */
    return BETOH16(a->class);
}

/**
 * @brief Returns the value of class
 *
 * @param a pointer to the answer record
 * @return uint32_t ttl value
 */
static inline uint32_t MDNSFrameAnswer_GetTTL(const mdns_answer_fields_t *a)
{
    /* return the ttl of the Answer */
    return BETOH32(a->ttl);
}

/**
 * @brief Returns the value of rdlength
 *
 * @param a pointer to the answer record
 * @return uint32_t rdlength value
 */
static inline uint16_t MDNSFrameAnswer_GetRDLength(const mdns_answer_fields_t *a)
{
    /* return the rdlength of the Answer */
    return BETOH16(a->rdlength);
}

/**
 * @brief Sets the type value
 *
 * @param a answer record fields pointer
 * @param type value to be se
 */
static inline void MDNSFrameAnswer_SetType(mdns_answer_fields_t *a,
    uint16_t type)
{
    /* set the value */
    a->type = HTOBE16(type);
}

/**
 * @brief Sets the class value
 *
 * @param a answer record fields pointer
 * @param class value to be se
 */
static inline void MDNSFrameAnswer_SetClass(mdns_answer_fields_t *a,
    uint16_t class)
{
    /* set the value */
    a->class = HTOBE16(class);
}

/**
 * @brief Sets the ttl value
 *
 * @param a answer record fields pointer
 * @param ttl value to be set
 */
static inline void MDNSFrameAnswer_SetTTL(mdns_answer_fields_t *a,
    uint32_t ttl)
{
    /* set the value */
    a->ttl = HTOBE32(ttl);
}

/**
 * @brief Sets the length value
 *
 * @param a answer record fields pointer
 * @param length value to be set
 */
static inline void MDNSFrameAnswer_SetRDLength(mdns_answer_fields_t *a,
    uint16_t length)
{
    /* set the value */
    a->rdlength = HTOBE16(length);
}


/* rdata field for the A type record */
typedef struct mdns_answer_rdata_a {
    /* ip address */
    uint32_t ip;
} PACKED mdns_answer_rdata_a_t;


/**
 * @brief Sets the ip value
 *
 * @param a rdata field pointer
 * @param ip value to be set
 */
static inline void MDNSFrameRDataA_SetIP(mdns_answer_rdata_a_t *a,
    tcpip_ip_addr_t ip)
{
    /* set the value */
    a->ip = HTOBE32(ip.u32);
}

/**
 * @brief Returns the value of ip address
 *
 * @param a pointer to the answer record
 * @return uint32_t rdlength value
 */
static inline tcpip_ip_addr_t MDNSFrameRDataA_GetIP(const mdns_answer_rdata_a_t *a)
{
    /* return the rdlength of the Answer */
    return (tcpip_ip_addr_t) { .u32 = BETOH32(a->ip) };
}


#endif /* NET_MDNS_FRAME_H */
