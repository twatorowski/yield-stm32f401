/**
 * @file esp.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-06
 *
 * @copyright Copyright (c) 2025
 */

#include "assert.h"
#include "err.h"
#include "dev/gpio.h"
#include "dev/gpio_signals.h"
#include "dev/usart.h"
#include "dev/usart_dev.h"
#include "dev/seed.h"
#include "sys/yield.h"
#include "sys/sleep.h"
#include "sys/ev.h"

#include "sys/queue.h"
#include "util/string.h"
#include "util/minmax.h"
#include "util/elems.h"
#include "util/forall.h"

#include <stdarg.h>

#define DEBUG DLVL_INFO
#include "debug.h"

/* line parsing modes */
typedef enum line_modes {
    RX_LINE,
    RX_IPD,
    RX_CIPRECVDATA,
    RX_BIN_DATA,
    RX_PROMPT
} line_modes_t;

/* security type */
typedef enum esp_ap_sec : int {
    ESP_AP_SEC_OPEN = 0,
    ESP_AP_SEC_WEP,
    ESP_AP_SEC_WPA_PSK,
    ESP_AP_SEC_WPA2_PSK,
    ESP_AP_SEC_WPA_WPA2_PSK,
} esp_ap_sec_t;

/* connection error codes */
typedef enum esp_conn_error_code : int {
    ESP_CONN_ERROR_CODE_UNKNOWN = 0,
    ESP_CONN_ERROR_CODE_TIMEOUT = 1,
    ESP_CONN_ERROR_CODE_PASSWORD = 2,
    ESP_CONN_ERROR_CODE_AP_NOT_FOUND = 3,
    ESP_CONN_ERROR_CODE_FAIL = 4,
} esp_conn_error_code_t;

/* wifi connection status */
typedef enum esp_wifi_status : int {
    ESP_WIDI_STATUS_GOT_IP = 2,
    ESP_WIDI_STATUS_CONNECTEED = 3,
    ESP_WIDI_STATUS_DISCONNECTED = 4,
    ESP_WIDI_STATUS_CONN_FAILED = 5,
} esp_wifi_status_t;


/* type of the protocol that we are running */
typedef enum esp_link_prot : int {
    ESP_LINK_PROT_TCP,
    ESP_LINK_PROT_UDP,
    ESP_LINK_PROT_SSL,
    ESP_LINK_PROT_UNKNOWN = -1,
} esp_link_prot_t;

/* link role */
typedef enum esp_link_role : int {
    ESP_LINK_ROLE_CLI,
    ESP_LINK_ROLE_SRV,
} esp_link_role_t;


/* connection status */
typedef struct esp_tcpip_link_status {
    /* connection slot number (0-4) */
    int link_id;
    /* remote ip address */
    char remote_ip[16];
    /* remote port number */
    uint16_t remote_port;
    /* local port number */
    uint16_t local_port;
    /* type of the protocol that we are running */
    esp_link_prot_t protocol;
    /* link role */
    esp_link_role_t role;
} esp_tcpip_link_status_t;

/* access point entry */
typedef struct esp_ap {
    /* type of the security used */
    esp_ap_sec_t sec;
    /* ssid name, and mac address */
    char ssid[24], mac[18];
    /* signal strength */
    int rssi;
} esp_ap_t;

/* size of data */
typedef struct esp_sock {
    /* pointer to the device descriptor itself */
    struct esp_dev *dev;
    /* socket states */
    enum esp_sock_state {
        ESP_SOCK_STATE_FREE,
        ESP_SOCK_STATE_CLOSED,
        ESP_SOCK_STATE_LISTEN,
        ESP_SOCK_STATE_CONNECTING,
        ESP_SOCK_STATE_OPEN
    } state;

    /* is this a server side connection? */
    esp_link_role_t role;
    /* pointer to the link that this socket is bound to */
    struct esp_dev_link *link;
    /* id of the link that this socket is bound to */
    int link_id;
    /* reception queue */
    queue_t *rxq;

    /* header of the currently received udp frame */
    struct esp_sock_udphdr {
        /* size of the data */
        size_t size;
        /* source address */
        uint32_t ip;
        /* source port */
        uint16_t port;
    } udp_hdr;

} esp_sock_t;

/* device descriptor */
typedef struct esp_dev {
    /* usart device */
    usart_dev_t *usart;
    /* reset pin */
    gpio_signal_t reset;
    /* communication baudrate */
    int baudrate;

    /* semaphore for sending commands */
    sem_t cmd_sem;
    /* current command parser state */
    enum { CMD_DONE, CMD_ACTIVE } cmd_state;
    /* command execution error code */
    err_t cmd_ec;

    /* id of the reception task */
    int rx_task_id;

    /* current response size and pointer */
    const void *rsp_ptr; size_t rsp_size;
    /* size and poiunter to the binary (data carrying part of the response )*/
    const void *rsp_bin_ptr; size_t rsp_bin_size;

    /* wifi state */
    int wifi_connected;

    /* semaphore for establishing connections */
    sem_t sock_sem;
    /* socket bound to this device */
    struct esp_sock sockets[5];

    /* if the module will run the tcp server then this number will
     * store the port on which we listen */
    uint16_t server_port;

    /* connection slots */
    struct esp_dev_link {
        /* is this connection slot being used */
        int used, connected;

        /* remote ip address */
        uint32_t remote_ip;
        /* remote port number */
        uint16_t remote_port;
        /* local port number */
        uint16_t local_port;
        /* connection role, server or client */
        enum esp_link_role role;
        /* protocol over which we communicate */
        enum esp_link_prot prot;

        /* socket that handles that connection */
        struct esp_sock *socket;

        /* size of the data that is still buffered on the esp itself (only valid for )*/
        size_t esp_data_size;
        /* reception event */
        ev_t rx_ev;
    } links[5];

} esp_dev_t;

/* ip address representations */
typedef union esp_ip_addr {
    /* 8 bit number representation */
    uint8_t u8[4];
    /* 32-bit word representation */
    uint32_t u32;
} esp_ip_addr_t;

/** maximal length of the command line */
#define ESP_MAX_LINE_LEN        256
/** enline ending */
#define ESP_LINE_END            "\r\n"



/* do a chip reset */
static err_t ESP_ResetChip(esp_dev_t *dev)
{
    /* reset pin was not set up */
    if (!dev->reset.gpio)
        return EFATAL;

    /* configure pin as output and drive it down */
    GPIOSig_CfgOutput(dev->reset, GPIO_OTYPE_PP, 0); Sleep(5);
    /* release the pin */
    GPIOSig_Set(dev->reset, 1); Sleep(5);

    /* report status */
    return EOK;
}

/* set the baudrate of the underlying interface */
static err_t ESP_SetBaudrate(esp_dev_t *dev, int bauds)
{
    /* reconfigure usart */
    return USART_SetBaudrate(dev->usart, bauds);
}

/* send data via esp interface */
static err_t ESP_Send(esp_dev_t *dev, const void *ptr, size_t size)
{
    /* do a send on uart */
    return USART_Send(dev->usart, ptr, size, 0);
}

/* receive data via the usart interface */
static err_t ESP_Recv(esp_dev_t *dev, void *ptr, size_t size)
{
    /* do a read on the uart */
    return USART_Recv(dev->usart, ptr, size, 0);
}

