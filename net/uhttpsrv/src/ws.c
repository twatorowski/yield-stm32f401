/**
 * @file ws.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-11
 * 
 * @copyright Copyright (c) 2025
 */

#include "assert.h"
#include "config.h"
#include "err.h"

#include "net/uhttpsrv/uhttpsrv.h"
#include "net/uhttpsrv/ws.h"
#include "dev/seed.h"
#include "util/base64.h"
#include "util/sha1.h"
#include "util/minmax.h"
#include "util/string.h"
#include "sys/yield.h"


/* header fields */
#define WS_HDR_FIN                              (0x8000)
#define WS_HDR_OPCODE                           (0x0f00)
#define WS_HDR_OPCODE_CONT                      (0x0000)
#define WS_HDR_OPCODE_TEXT                      (0x0100)
#define WS_HDR_OPCODE_BIN                       (0x0200)
#define WS_HDR_OPCODE_CLOSE                     (0x0800)
#define WS_HDR_OPCODE_PING                      (0x0900)
#define WS_HDR_OPCODE_PONG                      (0x0A00)
#define WS_HDR_MASKED                           (0x0080)
#define WS_HDR_PLD_LEN                          (0x007f)


/* websocket header structure */
typedef struct websocket_hdr {
    /* header field */
    uint16_t hdr;
    /* different variants that depend on the size encoding */
    union {
        /* variant for the size encoded in the header */
        uint8_t mask[4];
        /* variant for the 16-bit long size field */
        struct { uint16_t size; uint8_t mask[4]; } s16;
        /* variant for the 64-bit long size field */
        struct { uint64_t size; uint8_t mask[4]; } s64;
    };
} PACKED websocket_hdr_t;

