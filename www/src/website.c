/**
 * @file uhttpsrv_instance.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "ffs/ffs.h"
#include "util/string.h"
#include "net/uhttpsrv/uhttpsrv.h"
#include "util/string.h"
#include "util/elems.h"

#define DEBUG
#include "debug.h"


/* derive the mime type based on the file extension */
static const char * HTTPSrvWebsite_GetMimeTypeForFileExt(const char *ext)
{
    /* default mime */
    static const char *def = "text/plain";
    /* look up table for mime types and file extensions for them */
    static const struct {
        const char *mime_type, *ext;
    } *l, lut[] = {
        { "text/html", ".htm" },
        { "text/html", ".html" },
        { "text/javascript", ".js" },
        { "text/css", ".css" },
        { "image/svg+xml", ".svg" },
        { "image/jpeg", ".jpg" },
        { "image/jpeg", ".jpeg" },
        { "image/png", ".png" },
    };

    /* no extension or an empty string? */
    if (!ext || !*ext)
        return def;

    /* move to the end of the string since we are about to compare extensions */
    for (; *ext; ext++);
    /* now move back to the last '.' char */
    for (; *ext != '.'; ext--);

    /* go through the look up table */
    for (l = lut; l != lut + elems(lut); l++)
        if (strcmp(ext, l->ext) == 0)
            return l->mime_type;

    /* whops */
    return def;
}

/* handle requests */
static err_t HTTPSrvWebsite_Callback(struct uhttp_request *req)
{
    /* default status code */
    uhttp_status_code_t err_code = HTTP_STATUS_200_OK;
     /* default file name to be served */
    const char *fname = req->url;

    /* file pointer */
    ffs_file_t *fp = 0;
    /* file size */
    size_t fsize;
    /* buffer for transferrinng the file */
    uint8_t fbuf[64]; int fbuf_size;

    /* unsupported method */
    if (req->method != HTTP_METHOD_GET)
        err_code = HTTP_STATUS_405_METHOD_NOT_ALLOWED;

    /* REQUEST PART */
    /* read the header fields from the request header */
    for (uhttp_field_t field; req->state == HTTP_STATE_READ_FIELDS && 
        UHTTPSrv_ReadHeaderField(req, &field) >= EOK; ) {
        /* let's check if client accepts encoding. we need to check in that 
         * way since many methods may be enlisted in accept-encoding field 
         * value */
        if (field.name == HTTP_FIELD_NAME_ACCEPT_ENCODING)
            if (!strstr(field.value.s, "gzip"))
                return EFATAL;
    }
    /* drop the entire body */
    if (req->state == HTTP_STATE_READ_BODY)
        UHTTPSrv_ReadBody(req, 0, -1);


    /* OPEN FILE ACCORDING TO URL */
    /* substitute 'index.html' for the '/' path */
    if (strcmp("/", req->url) == 0)
        fname = "/index.html";
    /* try to open the file */
    if (!(fp = FFS_Open(fname, FFS_MODE_R)))
        err_code = HTTP_STATUS_404_NOT_FOUND;
    /* get the file size */
    if (fp && FFS_Size(fp, &fsize) < EOK)
        err_code = HTTP_STATUS_500_INTERNAL_SRV_ERR;
    

    /* RESPONSE HEADER part */
    /* send status of the response do not send file contents if an error has 
     * occured */
    UHTTPSrv_SendStatus(req, err_code, 
        fsize = (err_code != HTTP_STATUS_200_OK ? 0 : fsize));
    /* send all header fields */
    UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_ACCESS_CONTROL_ALLOW_ORIGIN, 
        "*");
    UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_CONNECTION, "keep-alive");
    /* this is a naive way of telling if the file is gzipped. first we check for 
     * the magic number 0x1f8b and then we check for the algorithm (which is 
     * expected to be DEFLATE denoted by 0x08) */
    if (FFS_Read(fp, fbuf, 3) == 3) {
        /* check the signature */
        if (fbuf[0] == 0x1f && fbuf[1] == 0x8b && fbuf[2] == 0x08)
            UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_CONTENT_ENCODING, 
                "gzip");
        /* rewind the file */
        FFS_Seek(fp, 0, FFS_SEEK_SET);
    }

    /* get the mime type */
    const char *mime_type = HTTPSrvWebsite_GetMimeTypeForFileExt(fname);
    /* put it into the header */
    UHTTPSrv_SendHeaderField(req, HTTP_FIELD_NAME_CONTENT_TYPE, mime_type);

    /* we are done with the headers */
    if (UHTTPSrv_EndHeader(req) != EOK)
        goto end;

    /* RESPONSE DATA part */
    /* read the file and put it into the response */
    for (; fsize; fsize -= fbuf_size) {
        /* read a chunk of the data */
        if ((fbuf_size = FFS_Read(fp, fbuf, sizeof(fbuf))) <= 0)
            break;
        /* send the data */
        if (UHTTPSrc_SendBody(req, fbuf, fbuf_size) < EOK)
            return EFATAL;
    }

    /* close the file */
    end: FFS_Close(fp);
    /* return the status */
    return EOK;
}

/* create a server instance for testing */
err_t HTTPSrvWebsite_Init(void)
{
    /* we need to ensure that this object is kept around after we exit 
     * this function */
    static uhttp_instance_t instance = {
        .port = 80,
        .timeout = 1000,
        .callback = HTTPSrvWebsite_Callback
    };
    /* start the server */
    return UHTTPSrv_InstanceInit(&instance);
}