/* convert the string notation to 32 bit number notation */
static uint32_t ESP_IpStrToIp32(const char *str, uint32_t *ip32)
{
    /* parts of an ip address */
    uint32_t a, b, c, d;

    /* unable to convert */
    if (sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
        return EFATAL;

    /* store the value */
    if (ip32)
        *ip32 = a | b << 8 | c << 16 | d << 24;
    /* return the address */
    return EOK;
}

/* get the protocol enum value from the string name */
static enum esp_link_prot ESP_ProtStrToProtType(const char *str)
{
    /* do the string comparisons to determine the protocol */
    if (strcmp("TCP", str) == 0) {
        return ESP_LINK_PROT_TCP;
    } else if (strcmp("UDP", str) == 0) {
        return ESP_LINK_PROT_UDP;
    } else if (strcmp("SSL", str) == 0) {
        return ESP_LINK_PROT_SSL;
    }

    /* unknown protocol */
    return ESP_LINK_PROT_UNKNOWN;
}

/* convert the enum value to string */
static const char * ESP_ProtTypeToProtStr(enum esp_link_prot prot)
{
    /* return the protocol string based on the enum value */
    switch (prot) {
    case ESP_LINK_PROT_TCP: return "TCP";
    case ESP_LINK_PROT_UDP: return "UDP";
    case ESP_LINK_PROT_SSL: return "SSL";
    default: return "";
    }
}

/* parse +link_conn: unsolicited response */
static err_t ESP_InputLinkCon(esp_dev_t *dev, const void *ptr, size_t size)
{

    /* data to be extracted from the sentence */
    int status_type, link_id, server_side; char type[5], remote_ip_str[16];
    int remote_port, local_port; uint32_t remote_ip;

    /* not the frame we are looking for */
    if (size < 10 || memcmp(ptr, "+LINK_CONN:", 11) != 0)
        return EFATAL;

    /* malformed frame */
    if (snscanf(ptr, size, "+LINK_CONN:%d,%d,\"%s\",%d,\"%s\",%d,%d",
        &status_type, &link_id, type, &server_side, remote_ip_str, &remote_port,
        &local_port) != 7)
        return EFATAL;

    /* unable to parse the ip string */
    if (ESP_IpStrToIp32(remote_ip_str, &remote_ip) != EOK)
        return EFATAL;

    /* weird shit */
    if (link_id < 0 || link_id > elems(dev->links))
        return EFATAL;

    /* get the protocol enum value */
    enum esp_link_prot prot = ESP_ProtStrToProtType(type);
    /* unknown and unsupported protocol name */
    if (prot == ESP_LINK_PROT_UNKNOWN)
        return EFATAL;

    /* get the connection block */
    struct esp_dev_link *link = &dev->links[link_id];
    /* link conn messages are not used for notifying about disconnects */
    if (status_type != 0)
        return EOK;

    /* copy the connection information */
    link->remote_port = remote_port;
    link->local_port = local_port;
    link->prot = prot;
    link->esp_data_size = 0;
    link->remote_ip = remote_ip;
    link->connected = 1;

    /* in server mode we look for sockets that are listening. in client
        * mode we look for socket that is connecting */
    enum esp_sock_state state = server_side ? ESP_SOCK_STATE_LISTEN :
        ESP_SOCK_STATE_CONNECTING;

    /* look for the socket */
    esp_sock_t *s; forall (s, dev->sockets)
        if (s->state == state)
            break;
    /* no socket was found, so simply consume the message and continue */
    if (s == arrend(dev->sockets))
        return EOK;

    /* update socket */
    s->link = link;
    s->state = ESP_SOCK_STATE_OPEN;
    /* clear the queue */
    Queue_DropAll(s->rxq);
    /* reset the udp size */
    s->udp_hdr = (struct esp_sock_udphdr){ 0 };
    /* setup socket within the link */
    link->socket = s;

    /* message consumed */
    return EOK;
}

/* parse <link_id>,CONNECTED/DISCONNECTED type of messages */
static err_t ESP_InputConnectionMsgs(esp_dev_t *dev, const void *ptr,
    size_t size)
{
    /* data parsed from the notifications */
    const uint8_t *p8 = ptr; int link_id; char status[12];

    /* check the size against the smallest of these messages */
    if (size < sizeof("0,CLOSED") - 1)
        return EFATAL;

    /* do the easy sanity checks first not to waste time */
    if (p8[1] != ',' || !isdigit(p8[0]))
        return EFATAL;

    /* extract the parameters */
    if (snscanf(ptr, size, "%d,%.*s%", &link_id, sizeof(status), status) != 2)
        return EFATAL;

    /* strange link id */
    if (link_id < 0 || link_id >= elems(dev->links))
        return EFATAL;

    /* get the connection block for this link */
    struct esp_dev_link *link = &dev->links[link_id];

    /* message says that we are connected */
    if (strcmp("CONNECT", status) == 0) {
        /* start the connection */
        link->esp_data_size = 0;
        link->connected = 1;

    } else if (strcmp("CLOSED", status) == 0) {
        /* terminate the connection */
        link->connected = 0;
        /* close associated socket */
        link->socket->state = ESP_SOCK_STATE_CLOSED;
        dprintf_i("connection %d is now closed\n", link_id);
    /* unknown message */
    } else {
        return EFATAL;
    }

    /* message is now consumed */
    return EOK;
}

/* parse <link_id>,CONNECTED/DISCONNECTED type of messages */
static err_t ESP_InputWiFiMsgs(esp_dev_t *dev, const void *ptr,
    size_t size)
{
    /* data parsed from the notifications */
    const char *p8 = ptr;

    /* check the size against the smallest of the messages */
    if (size < sizeof("WIFI GOT IP") - 1)
        return EFATAL;

    /* these messages shall always start with 'WIFI ' */
    if (*p8++ != 'W' || *p8++ != 'I' || *p8++ != 'F' || *p8++ != 'I' ||
        *p8++ != ' ')
        return EFATAL;

    /* get the size of the data that follows the 'WIFI ' */
    size_t size_rem = size - (p8 - (const char *)ptr);

    /* we are now conneted */
    if (size_rem == 9 && memcmp("CONNECTED", p8, 9) == 0) {
        dev->wifi_connected = 1;
    /* got dhcp address from the AP */
    } else if (size_rem == 6 && memcmp("GOT IP", p8, 6) == 0) {
    /* we are now disconnected */
    } else if (size_rem == 10 && memcmp("DISCONNECT", p8, 10) == 0) {
        /* wifi is no longer connected */
        dev->wifi_connected = 0;
        /* close all connections */
        forall (struct esp_dev_link *, link, dev->links) {
            /* drop the link */
            link->connected = 0;
            /* close the socket */
            if (link->socket && link->socket->state != ESP_SOCK_STATE_CLOSED) {
                link->socket->state = ESP_SOCK_STATE_CLOSED;
                link->socket->link = 0;
            }
            /* unlink from the socket */
            link->socket = 0;
        }
        /* tcp server was probably killed as well */
        dev->server_port = 0;
    /* unknown message */
    } else {
        return EFATAL;
    }

    /* message consumed */
    return EOK;
}

/* parse command status */
static err_t ESP_InputCommandStatus(esp_dev_t *dev, const void *ptr,
    size_t size)
{
    /* list of sentences that make the status lines */
    const struct cmd_stat {
        size_t size; err_t ec; const char *str;
    } *cs, statuses[] = {
        { .size = 2, .ec = EOK, .str = "OK",},
        { .size = 5, .ec = EOK, .str = "ready" },
        { .size = 5, .ec = EFATAL, .str = "ERROR" },
        { .size = 4, .ec = EFATAL, .str = "FAIL" },
        { .size = 9, .ec = EFATAL, .str = "busy p..." },
        { .size = 9, .ec = EFATAL, .str = "busy s..." },
    };

    /* look for a match with our little database of possible responses */
    forall (cs, statuses)
        if (size == cs->size && memcmp(ptr, cs->str, cs->size) == 0)
            break;
    /* could not find a match */
    if (cs == arrend(statuses))
        return EFATAL;

    /* no command is currently ongoing but we can swallow the status anyway */
    if (dev->cmd_state != CMD_ACTIVE)
        return EOK;
    /* update command parser state */
    dev->cmd_state = CMD_DONE, dev->cmd_ec = cs->ec;
    /* brag about the command status */
    dprintf_d("CMD DONE, ec = %d (%.*s)\n", dev->cmd_ec, size, ptr);
    /* message is consumed */
    return EOK;
}

/* parse data notification */
static err_t ESP_InputData(esp_dev_t *dev, const void *ptr,
    size_t size, const void *bin_ptr, size_t bin_size)
{
    /* data parsed from the notifications */
    const char *p8 = ptr; int params = 1;
    /* fields that were extracted */
    int link_id = 0; char remote_addr_str[16] = ""; unsigned int data_len;

    /* structure that we need to put into the queue for udp streams */
    struct esp_sock_udphdr size_addr = { 0 };

    /* compare against the smallest of the strings possible */
    if (size < sizeof("+IPD,0") - 1)
        return EFATAL;
    /* message shall start with '+IPD,' */
    if (*p8++ != '+' || *p8++ != 'I' || *p8++ != 'P' || *p8++ != 'D' ||
        *p8++ != ',')
        return EFATAL;

    /* easiest way to check what kind of message this is is to count the
     * number of semicolons up until the end of the text part */
    for (const char *p = p8; p - (const char *)ptr < size - bin_size; p++)
        if (*p == ',')
            params++;

    /* messages that contain the 'link id, length' */
    if (params == 2 && sscanf(p8, "%d,%d", &link_id, &data_len) == 2) {
    /* messages that contain the source port and address */
    } else if (params == 4 && sscanf(p8, "%d,%u,%s,%hd",
        &link_id, &data_len, remote_addr_str, &size_addr.port) == 4 &&
        ESP_IpStrToIp32(remote_addr_str, &size_addr.ip) == EOK) {
    /* unsupported format of the message */
    } else {
        return EFATAL;
    }

    /* malformed frame */
    if (bin_size && bin_size != data_len)
        return EFATAL;

    /* sanitize the link id, consume the message */
    if (link_id < 0 || link_id > elems(dev->links))
        return EOK;

    /* get the connection block for this link */
    struct esp_dev_link *link = &dev->links[link_id];
    /* socket associated with the link */
    struct esp_sock *sock = link->socket;

    /* socket must be connected */
    if (!sock || sock->state != ESP_SOCK_STATE_OPEN) {
        dprintf_w("no socket to handle data from link %d\n", link_id);
        return EOK;
    }

    /* tcp and ssl have similar logic  */
    if (link->prot == ESP_LINK_PROT_TCP ||
        link->prot == ESP_LINK_PROT_SSL) {
        /* binary data was provided */
        if (bin_size) {
            /* store as much as you can */
            if (Queue_PutWait(sock->rxq, bin_ptr, bin_size, 1000) < bin_size)
                dprintf_w("no space in rxq for tcp data (link = %d)\n", link_id);
        /* no binary data, this was just a notification and we need to read
         * the data from the esp by ourselves */
        } else {
            link->esp_data_size = data_len;
        }
    /* for udp mode we need to fit the header into the rxq as well */
    } else if (link->prot == ESP_LINK_PROT_UDP) {
        /* we need to store entire datagram, it does not make sense otherwise */
        if (Queue_GetFree(sock->rxq) < sizeof(struct esp_sock_udphdr) + bin_size) {
            dprintf_w("no space in rxq for udp data (link = %d)\n", link_id);
        /* we still have some space left */
        } else {
            /* store the header and the data */
            Queue_Put(sock->rxq, &size_addr, sizeof(size_addr));
            Queue_Put(sock->rxq, bin_ptr, sizeof(bin_ptr));
        }
    }

    /* message consumed */
    return EOK;
}

/* check if the message received is a prompt message */
static err_t ESP_InputPrompt(esp_dev_t *dev, const void *ptr, size_t size)
{
    /* data pointer */
    const char *p8 = ptr;
    /* return eok if this is a prompt message */
    err_t ec = (size == 1 && *p8 == '>') ? EOK : EFATAL;
    /* return the error code */
    return ec;
}

/* parse data from the esp module */
static void ESP_Input(esp_dev_t *dev, line_modes_t mode, void *ptr,
    size_t size, const void *bin_ptr, size_t bin_size)
{
    /* will get changed to EOK if one of the parsers consumes the message */
    err_t ec = EFATAL;

    /* try to parse messages */
    if (ec != EOK) ec = ESP_InputPrompt(dev, ptr, size);
    if (ec != EOK) ec = ESP_InputData(dev, ptr, size, bin_ptr, bin_size);
    if (ec != EOK) ec = ESP_InputCommandStatus(dev, ptr, size);
    if (ec != EOK) ec = ESP_InputLinkCon(dev, ptr, size);
    if (ec != EOK) ec = ESP_InputConnectionMsgs(dev, ptr, size);
    if (ec != EOK) ec = ESP_InputWiFiMsgs(dev, ptr, size);

    /* response recording is active */
    if ((dev->cmd_sem != SEM_RELEASED)) {
        /* wait as long as there is a command being processed and there
         * is no space in buffer */
        for (; dev->cmd_sem != SEM_RELEASED && dev->rsp_size; Yield());
        /* mark the response data as being ready to be processed by the
         * command processor  */
        if (dev->cmd_sem != SEM_RELEASED) {
            dev->rsp_ptr = ptr, dev->rsp_size = size;
            dev->rsp_bin_ptr = bin_ptr, dev->rsp_bin_size = bin_size;
        }
        /* wait till the data is being processed */
        for (; dev->cmd_sem != SEM_RELEASED && dev->rsp_size; Yield());
        /* drop the frame in case the command processor did not drop it */
        dev->rsp_size = 0;
    }

    dprintf_d("mode = %d, len %d, sentence = %.*s\n", mode, size, size, ptr);
}

/* test the esp connection */
static void ESP_RxPoll(void *arg)
{
    /* esp device */
    esp_dev_t *dev = arg;
    /* internal buffer */
    uint8_t buf[2048]; size_t buf_size = 0;
    /* we are either in the line mode, prompt receive mode '>' or
     * binary data receive mode */
    line_modes_t mode = RX_LINE;

    /* data carrying frames have a text part and a binary part */
    size_t text_len = 0, bin_len = 0;

    /* endless loop of frame polling */
    for (;; Yield()) {
        /* receive the data from the usart */
        err_t ec = ESP_Recv(dev, buf + buf_size, sizeof(buf) - buf_size);
        /* error during reception, no data received */
        if (ec <= EOK)
            continue;
        /* size of the data after reception */
        buf_size += (size_t)ec;

        /* data start, data end, buffer end */
        uint8_t *s = buf, *e, *end = buf + buf_size;
        /* process all the data in the buffer */
        for (;; Yield()) {
            /* parsing mode */
            again: switch (mode) {
            /* line reception mode */
            case RX_LINE: {
                /* swallow leading space */
                for (; s < end && isspace(*s); s++);
                /* we scanned only the whitespace */
                if (s == end)
                    goto end;

                /* go across the data until you find the '\n' */
                for (e = s; e < end && *e != '\n'; e++);

                /* maybe it's a prompt? */
                if (e - s >= 1 && *s == '>') {
                    mode = RX_PROMPT; goto again;
                /* if we have at least four characters in the buffer, then
                 * maybe we are dealing with +IPD notification? */
                } else if (e - s >= 4 && memcmp(s, "+IPD", 4) == 0) {
                    mode = RX_IPD; goto again;
                /* same as with +IPD but for the passive tcp reception */
                } else if (e - s >= 12 && memcmp(s, "+CIPRECVDATA", 12) == 0) {
                    mode = RX_CIPRECVDATA; goto again;
                /* all the other stuff is treated as a normal line. if there is
                 * no newline detected, then we need to end processing for now
                 * and wait for more data */
                } else if (e == end) {
                    goto end;
                }

                /* swallow the trailing space by going back */
                for (; e > s && isspace(*(e - 1)); e--);
                /* store text len */
                text_len = e - s;
            } break;
            /* data notification line that ends with ':' followed by the data
             * or just '\n' */
            case RX_IPD: {
                /* go across the data until you find the ':' or the newline */
                for (e = s; e < end && (*e != ':' && *e != '\n'); e++);
                /* no ending found */
                if (e == end)
                    goto end;

                /* check the syntax of the notification */
                if (snscanf((char *)s, e - s, "+IPD,%d,%d", 0, &bin_len) != 2) {
                    /* unable to parse, restart in line mode */
                    s = e + 1; mode = RX_LINE; goto again;
                }

                /* ipd that ends with ':' has the data that follows it.'\n'
                 * ending ipd has no data */
                if (*e == ':') {
                    /* store the line length */
                    text_len = e - s + 1;
                    /* advance to the data state */
                    mode = RX_BIN_DATA; goto again;
                /* this is a non-binary (xD) message */
                } else {
                    bin_len = 0;
                }
            } break;
            /* responses to the CIPRECVDATA command */
            case RX_CIPRECVDATA: {
                /* go across the data until you find the ':' */
                for (e = s; e < end && *e != ':'; e++);
                /* no ending found */
                if (e == end)
                    goto end;

                /* scan the length */
                if (snscanf((char *)s, e - s, "+CIPRECVDATA,%d", &bin_len) != 1) {
                    /* unable to parse, restart in line mode */
                    s = e + 1; mode = RX_LINE; goto again;
                /* parsing done */
                } else {
                    /* store the line length */
                    text_len = e - s + 1;
                    /* advance to the data state */
                    mode = RX_BIN_DATA; goto again;
                }
            } break;
            /* data part of the data notification */
            case RX_BIN_DATA: {
                /* not enough data gathered */
                if (end - s < bin_len + text_len)
                    goto end;
                /* update the end pointer */
                e = s + bin_len + text_len;
            } break;
            /* we are about to process prompt */
            case RX_PROMPT: {
                /* prompt is just a single character */
                e = s + 1; text_len = 1;
            } break;

            /* unknown mode */
            default: assert (0, "unsupported mode");
            }

            /* process the chunk of data and go again */
            ESP_Input(dev, mode, s, e - s, s + text_len, bin_len); s = e;
            /* go back to normal mode */
            mode = RX_LINE; bin_len = 0;
        }

        /* all the chunks have been processed rewind the buffer */
        end: buf_size = s >= end ? 0 : end - s;
        /* still got some bytes in the buffer that we need to bring back
         * to the beginning? */
        if (buf_size && s != buf)
            memcpy(buf, s, buf_size);
        /* prevent buffer dead-lock */
        if (buf_size == sizeof(buf)) {
            /* reset the parser */
            mode = RX_LINE; buf_size = 0; dprintf_w("rx buffer dumped\n", 0);
        }
    }
}

/* render parameters into sentence */
static err_t ESP_RenderParams(char *out, size_t size, const char *fmt,
    va_list args)
{
    /* no format string given */
    if (!fmt)
        return 0;

    /* render the parameters */
    return vsnprintf(out, size, fmt, args);
}

/* render complete command sentence */
static err_t ESP_RenderSentence(char *out, size_t size, const char *cmd_fmt,
    va_list args)
{
    /* current position within the string */
    size_t pos = 0;

    /* put command and continue with parameters */
    pos += ESP_RenderParams(out + pos, size - pos, cmd_fmt, args);
    /* finish up with line termination */
    pos += snprintf(out + pos, size - pos, ESP_LINE_END);

    /* return the size of the string */
    return pos;
}

/* scans parameter part and puts the results into variables */
static err_t ESP_ScanParams(const char *str, size_t size, const char *fmt,
    va_list args)
{
    /* number of parameter matches needed */
    int matches_needed, matches_found, token_mode = 0;
    /* formatting string */
    const char *f = fmt;
    /* no format string given */
    if (!fmt)
        return EOK;

    /* process the format string, count the number of tokens  */
    for  (matches_needed = 0; ; f++) {
        /* consume the initial '%' and switch on what follows */
        switch (*f) {
        /* start of the token? */
        case '%': token_mode = !token_mode; break;

        /* integers */
        case 'd' : case 'i' : case 'u' : case 'x' : case 'X' : case 'o' :
        /* doubles */
        case 'f' : case 'F': case 'e' : case 'E' : case 'g' : case 'G' :
        case 'a' : case 'A':
        /* all other types */
        case 's' : case 'c' : case 'p' : {
            if (token_mode)
                matches_needed++, token_mode = 0;
        } break;
        /* token that matches against the end of string */
        case '\0': {
            if (token_mode)
                matches_needed++, token_mode = 0;
            goto end;
        }
        /* token modifiers or other parts of the format string */
        default: break;
        }
    }

    /* token was not closed? */
    assert(!token_mode, "invalid syntax fot the format string");

    /* we are not matching againts parameters but agains the */
    end: if (matches_needed == 0) {
        /* no match */
        if (size < f - fmt || memcmp(str, fmt, f - fmt) != 0)
            return EFATAL;
        /* strings match */
        return EOK;
    }

    /* do the proper parsing */
    matches_found = vsnscanf(str, size, fmt, args);
    /* report an error if number of matches does not match the expected number */
    return matches_found != matches_needed ? EFATAL : EOK;
}

/* parse the response, put values into arguments 'args' */
static err_t ESP_ParseResponse(esp_dev_t *dev, const char *fmt,
    va_list args)
{
    /* scanf parameters string */
    return ESP_ScanParams(dev->rsp_ptr, dev->rsp_size, fmt, args);
}

/* drop the response that is currently in buffer */
static void ESP_DropResponse(esp_dev_t *esp)
{
    /* reset the size of the response to 0 */
    esp->rsp_size = 0; Yield();
}

/* get current command status */
static err_t ESP_GetCommandStatus(esp_dev_t *dev)
{
    /* return the error code based on the command status */
    return dev->cmd_state == CMD_DONE ? dev->cmd_ec : EBUSY;
}

/* process response data that was captured during transaction */
err_t ESP_GetResponse(esp_dev_t *dev, const char *fmt, ...)
{
    /* operation error code */
    err_t ec = EFATAL;

    /* this tblock does not have any responses */
    if (!dev->rsp_size)
        return EFATAL;

    /* prepare the list of variable arguments */
    va_list args; va_start(args, fmt);
    /* scan parameters */
    ec = ESP_ParseResponse(dev, fmt, args);
    /* finalize the variable arguments list */
    va_end(args);

    /* return processing error code */
    return ec;
}

/* get the binary part of the response */
err_t ESP_GetResponseBinData(esp_dev_t *dev, void *ptr, size_t size)
{
    /* this tblock does not have any responses */
    if (!dev->rsp_size)
        return EFATAL;
    /* limit the size */
    size = min(size, dev->rsp_bin_size);
    /* copy data */
    if (size)
        memcpy(ptr, dev->rsp_bin_ptr, size);
    /* return the number of bytes copied */
    return size;
}

/* perform the transaction */
int ESP_Transaction(esp_dev_t *dev, dtime_t timeout, const char *cmd_fmt,
    const char *rsp_fmt, ...)
{
    /* error code */
    err_t ec;
    /* support for variable argument list */
    va_list args;
    /* command sentence will be rendered here */
    char sentence[ESP_MAX_LINE_LEN];
    /* is the response parsed? */
    int rsp_parsed = 0;

    /* prepare variable argument list */
    va_start(args, rsp_fmt);
    /* render whole command sentence */
    ec = ESP_RenderSentence(sentence, sizeof(sentence), cmd_fmt, args);
    /* error during sentence rendering */
    if (ec < EOK)
        goto end;

    /* switch the command to the active mode */
    dev->cmd_state = CMD_ACTIVE;
    /* no error, error code contains string length */
    int len = ec;
    /* unable to send the sentence */
    if ((ec = ESP_Send(dev, sentence, len) < EOK))
        goto end;
    /* user only wanted to send the command, parsing all responses and OK or
      *ERROR will be handled by the caller */
    if (timeout < 0)
        return ec;

    /* look for the response and the end sentence (OK or ERROR)*/
    for (time_t ts = time(0); ; ESP_DropResponse(dev)) {
        /* check for timeout */
        if (timeout && dtime(time(0), ts) > timeout) {
            ec = ETIMEOUT; break;
        }

        /* try to parse the response */
        if (ESP_GetResponse(dev, rsp_fmt, args) >= EOK) {
            rsp_parsed = 1;
        /* command processing is done */
        } else if ((ec = ESP_GetCommandStatus(dev)) != EBUSY) {
            break;
        }
    }

    /* response format was given, try to extract the response from the response
     * queue */
    if (ec == EOK && rsp_fmt) {
        /* response format was given, but no response matched it */
        if (!rsp_parsed)
            ec = EFATAL;
    }

    /* drop variable argument list */
    end: va_end(args);
    /* reset the command mode */
    dev->cmd_state = CMD_DONE;
    /* return the status code */
    return ec;
}

#define poll_responses(dev, ec, timeout)                                        \
    for (time_t __ts = time(0);                                                 \
        (ec = (!timeout || dtime_now(__ts) < timeout ? EOK : ETIMEOUT)) == EOK; \
        ESP_DropResponse(dev))



/* send AT command */
err_t ESPCmd_AT(esp_dev_t *dev)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT", 0);
    /* return the error code */
    return ec;
}