/* magic uuid to be concatenated with client nonce */
static const char magic[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

/* receive websocket header */
static err_t UHTTPSrvWS_RecvHeader(struct uhttp_request *req, uint16_t *hdr,
    size_t *size, dtime_t timeout)
{
    /* data placeholders */
    err_t ec; uint8_t *mask_ptr;
    /* payload placeholder */
    websocket_hdr_t pld; size_t decoded_size, bytes_to_get = 0;
    /* underlying websocket */
    uhttp_ws_t *ws = &req->ws;

    /* receive the header */
    if ((ec = req->instance->sock_funcs.recv(req->sock, &pld.hdr, sizeof(pld.hdr),
        timeout)) < EOK)
        return ec;
    /* undo the endiannes */
    *hdr = BETOH16(pld.hdr);

    /* we shall never receive data that is not masked since we are the
     * server side */
    if (!(*hdr & WS_HDR_MASKED))
        return EFATAL;

    /* need to get the mask value from the wire */
    bytes_to_get += 4; mask_ptr = pld.mask;

    /* extract the size value from the header field */
    decoded_size = (*hdr & WS_HDR_PLD_LEN) >> LSB(WS_HDR_PLD_LEN);
    /* size */
    if (decoded_size < 126) {
        *size = decoded_size;
    /* 16-bit size */
    } else if (decoded_size == 126) {
        bytes_to_get += 2;
    /* 64-byte size */
    } else {
        bytes_to_get += 4;
    }

    /* get the rest of the payload */
    if ((ec = req->instance->sock_funcs.recv(req->sock, pld.mask,
            bytes_to_get, 0)) < EOK)
        return ec;

    /* size specified as 16-bit number */
    if (decoded_size == 126) {
        *size = BETOH16(pld.s16.size); mask_ptr = pld.s16.mask;
    /* size specified as 64 bit number */
    } else if (decoded_size == 127) {
        *size = BETOH64(pld.s64.size); mask_ptr = pld.s64.mask;
    }

    /* copy the masking field */
    memcpy(ws->mask.u8, mask_ptr, 4);

    /* header is parsed */
    return EOK;
}

/* send the header */
static err_t UHTTPSrvWS_SendHeader(struct uhttp_request *req, uint16_t hdr,
    size_t size)
{
    /* total header size */
    size_t pld_size = 2;
    /* payload placeholder */
    websocket_hdr_t pld;

    /* set the fin field */
    hdr |= WS_HDR_FIN;
    /* encode size in the header field  */
    if (size < 126) {
        hdr |= size << LSB(WS_HDR_PLD_LEN);
    /* 16-bit size field */
    } else if (size < 0xffff) {
        pld.s16.size = HTOBE16(size); hdr |= 126 << LSB(WS_HDR_PLD_LEN);
        pld_size += 2;
    /* 64-bit size field */
    } else {
        pld.s64.size = HTOBE64((uint64_t)size); hdr |= 127 << LSB(WS_HDR_PLD_LEN);
        pld_size += 8;
    }

    /* encode header byte */
    pld.hdr = HTOBE16(hdr);
    /* send the header */
    return req->instance->sock_funcs.send(req->sock, &pld, pld_size, 0);
}

/* receive the payload data and take care of unmasking */
static err_t UHTTPSrvWS_RecvPayload(struct uhttp_request *req, void *ptr,
    size_t size)
{
    /* masking buffer */
    uint8_t buf[32], *p8 = ptr;
    /* data offset */
    size_t offs; err_t ec;
    /* underlying websocket */
    uhttp_ws_t *ws = &req->ws;

    /* read all the bytes */
    for (offs = 0; offs < size; offs += ec) {
        /* receive data */
        ec = req->instance->sock_funcs.recv(req->sock, buf,
            min(sizeof(buf), size - offs), 0);
        /* error during reception */
        if (ec < EOK)
            return ec;
        /* client is supposed to mask all frames so we need to unmask them */
        for (size_t i = 0; i < ec; i++)
            buf[i] ^= ws->mask.u8[i % 4];
        /* store the data into the output buffer */
        if (ptr)
            memcpy(p8 + offs, buf, ec);
    }

    /* return the number of bytes received */
    return offs;
}

/* send payload and take care of masking */
static err_t UHTTPSrvWS_SendPayload(struct uhttp_request *req,
    const void *ptr, size_t size)
{
        /* masking buffer */
    uint8_t buf[32]; const uint8_t *p8 = ptr;
    /* data offset */
    size_t offs; err_t ec;

    /* read all the bytes */
    for (offs = 0; offs < size; offs += ec) {
        /* get the maximal transfer size */
        size_t t_size =  min(sizeof(buf), size - offs);
        /* store the data into the output buffer */
        memcpy(buf, p8 + offs, t_size);
        /* receive data */
        ec = req->instance->sock_funcs.send(req->sock, buf, t_size, 0);
        /* error during send */
        if (ec < EOK)
            return ec;
    }

    /* return the number of bytes received */
    return offs;
}

/* compute the accept value based on the key value */
static void UHTTPSrvWS_ComputeAccept(const char key[24], char accept[30])
{
    /* hash storage: sha1 returns 20 bytes worth of hash data */
    uint8_t hash[20];

    /* initialize hashing machinery */
    sha1_state_t sha1; SHA1_InitState(&sha1);

    /* do the sha1 over the key concatenated with magic value */
    SHA1_Digest(&sha1, 0, key, 24);
    SHA1_Digest(&sha1, 1, magic, sizeof(magic) - 1);
    /* store the hash value in the output array */
    int hash_size = SHA1_GetHashVal(&sha1, hash, sizeof(hash));

    /* encode the hash using base64 */
    size_t accept_size = Base64_Encode(hash, hash_size, accept, 29);
    /* zero terminate */
    accept[accept_size++] = '\0';
}

/* initialize common parts of the websocket support */
err_t UHTTPSrvWS_Init(void)
{
    /* report status */
    return EOK;
}

/* accept incoming connection request */
err_t UHTTPSrvWS_Accept(struct uhttp_request *req)
{
    /* computed accept value */
    char accept[30]; err_t ec = EOK;

    /* trying to establish websocket connection on non-websocket request */
    if (req->type != HTTP_REQ_TYPE_WEBSOCKET)
        return EARGVAL;

    /* crank the numbers to get the accept value */
    UHTTPSrvWS_ComputeAccept(req->ws_key, accept);

    /* send the response */
    ec |= UHTTPSrv_SendStatus(req, HTTP_STATUS_101_SWITCHING_PROTOCOLS, 0);
    /* send static header fields */
    ec |= UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_UPGRADE, "websocket");
    ec |= UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_CONNECTION, "Upgrade");
    /* send the computed accept value */
    ec |= UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_SEC_WS_ACCEPT, accept);
    /* finalize the response with an empty line */
    ec |= UHTTPSrv_EndHeader(req);

    /* error code indicates that eve*/
    if (ec >= EOK)
        req->ws.is_open = 1;

    /* return the error code */
    return ec;
}

