/**
 * @file uhttpsrv.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief
 * @date 2024-07-18
 * 
 * @copyright Copyright (c) 2024
 *
 */

#ifndef NET_UHTTPSRV_UHTTPSRV_H
#define NET_UHTTPSRV_UHTTPSRV_H


#include "err.h"
#include "net/tcpip/tcp_sock.h"
#include "util/bit.h"
#include "sys/sem.h"

/** method encoding enum */
typedef enum uhttp_method {
    HTTP_METHOD_UNKNOWN,
    HTTP_METHOD_GET = BIT_VAL(0),
    HTTP_METHOD_HEAD = BIT_VAL(1),
    HTTP_METHOD_POST = BIT_VAL(2),
    HTTP_METHOD_PUT = BIT_VAL(3),
    HTTP_METHOD_DELETE = BIT_VAL(4),
    HTTP_METHOD_CONNECT = BIT_VAL(5),
    HTTP_METHOD_OPTIONS = BIT_VAL(6),
    HTTP_METHOD_TRACE = BIT_VAL(7),
    HTTP_METHOD_PATCH = BIT_VAL(8)
} uhttp_method_t;

/** status codes */
typedef enum uhttp_status_code {
    HTTP_STATUS_UNKNOWN,
    HTTP_STATUS_200_OK,
    HTTP_STATUS_101_SWITCHING_PROTOCOLS,
    HTTP_STATUS_400_BAD_REQUEST,
    HTTP_STATUS_404_NOT_FOUND,
    HTTP_STATUS_405_METHOD_NOT_ALLOWED,
    HTTP_STATUS_500_INTERNAL_SRV_ERR,
} uhttp_status_code_t;

/** field enum that encodes the name */
typedef enum uhttp_field_name {
    HTTP_FIELD_NAME_UNKNOWN,
    HTTP_FIELD_NAME_EMPTY,
    HTTP_FIELD_NAME_CONTENT_LENGTH,
    HTTP_FIELD_NAME_SERVER,
    HTTP_FIELD_NAME_HOST,
    HTTP_FIELD_NAME_CONTENT_ENCODING,
    HTTP_FIELD_NAME_ACCEPT_ENCODING,
    HTTP_FIELD_NAME_CONNECTION,
    HTTP_FIELD_NAME_CONTENT_TYPE,
    HTTP_FIELD_NAME_ORIGIN,
    HTTP_FIELD_NAME_ALLOW,
    HTTP_FIELD_NAME_ACCESS_CONTROL_REQUEST_METHOD,
    HTTP_FIELD_NAME_ACCESS_CONTROL_REQUEST_HEADERS,
    HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_ORIGIN,
    HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_METHODS,
    HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_HEADERS,
    HTTP_FIELD_NAME_UPGRADE,
    HTTP_FIELD_NAME_SEC_WS_KEY,
    HTTP_FIELD_NAME_SEC_WS_PROTOCOL,
    HTTP_FIELD_NAME_SEC_WS_VERSION,
    HTTP_FIELD_NAME_SEC_WS_ACCEPT
} uhttp_field_name_t;