/* send ATE command */
err_t ESPCmd_ATE(esp_dev_t *dev, int enable_echo)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "ATE%d", 0, enable_echo);
    /* return the error code */
    return ec;
}

/* re-boot the module */
err_t ESPCmd_RestartModule(esp_dev_t *dev)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem) {
        /* do the transaction */
        roe (ESP_Transaction(dev, 5000, "AT+RST", 0));
        /* look for responses */
        poll_responses (dev, ec, 5000)
            if (ESP_GetResponse(dev, "ready") >= EOK)
                return EOK;
    }

    /* return the error code */
    return ec;
}

/* restore factory defaults */
err_t ESPCmd_RestoreFactorySettings(esp_dev_t *dev)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem) {
        /* do the transaction */
        roe (ec = ESP_Transaction(dev, 1000, "AT+RESTORE", 0));
        /* this is the funny part, the module is ready not after the ok
         * but arter ready command */
        poll_responses (dev, ec, 5000)
            if (ESP_GetResponse(dev, "ready%") >= EOK)
                return EOK;
    }

    /* return the error code */
    return ec;
}

/* configure usart */
err_t ESPCmd_ConfigureUART(esp_dev_t *dev, int baudrate)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+UART_CUR=%d,8,1,0,0", 0,
            baudrate);
    /* return the error code */
    return ec;
}

