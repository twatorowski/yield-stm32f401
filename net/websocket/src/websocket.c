/**
 * @file wssrv.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-12-26
 *
 * @copyright Copyright (c) 2024
 */

#include "assert.h"
#include "config.h"
#include "err.h"

#include "dev/seed.h"
#include "net/tcpip/tcp_sock.h"
#include "net/websocket/websocket.h"
#include "sys/heap.h"
#include "sys/yield.h"
#include "util/base64.h"
#include "util/elems.h"
#include "util/lfsr32.h"
#include "util/sha1.h"
#include "util/stdio.h"
#include "util/string.h"
#include "util/minmax.h"

#define DEBUG DLVL_INFO
#include "debug.h"


/* http version specifier */
typedef struct websocket_http_version_spec {
    /* version enum */
    enum websocket_http_version {
        HTTP_VER_UNKNOWN = 1,
        HTTP_VER_1V0,
        HTTP_VER_1V1,
        HTTP_VER_2V0,
        HTTP_VER_3V0,
    } version;
    /* string representation */
    const char *str;
} websocket_http_version_spec_t;

/** field enum that encodes the name */
typedef enum websocket_field_name {
    WS_FIELD_NAME_UNKNOWN = 1,
    /* empty line */
    WS_FIELD_NAME_EMPTY,
    WS_FIELD_NAME_HOST,
    WS_FIELD_NAME_ORIGIN,
    WS_FIELD_NAME_CONNECTION,
    WS_FIELD_NAME_UPGRADE,
    /* security related fields  */
    WS_FIELD_NAME_SEC_WS_KEY,
    WS_FIELD_NAME_SEC_WS_PROTOCOL,
    WS_FIELD_NAME_SEC_WS_VERSION,
    WS_FIELD_NAME_SEC_WS_ACCEPT,
} websocket_field_name_t;

/** status codes */
typedef enum websocket_status_code {
    WS_STATUS_UNKNOWN = 1,
    WS_STATUS_101_SWITCHING_PROTOCOLS,
    WS_STATUS_400_BAD_REQUEST,
    WS_STATUS_404_NOT_FOUND,
    WS_STATUS_500_INTERNAL_SRV_ERR,
} websocket_status_code_t;

/* status code specifier */
typedef struct websocket_status_code_spec {
    /* status code enum */
    websocket_status_code_t code;
    /* code value */
    int value;
    /* string message */
    const char *msg;
} websocket_status_code_spec_t;

/* header field value */
typedef struct websocket_field {
    /* field name */
    websocket_field_name_t name;
    /* possible values */
    union websocket_field_value {
        int i; float f; char s[32];
    } value;
} websocket_field_t;

/* specification of a field */
typedef struct websocket_field_spec {
    /* field name */
    websocket_field_name_t name;
    /* field value type */
    enum websocket_field_type {
        WS_FIELD_TYPE_INT = 1,
        WS_FIELD_TYPE_FLOAT,
        WS_FIELD_TYPE_STR,
    } type;
    /* field string name */
    const char *str;
} websocket_field_spec_t;

/* error codes */
typedef enum websocket_reason_code {
    WS_REASON_CODE_NO_REASON = 0,
    WS_REASON_CODE_NORMAL_CLOSURE = 1000,
    WS_REASON_CODE_GOING_AWAY = 1001,
    WS_REASON_CODE_PROTOCOL_ERROR = 1002,
    WS_REASON_CODE_UNSUPPORTED_DATA = 1003,
    WS_REASON_CODE_NO_CODE = 1005,
    WS_REASON_CODE_ABNORMAL_CLOSE = 1006,
    WS_REASON_CODE_INV_PAYLOAD = 1007,
    WS_REASON_CODE_POLICY_VIOLATED = 1008,
    WS_REASON_CODE_MESSAGE_TOO_BIG = 1009,
    WS_REASON_CODE_UNSUPPORTED_EXTENSION = 1010,
    WS_REASON_CODE_INTERNAL_SERVER_ERROR = 1011,
} websocket_reason_code_t;

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

/* returns the version specifier for given enum or string name */
static const websocket_http_version_spec_t * WebSocket_GetVersionSpec(
    enum websocket_http_version version, const char *str)
{
    /* look up table for the conversion */
    static const websocket_http_version_spec_t  *l, lut[] =  {
        { HTTP_VER_1V0, "HTTP/1.0" },
        { HTTP_VER_1V1, "HTTP/1.1" },
        { HTTP_VER_2V0, "HTTP/2" },
        { HTTP_VER_3V0, "HTTP/3" },
    };

    /* sanity check */
    assert(!(version && str), "only one thing can be specified");
    assert(version || str, "at least one thing must be specified");

    /* go through the table */
    for (l = lut; l != lut + elems(lut); l++) {
        /* enum based search */
        if (version && version == l->version) return l;
        /* string based search */
        if (str && strcmp(l->str, str) == 0) return l;
    }

    /* nothing was found ;-( */
    return 0;
}


