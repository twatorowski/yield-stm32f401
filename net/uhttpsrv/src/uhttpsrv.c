/**
 * @file uhttpsrv.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdarg.h>

#include "assert.h"
#include "config.h"
#include "err.h"
#include "net/tcpip/tcp_sock.h"
#include "net/uhttpsrv/uhttpsrv.h"
#include "sys/sleep.h"
#include "sys/yield.h"
#include "util/elems.h"
#include "util/minmax.h"
#include "util/stdio.h"
#include "util/string.h"

#define DEBUG
#include "debug.h"

/* http method specifier */
typedef struct uhttp_method_spec {
    /* method encoding enum */
    enum uhttp_method method;
    /* string version of the method name */
    const char *str;
} uhttp_method_spec_t;

/* http version specifier */
typedef struct uhttp_version_spec {
    /* version enum */
    enum uhttp_version {
        HTTP_VER_UNKNOWN,
        HTTP_VER_1V0,
        HTTP_VER_1V1,
        HTTP_VER_2V0,
        HTTP_VER_3V0,
    } version;
    /* string representation */
    const char *str;
} uhttp_version_spec_t;

/* specification of a field */
typedef struct uhttp_field_spec {
    /* field name */
    uhttp_field_name_t name;
    /* field value type */
    enum uhttp_field_type {
        HTTP_FIELD_TYPE_INT,
        HTTP_FIELD_TYPE_FLOAT,
        HTTP_FIELD_TYPE_STR,
    } type;
    /* field string name */
    const char *str;
} uhttp_field_spec_t;

/* status code specifier */
typedef struct uhttp_status_code_t {
    /* status code enum */
    uhttp_status_code_t code;
    /* code value */
    int value;
    /* string message */
    const char *msg;
} uhttp_status_code_spec_t;


/* returns the method specifier for given enum or string name */
static const uhttp_method_spec_t * UHTTPSrv_GetMethodSpec(
    enum uhttp_method method, const char *str)
{
    /* look up table */
    static const uhttp_method_spec_t *l, lut[] =  {
        { HTTP_METHOD_GET, "GET" },
        { HTTP_METHOD_HEAD, "HEAD" },
        { HTTP_METHOD_POST, "POST" },
        { HTTP_METHOD_PUT, "PUT" },
        { HTTP_METHOD_DELETE, "DELETE" },
        { HTTP_METHOD_CONNECT, "CONNECT" },
        { HTTP_METHOD_OPTIONS, "OPTIONS" },
        { HTTP_METHOD_TRACE, "TRACE" },
        { HTTP_METHOD_PATCH, "PATCH" },
    };

    /* sanity check */
    assert(!(method && str), "only one thing can be specified");
    assert(method || str, "at least one thing must be specified");
    
    /* go through the table */
    for (l = lut; l != lut + elems(lut); l++) {
        /* enum based search */
        if (method && method == l->method) return l;
        /* string based search */
        if (str && strcmp(l->str, str) == 0) return l;
    }

    /* nothing was found ;-( */
    return 0;
}

