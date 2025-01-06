/**
 * @file uhttpsrv_api.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "dev/led.h"
#include "ffs/ffs.h"
#include "net/uhttpsrv/uhttpsrv.h"
#include "util/elems.h"
#include "util/stdio.h"
#include "util/string.h"

#define DEBUG
#include "debug.h"

/* specification of an endpoint */
typedef struct endpoint_spec {
    /* api url */
    const char *url;
    /* allowed methods */
    uhttp_method_t methods;
} endpoint_spec_t;

/* method specifiers */
typedef struct method_spec {
    /* method flag */
    uhttp_method_t method;
    /* string name of the method as used in headers */
    const char *name;
} method_spec_t;


/* get the specification of supported api methods*/
static const method_spec_t * HTTPSrvApi_GetMethodSpec(
    uhttp_method_t method, const char *name)
{
    /* specification of supported endpoints */
    static const method_spec_t *l, lut[] = {
        { HTTP_METHOD_POST, "POST" },
        { HTTP_METHOD_GET, "GET" },
        { HTTP_METHOD_OPTIONS, "OPTIONS" },
    };

    /* look for spec for this endpoint */
    for (l = lut; l != lut + elems(lut); l++) {
        /* compare by method */
        if (method && method == l->method) return l;
        /* compare by string name */
        if (name && strcicmp(l->name, name) == 0) return l;
    }

    /* nothing was found */
    return 0;
}

/* get the specification of supported api endpoints */
static const endpoint_spec_t * HTTPSrvApi_GetEnpointSpec(const char *url)
{
    /* specification of supported endpoints */
    static const endpoint_spec_t *l, lut[] = {
        { "/", HTTP_METHOD_POST | HTTP_METHOD_OPTIONS },
    };

    /* look for spec for this endpoint */
    for (l = lut; l != lut + elems(lut); l++)
        if (strcmp(l->url, url) == 0)
            return l;

    /* nothing was found */
    return 0;
}

/* process the post request */
static uhttp_status_code_t HTTPSrvApi_ProcessPost(uhttp_request_t *req,
    const endpoint_spec_t *es)
{
    /* payload data */
    char data[256]; int data_len;

    /* consume the fields if any */
    for (uhttp_field_t f; req->state == HTTP_STATE_READ_FIELDS; 
        UHTTPSrv_ReadHeaderField(req, &f));
    
    /* read the data */
    if ((data_len = UHTTPSrv_ReadBody(req, data, sizeof(data))) < EOK)
        return HTTP_STATUS_400_BAD_REQUEST;
    /* zero terminate if needed */
    /* process the command */
    err_t ec = EFATAL;// TODO: at command interface PMUXRxTxMem_Process(data, data_len, data, sizeof(data));
    /* fuck! */
    if (ec < EOK)
        return HTTP_STATUS_400_BAD_REQUEST;

    /* respond with the status */
    UHTTPSrv_SendStatus(req, HTTP_STATUS_200_OK, ec);
    UHTTPSrv_EndHeader(req);
    /* send the data */
    UHTTPSrc_SendBody(req, data, ec);

    /* report happy end */
    return EOK;
}

/* handle options requests */
static uhttp_status_code_t HTTPSrvApi_ProcessOptions(uhttp_request_t *req,
    const endpoint_spec_t *es)
{
    /* consume all the headers */
    for (uhttp_field_t field; req->state == HTTP_STATE_READ_FIELDS &&
        UHTTPSrv_ReadHeaderField(req, &field) >= EOK;);

    /* response time */
    /* time to prepare the response */
    UHTTPSrv_SendStatus(req, HTTP_STATUS_200_OK, 0);
    
    /* let's build the allowed method string */
    char methods_str[64] = ""; int offs = 0;
    /* go through all flags */
    for (uhttp_method_t m = 1, em = es->methods; em; m = m << 1) {
        /* method is not supported */
        if ((em & m) == 0) 
            continue;
        /* clear out the flag */
        em &= ~m;
        /* append to the list of methods */
        offs += snprintf(methods_str + offs, sizeof(methods_str) - offs - 1, 
            "%s%s", HTTPSrvApi_GetMethodSpec(m, 0)->name, em ? ", " : "");
    }

    /* send the field */
    UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_METHODS, 
        methods_str);
    /* send the allowed headers */
    UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_HEADERS, 
        "content-type");
    /* end the frame */
    UHTTPSrv_EndHeader(req);

    /* we've already sent the response, no need for the underlying 
     * layer to do so */
    return HTTP_STATUS_UNKNOWN;
}

/* handle requests */
static err_t HTTPSrvApi_ProcessRequest(uhttp_request_t *req)
{
    /* status code with which we respond */
    uhttp_status_code_t sc = HTTP_STATUS_200_OK;

    /* get the endpoint specification for requested endpoint */
    const endpoint_spec_t *es = HTTPSrvApi_GetEnpointSpec(req->url);
    /* method not supported */
    if (!es)
        sc = HTTP_STATUS_404_NOT_FOUND;
    /* options method is not supported */
    if (es && !(es->methods & req->method))
        sc = HTTP_STATUS_405_METHOD_NOT_ALLOWED;
    /* handle error generation */
    if (sc != HTTP_STATUS_200_OK)
        goto send_status;

    /* process method */
    switch (req->method) {
    /* http options method, used annd abused by javascript 'axios' */
    case HTTP_METHOD_OPTIONS: 
        sc = HTTPSrvApi_ProcessOptions(req, es); break;
    /* http post method */
    case HTTP_METHOD_POST: 
        sc = HTTPSrvApi_ProcessPost(req, es); break;
    /* please add support for more methods */
    default: return EFATAL;
    }

    /* if instead of the status code we are given the EOK then it means that 
     * underlying routine have already sent the http status response */
    if (sc == HTTP_STATUS_UNKNOWN)
        return EOK;
    
    /* handle generating proper error response */
    send_status : {
        /* consume the fields if any */
        for (uhttp_field_t f; req->state == HTTP_STATE_READ_FIELDS; 
            UHTTPSrv_ReadHeaderField(req, &f));
        /* drop the body */
        if (req->state == HTTP_STATE_READ_BODY) 
            UHTTPSrv_ReadBody(req, 0, -1);
        
        /* respond with the status */
        UHTTPSrv_SendStatus(req, sc, 0);
        /* end the request */
        UHTTPSrv_EndHeader(req);
    }

    /* report status */
    return EOK;
}

/* create a server instance for testing */
err_t HTTPSrvApi_Init(void)
{
    /* we need to ensure that this object is kept around after we exit 
     * this function */
    static uhttp_instance_t instance = {
        .port = 6969,
        .timeout = 2000,
        .callback = HTTPSrvApi_ProcessRequest
    };

    /* start the server */
    return UHTTPSrv_InstanceInit(&instance);
}