/* field name bitmask */
typedef enum uhttp_field_name_mask {
    HTTP_FIELD_MASK_UNKNOWN
        = BIT_VAL(HTTP_FIELD_NAME_UNKNOWN),
    HTTP_FIELD_MASK_EMPTY
        = BIT_VAL(HTTP_FIELD_NAME_EMPTY),
    HTTP_FIELD_MASK_CONTENT_LENGTH
        = BIT_VAL(HTTP_FIELD_NAME_CONTENT_LENGTH),
    HTTP_FIELD_MASK_SERVER
        = BIT_VAL(HTTP_FIELD_NAME_SERVER),
    HTTP_FIELD_MASK_HOST
        = BIT_VAL(HTTP_FIELD_NAME_HOST),
    HTTP_FIELD_MASK_CONTENT_ENCODING
        = BIT_VAL(HTTP_FIELD_NAME_CONTENT_ENCODING),
    HTTP_FIELD_MASK_ACCEPT_ENCODING
        = BIT_VAL(HTTP_FIELD_NAME_ACCEPT_ENCODING),
    HTTP_FIELD_MASK_CONNECTION
        = BIT_VAL(HTTP_FIELD_NAME_CONNECTION),
    HTTP_FIELD_MASK_CONTENT_TYPE
        = BIT_VAL(HTTP_FIELD_NAME_CONTENT_TYPE),
    HTTP_FIELD_MASK_ORIGIN
        = BIT_VAL(HTTP_FIELD_NAME_ORIGIN),
    HTTP_FIELD_MASK_ALLOW
        = BIT_VAL(HTTP_FIELD_NAME_ALLOW),
    HTTP_FIELD_MASK_ACCESS_CONTROL_REQUEST_METHOD
        = BIT_VAL(HTTP_FIELD_NAME_ACCESS_CONTROL_REQUEST_METHOD),
    HTTP_FIELD_MASK_ACCESS_CONTROL_REQUEST_HEADERS
        = BIT_VAL(HTTP_FIELD_NAME_ACCESS_CONTROL_REQUEST_HEADERS),
    HTTP_FIELD_MASK_ACCESS_CONTROL_ALLOW_ORIGIN
        = BIT_VAL(HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_ORIGIN),
    HTTP_FIELD_MASK_ACCESS_CONTROL_ALLOW_METHODS
        = BIT_VAL(HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_METHODS),
    HTTP_FIELD_MASK_ACCESS_CONTROL_ALLOW_HEADERS
        = BIT_VAL(HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_HEADERS),
    HTTP_FIELD_MASK_UPGRADE
        = BIT_VAL(HTTP_FIELD_NAME_UPGRADE),
    HTTP_FIELD_MASK_SEC_WS_KEY
        = BIT_VAL(HTTP_FIELD_NAME_SEC_WS_KEY),
    HTTP_FIELD_MASK_SEC_WS_PROTOCOL
        = BIT_VAL(HTTP_FIELD_NAME_SEC_WS_PROTOCOL),
    HTTP_FIELD_MASK_SEC_WS_VERSION
        = BIT_VAL(HTTP_FIELD_NAME_SEC_WS_VERSION),
    HTTP_FIELD_MASK_SEC_WS_ACCEPT
        = BIT_VAL(HTTP_FIELD_NAME_SEC_WS_ACCEPT)
} uhttp_field_name_mask_t;

/* header field value */
typedef struct uhttp_field {
    /* field name */
    enum uhttp_field_name name;
    /* field name in string format */
    const char *name_str;
    /* length of the field name */
    size_t name_str_len;
    /* possible values */
    union uhttp_field_value {
        int i; float f; const char *s;
    } value;
} uhttp_field_t;

/* websocket structure */
typedef struct uhttp_ws {
    /* is the connection open? */
    int is_open;

    /* semaphores that guard the usage of the tcp socket */
    sem_t tx_sem, rx_sem;

    /* current frame data size and read offset */
    size_t rx_size, rx_offs;
    /* opcode of the frame being currently processed */
    uint16_t rx_opcode;
    /* socket mask */
    union { uint8_t u8[4]; uint32_t u32; } mask;

} uhttp_ws_t;