/* returns the version specifier for given enum or string name */
static const uhttp_version_spec_t * UHTTPSrv_GetVersionSpec(
    enum uhttp_version version, const char *str)
{
    /* look up table for the conversion */
    static const uhttp_version_spec_t  *l, lut[] =  {
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

/* get field specification either by name or by field name enum */
static const uhttp_field_spec_t * UHTTPSrv_GetFieldSpec(
    enum uhttp_field_name name, const char *str)
{
    /* list of supported field types and their parsers */
    static const uhttp_field_spec_t *l, lut[] = {
        { HTTP_FIELD_NAME_HOST, HTTP_FIELD_TYPE_STR, "host" },
        { HTTP_FIELD_NAME_CONTENT_LENGTH, HTTP_FIELD_TYPE_INT, "content-length" },
        { HTTP_FIELD_NAME_SERVER, HTTP_FIELD_TYPE_STR, "server" },
        { HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_ORIGIN, HTTP_FIELD_TYPE_STR, 
            "access-control-allow-origin" }, 
        { HTTP_FIELD_NAME_CONTENT_ENCODING, HTTP_FIELD_TYPE_STR, 
            "content-encoding" },
        { HTTP_FIELD_NAME_ACCEPT_ENCODING, HTTP_FIELD_TYPE_STR, 
            "accept-encoding" },
        { HTTP_FIELD_NAME_CONNECTION, HTTP_FIELD_TYPE_STR, "connection" },
        { HTTP_FIELD_NAME_CONTENT_TYPE, HTTP_FIELD_TYPE_STR, "content-type" },
        { HTTP_FIELD_NAME_ALLOW, HTTP_FIELD_TYPE_STR, "allow" },
        { HTTP_FIELD_NAME_ORIGIN, HTTP_FIELD_TYPE_STR, "origin" },
        { HTTP_FIELD_NAME_ACCESS_CONTROL_REQUEST_METHOD, HTTP_FIELD_TYPE_STR,
            "access-control-request-method" },
        { HTTP_FIELD_NAME_ACCESS_CONTROL_REQUEST_HEADERS, HTTP_FIELD_TYPE_STR,
            "access-control-request-headers" },
        { HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_ORIGIN, HTTP_FIELD_TYPE_STR,
            "access-control-allow-origin" },
        { HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_METHODS, HTTP_FIELD_TYPE_STR,
            "access-control-allow-methods" },
        { HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_HEADERS, HTTP_FIELD_TYPE_STR,
            "access-control-allow-headers" },
        { HTTP_FIELD_NAME_UPGRADE, HTTP_FIELD_TYPE_STR, "Upgrade"},
        { HTTP_FIELD_NAME_SEC_WS_KEY, HTTP_FIELD_TYPE_STR,
            "Sec-WebSocket-Key" },
        { HTTP_FIELD_NAME_SEC_WS_PROTOCOL, HTTP_FIELD_TYPE_STR,
            "Sec-WebSocket-Protocol" },
        { HTTP_FIELD_NAME_SEC_WS_VERSION, HTTP_FIELD_TYPE_INT,
            "Sec-WebSocket-Version" },
        { HTTP_FIELD_NAME_SEC_WS_ACCEPT, HTTP_FIELD_TYPE_STR,
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

/* get status code specification */
static const uhttp_status_code_spec_t * UHTTPSrv_GetStatusCodeSpec(
    enum uhttp_status_code code, const char *msg)
{
    /* list of supported field types and their parsers */
    static const uhttp_status_code_spec_t *l, lut[] = {
        { HTTP_STATUS_200_OK, 200, "OK" },
        { HTTP_STATUS_101_SWITCHING_PROTOCOLS, 101, "Switching Protocols" },
        { HTTP_STATUS_400_BAD_REQUEST, 400, "Bad Request" },
        { HTTP_STATUS_404_NOT_FOUND, 404, "Not Found" },
        { HTTP_STATUS_405_METHOD_NOT_ALLOWED, 405, "Method Not Allowed" },
        { HTTP_STATUS_500_INTERNAL_SRV_ERR, 500, "Internal Server Error" }, 
    };

    /* sanity check */
    assert(!(code && msg), "only one thing can be specified");
    assert(code || msg, "at least one thing must be specified");

    /* look for the entry in the lut */
    for (l = lut; l != lut + elems(lut); l++) {
        /* matching by code */
        if (code && l->code == code) 
            return l;
        /* matching by name string */
        if (msg && strncicmp(l->msg, msg, strlen(l->msg)) == 0)
            return l;
    }
    /* did not find anything */
    return 0;
}

/* parse the request line from the server */
static err_t UHTTPSrv_ParseRequestLine(const char *line, size_t line_len, 
    enum uhttp_method *method, char *url, size_t url_size, 
    enum uhttp_version *version)
{
    /* placeholders */
    char method_str[10], version_str[10];
    /* scan the line, it should be splittable into three substrings separated by 
     * space: method url version */
    if (snscanf(line, line_len, "%.*s %.*s %.*s",
        sizeof(method_str) - 1, method_str, url_size, url, 
        sizeof(version_str) - 1, version_str ) != 3)
        return EARGVAL;

    
    /* parse the method */
    const uhttp_method_spec_t *m_spec = UHTTPSrv_GetMethodSpec(0, method_str);
    /* unable to parse method */
    if (!m_spec)
        return EFATAL;

    /* parse the version */
    const uhttp_version_spec_t *v_spec = UHTTPSrv_GetVersionSpec(0, version_str);
    /* unable to parse method */
    if (!m_spec)
        return EFATAL;

    /* store data */
    *method = m_spec->method;
    *version = v_spec->version;
    
    /* return the status of parsing */
    return EOK;
}

/* try to parse a line as it is a field value: "field: value" */
static err_t UHTTPSrv_ParseFieldLine(const char *line, size_t line_len, 
    uhttp_field_t *field) 
{
    /* pointer to where the value starts */
    const char *vptr;

    /* go across the field name until you meet ':' or eol (which is an error) */
    for (vptr = line; vptr < line + line_len && *vptr && *vptr != ':'; vptr++);
    /* if it does not end with ':' then we have a problem */
    if (*vptr != ':')
        goto fail;

    /* store the length of the name string */
    size_t name_str_len = (vptr - line);
    /*.. and move the ':' and the whitespaces */
    for (vptr = vptr + 1; vptr < line + line_len && isspace(*vptr); vptr++);

    /* store the value size */
    size_t vsize = line_len - (vptr - line);
    
    /* if the field is known then we'll fill that information later on */
    field->name = HTTP_FIELD_NAME_UNKNOWN;
    /* store the sizes */
    field->name_str = line; field->name_str_len = name_str_len;

    /* try to locate the field specification in the data-base */
    const uhttp_field_spec_t *fs = UHTTPSrv_GetFieldSpec(0, line);
    /* unknown field */
    if (!fs)
        goto unknown;
    
     /* conversion ok flag */
    int conv_ok = 0; union uhttp_field_value fv;
    /* parse the value */
    switch (fs->type) {
    /* integers and floats are dealt with snscanf */
    case HTTP_FIELD_TYPE_INT:
        conv_ok = snscanf(vptr, vsize, "%i", &fv.i) == 1; break;
    case HTTP_FIELD_TYPE_FLOAT:
        conv_ok = snscanf(vptr, vsize, "%a", &fv.f) == 1; break;
    /* strings are just pointed to, they are not copied */
    case HTTP_FIELD_TYPE_STR:
        conv_ok = 1; fv.s = vptr; break;
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
        /* since we are here then it means that we were not able to parse 
        * the parameter. let's just simply store the string value */
        field->value.s = vptr;
        /* could not parse the field */
        return EOK;
    }

    /* improper syntax lines go here */
    fail: {
        return EFATAL;
    }
}

/* render field's value to the output string */
static err_t UHTTPSrv_RenderFieldLine(char *out, size_t size, 
    enum uhttp_field_name name, va_list args)
{
    /* error code */
    err_t ec;
    /* get the field specification */
    const uhttp_field_spec_t *fs = UHTTPSrv_GetFieldSpec(name, 0);

    /* unknown field */
    if (!fs)
        return EFATAL;

    /* get the field formatter */
    switch (fs->type) {
    case HTTP_FIELD_TYPE_INT: {
        ec = snprintf(out, size, "%s: %i\r\n", fs->str, va_arg(args, int));
    } break;
    case HTTP_FIELD_TYPE_FLOAT: {
        ec = snprintf(out, size, "%s: %f\r\n", fs->str, va_arg(args, double));
    } break;
    case HTTP_FIELD_TYPE_STR: {
        ec = snprintf(out, size, "%s: %s\r\n", fs->str, va_arg(args, char *));
    } break;
    /* complain */
    default: assert(0, "unsupported type");
    }

    /* return the size of the sentence being rendered */
    return ec;
}

/* receive a line from the socket */
static err_t UHTTPSrv_RecvLine(tcpip_tcp_sock_t *sock,
    uhttp_instance_t *instance, char *line, size_t size)
{
    /* error code */
    err_t ec = EOK;
    /* current offset pointer and the last non whitespace character */
    char *c = line;

    /* loop until whole line is received */
    while (1) {
        /* error during reception */
        if ((ec = TCPIPTcpSock_Recv(sock, c, 1, instance->timeout)) < EOK)
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

/* respond with a status line */
static err_t UHTTPSrv_SendStatusLine(tcpip_tcp_sock_t *sock,
    uhttp_instance_t *instance, enum uhttp_version version,
    uhttp_status_code_t code)
{
    /* render the response */
    char response[32];
    /* obtain the version specifier */
    const uhttp_version_spec_t *vspec = UHTTPSrv_GetVersionSpec(version, 0);
    /* unknown version specifier */
    if (!vspec)
        return EFATAL;
    
    /* look for status code specifier */
    const uhttp_status_code_spec_t *cspec = UHTTPSrv_GetStatusCodeSpec(code, 0);
    /* unknown status code */
    if (!cspec)
        return EFATAL;

    /* render the response */
    size_t len = snprintf(response, sizeof(response), "%s %d %s\r\n", 
        vspec->str, cspec->value, cspec->msg);
    /* send the response */
    return TCPIPTcpSock_Send(sock, response, len, instance->timeout);
}

/* respond with an empty line to denote the end of the header */
static err_t UHTTPSrv_SendEmptyLine(tcpip_tcp_sock_t *sock,
    uhttp_instance_t *instance)
{
    /* send the response */
    return TCPIPTcpSock_Send(sock, "\r\n", 2, instance->timeout);
}

/* serving task for the uhttpsrv */
static void UHTTPSrv_ServeTask(void *arg)
{
    /* this is the instance that we've created the server for */
    uhttp_instance_t *instance = arg;

    /* prepare the socket */
    tcpip_tcp_sock_t *sock = TCPIPTcpSock_Create(256, 256);
    /* sanity check */
    assert(sock != 0, "unable to create socket");

    /* line buffer, line length */
    char line[UHTTPSRV_MAX_LINE_LEN + 1]; 
    /* current line length */
    int line_len;
    
    /* http method */
    enum uhttp_method method = HTTP_METHOD_UNKNOWN;
    /* http request version */ 
    enum uhttp_version version = HTTP_VER_UNKNOWN;
    /* url field */
    char url[UHTTPSRV_MAX_LINE_LEN + 1];

    /* endless serving loop */
    for (;; Yield()) {
        /* listen on the port */
        if (TCPIPTcpSock_Listen(sock, instance->port, 0) < EOK)
            continue;

        /* we can play the game of keeping the connection alive after the 
         * request has been processed */
        for (;; Yield()) {
            /* receive a line of text */
            if ((line_len = UHTTPSrv_RecvLine(sock, instance, line,
                UHTTPSRV_MAX_LINE_LEN)) < EOK)
                break;
            /* let's try to parse the request line */
            if (UHTTPSrv_ParseRequestLine(line, line_len, &method, url, 
                    UHTTPSRV_MAX_LINE_LEN, &version) < EOK)
                break;
            
            /* let's prepare the callback argument */
            uhttp_request_t req = {
                .instance = instance,
                .sock = sock,
                .method = method,
                .line = line,
                .line_len = line_len,
                .url = url, 
                .body_bleft = 0,
                .resp_bleft = 0,
                .state = HTTP_STATE_READ_FIELDS,
                /* websocket stuff */
                .ws = { .is_open = 0 },
            };

            /* rest is done by the callback. from now we may 
            * proceed with calling the underlying logic */
            err_t ec = instance->callback(&req);
            /* an error happened and it's best to close the connection */
            if (ec != EOK || req.state != HTTP_STATE_DONE)
                break;
        }

        /* close the connection */
        TCPIPTcpSock_Close(sock, instance->timeout);
    }
}

/* test the server */
err_t UHTTPSrv_Init(void)
{
    /* start sevring */
    return EOK;
}

/* initialize the instance of the server */
err_t UHTTPSrv_InstanceInit(uhttp_instance_t *instance)
{
    /* error code */
    err_t ec = EOK;
    /* create a task that will serve the http */
    for (size_t i = 0; i < instance->max_connections; i++) {
        /* try to create an instance of the server */
        ec = Yield_Task(UHTTPSrv_ServeTask, instance, instance->stack_size);
        if (ec < EOK)
            break;
    }
    /* report status */
    return ec;
}

/* read the field from the header */
err_t UHTTPSrv_ReadHeaderField(uhttp_request_t *req, uhttp_field_t *field)
{
    /* wrong state */
    if (req->state != HTTP_STATE_READ_FIELDS)
        req->state = HTTP_STATE_ERROR;
    /* in error state */
    if (req->state == HTTP_STATE_ERROR)
        return EFATAL;

    /* start by receiving a line of text */
    if ((req->line_len = UHTTPSrv_RecvLine(req->sock, req->instance, req->line,
        UHTTPSRV_MAX_LINE_LEN)) < EOK) {
        req->state = HTTP_STATE_ERROR; return EFATAL;
    }
    
    /* last header line is empty */
    if (req->line_len == 0) {
        /* store the field type */
        field->name = HTTP_FIELD_NAME_EMPTY;
        /* determine the next state */
        req->state = req->body_bleft ? HTTP_STATE_READ_BODY : 
            HTTP_STATE_SEND_STATUS;

        /* list of fields that are required for websocket connection */
        static const uhttp_field_name_mask_t ws_fields_req =
            HTTP_FIELD_MASK_HOST |
            HTTP_FIELD_MASK_SEC_WS_VERSION |
            HTTP_FIELD_MASK_SEC_WS_KEY |
            HTTP_FIELD_MASK_CONNECTION |
            HTTP_FIELD_MASK_UPGRADE;


        /* maybe it's the websocket type of request? */
        if (req->method == HTTP_METHOD_GET && req->body_bleft == 0 &&
            req->ws_fields == ws_fields_req) {
                req->type = HTTP_REQ_TYPE_WEBSOCKET;
        /* all other requests are assumed to be standard http requests */
        } else {
            req->type = HTTP_REQ_TYPE_STANDARD;
        }

    /* line with content */
    } else {
        /* this only checks if the syntax is valid, if it can parse 
        * then it parses. if the field is unknown then it will still 
        * return EOK. only syntax error cause it to fail */
        if (UHTTPSrv_ParseFieldLine(req->line, req->line_len, field) < EOK)
            return req->state = HTTP_STATE_ERROR;
        
        /* use the paremeters */
        switch (field->name) {
        /* size of the message body - we need to consume it to read 
        * to the request end */
        case HTTP_FIELD_NAME_CONTENT_LENGTH:
            req->body_bleft = field->value.i; break;

        /** websocket fields parsing */
        /* got the header that indicates that we switch connection mode to
         * websocket */
        case HTTP_FIELD_NAME_CONNECTION: {
            if (strcicmp(field->value.s, "Upgrade") == 0)
                req->ws_fields |= HTTP_FIELD_MASK_CONNECTION;
        } break;
        /* we are upgrading to a websocket */
        case HTTP_FIELD_NAME_UPGRADE: {
            if (strcicmp(field->value.s, "websocket") == 0)
                req->ws_fields |= HTTP_FIELD_MASK_UPGRADE;
        } break;
        /* this is the version that we support */
        case HTTP_FIELD_NAME_SEC_WS_VERSION: {
            if (field->value.i == 13)
                req->ws_fields |= HTTP_FIELD_MASK_SEC_WS_VERSION;
        } break;
        /* host field is needed in case of websocket connections */
        case HTTP_FIELD_NAME_HOST: {
            if (strlen(field->value.s) != 0)
                req->ws_fields |= HTTP_FIELD_MASK_HOST;
        } break;
        /* connection key value */
        case HTTP_FIELD_NAME_SEC_WS_KEY: {
            if (strlen(field->value.s) == 24) {
                /* store it within the request */
                strncpy(req->ws_key, field->value.s, sizeof(req->ws_key));
                /* mark as present */
                req->ws_fields |= HTTP_FIELD_MASK_SEC_WS_KEY;
            }
        } break;

        /* unknown parameter */
        default: break;
        }
    }

    /* return status */
    return EOK;
}

/* read the message body */
err_t UHTTPSrv_ReadBody(uhttp_request_t *req, void *ptr, size_t size)
{
    /* data pointers */
    uint8_t *p8, tmp[16]; size_t brcvd = 0;

    /* wrong state */
    if (req->state != HTTP_STATE_READ_BODY)
        req->state = HTTP_STATE_ERROR;

    /* already at an error state s*/
    if (req->state == HTTP_STATE_ERROR)
        return req->state;

    /* limit the size that we are able to read */
    if (!(size = min(size, req->body_bleft)))
        return EOK;

    /* do the repeated reads until we receive as much as we need */
    for (p8 = ptr, brcvd = 0; brcvd < size; ) {
        /* try to read the data */
        err_t ec = TCPIPTcpSock_Recv(req->sock, ptr ? p8 : tmp,
            ptr ? size - brcvd : min(sizeof(tmp), size - brcvd), 
            req->instance->timeout);
        /* error during read */
        if (ec < EOK)
            return (req->state = HTTP_STATE_ERROR);
        /* move the stuff around */
        brcvd += ec; p8 += ec;
    }

    /* reduce the number of bytes left */
    req->body_bleft -= brcvd;
    /* body reading is done */
    if (!req->body_bleft)
        req->state = HTTP_STATE_SEND_STATUS;
    /* return the error code */
    return brcvd;
}

/* send the response status information */
err_t UHTTPSrv_SendStatus(uhttp_request_t *req, uhttp_status_code_t code, 
    size_t res_size)
{
    /* error code */
    err_t ec = req->state;

    /* already at an error state */
    if (req->state == HTTP_STATE_ERROR)
        return req->state;

    /* send the status line */
    if ((ec = UHTTPSrv_SendStatusLine(req->sock, req->instance,
        HTTP_VER_1V1, code)) < EOK)
        return req->state = HTTP_STATE_ERROR;
    
    /* move to the next state, otherwise the following methods will fail */
    req->state = HTTP_STATE_SEND_FIELDS;
    /* follow with some basic fields */
    UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_CONTENT_LENGTH, res_size);
    UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_SERVER, "uHTTP");
    UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_CONNECTION, "keep-alive");
    UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_ORIGIN, 
        "*");

    /* store the amount of data that we are about to send */
    req->resp_bleft = res_size;
    req->state = HTTP_STATE_SEND_FIELDS;
    /* return the error code */
    return req->state == HTTP_STATE_ERROR ? EFATAL : EOK;
}

/* send a field line */
err_t UHTTPSrv_SendHeaderField(uhttp_request_t *req, 
    enum uhttp_field_name name, ...)
{
    /* variable arguments list */
    va_list args; err_t ec;
    /* placeholders */
    char line[256];

    /* wrong state */
    if (req->state != HTTP_STATE_SEND_FIELDS)
        req->state = HTTP_STATE_ERROR;
    /* already at an error state */
    if (req->state == HTTP_STATE_ERROR)
        return req->state;

    /* map the list */
    va_start(args, name);
    /* render the line */
    ec = UHTTPSrv_RenderFieldLine(line, sizeof(line), name, args);
    /* drop argument list */
    va_end(args);

    /* unable to render the line? unable to push the data through tcp? */
    if (ec < EOK || (ec = TCPIPTcpSock_Send(req->sock, line, ec,
        req->instance->timeout)) < EOK)
        return req->state = HTTP_STATE_ERROR;
    
    /* send the line */
    return EOK;
}

/* end response header */
err_t UHTTPSrv_EndHeader(uhttp_request_t *req)
{
    /* wrong state */
    if (req->state != HTTP_STATE_SEND_FIELDS)
        req->state = HTTP_STATE_ERROR;
    /* already at an error state */
    if (req->state == HTTP_STATE_ERROR)
        return req->state;
    
    /* send a line */
    if (UHTTPSrv_SendEmptyLine(req->sock, req->instance) < EOK) {
        req->state = HTTP_STATE_ERROR; return EFATAL;
    }
    /* next state depends on the fact if we are about to send the body of 
     * the message */
    req->state = req->resp_bleft ? HTTP_STATE_SEND_BODY : HTTP_STATE_DONE;
    /* end the header with an empty line */
    return EOK;
}

/* send body in response to the http request */
err_t UHTTPSrc_SendBody(uhttp_request_t *req, const void *ptr, size_t size)
{
    /* wrong state */
    if (req->state != HTTP_STATE_SEND_BODY)
        req->state = HTTP_STATE_ERROR;
    /* already at an error state */
    if (req->state == HTTP_STATE_ERROR)
        return req->state;
    /* nothing to send */
    if (!size)
        return EOK;
    /* trying to push too many bytes */
    if (size > req->resp_bleft)
        return EFATAL;
    
    /* send the data over the socket */
    err_t ec = TCPIPTcpSock_Send(req->sock, ptr, size,
        req->instance->timeout);
    /* error during sending? */
    if (ec < EOK)
        return req->state = HTTP_STATE_ERROR;
    
    /* reduce the number of bytes left */
    req->resp_bleft -= ec;
    /* we are done sending data */
    if (!req->resp_bleft)
        req->state = HTTP_STATE_DONE;
    /* return the number of bytes sent */
    return ec;
}