/* receive data from the socket */
err_t UHTTPSrvWS_Recv(struct uhttp_request *req, uhttp_ws_data_type_t *dtype,
    void *ptr, size_t size, dtime_t timeout)
{
    /* header word and error code */
    uint16_t hdr; err_t ec;
    /* ping interval */
    const dtime_t ping_ival = 1000;
    /* counter for the number of ping sent and pongs received */
    uint32_t ping_cnt = 0, pong_cnt = 0;
    /* underlying websocket */
    uhttp_ws_t *ws = &req->ws;

    /* time to receive the header for the next frame */
    again: Sem_Lock(&ws->rx_sem, 0);
    /* need to receive the header? timeout*/
    if (ws->rx_size == ws->rx_offs) {
        /* we wait at most one second for the header to appear */
        dtime_t hdr_to = timeout ? min(timeout, ping_ival) : ping_ival;
        /* receive and parse the header */
        if ((ec = UHTTPSrvWS_RecvHeader(req, &hdr, &ws->rx_size, hdr_to)) < EOK) {
            /* we are not connected */
            if (ec == ENOCONNECT)
                goto end;
            /* this time we were really expecting to see the header */
            if (hdr_to < ping_ival)
                goto end;
            /* pings are leading by more than tree frames? */
            if (ping_cnt > pong_cnt + 3)
                goto end;
            /* send ping in order to see whether the connection is still alive */
            with_sem (&ws->tx_sem)
                ec = UHTTPSrvWS_SendHeader(req, WS_HDR_OPCODE_PING, 0);
            /* unable to send the ping packet */
            if (ec < EOK)
                goto end;
            /* subtract from timeout */
            if (timeout)
                timeout -= ping_ival;
            /* restart the whole process */
            goto again;
        }
        /* extract the opcode from the header */
        ws->rx_opcode = hdr & WS_HDR_OPCODE;
        /* reset the offset */
        ws->rx_offs = 0;
    }

    /* switch on currently receceived frame */
    switch (ws->rx_opcode) {
    /* data carrying frames */
    case WS_HDR_OPCODE_TEXT:
    case WS_HDR_OPCODE_BIN: {
        /* get the payload */
        ec = UHTTPSrvWS_RecvPayload(req, ptr, min(size, ws->rx_size - ws->rx_offs));
        /* update the number of bytes received */
        if (ec >= EOK)
            ws->rx_offs += ec;

        /* caller wants to know the data type? */
        if (dtype) {
            /* switch on current opcode */
            switch (ws->rx_opcode) {
            case WS_HDR_OPCODE_TEXT: *dtype = HTTP_WS_DATA_TYPE_TEXT; break;
            case WS_HDR_OPCODE_BIN: *dtype = HTTP_WS_DATA_TYPE_BIN; break;
            }
        }
        /* end of reception */
        Sem_Release(&ws->rx_sem);
    } break;
    /* pong was received */
    case WS_HDR_OPCODE_PONG: {
        /* discard the payload */
        ec = UHTTPSrvWS_RecvPayload(req, 0, ws->rx_size);
        /* reset variables */
        ws->rx_size = ws->rx_offs = 0;
        /* go back to receiving */
        if (ec >= EOK) {
            pong_cnt++; goto again;
        }
    } break;
    /* connection close or ping */
    case WS_HDR_OPCODE_PING:
    case WS_HDR_OPCODE_CLOSE: {
        /* close frames may carry reason that needs to be sent back, ping
         * frames carry payload */
        uint8_t pld[125];
        /* receive the payload  */
        ec = UHTTPSrvWS_RecvPayload(req, pld, min(sizeof(pld), ws->rx_size));
        /* end of reception */
        Sem_Release(&ws->rx_sem);
        /* handle errors */
        if (ec < EOK)
            break;
        /* store the payload size */
        size_t pld_size = ec;
        /* ensure that sending is uninterrupted by other sends and that we also
         * don't start sending data right in the middle of someone else's send
         */
        with_sem (&ws->tx_sem) {
            /* derive the response header */
            uint16_t rsp_opcode = ws->rx_opcode == WS_HDR_OPCODE_CLOSE ?
                WS_HDR_OPCODE_CLOSE : WS_HDR_OPCODE_PONG;
            /* respond with close segment */
            if (ec >= EOK) ec = UHTTPSrvWS_SendHeader(req, rsp_opcode, ec);
            if (ec >= EOK) ec = UHTTPSrvWS_SendPayload(req, pld, pld_size);
            /* we replied to close frame */
            if (rsp_opcode == WS_HDR_OPCODE_CLOSE) {
                // Sleep(100);
                ec = ENOCONNECT;
            }
        }
        /* reset variables */
        ws->rx_size = ws->rx_offs = 0;
        /* go and wait for the next frame */
        if (ec >= EOK)
            goto again;
    } break;
    /* unsupported opcode */
    default: ec = EFATAL; break;
    }

    /* error during reception */
    end: if (ec < EOK)
        ws->is_open = 0;

    /* return status */
    return ec;
}