/* get status code specification */
static const websocket_status_code_spec_t * WebSocket_GetStatusCodeSpec(
    websocket_status_code_t code, int code_val, const char *msg)
{
    /* list of supported field types and their parsers */
    static const websocket_status_code_spec_t *l, lut[] = {
        { WS_STATUS_101_SWITCHING_PROTOCOLS, 101, "Switching Protocols" },
        { WS_STATUS_400_BAD_REQUEST, 400, "Bad Request" },
        { WS_STATUS_404_NOT_FOUND, 404, "Not Found" },
        { WS_STATUS_500_INTERNAL_SRV_ERR, 500, "Internal Server Error" },
    };

    /* sanity check */
    assert(!(code && msg && code_val), "only one thing can be specified");
    assert(code || msg || code_val, "at least one thing must be specified");

    /* look for the entry in the lut */
    for (l = lut; l != lut + elems(lut); l++) {
        /* matching by code */
        if (code && l->code == code)
            return l;
        /* match by code value */
        if (code_val && code_val == l->value)
            return l;
        /* matching by name string */
        if (msg && strncicmp(l->msg, msg, strlen(l->msg)) == 0)
            return l;
    }
    /* did not find anything */
    return 0;
}

/* get field specification either by name or by field name enum */
static const websocket_field_spec_t * WebSocket_GetFieldSpec(
    websocket_field_name_t name, const char *str)
{
    /* list of supported field types and their parsers */
    static const websocket_field_spec_t *l, lut[] = {
        { WS_FIELD_NAME_HOST, WS_FIELD_TYPE_STR, "Host" },
        { WS_FIELD_NAME_ORIGIN, WS_FIELD_TYPE_STR, "Origin" },
        { WS_FIELD_NAME_CONNECTION, WS_FIELD_TYPE_STR, "Connection" },
        { WS_FIELD_NAME_UPGRADE, WS_FIELD_TYPE_STR, "Upgrade" },

        { WS_FIELD_NAME_SEC_WS_KEY, WS_FIELD_TYPE_STR,
            "Sec-WebSocket-Key" },
        { WS_FIELD_NAME_SEC_WS_PROTOCOL, WS_FIELD_TYPE_STR,
            "Sec-WebSocket-Protocol" },
        { WS_FIELD_NAME_SEC_WS_VERSION, WS_FIELD_TYPE_INT,
            "Sec-WebSocket-Version" },
        { WS_FIELD_NAME_SEC_WS_ACCEPT, WS_FIELD_TYPE_STR,
            "Sec-WebSocket-Accept" },
    };

    /* sanity check */
    assert(!(name && str), "only one thing can be specified");
    assert(name || str, "at least one thing must be specified");

    /* look for the entry in the lut */
    for (l = lut; l != lut + elems(lut); l++) {
        /* matching by name */
        if (name && l->name == name)
            return l;
        /* matching by name string */
        if (str && strncicmp(l->str, str, strlen(l->str)) == 0)
            return l;
    }
    /* did not find anything */
    return 0;
}

/* parse the request line from the server */
static err_t WebSocket_ParseRequestLine(const char *line, size_t line_len,
    char *url, size_t url_size)
{
    /* placeholders */
    char method_str[10], version_str[10];
    /* scan the line, it should be splittable into three substrings separated by
     * space: method url version */
    if (snscanf(line, line_len, "%.*s %.*s %.*s",
        sizeof(method_str) - 1, method_str, url_size, url,
        sizeof(version_str) - 1, version_str ) != 3)
        return EARGVAL;

    /* only get method is supported */
    if (strcmp(method_str, "GET"))
        return EARGVAL;

    /* parse the version */
    const websocket_http_version_spec_t *v_spec = WebSocket_GetVersionSpec(0,
        version_str);
    /* unable to parse method */
    if (!v_spec)
        return EFATAL;

    /* return the status of parsing */
    return EOK;
}

/* parse the status line from the client */
static err_t WebSocket_ParseStatusLine(const char *line, size_t line_len,
    enum websocket_http_version *version, websocket_status_code_t *code)
{
    /* placeholders */
    char version_str[16], code_str[64]; int code_value;
    /* scan the line, it should be splittable into three substrings separated by
     * space: method url version */
    if (snscanf(line, line_len, "%.*s %d %.*s",
        sizeof(version_str) - 1, version_str,
        &code_value,
        sizeof(code_str) - 1, code_str) != 3)
        return EARGVAL;

    /* parse the version */
    const websocket_http_version_spec_t *v_spec = WebSocket_GetVersionSpec(0,
        version_str);
    /* unable to parse method */
    if (!v_spec)
        return EFATAL;
    /* store the version value */
    if (version)
        *version = v_spec->version;

    /* parse the status code */
    const websocket_status_code_spec_t *s_spec = WebSocket_GetStatusCodeSpec(0,
        code_value, 0);
    /* unable to parse method */
    if (!v_spec)
        return EFATAL;
    /* store the code value */
    if (code)
        *code = s_spec->code;

    /* return the status of parsing */
    return EOK;
}