/** wifi modes */
typedef enum esp_wifi_mode : int {
    ESP_WIFI_MODE_STATION = 1,
    ESP_WIFI_MODE_SOFTAP = 2,
    ESP_WIFI_MODE_STATION_SOFTAP = 3,
} esp_wifi_mode_t;

/* sets the wifi mode */
err_t ESPCmd_SetCurrentWiFiMode(esp_dev_t *dev, esp_wifi_mode_t mode)
{
    /* error code */
    err_t ec = EFATAL;
    /* execute the command */
    with_sem (&dev->cmd_sem)
        return ESP_Transaction(dev, 1000, "AT+CWMODE_CUR=%d", 0, mode);
    /* return the error code */
    return ec;
}

/* gets the wifi mode */
err_t ESPCmd_GetCurrentWiFiMode(esp_dev_t *dev, esp_wifi_mode_t *mode)
{
    /* error code */
    err_t ec = EFATAL;
    /* execute the command */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+CWMODE_CUR?", "+CWMODE_CUR:%i%",
            mode);
    /* return the error code */
    return ec;
}

/* set the discovery results options */
err_t ESPCmd_SetDiscoverOptions(esp_dev_t *dev, int sort_by_rssi)
{
    /* error code */
    err_t ec = EFATAL;
    /* execute the command */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 5000, "AT+CWLAPOPT=%d,%d", 0,
            !!sort_by_rssi, 127);
    /* return the error code */
    return ec;
}