/* send the frame */
err_t UHTTPSrvWS_Send(struct uhttp_request *req, uhttp_ws_data_type_t dtype,
    const void *ptr, size_t size)
{
    /* error code */
    err_t ec = EOK; uint16_t opcode;
    /* underlying websocket */
    uhttp_ws_t *ws = &req->ws;

    /* derive the opcode from the data type */
    switch (dtype) {
    case HTTP_WS_DATA_TYPE_TEXT: opcode = WS_HDR_OPCODE_TEXT; break;
    case HTTP_WS_DATA_TYPE_BIN: opcode = WS_HDR_OPCODE_BIN; break;
    /* wtf? */
    default: return EARGVAL;
    }

    /* we are not connected */
    if (!ws->is_open)
        return ENOCONNECT;

    /* lock tcp sending */
    with_sem (&ws->tx_sem) {
        /* send the header and the payload */
        if (ec >= EOK) ec = UHTTPSrvWS_SendHeader(req, opcode, size);
        if (ec >= EOK) ec = UHTTPSrvWS_SendPayload(req, ptr, size);
    }

    /* error during reception */
    if (ec < EOK) {
        /* mark socket as closed */
        ws->is_open = 0;
    }

    /* return the number of bytes sent or error */
    return ec;
}

/* close underlying websocket */
err_t UHTTPSrvWS_Close(struct uhttp_request *req)
{
    uint16_t hdr; size_t size;
    /* processing error code */
    err_t ec = EOK;
    /* underlying websocket */
    uhttp_ws_t *ws = &req->ws;

    /* socket is open */
    if (ws->is_open) {
        /* send the close frame */
        with_sem (&ws->tx_sem) {
            /* send the header */
            ec = UHTTPSrvWS_SendHeader(req, WS_HDR_OPCODE_CLOSE, 0);
        }
        /* poll rest of the frames */
        for (;; Yield()) {
            /* wait for the response */
            with_sem (&ws->rx_sem) {
                if (ec >= EOK) ec = UHTTPSrvWS_RecvHeader(req, &hdr, &size, 0);
                if (ec >= EOK) ec = UHTTPSrvWS_RecvPayload(req, 0, size);
            }
            /* error during reception */
            if (ec < EOK)
                break;
            /* close frame was received? */
            if ((hdr & WS_HDR_OPCODE) == WS_HDR_OPCODE_CLOSE)
                break;
        }
    }

    /* mark as closed */
    ws->is_open = 0;

    /* report success */
    return EOK;
}