/* try to parse a line as it is a field value: "field: value" */
static err_t WebSocket_ParseFieldLine(const char *line, size_t line_len,
    websocket_field_t *field)
{
    /* pointer to where the value starts */
    const char *vptr;

    /* handle the cases where the field is an empty line */
    if (*line == 0)
        goto empty;

    /* go across the field name until you meet ':' or eol (which is an error) */
    for (vptr = line; vptr < line + line_len && *vptr && *vptr != ':'; vptr++);
    /* if it does not end with ':' then we have a problem */
    if (*vptr != ':')
        goto fail;

    size_t name_len = vptr - line;
    /*.. and move the ':' and the whitespaces */
    for (vptr = vptr + 1; vptr < line + line_len && isspace(*vptr); vptr++);

    /* store the value size */
    size_t vsize = line_len - (vptr - line);

    /* if the field is known then we'll fill that information later on */
    field->name = WS_FIELD_NAME_UNKNOWN;

    /* try to locate the field specification in the data-base */
    const websocket_field_spec_t *fs = WebSocket_GetFieldSpec(0, line);
    /* unknown field */
    if (!fs)
        goto unknown;

    /* conversion ok flag */
    int conv_ok = 0; union websocket_field_value fv;
    /* parse the value */
    switch (fs->type) {
    /* integers and floats are dealt with snscanf */
    case WS_FIELD_TYPE_INT:
        conv_ok = snscanf(vptr, vsize, "%i", &fv.i) == 1; break;
    case WS_FIELD_TYPE_FLOAT:
        conv_ok = snscanf(vptr, vsize, "%a", &fv.f) == 1; break;
    case WS_FIELD_TYPE_STR:
        /* send out a warning */
        if (vsize >= sizeof(fv.s))
            dprintf_w("field %.*s value will be truncated!", name_len, line);
        /* do the copying */
        conv_ok = 1; strlcpy(fv.s, vptr, sizeof(fv.s)); break;
    /* unsupported conversion */
    default: assert(0, "unsupported converter");
    }
    /* conversion succeded? */
    if (!conv_ok)
        goto fail;

    /* store the data */
    field->name = fs->name; memcpy(&field->value, &fv, sizeof(fv));
    /* report success */
    return EOK;

    /* properly syntaxed but unknown parameters go here */
    unknown: {
        /* mark the field as unknown */
        field->name = WS_FIELD_NAME_UNKNOWN;
        /* since we are here then it means that we were not able to parse
        * the parameter. let's just simply store the string value */
        strlcpy(field->value.s, vptr, min(sizeof(field->value.s), vsize));
        /* send out a warning */
        if (vsize >= sizeof(fv.s))
            dprintf_w("field %.*s value will be truncated!\n",
            name_len, line);
        /* could not parse the field */
        return EOK;
    }

    /* improper syntax lines go here */
    fail: {
        return EFATAL;
    }

    /* case of the empty line */
    empty: {
        /* setup fields */
        field->name = WS_FIELD_NAME_EMPTY;
        /* empty line is not an error */
        return EOK;
    }
}

/* send empty line that denotes the end of the header */
static err_t WebSocket_SendEmptyLine(websocket_t *ws)
{
    /* send \r\n sequence over underlyint tcp socket */
    return TCPIPTcpSock_Send(ws->sock, "\r\n", 2, 0);
}

/* send and process the request line ("GET /endpoint HTTP/1.1") */
static err_t WebSocket_SendRequestLine(websocket_t *ws, const char *url)
{
    /* line buffer, line length */
    char line[WEBSOCKETS_MAX_LINE_LEN + 1];

    /* obtain the version specifier */
    const websocket_http_version_spec_t *vspec =
        WebSocket_GetVersionSpec(HTTP_VER_1V1, 0);

    /* render the response */
    size_t len = snprintf(line, sizeof(line), "GET %s %s\r\n",
        url ? "/" : url, vspec->str);
    /* send the response */
    return TCPIPTcpSock_Send(ws->sock, line, len, 0);
}

/* respond with a status line */
static err_t WebSocket_SendStatusLine(websocket_t *ws,
    enum websocket_http_version version, websocket_status_code_t code)
{
    /* render the response */
    char response[WEBSOCKETS_MAX_LINE_LEN];
    /* obtain the version specifier */
    const websocket_http_version_spec_t *vspec =
        WebSocket_GetVersionSpec(version, 0);
    /* unknown version specifier */
    if (!vspec)
        return EFATAL;

    /* look for status code specifier */
    const websocket_status_code_spec_t *cspec =
        WebSocket_GetStatusCodeSpec(code, 0, 0);
    /* unknown status code */
    if (!cspec)
        return EFATAL;

    /* render the response */
    size_t len = snprintf(response, sizeof(response), "%s %d %s\r\n",
        vspec->str, cspec->value, cspec->msg);
    /* send the response */
    return TCPIPTcpSock_Send(ws->sock, response, len, 0);
}