/** http callback argument */
typedef struct uhttp_request {
    /* server instance */
    struct uhttp_instance *instance;
    /* method */
    enum uhttp_method method;
    /* url related fields */
    const char *url;

    /* socket over which the communication takes place */
    tcpip_tcp_sock_t *sock;

    /* request type */
    enum uhttp_req_type {
        HTTP_REQ_TYPE_STANDARD,
        HTTP_REQ_TYPE_WEBSOCKET,
    } type;

    /* current request error code */
    enum uhttp_state {
        HTTP_STATE_ERROR = -1,
        HTTP_STATE_READ_FIELDS,
        HTTP_STATE_READ_BODY,
        HTTP_STATE_SEND_STATUS,
        HTTP_STATE_SEND_FIELDS,
        HTTP_STATE_SEND_BODY,
        HTTP_STATE_DONE,
    } state;

    /* current request line */
    char *line;
    /* length of the line */
    size_t line_len;

    /* body bytes left */
    size_t body_bleft;
    /* response bytes left */
    size_t resp_bleft;


    /** websocket support */
    /* websocket fields received in the request */
    uhttp_field_name_mask_t ws_fields;
    /* websocket connection key value */
    char ws_key[24];
    /* websocket itself */
    uhttp_ws_t ws;


} uhttp_request_t;

/** instance definition */
typedef struct uhttp_instance {
    /* port on which we listen */
    tcpip_tcp_port_t port;

    /* timeout for the send and receive functions */
    dtime_t timeout;
    /* maximal number of simultaneous connections */
    int max_connections;
    /* stack size for the serving task */
    size_t stack_size;
    /* callback function */
    err_t (*callback) (struct uhttp_request *req);

    /* socket control functions */
    struct uhttp_instance_sock_funcs {
        /* create a socket and return a pointer to it */
        void* (*create) (void);
        /* start listening on given port */
        err_t (*listen) (void *sock, int port);
        /* receive data from the socket */
        err_t (*recv) (void *sock, void *ptr, size_t size, dtime_t timeout);
        /* send data through the socket */
        err_t (*send) (void *sock, const void *ptr, size_t size, dtime_t timeout);
        /* close the connection */
        err_t (*close) (void *sock);
    } sock_funcs;

} uhttp_instance_t;


/**
 * @brief initiate the server
 *
 * @return err_t error code
 */
err_t UHTTPSrv_Init(void);

/**
 * @brief initialize the instance of the server
 *
 * @param inst instance struct with configuration parameters
 *
 * @return err_t error code
 */
err_t UHTTPSrv_InstanceInit(uhttp_instance_t *inst);

/**
 * @brief read the field from the header
 *
 * @param req request that we are processing
 * @param field field placeholder
 *
 * @return err_t error code
 */
err_t UHTTPSrv_ReadHeaderField(uhttp_request_t *req, uhttp_field_t *field);

/**
 * @brief Callback method: reads a chunk of message body
 *
 * @param req request that we are processing
 * @param ptr pointer to where to store the body part
 * @param size size of the chunk that we wish to read
 *
 * @return err_t error code
 */
err_t UHTTPSrv_ReadBody(uhttp_request_t *req, void *ptr, size_t size);

/**
 * @brief send the response status information
 *
 * @param req request that we are processing
 * @param code status code that one whishes to return
 * @param res_size size of the response data
 *
 * @return err_t error code
 */
err_t UHTTPSrv_SendStatus(uhttp_request_t *req, uhttp_status_code_t code,
    size_t res_size);

/**
 * @brief send a field line after the status
 *
 * @param req request to which we are responding
 * @param name name of the field
 * @param ...
 *
 * @return err_t
 */
err_t UHTTPSrv_SendHeaderField(uhttp_request_t *req,
    enum uhttp_field_name name, ...);

/**
 * @brief end response header. to be called after all response
 * fields were sent
 *
 * @param req request to which we are responding
 *
 * @return err_t error code
 */
err_t UHTTPSrv_EndHeader(uhttp_request_t *req);

/**
 * @brief send data in response to the http request
 *
 * @param req request that we are processing
 * @param ptr pointer to the data chunk
 * @param size size of the chunk to be sent
 *
 * @return err_t error code
 */
err_t UHTTPSrc_SendBody(uhttp_request_t *req, const void *ptr, size_t size);

#endif /* NET_UHTTPSRV_UHTTPSRV_H */