/* discover access point nearby */
err_t ESPCmd_DiscoverAPs(esp_dev_t *dev, esp_ap_t *aps, size_t aps_num)
{
    /* operation error code */
    err_t ec = EFATAL; esp_ap_t *ap = aps;
    /* lock the command interface */
    with_sem (&dev->cmd_sem) {
        /* do the transaction, but do not wait for OK or ERROR */
        roe (ESP_Transaction(dev, -1, "AT+CWLAP", 0));
        /* process incoming responses */
        poll_responses (dev, ec, 5000) {
            /* command complete? if so then return the number of records if
             * command succeeded */
            if ((ec = ESP_GetCommandStatus(dev)) != EBUSY)
                return ec >= EOK ? ap - aps : ec;
            /* no space for further access point records */
            if (ap - aps >= aps_num)
                continue;
            /* parse the response (here we do not use all of the arguments,
             * as we dont really need them) */
            if (ESP_GetResponse(dev, "+CWLAP:(%d,\"%.*s\",%d,\"%.*s\",", 0,
                sizeof(ap->ssid), ap->ssid, &ap->rssi,
                sizeof(ap->mac), ap->mac) >= EOK)
                ap++;
        }
    }

    /* report the status */
    return ec;
}

/* connect to the access point */
err_t ESPCmd_ConnectToAP(esp_dev_t *dev, const char *ssid, const char *pass,
    const char *mac, esp_conn_error_code_t *code)
{
    /* error code */
    err_t ec;
    /* execute the command */
    with_sem (&dev->cmd_sem) {
        /* do the transaction */
        roe (ESP_Transaction(dev, -1, "AT+CWJAP_CUR=\"%s\",\"%s\"%s%s%s", 0,
            ssid, pass, mac ? ",\"" : "", mac ? mac : "", mac ? "\"" : ""));
        /* poll on responses */
        poll_responses (dev, ec, 15000) {
            /* command has ended */
            if ((ec = ESP_GetCommandStatus(dev)) != EBUSY)
                return ec;
            /* try to parse the reason code */
            ESP_GetResponse(dev, "+CWJAP:%d%", code);
        }
    }
    /* return the error code */
    return ec;
}

/* connect to the access point */
err_t ESPCmd_DisconnectFromAP(esp_dev_t *dev)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 5000, "AT+CWQAP", 0);
    /* report status */
    return ec;
}

/* dhcp configuration modes */
typedef enum esp_dhcp_mode : int {
    ESP_DHCP_MODE_SERVER = 0,
    ESP_DHCP_MODE_CLIENT = 1,
    ESP_DHCP_MODE_BOTH = 2,
} esp_dhcp_mode_t;

/* set the dhcp mode */
err_t ESPCmd_ConfigureDHCP(esp_dev_t *dev, esp_dhcp_mode_t mode, int enabled)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 5000, "AT+CWDHCP_DEF=%d,%d", 0, mode,
            !!enabled);
    /* report status */
    return ec;
}

/* set the ip address of the module if in static mode */
err_t ESPCmd_SetIPAddress(esp_dev_t *dev, const char *addr, const char *gateway,
    const char *netmask)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* if gateway is given then the netmask must be given as well */
    if ((gateway && !netmask) || (netmask && !gateway))
        return EARGVAL;

    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 5000,
            "AT+CIPSTA_CUR=\"%s\"%s%s%s%s%s", 0,
            addr,
            gateway ? ",\"" : "", gateway ? gateway : "",
            gateway ? "\"," : "", gateway ? netmask : "",
            gateway ? "\""  : "");

    /* report status */
    return ec;
}

/* set the ip address of the module if in static mode */
err_t ESPCmd_GetIPAddress(esp_dev_t *dev, char addr[16], char gateway[16],
    char netmask[16])
{
    /* operation error code */
    err_t ec = EFATAL;
    /* bitflags of responses that we've received */
    enum {
        CIPSTA_IP = BIT_VAL(0),
        CIPSTA_GATEWAY = BIT_VAL(1),
        CIPSTA_NETMASK = BIT_VAL(2),
        CIPSTA_ALL = CIPSTA_IP | CIPSTA_GATEWAY |  CIPSTA_NETMASK
    } responses = 0;

    /* lock the command interface */
    with_sem (&dev->cmd_sem) {
        /* do the transaction */
        roe(ESP_Transaction(dev, -1, "AT+CIPSTA_CUR?", 0));
        /* transaction complete, parse responses */
        poll_responses(dev, ec, 1000) {
            /* command complete */
            if ((ec = ESP_GetCommandStatus(dev)) != EBUSY)
                break;
            /* parse our ip address */
            if (ESP_GetResponse(dev, "+CIPSTA_CUR:ip:\"%s\"", addr) >= EOK)
                responses |= CIPSTA_IP;
            /* parse gateway */
            if (ESP_GetResponse(dev, "+CIPSTA_CUR:gateway:\"%s\"", gateway) >= EOK)
                responses |= CIPSTA_NETMASK;
            /* parse netmask */
            if (ESP_GetResponse(dev, "+CIPSTA_CUR:netmask:\"%s\"", netmask) >= EOK)
                responses |= CIPSTA_NETMASK;
        }
    }
    /* we did not receive all responses */
    if (ec >= EOK && responses == CIPSTA_ALL)
        ec = EFATAL;
    /* report status */
    return ec;
}

/* set the mdns server */
err_t ESPCmd_ConfigureMDNS(esp_dev_t *dev, int enable, const char *name)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* conmand format depends on enable flag state */
    const char *fmt = enable ? "AT+MDNS=%d,\"%s\",\"esp\",5353" : "AT+MDNS=%d";
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 5000, fmt, 0, !!enable, name);
    /* report status */
    return ec;
}

/* set the ip address of the module if in static mode */
err_t ESPCmd_GetConnectionStatus(esp_dev_t *dev, esp_wifi_status_t *wifi,
    esp_tcpip_link_status_t *status, int link_id_start, int link_id_end)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* pointer to the connection status buffer */
    esp_tcpip_link_status_t *s = status;

    /* lock the command interface */
    with_sem (&dev->cmd_sem) {
        /* do the transaction */
        roe (ESP_Transaction(dev, -1, "AT+CIPSTATUS", 0));

        /* placeholder for connection protocol */
        char protocol[4];
        /* parse the responses  */
        poll_responses(dev, ec, 1000) {
            /* command complete */
            if ((ec = ESP_GetCommandStatus(dev)) != EBUSY)
                break;
            /* extract the connection status */
            ESP_GetResponse(dev, "STATUS:%d%", wifi);
            /* no space for more records */
            if (s - status >= link_id_end - link_id_start)
                continue;
            /* process link status responses */
            if (ESP_GetResponse(dev, "+CIPSTATUS:%d,%.*s,\"%s\",%hd,%hd,%d%",
                &s->link_id, sizeof(protocol), protocol, s->remote_ip,
                &s->remote_port, &s->local_port, s->role) >= EOK) {
                /* update the pointer only if this is the link that caller
                 * wants */
                if (s->link_id >= link_id_start && s->link_id < link_id_end)
                    s->protocol = ESP_ProtStrToProtType(protocol), s = s + 1;
            }
        }
    }

    /* report status */
    return ec >= EOK ? s - status : ec;
}

/* establishes a connection to the remote address */
err_t ESPCmd_StartTCPConnection(esp_dev_t *dev, int link_id,
    const char *addr, uint16_t port, int use_ssl)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* type of conenction */
    const char *prot = ESP_ProtTypeToProtStr(
        use_ssl ? ESP_LINK_PROT_SSL : ESP_LINK_PROT_TCP);

    /* lock the command interface */
    with_sem (&dev->cmd_sem) {
        /* do the transaction */
        roe (ESP_Transaction(dev, -1, "AT+CIPSTART=%d,\"%s\",\"%s\",%d", 0,
            link_id, prot, addr, port));
        /* this command may produce the awkward "already connected" message */
        poll_responses(dev, ec, 5000) {
            /* did the command end? */
            if ((ec = ESP_GetCommandStatus(dev)) != EBUSY)
                break;
            /* i guess we cannot have more than one tcp connection to the same
             * remote host/port combination */
            if (ESP_GetResponse(dev, "ALREADY CONNECTED") >= EOK)
                return EFATAL;
        }
    }

    /* report status */
    return ec;
}

/* establishes a connection to the remote address */
err_t ESPCmd_RegisterUDPPort(esp_dev_t *dev, int link_id,
    const char *addr, uint16_t remote_port, uint16_t local_port)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* get the local port number */
    if (!local_port)
        local_port = Seed_GetRandInt(10000, 20000);
    /* lock the command interface */
    with_sem (&dev->cmd_sem) {
        ec = ESP_Transaction(dev, 1000, "AT+CIPSTART=%d,\"UDP\",\"%s\",%d,%d,2",
            0, link_id, addr, remote_port, local_port);
        /* this command may produce the awkward "already connected" message */
        poll_responses(dev, ec, 5000) {
            /* did the command end? */
            if ((ec = ESP_GetCommandStatus(dev)) != EBUSY)
                break;
            /* i guess we cannot have more than one udp connection to the same
             * remote host/port combination */
            if (ESP_GetResponse(dev, "ALREADY CONNECTED") >= EOK)
                return EFATAL;
        }
    }

    /* report status */
    return ec;
}