/* render field's value to the output string */
static err_t WebSocket_SendFieldLine(websocket_t *ws,
    websocket_field_name_t name, ...)
{
    /* list of arguments */
    va_list args;
    /* placeholder for the line */
    char line[WEBSOCKETS_MAX_LINE_LEN + 1];
    /* error code */
    err_t ec;
    /* get the field specification */
    const websocket_field_spec_t *fs = WebSocket_GetFieldSpec(name, 0);

    /* unknown field */
    if (!fs)
        return EFATAL;

    /* initialize the argument list */
    va_start(args, name);
    /* get the field formatter */
    switch (fs->type) {
    case WS_FIELD_TYPE_INT: {
        ec = snprintf(line, sizeof(line), "%s: %i\r\n", fs->str,
        va_arg(args, int));
    } break;
    case WS_FIELD_TYPE_FLOAT: {
        ec = snprintf(line, sizeof(line), "%s: %f\r\n", fs->str,
        va_arg(args, double));
    } break;
    case WS_FIELD_TYPE_STR: {
        ec = snprintf(line, sizeof(line), "%s: %s\r\n", fs->str,
        va_arg(args, char *));
    } break;
    /* complain */
    default: assert(0, "unsupported type");
    }
    /* finalize the argument list */
    va_end(args);

    /* unable to renfer the line */
    if (ec < EOK)
        return ec;

    /* try to send the field */
    return TCPIPTcpSock_Send(ws->sock, line, ec, 0);
}

/* receive a line from the socket */
static err_t WebSocket_RecvLine(websocket_t *ws, char *line, size_t size)
{
    /* error code */
    err_t ec = EOK;
    /* current offset pointer and the last non whitespace character */
    char *c = line;

    /* loop until whole line is received */
    while (1) {
        /* error during reception */
        if ((ec = TCPIPTcpSock_Recv(ws->sock, c, 1, 0)) < EOK)
            return ec;
        /* complete line received? */
        if (*c == '\n') {
            break;
        /* buffer got used up */
        } else if (++c - line == size) {
            return EFATAL;
        }
    }
    /* remove trailing whitespace */
    for (; c > line && isspace(*(c - 1)); c--);
    /* terminate with zero */
    *c = '\0';

    /* return the length of the line */
    return c - line;
}

/* receive the status line from the server */
static err_t WebSocket_RecvStatusLine(websocket_t *ws,
    websocket_status_code_t *code)
{
    /* line buffer, line length */
    char line[WEBSOCKETS_MAX_LINE_LEN + 1];
    /* current line length */
    int line_len;

    /* receive a line of text */
    if ((line_len = WebSocket_RecvLine(ws, line, sizeof(line) - 1)) < EOK)
        return EFATAL;

    /* unable to parse the line */
    if (WebSocket_ParseStatusLine(line, line_len, 0, code) < EOK)
        return EFATAL;

    /* report success */
    return EOK;
}

/* receive and process the request line ("GET /endpoint HTTP/1.1") */
static err_t WebSocket_RecvRequestLine(websocket_t *ws, char *url,
    size_t url_size)
{
    /* line buffer, line length */
    char line[WEBSOCKETS_MAX_LINE_LEN + 1];
    /* current line length */
    int line_len;


    /* receive a line of text */
    if ((line_len = WebSocket_RecvLine(ws, line, sizeof(line) - 1)) < EOK)
        return EFATAL;

    /* let's try to parse the request line */
    if (WebSocket_ParseRequestLine(line, line_len, url, url_size) < EOK)
        return EFATAL;

    /* request line makes sense, report ok */
    return EOK;
}

/* receive a single line that contains the field "fieldName: fieldValue" */
static err_t Websocket_RecvFieldLine(websocket_t *ws, websocket_field_t *field)
{
    /* line buffer, line length */
    char line[WEBSOCKETS_MAX_LINE_LEN + 1];
    /* current line length */
    int line_len;

    /* receive a line of text */
    if ((line_len = WebSocket_RecvLine(ws, line, sizeof(line))) < EOK)
        return EFATAL;
    /* parse as a field */
    if (WebSocket_ParseFieldLine(line, line_len, field) < EOK)
        return EFATAL;

    /* line was parsed */
    return EOK;
}

/* send an error reply to the client */
static err_t WebSocket_SendErrorReply(websocket_t *ws,
    websocket_status_code_t code)
{
    /* send status */
    err_t ec = EOK;
    /* send the error frame */
    if (ec >= EOK) ec = WebSocket_SendStatusLine(ws, HTTP_VER_1V1, code);
    if (ec >= EOK) ec = WebSocket_SendEmptyLine(ws);
    /* return the status */
    return ec;
}

/* receive websocket header */
static err_t WebSocket_RecvHeader(websocket_t *ws, uint16_t *hdr, size_t *size,
    dtime_t timeout)
{
    /* data placeholders */
    err_t ec; uint8_t *mask_ptr;
    /* payload placeholder */
    websocket_hdr_t pld; size_t decoded_size, bytes_to_get = 0;

    /* receive the header */
    if ((ec = TCPIPTcpSock_Recv(ws->sock, &pld.hdr, sizeof(pld.hdr), timeout)) < EOK) {
        dprintf_w("TCP ec = %d\n", ec);
        return ec;
    }
    dprintf_w("TCP ec = %d\n", ec);
    /* undo the endiannes */
    *hdr = BETOH16(pld.hdr);

    /* server shal never mask, client shall always mask */
    if ((ws->role == WS_ROLE_SERVER && !(*hdr & WS_HDR_MASKED)) ||
        (ws->role == WS_ROLE_CLIENT &&  (*hdr & WS_HDR_MASKED)))
        return EFATAL;

    /* masking is present? */
    if (*hdr & WS_HDR_MASKED) {
        bytes_to_get += 4; mask_ptr = pld.mask;
    }

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
    dprintf_w("HERE %d %d\n", ec, bytes_to_get);
    if ((ec = TCPIPTcpSock_Recv(ws->sock, pld.mask, bytes_to_get, 0)) < EOK) {
        dprintf_w("TCP2 ec = %d\n", ec);
        return ec;
    }
    /* size specified as 16-bit number */
    if (decoded_size == 126) {
        *size = BETOH16(pld.s16.size); mask_ptr = pld.s16.mask;
    /* size specified as 64 bit number */
    } else if (decoded_size == 127) {
        *size = BETOH64(pld.s64.size); mask_ptr = pld.s64.mask;
    }

    /* copy the masking field */
    if (*hdr & WS_HDR_MASKED)
        memcpy(ws->mask.u8, mask_ptr, 4);

    /* header is parsed */
    return EOK;
}

/* send the header */
static err_t WebSocket_SendHeader(websocket_t *ws, uint16_t hdr, size_t size)
{
    /* total header size */
    size_t pld_size = 2; uint8_t *mask_dst;
    /* payload placeholder */
    websocket_hdr_t pld;

    /* set the fin field */
    hdr |= WS_HDR_FIN;
    /* encode size in the header field  */
    if (size < 126) {
        hdr |= size << LSB(WS_HDR_PLD_LEN); mask_dst = pld.mask;
    /* 16-bit size field */
    } else if (size < 0xffff) {
        pld.s16.size = HTOBE16(size); hdr |= 126 << LSB(WS_HDR_PLD_LEN);
        mask_dst = pld.s16.mask; pld_size += 2;
    /* 64-bit size field */
    } else {
        pld.s64.size = HTOBE64((uint64_t)size); hdr |= 127 << LSB(WS_HDR_PLD_LEN);
        mask_dst = pld.s64.mask; pld_size += 8;
    }

    /* we are the client */
    if (ws->role == WS_ROLE_CLIENT) {
        /* regenerate the mask */
        ws->mask.u32 = time(0);
        /* set the 'masked' bit in the header */
        hdr |= WS_HDR_MASKED;
        /* append the mask */
        memcpy(mask_dst, ws->mask.u8, 4);
        /* additional 4 bytes for the mask */
        pld_size += 4;
    }

    /* encode header byte */
    pld.hdr = HTOBE16(hdr);
    /* send the header */
    return TCPIPTcpSock_Send(ws->sock, &pld, pld_size, 0);
}

/* receive the payload data and take care of unmasking */
static err_t WebSocket_RecvPayload(websocket_t *ws, void *ptr, size_t size)
{
    /* masking buffer */
    uint8_t buf[32], *p8 = ptr;
    /* data offset */
    size_t offs; err_t ec;

    /* read all the bytes */
    for (offs = 0; offs < size; offs += ec) {
        /* receive data */
        ec = TCPIPTcpSock_Recv(ws->sock, buf, min(sizeof(buf), size - offs), 0);
        /* error during reception */
        if (ec < EOK)
            return ec;
        /* client is supposed to mask all frames so we need to unmask them */
        if (ws->role == WS_ROLE_SERVER) {
            for (size_t i = 0; i < ec; i++)
                buf[i] ^= ws->mask.u8[i % 4];
        }
        /* store the data into the output buffer */
        if (ptr)
            memcpy(p8 + offs, buf, ec);
    }

    dprintf_w("PLD size = %d\n", offs);
    /* return the number of bytes received */
    return offs;
}

/* send payload and take care of masking */
static err_t WebSocket_SendPayload(websocket_t *ws, const void *ptr, size_t size)
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
        /* client is supposed to mask all frames */
        if (ws->role == WS_ROLE_CLIENT) {
            for (size_t i = 0; i < t_size; i++)
                buf[i] ^= ws->mask.u8[i % 4];
        }
        /* receive data */
        ec = TCPIPTcpSock_Send(ws->sock, buf, t_size, 0);
        /* error during send */
        if (ec < EOK)
            return ec;
    }

    /* return the number of bytes received */
    return offs;
}

/* compute the accept value based on the key value */
static void WebSocket_ComputeAccept(const uint8_t key[24], uint8_t accept[30])
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

/* common initialization logic */
err_t WebSockSrv_Init(void)
{
    /* return status */
    return EOK;
}