/* set the ssl buffer size */
err_t ESPCmd_SetSSLBufferSize(esp_dev_t *dev, size_t size)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+CIPSSLSIZE=%d", 0, size);
    /* report status */
    return ec;
}

/* send tcp data  */
err_t ESPCmd_SendDataTCP(esp_dev_t *dev, int link_id, const void *ptr,
    size_t size)
{
    /* error code*/
    err_t ec = EFATAL; int data_sent = 0;
    /* lock the command interface */
    with_sem (&dev->cmd_sem) {
        /* do the transaction */
        roe (ESP_Transaction(dev, 1000, "AT+CIPSEND=%d,%d", 0, link_id, size));
        /* parse the responses  */
        poll_responses(dev, ec, 1000) {
            /* got the prompt */
            if (!data_sent && ESP_GetResponse(dev, ">") >= EOK) {
                ESP_Send(dev, ptr, size); data_sent = 1;
            /* data accepted? */
            } else if (ESP_GetResponse(dev, "SEND OK") >= EOK) {
                break;
            }
        }
    }

    /* report status */
    return ec >= EOK ? size : ec;
}

/* send the udp data  */
err_t ESPCmd_SendDataUDP(esp_dev_t *dev, int link_id, const void *ptr,
    size_t size, const char *addr, uint16_t port)
{
    /* error code*/
    err_t ec = EFATAL;
    /* command format */
    static const char *fmt = "AT+CIPSEND=%d,%d,\"%s\",%d";
    /* no address is specified */
    if (!addr)
        fmt = "AT+CIPSEND=%d,%d";

    /* lock the command interface */
    with_sem (&dev->cmd_sem) {
        /* do the transaction */
        roe (ESP_Transaction(dev, -1, fmt, 0, link_id, size, addr, port));
        /* parse the responses  */
        poll_responses(dev, ec, 1000) {
            /* got the prompt */
            if (ESP_GetResponse(dev, ">") >= EOK) {
                ESP_Send(dev, ptr, size);
            /* data accepted? */
            } else if (ESP_GetResponse(dev, "SEND OK") >= EOK) {
                break;
            }
        }
    /* release the command interface */
    }

    /* report status */
    return ec >= EOK ? size : ec;
}

/* close connection */
err_t ESPCmd_CloseConnection(esp_dev_t *dev, int link_id)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+CIPCLOSE=%d", 0, link_id);

    /* report status */
    return ec;
}

/* set the maximal number of connections*/
err_t ESPCmd_SetMaxNumberOfTCPConnections(esp_dev_t *dev, int max_conns)
{
    /* operation error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+CIPSERVERMAXCONN=%d", 0, max_conns);

    /* report status */
    return ec;
}

/* enable multiple connections */
err_t ESPCmd_EnableMultipleConnections(esp_dev_t *dev, int enable)
{
    /* error code*/
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+CIPMUX=%d", 0, !!enable);

    /* report status */
    return ec;
}

/* start tcp server */
err_t ESPCmd_StartTCPServer(esp_dev_t *dev, uint16_t port)
{
    /* error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+CIPSERVER=1,%d", 0, port);

    /* store the port */
    if (ec >= EOK)
        dev->server_port = port;
    /* report status */
    return ec;
}

/* stop tcp server */
err_t ESPCmd_StopTCPServer(esp_dev_t *dev)
{
    /* error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+CIPSERVER=0", 0);

    /* reset the port */
    if (ec >= EOK)
        dev->server_port = 0;
    /* report status */
    return ec;
}

/* configure the timeout for the tcp server */
err_t ESPCmd_SetTCPServerTimeout(esp_dev_t *dev, int timeout_seconds)
{
    /* error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+CIPSTO=%d", 0, timeout_seconds);

    /* report status */
    return ec;
}

/* ping the other side */
err_t ESPCmd_Ping(esp_dev_t *dev, const char *addr, int *latency)
{
    /* error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+PING=\"%s\"", "+%d%", addr,
            latency);

    /* report status */
    return ec;
}

/* append addressing information to all +IPD reports */
err_t ESPCmd_AppendRemoteAddrToRxFrames(esp_dev_t *dev, int enable)
{
    /* error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+CIPDINFO=%d", 0, !!enable);

    /* report status */
    return ec;
}

/* set the way we receive frames */
err_t ESPCmd_SetTCPReceiveMode(esp_dev_t *dev, int passive)
{
    /* error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+CIPRECVMODE=%d", 0, !!passive);

    /* report status */
    return ec;
}

/* read the data for given id */
err_t ESPCmd_GetTCPReceivedData(esp_dev_t *dev, int link_id, void *ptr,
    size_t size)
{
    /* error code */
    err_t ec = EFATAL; size_t actual_size = 0;
    /* link descriptor */
    struct esp_dev_link *link = &dev->links[link_id];

    /* lock the command interface */
    with_sem (&dev->cmd_sem) {
        /* do the transaction */
        roe (ESP_Transaction(dev, -1, "AT+CIPRECVDATA=%d,%d", 0, link_id, size));
        /* process incoming frames */
        poll_responses(dev, ec, 1000) {
            /* end of the command processing */
            if ((ec = ESP_GetCommandStatus(dev)) != EBUSY)
                break;

            /* get the data carrying response */
            if (ESP_GetResponse(dev, "+CIPRECVDATA,%d,", &actual_size) >= EOK) {
                /* try to extract the binary part of the response */
                roe (ec = ESP_GetResponseBinData(dev, ptr, size));
                /* store the actual size */
                actual_size = ec;
                /* mark these bytes as consumed */
                if (link->esp_data_size >= actual_size) {
                    link->esp_data_size -= actual_size;
                /* some weirdness happened */
                } else {
                    link->esp_data_size = 0;
                }
            }
        }
    /* release the command interface */
    } Sem_Release(&dev->cmd_sem);

    /* report status */
    return ec >= EOK ? actual_size : ec;
}

/* system messages configuration bits */
typedef enum esp_sysmsg : int {
    ESP_SYSMSG_ENABLE_PASSTHROUGH_MSG = BIT_VAL(0),
    ESP_SYSMSG_ENABLE_LINK_CONN_MSG = BIT_VAL(1),
} esp_sysmsg_t;

/* set the way we receive frames */
err_t ESPCmd_ConfigureSystemMessages(esp_dev_t *dev, esp_sysmsg_t sysmsg)
{
    /* error code */
    err_t ec = EFATAL;
    /* lock the command interface */
    with_sem (&dev->cmd_sem)
        ec = ESP_Transaction(dev, 1000, "AT+SYSMSG_CUR=%d", 0, sysmsg);

    /* report status */
    return ec;
}

/* create tcp socket */
esp_sock_t * ESPTcpSock_Create(esp_dev_t *dev, size_t rx_size)
{
    /* pointer to the socket */
    esp_sock_t *sock;
    /* we look for an empty slot within sockets*/
    forall (sock, dev->sockets)
        if (sock->state == ESP_SOCK_STATE_FREE)
            break;

    /* no free sockets */
    if (sock == arrend(dev->sockets))
        return 0;

    /* allocate space for the queue */
    queue_t *rxq = Queue_Create(1, rx_size);
    /* buy more ram lol */
    if (!rxq)
        return 0;

    /* allocate queue */
    sock->dev = dev;
    sock->link = 0;
    sock->state = ESP_SOCK_STATE_CLOSED;
    sock->rxq = rxq;

    /* return the socket */
    return sock;
}

/* wait for a connection to become established */
err_t ESPTcp_Listen(esp_sock_t *sock, uint16_t port, dtime_t timeout)
{
    /* improper state */
    if (sock->state != ESP_SOCK_STATE_CLOSED)
        return EARGVAL;

    /* esp module is currently not listening */
    if (!sock->dev->server_port)
        roe(ESPCmd_StartTCPServer(sock->dev, port));
    /* server is listening on some other port TODO: if there are no more
     * sockets listening on that port maybe we should allow to restart the
     * server on some other port? */
    else if (sock->dev->server_port != port)
        return EFATAL;

    /* mark as listening to incoming connections */
    sock->state = ESP_SOCK_STATE_LISTEN;
    sock->role = ESP_LINK_ROLE_SRV;
    /* loop until connection is established */
    for (time_t ts = time(0); ; Yield()) {
        /* check for timeout condition */
        if (timeout && dtime_now(ts) > timeout) {
            sock->state = ESP_SOCK_STATE_CLOSED;
            return ETIMEOUT;
        }

        /* we are now connected */
        if (sock->state == ESP_SOCK_STATE_OPEN)
            break;
    }

    /* report status */
    return EOK;
}