/* create a websocket */
websocket_t * WebSocket_Create(void)
{
    /* allocate memory for a web-socket? */
    websocket_t *ws = Heap_Malloc(sizeof(websocket_t));
    /* we really need that memeory */
    assert(ws, "no memory for the web socket");

    /* underlying tcp socket */
    ws->sock = TCPIPTcpSock_Create(256, 256);
    /* no point if we cannot create underlying tcp socket */
    assert(ws->sock, "no memory for the tcp socket");

    /* return the allocated socket */
    return ws;
}

/* connect to the websocket server */
err_t WebSocket_Connect(websocket_t *ws, tcpip_ip_addr_t ip,
    tcpip_tcp_port_t port, const char *url)
{
    /* error code */
    err_t ec;
    /* server status code */
    websocket_status_code_t code;

    /* currently processed field */
    websocket_field_t field;
    /* this mask will contain the bits that represent the header files that
     * we've received */
    uint32_t field_mask = 0;
    /* fields required for the connection request */
    uint32_t field_req_mask = BIT_VAL(WS_FIELD_NAME_SEC_WS_ACCEPT) |
        BIT_VAL(WS_FIELD_NAME_CONNECTION) |
        BIT_VAL(WS_FIELD_NAME_UPGRADE);
    /* accept value */
    uint8_t accept[30];

    /* validate the data */
    if (ws->state != WS_STATE_CLOSED || !port)
        return EARGVAL;

    /* from now on this socket works as a client */
    ws->role = WS_ROLE_CLIENT;

    /* try to connect to the server */
    ec = TCPIPTcpSock_Connect(ws->sock, ip, port, 0);
    /* unable to connect? */
    if (ec < EOK)
        goto error;

    /* send the request line */
    if ((ec = WebSocket_SendRequestLine(ws, url)) < EOK)
        goto error;

    /* time to generate the key */
    uint32_t nonce[4] = { time(0) }; uint8_t key[25];
    /* get some pseudo-randomness generated */
    for (size_t i = 0; i <elems(nonce) + 1; i++)
        nonce[i] = Seed_GetRand();
    /* convert that into a key */
    size_t key_size = Base64_Encode(nonce, sizeof(nonce), key, sizeof(key));
    /* zero terminate */
    key[key_size++] = 0;

    /* send the fields */
    ec |= WebSocket_SendFieldLine(ws, WS_FIELD_NAME_UPGRADE, "websocket");
    ec |= WebSocket_SendFieldLine(ws, WS_FIELD_NAME_CONNECTION, "Upgrade");
    ec |= WebSocket_SendFieldLine(ws, WS_FIELD_NAME_SEC_WS_KEY, key);
    ec |= WebSocket_SendFieldLine(ws, WS_FIELD_NAME_SEC_WS_VERSION, 13);
    ec |= WebSocket_SendEmptyLine(ws);
    /* check if the fields were sent sucessfully */
    if (ec < EOK)
        goto error;

    /* receive the status line. the only acceptable response is 101 */
    if (WebSocket_RecvStatusLine(ws, &code) < EOK ||
        code != WS_STATUS_101_SWITCHING_PROTOCOLS)
        goto error;

    /* poll as long as the fields are comming */
    while (1) {
        /* unable to parse the field */
        if (Websocket_RecvFieldLine(ws, &field) < EOK)
            goto error;

        /* empty line denotes end of the fields */
        if (field.name == WS_FIELD_NAME_EMPTY)
            break;

        /* set the bit in the field mask */
        field_mask |= BIT_VAL(field.name);

        /* switch on the field name */
        switch (field.name) {
        /* connection field must contain the word "Upgrade" */
        case WS_FIELD_NAME_CONNECTION: {
            if (strcistr(field.value.s, "Upgrade") == 0)
                goto error;
        } break;
        /* check the type of the upgrade */
        case WS_FIELD_NAME_UPGRADE: {
            if (strcicmp(field.value.s, "websocket"))
                goto error;
        } break;
        /* copy the servers accept value */
        case WS_FIELD_NAME_SEC_WS_ACCEPT: {
            /* suspicious */
            if (strlen(field.value.s) != 28)
                goto error;

            /* compute out accept value for the sake of comparison */
            WebSocket_ComputeAccept(key, accept);
            /* mismatch detected? */
            if (memcmp(accept, field.value.s, 28) != 0)
                goto error;
        } break;

        /* other fields go here */
        default: break;
        }
    }

    /* do we have all of the fields? */
    if ((field_mask & field_req_mask) != field_req_mask)
        goto error;

    /* reset the reception engine */
    ws->rx_size = ws->rx_offs = 0;
    /* connection is now open */
    ws->state = WS_STATE_OPEN;

    /* report success */
    return EOK;

    /* handle errors */
    error: {
        /* terminate the connection */
        TCPIPTcpSock_Close(ws->sock, 0);
        /* we are no longer listening */
        ws->state = WS_STATE_CLOSED;
        /* report a problem */
        return EFATAL;
    }
}

/* listen for incomming connections on given port and for given url */
err_t WebSocket_Listen(websocket_t *ws, tcpip_tcp_port_t port, const char *url)
{
    /* error code */
    err_t ec; websocket_status_code_t error_code = WS_STATUS_UNKNOWN;
    /* url buffer */
    char req_url[WEBSOCKETS_MAX_LINE_LEN + 1];

    /* currently processed field */
    websocket_field_t field;
    /* this mask will contain the bits that represent the header files that
     * we've received */
    uint32_t field_mask = 0;
    /* fields required for the connection request */
    uint32_t field_req_mask = BIT_VAL(WS_FIELD_NAME_HOST) |
        BIT_VAL(WS_FIELD_NAME_SEC_WS_VERSION) |
        BIT_VAL(WS_FIELD_NAME_SEC_WS_KEY) |
        BIT_VAL(WS_FIELD_NAME_CONNECTION) |
        BIT_VAL(WS_FIELD_NAME_UPGRADE);

    /* client key and the computed accept value */
    uint8_t key[24], accept[30];

    /* validate the data */
    if (ws->state != WS_STATE_CLOSED || !port)
        return EARGVAL;

    /* from now on this socket works as a server */
    ws->role = WS_ROLE_SERVER; ws->state = WS_STATE_LISTEN;

    /* wait for the tcp connection */
    while ((ec = TCPIPTcpSock_Listen(ws->sock, port)) < EOK);

    /* wait for the request line */
    if ((ec = WebSocket_RecvRequestLine(ws, req_url, sizeof(req_url))) < EOK)
        goto error;
    /* sanity check the url */
    if (url && strcmp(url, req_url) != 0) {
        error_code = WS_STATUS_404_NOT_FOUND; goto error;
    }

    /* now, let'ts process all the fields */
    while (1) {
        /* malformed field line */
        if ((ec = Websocket_RecvFieldLine(ws, &field)) < EOK) {
            /* handle different error types */
            switch (ec) {
            case EFATAL: error_code = WS_STATUS_400_BAD_REQUEST;
            default: goto error;
            }
        }

        /* empty line denotes end of the fields */
        if (field.name == WS_FIELD_NAME_EMPTY)
            break;


        /* set the bit in the field mask */
        field_mask |= BIT_VAL(field.name);

        /* parse fields */
        switch (field.name) {
        /* connection field must contain the word "Upgrade" */
        case WS_FIELD_NAME_CONNECTION: {
            if (strcistr(field.value.s, "Upgrade") == 0)
                error_code = WS_STATUS_400_BAD_REQUEST;
        } break;
        /* check the type of the upgrade */
        case WS_FIELD_NAME_UPGRADE: {
            if (strcicmp(field.value.s, "websocket"))
                error_code = WS_STATUS_400_BAD_REQUEST;
        } break;
        /* check if this is the supprted version */
        case WS_FIELD_NAME_SEC_WS_VERSION: {
            if (field.value.i != 13)
                error_code = WS_STATUS_400_BAD_REQUEST;
        } break;

        /* connection key */
        case WS_FIELD_NAME_SEC_WS_KEY: {
            /* nonce is a 16 byte value passed through base64 */
            if (strlen(field.value.s) != 24) {
                error_code = WS_STATUS_400_BAD_REQUEST;
            /* copy the value */
            } else {
                memcpy(key, field.value.s, 24);
            }
        } break;

        /* other fields go here */
        default: break;
        }

        /* error has occured */
        if (error_code != WS_STATUS_UNKNOWN)
            goto error;
    }

    /* do we have all of the fields? */
    if ((field_mask & field_req_mask) != field_req_mask) {
        error_code = WS_STATUS_400_BAD_REQUEST; goto error;
    }

    /* crank the numbers to get the accept value */
    WebSocket_ComputeAccept(key, accept);

    /* send the response */
    ec |= WebSocket_SendStatusLine(ws, HTTP_VER_1V1,
        WS_STATUS_101_SWITCHING_PROTOCOLS);
    /* send static header fields */
    ec |= WebSocket_SendFieldLine(ws, WS_FIELD_NAME_UPGRADE, "websocket");
    ec |= WebSocket_SendFieldLine(ws, WS_FIELD_NAME_CONNECTION, "Upgrade");
    /* send the computed accept value */
    ec |= WebSocket_SendFieldLine(ws, WS_FIELD_NAME_SEC_WS_ACCEPT, accept);
    /* finalize the response with an empty line */
    ec |= WebSocket_SendEmptyLine(ws);
    /* unable to send the response */
    if (ec < EOK)
        goto error;

    /* reset the reception engine */
    ws->rx_size = ws->rx_offs = 0;
    /* connection is now open */
    ws->state = WS_STATE_OPEN;

    /* report success */
    return EOK;

    /* close the socket */
    error: {
        /* error code was specified? */
        if (error_code != WS_STATUS_UNKNOWN)
            WebSocket_SendErrorReply(ws, error_code);
        /* terminate the connection */
        TCPIPTcpSock_Close(ws->sock, 0);
        /* we are no longer listening */
        ws->state = WS_STATE_CLOSED;
        /* report a problem */
        return EFATAL;
    }
}