/* try to connect to remote host */
err_t ESPTcp_Connect(esp_sock_t *sock, const char *addr, uint16_t port)
{
    /* link that will hold the connection */
    struct esp_dev_link *link;
    /* invalid state for the socket */
    if (sock->state != ESP_SOCK_STATE_CLOSED)
        return EARGVAL;

    /* check if we have a free connection block */
    forall (link, sock->dev->links)
        if (!link->connected)
            break;
    /* no free links available */
    if (link == arrend(sock->dev->links))
        return EFATAL;

    /* do not allow others to mess with our connection process */
    with_sem (&sock->dev->sock_sem) {
        /* start connecting */
        sock->state = ESP_SOCK_STATE_CONNECTING;
        sock->role = ESP_LINK_ROLE_CLI;
        /* establish a connection */
        err_t ec = ESPCmd_StartTCPConnection(sock->dev, link - sock->dev->links,
            addr, port, 0);
        /* unable to connect */
        if (ec < EOK) {
            sock->state = ESP_SOCK_STATE_CLOSED; return ec;
        }

        /* wait for the socket to become connected */
        for (; sock->state != ESP_SOCK_STATE_OPEN; Yield());
    }
    /* report error */
    return EOK;
}

/* error code */
err_t ESPTcp_Recv(esp_sock_t *sock, void *ptr, size_t size, dtime_t timeout)
{
    /* error code */
    err_t ec = EFATAL;

    /* try to obtain data */
    for (time_t ts = time(0); ; Yield()) {
        /* timeout condition */
        if (timeout && dtime_now(ts) > timeout)
            return ETIMEOUT;

        /* got something from the queue */
        if ((ec = Queue_Get(sock->rxq, ptr, size)) > 0)
            return ec;

        /* esp did not notify of the data that it has */
        if (sock->link->esp_data_size) {
            /* let's fill the reception queue if it's possible */
            with_sem (&sock->rxq->sem) {
                /* get access to linear space */
                size_t bfree; void *qptr = Queue_GetFreeLinearMem(sock->rxq,
                    &bfree);
                /* got the data from the socket? */
                if ((ec = ESPCmd_GetTCPReceivedData(sock->dev,
                        sock->link - sock->dev->links, qptr, bfree)) > EOK)
                    Queue_IncreaseCount(sock->rxq, ec);
            }
        /* disconnected? */
        } else if (sock->state != ESP_SOCK_STATE_OPEN) {
            return ENOCONNECT;
        }
    }

    /* return the size of the data received */
    return ec;
}

/* send data over the socket */
err_t ESPTcp_Send(esp_sock_t *sock, const void *ptr, size_t size)
{
    /* we are not even connected */
    if (sock->state != ESP_SOCK_STATE_OPEN)
        return ENOCONNECT;

    /* send the data over the tcp */
    return ESPCmd_SendDataTCP(sock->dev, sock->link - sock->dev->links,
        ptr, size);
}

/* close the connection */
err_t ESPTcp_Close(esp_sock_t *sock)
{
    /* we are not connected */
    if (sock->state != ESP_SOCK_STATE_OPEN)
        return ENOCONNECT;
    /* send the close command */
    return ESPCmd_CloseConnection(sock->dev, sock->link - sock->dev->links);
}

/* bind socket to a specific port */
err_t ESPUdp_Bind(esp_sock_t *sock, uint16_t port)
{
    /* link that will hold the connection */
    struct esp_dev_link *link;
    /* invalid state for the socket */
    if (sock->state != ESP_SOCK_STATE_CLOSED)
        return EARGVAL;

    /* check if we have a free connection block */
    forall (link, sock->dev->links)
        if (!link->connected)
            break;
    /* no free links available */
    if (link == arrend(sock->dev->links))
        return EFATAL;

    /* do not allow others to mess with our connection process */
    with_sem (&sock->dev->sock_sem) {
        /* mark socket as connecting */
        sock->state = ESP_SOCK_STATE_CONNECTING;
        /* register port */
        err_t ec = ESPCmd_RegisterUDPPort(sock->dev, link - sock->dev->links,
            "123.123.123.123", port, port);
        /* unable to bind the socket to the port */
        if (ec < EOK) {
            sock->state = ESP_SOCK_STATE_CONNECTING; return ec;
        }

        /* wait for the socket to become connected */
        for (; sock->state != ESP_SOCK_STATE_OPEN; Yield());
    }
    /* report error */
    return EOK;
}

/* send data over udp */
err_t ESPUdp_SendTo(esp_sock_t *sock, const char *addr, uint16_t port,
    const void *ptr, size_t size)
{
    /* we are not even connected */
    if (sock->state != ESP_SOCK_STATE_OPEN)
        return ENOCONNECT;

    /* send the data over the tcp */
    return ESPCmd_SendDataUDP(sock->dev, sock->link - sock->dev->links,
        ptr, size, addr, port);
}

/* receive data from udp socket */
err_t ESPUdp_RecvFrom(esp_sock_t *sock, uint32_t *addr, uint16_t *port,
    void *ptr, size_t size, dtime_t timeout)
{
    /* error code */
    err_t ec = EFATAL;
    /* try to obtain data */
    for (time_t ts = time(0); ; Yield()) {
        /* timeout condition */
        if (timeout && dtime_now(ts) > timeout)
            return ETIMEOUT;

        /* wait for the header in the reception queue */
        if (!sock->udp_hdr.size && Queue_GetUsed(sock->rxq) >
            sizeof(struct esp_sock_udphdr))
            Queue_Get(sock->rxq, &sock->udp_hdr, sizeof(sock->udp_hdr));

        /* no data and no connection? */
        if (!sock->udp_hdr.size) {
            if (sock->state != ESP_SOCK_STATE_OPEN)
                return ENOCONNECT;
        /* data is buffered */
        } else if (sock->udp_hdr.size) {
            /* get from the queue */
            size_t b_consumed = Queue_Get(sock->rxq, ptr,
                min(sock->udp_hdr.size, size));
            /* store the address and the port */
            if (addr) *addr = sock->udp_hdr.ip;
            if (addr) *port = sock->udp_hdr.port;
            /* subtract */
            sock->udp_hdr.size -= b_consumed;
            /* return the number of bytes consumed */
            return b_consumed;
        }
    }

    /* return the size of the data received */
    return ec;
}





/* wifi credentials */
static const char pass[] = "dbtn3kmhds45g6p9";
static const char ssid[] = "MT7915-2G";
static const char bssid[] = "ac:91:9b:fb:a7:72";

/* wifi credentials */
// static const char pass[] = "cC25R9hX";
// static const char ssid[] = "INEA-0444_2.4G";
// static const char bssid[] = "04:20:84:32:4f:27";


static void KeepAlive(void *arg)
{
    //esp_dev_t *dev = arg;
    for (;; Sleep(1000 * 5)) {
        // ESPCmd_Ping(dev, "192.168.1.1", 0);
        // ESPCmd_ConfigureMDNS(dev, 0, "stm32");
        // ESPCmd_ConfigureMDNS(dev, 1, "stm32");
        // dprintf_i("restarting mdns\n", 0);

    }
}


/* initialize esp device */
err_t ESP_DevInit(esp_dev_t *dev)
{
    /* reset the module */
    ESP_ResetChip(dev);
    /* set the default baudrate */
    ESP_SetBaudrate(dev, 115200);

    /* start the task */
    if (dev->rx_task_id == 0)
        dev->rx_task_id = Yield_Task(ESP_RxPoll, dev, 2700);
    /* sanity check */
    assert(dev->rx_task_id > 0, "unable create a task for handling rx");

    /* probe communication with the module by using at command */
    for (time_t ts = time(0); ; Sleep(100)) {
        /* execute command */
        if (ESPCmd_AT(dev) >= EOK)
            break;
        /* no response from module */
        if (dtime_now(ts) > 2000)
            return ETIMEOUT;
    }

    /* reset the module */
    roe (ESPCmd_RestartModule(dev));
    roe (ESPCmd_AT(dev));
    roe (ESPCmd_ATE(dev, 0));
    roe (ESPCmd_ConfigureUART(dev, dev->baudrate));

    /* set the baudrate */
    ESP_SetBaudrate(dev, dev->baudrate); Sleep(100);

    /* initial configuration */
    roe (ESPCmd_EnableMultipleConnections(dev, 1));
    roe (ESPCmd_AppendRemoteAddrToRxFrames(dev, 1));
    // roe (ESPCmd_SetTCPReceiveMode(dev, 1));
    roe (ESPCmd_ConfigureSystemMessages(dev, ESP_SYSMSG_ENABLE_LINK_CONN_MSG));

    /* configure the radio */
    roe (ESPCmd_SetCurrentWiFiMode(dev, ESP_WIFI_MODE_STATION));
    roe (ESPCmd_SetDiscoverOptions(dev, 1));
    roe (ESPCmd_ConfigureDHCP(dev, ESP_DHCP_MODE_CLIENT, 1));


    Yield_Task(KeepAlive, dev, 1024);

    /* report success */
    return EOK;
}

/* device descriptor */
esp_dev_t dev = { .usart = &usart2, .reset = GPIO_SIGNAL_BLACKPILL_A4,
    .baudrate = 115200 * 20 };



/* wrapper for the socket create */
static void * _Create(void)
{
    /* create esp socket */
    return ESPTcpSock_Create(&dev, 256);
}

/* wrapper for the listening to incoming connections */
static err_t _Listen(void *sock, int port)
{
    return ESPTcp_Listen(sock, port, 0);
}

/* wrapper for the receive data */
static err_t _Recv(void *sock, void *ptr, size_t size, dtime_t timeout)
{
    return ESPTcp_Recv(sock, ptr, size, timeout);
}

/* wrapper for the connection send */
static err_t _Send(void *sock, const void *ptr, size_t size,
    dtime_t timeout)
{
    return ESPTcp_Send(sock, ptr, size);
}

/* wrapper for the connection close */
static err_t _Close(void *sock)
{
    return ESPTcp_Close(sock);
}

#include "net/uhttpsrv/uhttpsrv.h"
extern err_t HTTPSrvWebsite_Callback(struct uhttp_request *req);

/* create a server instance for testing */
err_t Website_Init(void)
{
    /* we need to ensure that this object is kept around after we exit
     * this function */
    static uhttp_instance_t instance = {
        .port = 80,
        .timeout = 1000,
        .max_connections = 2,
        .stack_size = 2048,
        .callback = HTTPSrvWebsite_Callback,

        .sock_funcs = {
            .create = _Create,
            .listen = _Listen,
            .recv = _Recv,
            .send = _Send,
            .close = _Close,
        }
    };
    /* start the server */
    err_t ec = UHTTPSrv_InstanceInit(&instance);
    /* check if we can create the server task */
    assert(ec >= EOK, "unable to create the server task");

    /* report the status code */
    return ec;
}


/* test for the esp tcp echo server */
static void TestESP_EchoPoll(void *arg)
{
    /* descriptor */
    esp_dev_t *dev = arg;
    /* error code */
    err_t ec;
    esp_conn_error_code_t con_error_code;

    uint8_t buf[64];

    /* server processing loop */
    for (;; Yield()) {
        /* try to connect to the ap */
        ec = ESPCmd_ConnectToAP(dev, ssid, pass, bssid, &con_error_code);
        dprintf_i("wifi conn ec = %d, code = %d\n", ec, con_error_code);
        if (ec < EOK)
            continue;

            /* setup mdns */
        if ((ec = ESPCmd_ConfigureMDNS(dev, 1, "stm32")) < EOK)
            continue;

        ESPCmd_SetMaxNumberOfTCPConnections(dev, 2);
        // /* start the tcp server */
        // if ((ec = ESPCmd_StartTCPServer(dev, 80)) < EOK)
        //     continue;

        Website_Init();
        for (;; Yield());

        esp_sock_t *sock = ESPTcpSock_Create(dev, 386);

        /* tcp processing loop */
        for (;; Yield()) {
            // /* listen to incoming connections on the link 0 */
            ec = ESPTcp_Listen(sock, 80, 0);
            dprintf_i("tcp conn ec = %d\n", ec);
            if (ec < EOK)
                continue;

            // ec = ESPTcp_Connect(sock, "192.168.1.4", 12345);
            // dprintf_i("tcp conn ec = %d\n", ec);
            // if (ec < EOK)
            //     continue;


            for (;; Yield()) {
                /* try to receive the data */
                ec = ESPTcp_Recv(sock, buf, sizeof(buf), 0);
                dprintf_i("rx data ec = %d\n", ec);
                if (ec < EOK)
                    break;
                /* try to send the data back */
                ec = ESPTcp_Send(sock, buf, ec);
                dprintf_i("tx data ec = %d\n", ec);
                if (ec < EOK)
                    break;
            }

            /* close the connection */
            ESPTcp_Close(sock);
        }
    }
}



#if 0
/* test the esp connection */
static void TestESP_Poll(void *arg)
{
    /* device descriptor */
    esp_dev_t *dev = arg;
    /* error code */
    err_t ec;

    esp_wifi_mode_t mode;
    esp_conn_error_code_t con_error_code;

    esp_ap_t aps[20];

    char addr[16], gateway[16], netmask[16], buf[16];

    /* testing loop */
    for (;; Sleep(5000)) {
        /* reset the module */
        ec = ESPCmd_RestartModule(dev);
        ec = ESPCmd_AT(dev);
        /* initial configuration */
        ec = ESPCmd_EnableMultipleConnections(dev, 1);
        ec = ESPCmd_AppendRemoteAddrToRxFrames(dev, 1);
        ec = ESPCmd_SetTCPReceiveMode(dev, 1);
        ec = ESPCmd_ConfigureSystemMessages(dev, ESP_SYSMSG_ENABLE_LINK_CONN_MSG);

        /* configure the radio */
        ec = ESPCmd_SetCurrentWiFiMode(dev, ESP_WIFI_MODE_STATION);
        ec = ESPCmd_SetDiscoverOptions(dev, 1);
        ec = ESPCmd_ConfigureDHCP(dev, ESP_DHCP_MODE_CLIENT, 1);
        /* discover APs nearby */
        ec = ESPCmd_DiscoverAPs(dev, aps, sizeof(aps));
        if (ec > 0) {
            for (esp_ap_t *ap = aps; ec != ap - aps; ap++) {
                dprintf_i("rssi = %d, ssid = %s, mac = %s\n", ap->rssi,
                    ap->ssid, ap->mac);
            }
        }
        dprintf_i("discover ec = %d\n", ec);


        /* try to connect to the ap */
        ec = ESPCmd_ConnectToAP(dev, ssid, pass, bssid, &con_error_code);
        dprintf_i("connection ec = %d, code = %d\n", ec, con_error_code);
        // Sleep(2000);

        // ec = ESPCmd_ConfigureMDNS(dev, 1, "test");

        // // ec = ESPCmd_SetIPAddress(dev, "192.168.1.123", 0, 0);
        ec = ESPCmd_GetIPAddress(dev, addr, gateway, netmask);
        /* show addresses */
        dprintf_i("ip addess is = %.*s\n", sizeof(addr), addr);
        dprintf_i("gateway addess is = %.*s\n", sizeof(gateway), gateway);
        dprintf_i("netmask addess is = %.*s\n", sizeof(netmask), netmask);


        // /* client */
        // ec = ESPCmd_RegisterUDPPort(dev, 0, "192.168.1.4", 5555, 0);
        // if (ec >= EOK) {
        //     for (int i = 0; i < 5; Sleep(1000), i++) {
        //         ec = ESPCmd_SendDataUDP(dev, 0, "tomek\n", 6, "192.168.1.4", 5556);
        //         dprintf_i("send ec = %d\n", ec);
        //     }
        //     ec = ESPCmd_CloseConnection(dev, 0);
        //     dprintf_i("close ec = %d\n", ec);
        // }


        /* tcp server */
        ec = ESPCmd_StartTCPServer(dev, 6969);
        if (ec >= EOK) {
            for (;; Sleep(1000)) {

                esp_tcpip_link_status_t cs[5];
                enum esp_wifi_status wifi;
                ec = ESPCmd_GetConnectionStatus(dev, &wifi, cs, 0, 5);
                for (int i = 0; i < ec; i++) {
                    dprintf_i("link_id = %d, l_port = %d, rem_ip = %s, rem_port = %d\n",
                        cs[i].link_id, cs[i].local_port, cs[i].remote_ip, cs[i].remote_port);
                }
                dprintf_i("conn_stat ec = %d, wifi_stat = %d\n", ec, wifi);

                /* try to get the data from the socket */
                ec = ESPCmd_GetTCPReceivedData(dev, 0, buf, sizeof(buf));
                if (ec > EOK) {
                    dprintf_i("GOT DATA, %d, %.*s\n", ec, ec, buf);
                }
                dprintf_i("receive ec=%d\n", ec);
            }
        }


        while (1)
            Sleep(30000);



        // ec = ESPCmd_DisconnectFromAP(dev);
        // dprintf_i("disconnected ec = %d\n", ec);
        // Sleep(2000);


        /* display the result */
        // dprintf_i("command done, ec = %d\n", ec);
    }
}
#endif



/* initialize the test */
err_t TestESP_Init(void)
{

    /* initialize the device */
    ESP_DevInit(&dev);
    /* start the testing task */
    // Yield_Task(TestESP_Poll, &dev, 2000);
    // Yield_Task(TestESP_Poll, &dev, 2000);
    Yield_Task(TestESP_EchoPoll, &dev, 2000);


    return EOK;
}