/* receive data from the socket */
err_t WebSocket_Recv(websocket_t *ws, websocket_data_type_t *dtype,
    void *ptr, size_t size, dtime_t timeout)
{
    /* header word and error code */
    uint16_t hdr; err_t ec;

    /* time to receive the header for the next frame */
    again: Sem_Lock(&ws->rx_sem, 0);
    /* need to receive the header? timeout*/
    if (ws->rx_size == ws->rx_offs) {
        /* receive and parse the header */
        if ((ec = WebSocket_RecvHeader(ws, &hdr, &ws->rx_size, timeout)) < EOK)
            goto end;
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
        ec = WebSocket_RecvPayload(ws, ptr, min(size, ws->rx_size - ws->rx_offs));
        /* update the number of bytes received */
        if (ec >= EOK)
            ws->rx_offs += ec;

        /* caller wants to know the data type? */
        if (dtype) {
            /* switch on current opcode */
            switch (ws->rx_opcode) {
            case WS_HDR_OPCODE_TEXT: *dtype = WS_DATA_TYPE_TEXT; break;
            case WS_HDR_OPCODE_BIN: *dtype = WS_DATA_TYPE_BIN; break;
            }
        }
        /* end of reception */
        Sem_Release(&ws->rx_sem);
    } break;
    /* connection close or ping */
    case WS_HDR_OPCODE_PING:
    case WS_HDR_OPCODE_CLOSE: {
        /* close frames may carry reason that needs to be sent back, ping
         * frames carry payload */
        uint8_t pld[125];
        /* receive the payload  */
        ec = WebSocket_RecvPayload(ws, pld, min(sizeof(pld), ws->rx_size));
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
            if (ec >= EOK) ec = WebSocket_SendHeader(ws, rsp_opcode, ec);
            if (ec >= EOK) ec = WebSocket_SendPayload(ws, pld, pld_size);
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
    }

    /* error during reception */
    end: if (ec < EOK) {
        /* mark socket as closed */
        ws->state = WS_STATE_CLOSED;
        /* close underlying tcp connection */
        TCPIPTcpSock_Close(ws->sock, 1000);
        /* return the error code */
        return ec;
    }

    /* return status */
    return ec;
}

/* send the frame */
err_t WebSocket_Send(websocket_t *ws, websocket_data_type_t dtype,
    const void *ptr, size_t size)
{
    /* error code */
    err_t ec = EOK; uint16_t opcode;

    /* derive the opcode from the data type */
    switch (dtype) {
    case WS_DATA_TYPE_TEXT: opcode = WS_HDR_OPCODE_TEXT; break;
    case WS_DATA_TYPE_BIN: opcode = WS_HDR_OPCODE_BIN; break;
    /* wtf? */
    default: return EARGVAL;
    }

    /* we are not connected */
    if (ws->state != WS_STATE_OPEN)
        return ENOCONNECT;

    /* lock tcp sending */
    with_sem (&ws->tx_sem) {
        /* send the header and the payload */
        if (ec >= EOK) ec = WebSocket_SendHeader(ws, opcode, size);
        if (ec >= EOK) ec = WebSocket_SendPayload(ws, ptr, size);
    }

    /* error during reception */
    if (ec < EOK) {
        /* mark socket as closed */
        ws->state = WS_STATE_CLOSED;
        /* close underlying tcp connection */
        TCPIPTcpSock_Close(ws->sock, 1000);
    }

    /* return the number of bytes sent or error */
    return ec;
}

/* close underlying websocket */
err_t WebSocket_Close(websocket_t *ws)
{
    uint16_t hdr; size_t size;
    /* processing error code */
    err_t ec = EOK;

    /* socket is open */
    if (ws->state == WS_STATE_OPEN) {
        /* send the close frame */
        with_sem (&ws->tx_sem) {
            /* send the header */
            ec = WebSocket_SendHeader(ws, WS_HDR_OPCODE_CLOSE, 0);
        }
        /* poll rest of the frames */
        for (;; Yield()) {
            /* wait for the response */
            with_sem (&ws->rx_sem) {
                if (ec >= EOK) ec = WebSocket_RecvHeader(ws, &hdr, &size, 0);
                if (ec >= EOK) ec = WebSocket_RecvPayload(ws, 0, size);
            }
            /* error during reception */
            if (ec < EOK)
                break;
            /* close frame was received? */
            if ((hdr & WS_HDR_OPCODE) == WS_HDR_OPCODE_CLOSE)
                break;
        }
    }

    /* closing is now done */
    if (ws->state != WS_STATE_CLOSED) {
        /* mark as closed */
        ws->state = WS_STATE_CLOSED;
        /* close the underlying socket */
        TCPIPTcpSock_Close(ws->sock, 1000);
    }

    /* report success */
    return EOK;
}