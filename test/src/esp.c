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

#include "sys/queue.h"
#include "util/string.h"
#include "util/minmax.h"

#include <stdarg.h>

#define DEBUG DLVL_DEBUG
#include "debug.h"

/* line parsing modes */
typedef enum line_modes {
    RX_LINE,
    RX_IPD_LINE,
    RX_IPD_DATA,
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

/* access point entry */
typedef struct esp_ap {
    /* type of the security used */
    esp_ap_sec_t sec;
    /* ssid name, and mac address */
    char ssid[24], mac[18];
    /* signal strength */
    int rssi;
} esp_ap_t;

typedef struct esp_dev {
    /* usart device */
    usart_dev_t *usart;
    /* reception queue */
    queue_t *rxq;

    /* semaphore for sending commands */
    sem_t cmd_sem;
    /* current command parser state */
    enum { CMD_DONE, CMD_ACTIVE } cmd_state;
    /* command execution error code */
    err_t cmd_ec;

    /* response queue */
    queue_t *responses;
    /* number of responses in the queue */
    size_t responses_num;

} esp_dev_t;

/** maximal length of the command line */
#define ESP_MAX_LINE_LEN        256
/** enline ending */
#define ESP_LINE_END            "\r\n"


/* start listening to the frames from esp module */
static void ESP_Listen(esp_dev_t *esp)
{
    /* clear all data prior to listening */
    Queue_Drop(esp->responses, Queue_GetUsed(esp->responses));
    /* mark as ready to accept new data */
    esp->responses_num = 0;
}

/* parse data from the esp module */
static void ESP_Input(esp_dev_t *esp, line_modes_t mode, void *ptr,
    size_t size)
{
    /* response recording is active */
    if ((esp->cmd_sem != SEM_RELEASED) && mode != RX_IPD_DATA) {
        /* is the command parser in active mode */
        if (esp->cmd_state == CMD_ACTIVE) {
            /* finalizing OK sentence received */
            if (size == 2 && memcmp(ptr, "OK", 2) == 0) {
                esp->cmd_state = CMD_DONE, esp->cmd_ec = EOK;
            /* finalizing ERROR sentence received */
            } else if (size == 5 && memcmp(ptr, "ERROR", 5) == 0) {
                esp->cmd_state = CMD_DONE, esp->cmd_ec = EFATAL;
            /* esp never ceases to amaze me: CWJAP command does not result in
             * an ERROR. It results in a FAIL */
            } else if (size == 4 && memcmp(ptr, "FAIL", 4) == 0) {
                esp->cmd_state = CMD_DONE, esp->cmd_ec = EFATAL;
            }
        }

        /* wait as long as anyone is listening */
        while ((esp->cmd_sem != SEM_RELEASED) &&
            Queue_GetFree(esp->responses) < size + sizeof(size))
            Yield();

        /* put the data into the responses queue */
        if (esp->cmd_sem != SEM_RELEASED) {
            /* put the data into the queue */
            Queue_Put(esp->responses, &size, sizeof(size));
            Queue_Put(esp->responses, ptr, size);
            /* bump up the number of responses */
            esp->responses_num++;
        }
    }

    dprintf_i("mode = %d, len %d, sentence = %.*s\n", mode, size, size, ptr);
}

/* test the esp connection */
static void ESP_RxPoll(void *arg)
{
    /* esp device */
    esp_dev_t *esp = arg;
    /* internal buffer */
    uint8_t buf[2048]; size_t buf_size = 0;
    /* we are either in the line mode, prompt receive mode '>' or
     * binary data receive mode */
    line_modes_t mode = RX_LINE;

    /* ipd (binary data carrying) sentence variables */
    size_t ipd_line_len, ipd_data_len, ipd_ch;

    /* endless loop of frame polling */
    for (;; Sleep(300)) {
        /* receive the data from the usart */
        err_t ec = USART_Recv(esp->usart, buf + buf_size,
            sizeof(buf) - buf_size, 0);
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
                if (s == end) goto end;

                /* go across the data until you find the '\n' */
                for (e = s; e < end && *e != '\n'; e++);

                /* maybe it's a prompt? */
                if (e - s >= 1 && *s == '>') {
                    mode = RX_PROMPT; goto again;
                /* if we have at least four characters in the buffer, then
                 * maybe we are dealing with +IPD notification? */
                } else if (e - s >= 4 && memcmp(s, "+IPD", 4) == 0) {
                    mode = RX_IPD_LINE; goto again;
                /* all the other stuff is treated as a normal line. if there is
                 * no newline detected, then we need to end processing for now
                 * and wait for more data */
                } else if (e == end) {
                    goto end;
                }

                /* swallow the trailing space by going back */
                for (; e > s && isspace(*(e-1)); e--);
            } break;
            /* data notification line that ends with ':' */
            case RX_IPD_LINE: {
                /* go across the data until you find the ':' */
                for (e = s; e < end && *e != ':'; e++);
                /* no ending found */
                if (e == end) goto end;
                /* store the line length */
                ipd_line_len = e - s + 1;

                /* extract the length field */
                if (snscanf((char *)s, e - s, "+IPD, %d, %d", &ipd_ch,
                    &ipd_data_len) != 2) {
                    dprintf_w("unable to parse +ipd sentence: %.*s\n",
                        e - s, s);
                    /* unable to parse, restart in line mode */
                    s = e + 1; mode = RX_LINE; goto again;
                }
                /* advance to the data state */
                mode = RX_IPD_DATA; goto again;
            } break;
            /* data part of the data notification */
            case RX_IPD_DATA: {
                /* not enough data gathered */
                if (end - s < ipd_data_len + ipd_line_len)
                    goto end;
                /* update the end pointer */
                e = s + ipd_data_len + ipd_line_len;
            } break;
            /* we are about to process prompt */
            case RX_PROMPT: {
                /* prompt is just a single character */
                e = s + 1;
            } break;

            /* unknown mode */
            default: assert (0, "unsupported mode");
            }

            /* process the chunk of data and go again */
            ESP_Input(esp, mode, s, e - s); s = e;
            /* go back to normal mode */
            mode = RX_LINE;
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

/* send data via esp interface */
static err_t ESP_Send(esp_dev_t *dev, const void *ptr, size_t size)
{
    return USART_Send(dev->usart, ptr, size, 0);
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
static err_t ESP_RenderSentence(char *out, size_t size,
    const char *cmd, const char *fmt, va_list args)
{
    /* current position within the string */
    size_t pos = 0;

    /* append command */
    pos += snprintf(out + pos, size - pos, "%s", cmd);
    /* continue with parameters */
    pos += ESP_RenderParams(out + pos, size - pos, fmt, args);
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

    /* do the proper parsing */
    end: matches_found = vsnscanf(str, size, fmt, args);
    /* report an error if number of matches does not match the expected number */
    return matches_found != matches_needed ? EFATAL : EOK;
}

/* parse the response, put values into arguments 'args' */
static err_t ESP_ParseResponse(esp_dev_t *esp, const char *fmt,
    va_list args)
{
    /* operation error code */
    err_t ec = EOK;
    /* size of the response */
    size_t size = 0; char params[ESP_MAX_LINE_LEN];

    /* this tblock does not have any responses */
    if (!esp->responses_num)
        return EFATAL;
    /* get the response size and the response itself */
    Queue_Get(esp->responses, &size, sizeof(size));
    Queue_Get(esp->responses, params, size);
    /* consume response */
    esp->responses_num--;
    /* zero terminate param string */
    params[size] = '\0';

    /* scanf parameters string */
    ec = ESP_ScanParams(params, size, fmt, args);

    /* return status */
    return ec;
}


/* perform the transaction */
int ESP_Transaction(esp_dev_t *dev, dtime_t timeout, const char *cmd,
    const char *cmd_fmt, const char *rsp_fmt, ...)
{
    /* error code */
    err_t ec;
    /* support for variable argument list */
    va_list args;
    /* command sentence will be rendered here */
    char sentence[ESP_MAX_LINE_LEN];

    /* prepare variable argument list */
    va_start(args, rsp_fmt);
    /* render whole command sentence */
    ec = ESP_RenderSentence(sentence, sizeof(sentence), cmd, cmd_fmt, args);
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

    /* loop until command is processed or timeout */
    for (time_t ts = time(0); ; Yield()) {
        /* check for timeout */
        if (timeout && dtime(time(0), ts) > timeout) {
            ec = ETIMEOUT; break;
        }
        /* was the command processed? */
        if (dev->cmd_state == CMD_DONE) {
            ec = dev->cmd_ec; break;
        }
    }

    /* response format was given, try to extract the response from the response
     * queue */
    if (ec == EOK && rsp_fmt) {
        /* since the response format was given it means that we expect the
         * response to come */
        ec = EFATAL;
        /* do the parsing */
        while (dev->responses_num)
            if ((ec = ESP_ParseResponse(dev, rsp_fmt, args)) >= EOK)
                break;
    }

    /* drop variable argument list */
    end: va_end(args);
    /* reset the command mode */
    dev->cmd_state = CMD_DONE;
    /* return the status code */
    return ec;
}


/* process response data that was captured during transaction */
err_t ESP_GetResponse(esp_dev_t *dev, const char *fmt, ...)
{
    /* operation error code */
    err_t ec = EOK;

    /* prepare the list of variable arguments */
    va_list args; va_start(args, fmt);
    /* scan parameters */
    ec = ESP_ParseResponse(dev, fmt, args);
    /* finalize the variable arguments list */
    va_end(args);

    /* return processing error code */
    return ec;
}

/* initialize esp device */
err_t ESP_DevInit(esp_dev_t *dev)
{
    /* start the task */
    err_t ec = Yield_Task(ESP_RxPoll, dev, 2700);
    /* sanity check */
    assert(ec >= EOK, "unable create a task for handling rx");

    /* allocate command response queue */
    dev->responses = Queue_Create(1, 256);
    /* sanitize */
    assert(dev->responses, "unable to allocate response queue");

    /* report success */
    return EOK;
}

/* send AT command */
err_t ESPCmd_AT(esp_dev_t *dev)
{
    /* operation error code */
    err_t ec = EOK;
    /* lock the command interface */
    Sem_Lock(&dev->cmd_sem, 0); {
        /* start listening to responses */
        ESP_Listen(dev);
        /* do the transaction */
        ec = ESP_Transaction(dev, 1000, "AT", 0, 0);
    /* release the command interface */
    } Sem_Release(&dev->cmd_sem);

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
    /* operation error code */
    err_t ec = EOK;
    /* lock the command interface */
    Sem_Lock(&dev->cmd_sem, 0); {
        /* start listening to responses */
        ESP_Listen(dev);
        /* do the transaction */
        ec = ESP_Transaction(dev, 1000, "AT+CWMODE_CUR=", "%d", 0, mode);
    /* release the command interface */
    } Sem_Release(&dev->cmd_sem);

    /* return the error code */
    return ec;
}

/* gets the wifi mode */
err_t ESPCmd_GetCurrentWiFiMode(esp_dev_t *dev, esp_wifi_mode_t *mode)
{
    /* operation error code */
    err_t ec = EOK;
    /* lock the command interface */
    Sem_Lock(&dev->cmd_sem, 0); {
        /* start listening to responses */
        ESP_Listen(dev);
        /* do the transaction */
        ec = ESP_Transaction(dev, 1000, "AT+CWMODE_CUR?", 0,
            "+CWMODE_CUR:%i%", mode);
    /* release the command interface */
    } Sem_Release(&dev->cmd_sem);

    /* return the error code */
    return ec;
}

/* set the discovery results options */
err_t ESPCmd_SetDiscoverOptions(esp_dev_t *dev, int sort_by_rssi)
{
    /* operation error code */
    err_t ec = EOK;
    /* lock the command interface */
    Sem_Lock(&dev->cmd_sem, 0); {
        /* start listening to responses */
        ESP_Listen(dev);
        /* do the transaction */
        ec = ESP_Transaction(dev, 5000, "AT+CWLAPOPT=", "%d,%d", 0,
            !!sort_by_rssi, 127);
        /* release the command interface */
    } Sem_Release(&dev->cmd_sem);

    /* report status */
    return ec;
}

/* discover access point nearby */
err_t ESPCmd_DiscoverAPs(esp_dev_t *dev, esp_ap_t *aps, size_t aps_num)
{
    /* operation error code */
    err_t ec = EOK;
    /* lock the command interface */
    Sem_Lock(&dev->cmd_sem, 0); {
        /* start listening to responses */
        ESP_Listen(dev);
        /* do the transaction */
        ec = ESP_Transaction(dev, 5000, "AT+CWLAP", 0, 0);
        /* transaction complete? */
        if (ec >= EOK) {
            /* ap */
            esp_ap_t *ap = aps;
            /* go through all responses */
            while (dev->responses_num && ap - aps < aps_num) {
                /* parse the response (here we do not use all of the arguments,
                 * as we dont really need them) */
                if (ESP_GetResponse(dev, "+CWLAP:(%d,\"%.*s\",%d,\"%.*s\",", 0,
                    sizeof(ap->ssid), ap->ssid, &ap->rssi,
                    sizeof(ap->mac), ap->mac) >= EOK)
                    ap++;
            }
            /* store the access point number */
            ec = ap - aps;
        }
    /* release the semaphore */
    } Sem_Release(&dev->cmd_sem);

    /* report the status */
    return ec;
}

/* connect to the access point */
err_t ESPCmd_ConnectToAP(esp_dev_t *dev, const char *ssid, const char *pass,
    const char *mac, esp_conn_error_code_t *code)
{
    /* operation error code */
    err_t ec = EOK; //TODO: escaping
    /* connection error code */
    esp_conn_error_code_t _code = ESP_CONN_ERROR_CODE_UNKNOWN;
    /* lock the command interface */
    Sem_Lock(&dev->cmd_sem, 0); {
        /* start listening to responses */
        ESP_Listen(dev);
        /* do the transaction */
        ec = ESP_Transaction(dev, 10000, "AT+CWJAP_CUR=", "\"%s\",\"%s\"%s%s%s", 0,
            ssid, pass, mac ? ",\"" : "", mac ? mac : "", mac ? "\"" : "");
        /* transaction complete? */
        if (ec != ETIMEOUT) {
            /* go through all responses */
            while (dev->responses_num)
                if (ESP_GetResponse(dev, "+CWJAP:%d", &code) >= EOK)
                    break;
        }

        /* release the command interface */
    } Sem_Release(&dev->cmd_sem);

    /* caller wants to know the code */
    if (code) *code = _code;
    /* report status */
    return ec;
}

/* connect to the access point */
err_t ESPCmd_DisconnectFromAP(esp_dev_t *dev)
{
    /* operation error code */
    err_t ec = EOK;
    /* lock the command interface */
    Sem_Lock(&dev->cmd_sem, 0); {
        /* start listening to responses */
        ESP_Listen(dev);
        /* do the transaction */
        ec = ESP_Transaction(dev, 5000, "AT+CWQAP", 0, 0);
        /* release the command interface */
    } Sem_Release(&dev->cmd_sem);

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
    err_t ec = EOK;
    /* lock the command interface */
    Sem_Lock(&dev->cmd_sem, 0); {
        /* start listening to responses */
        ESP_Listen(dev);
        /* do the transaction */
        ec = ESP_Transaction(dev, 5000, "AT+CWDHCP_DEF=", "%d,%d", 0, mode,
            !!enabled);
        /* release the command interface */
    } Sem_Release(&dev->cmd_sem);

    /* report status */
    return ec;
}

/* set the ip address of the module if in static mode */
err_t ESPCmd_SetIPAddress(esp_dev_t *dev, const char *addr, const char *gateway,
    const char *netmask)
{
    /* operation error code */
    err_t ec = EOK;
    /* if gateway is given then the netmask must be given as well */
    if ((gateway && !netmask) || (netmask && !gateway))
        return EARGVAL;

    /* lock the command interface */
    Sem_Lock(&dev->cmd_sem, 0); {
        /* start listening to responses */
        ESP_Listen(dev);
        /* do the transaction */
        ec = ESP_Transaction(dev, 5000, "AT+CIPSTA_CUR=", "\"%s\"%s%s%s%s%s", 0,
            addr,
            gateway ? ",\"" : "", gateway ? gateway : "",
            gateway ? "\"," : "", gateway ? netmask : "",
            gateway ? "\""  : "");
        /* release the command interface */
    } Sem_Release(&dev->cmd_sem);

    /* report status */
    return ec;
}

/* set the ip address of the module if in static mode */
err_t ESPCmd_GetIPAddress(esp_dev_t *dev, char addr[16], char gateway[16],
    char netmask[16])
{
    /* operation error code */
    err_t ec = EOK;

    /* lock the command interface */
    Sem_Lock(&dev->cmd_sem, 0); {
        /* start listening to responses */
        ESP_Listen(dev);
        /* do the transaction */
        ec = ESP_Transaction(dev, 5000, "AT+CIPSTA_CUR?", 0,
            0, 0, addr);

        /* transaction complete? */
        if (ec >= EOK) {
            /* state machine */
            enum { CIPSTA_IP, CIPSTA_GATEWAY, CIPSTA_NETMASK,
                CIPSTA_DONE } state = CIPSTA_IP;
            /* parse responses in order */
            while (dev->responses_num && state != CIPSTA_DONE) {
                /* process responses in order */
                switch (state) {
                /* wait for ip address */
                case CIPSTA_IP: {
                    if (ESP_GetResponse(dev, "+CIPSTA_CUR:ip:\"%s\"",
                        addr) >= EOK)
                        state = CIPSTA_GATEWAY;
                } break;
                /* wait for the gateway address */
                case CIPSTA_GATEWAY: {
                    if (ESP_GetResponse(dev, "+CIPSTA_CUR:gateway:\"%s\"",
                        gateway) >= EOK)
                        state = CIPSTA_NETMASK;
                } break;
                /* wait for the netmask address */
                case CIPSTA_NETMASK: {
                    if (ESP_GetResponse(dev, "+CIPSTA_CUR:netmask:\"%s\"",
                        netmask) >= EOK)
                        state = CIPSTA_DONE;
                } break;
                }
            }

            /* bro, we are missing responses */
            if (state != CIPSTA_DONE)
                ec = EFATAL;
        }

        /* release the command interface */
    } Sem_Release(&dev->cmd_sem);

    /* report status */
    return ec;
}

/* wifi credentials */
static const char pass[] = "dbtn3kmhds45g6p9";
static const char ssid[] = "MT7915-2G";
static const char bssid[] = "ac:91:9b:fb:a7:72";

/* test the esp connection */
static void TestESP_Poll(void *arg)
{
    /* device descriptor */
    esp_dev_t *dev = arg;
    /* error code */
    err_t ec;

    esp_wifi_mode_t mode;
    esp_conn_error_code_t con_error_code;

    esp_ap_t aps[10];

    char addr[16], gateway[16], netmask[16];

    /* testing loop */
    for (;; Sleep(1000)) {
        /* execute command */
        ec = ESPCmd_AT(dev);
        ec = ESPCmd_SetCurrentWiFiMode(dev, ESP_WIFI_MODE_STATION);
        ec = ESPCmd_SetDiscoverOptions(dev, 1);
        ec = ESPCmd_ConfigureDHCP(dev, ESP_DHCP_MODE_CLIENT, 0);
        ec = ESPCmd_DiscoverAPs(dev, aps, sizeof(aps));
        if (ec > 0) {
            for (esp_ap_t *ap = aps; ec != ap - aps; ap++) {
                dprintf_i("rssi = %d, ssid = %s, mac = %s\n", ap->rssi,
                    ap->ssid, ap->mac);
            }
        }

        /* try to connect to the ap */
        ec = ESPCmd_ConnectToAP(dev, ssid, pass, bssid, &con_error_code);
        dprintf_i("connection ec = %d, code = %d\n", ec, con_error_code);
        // Sleep(2000);

        ec = ESPCmd_SetIPAddress(dev, "192.168.1.123", 0, 0);
        ec = ESPCmd_GetIPAddress(dev, addr, gateway, netmask);
        /* show addresses */
        dprintf_i("ip addess is = %.*s\n", sizeof(addr), addr);
        dprintf_i("gateway addess is = %.*s\n", sizeof(gateway), gateway);
        dprintf_i("netmask addess is = %.*s\n", sizeof(netmask), netmask);



        Sleep(2000);

        ec = ESPCmd_DisconnectFromAP(dev);
        dprintf_i("disconnected ec = %d\n", ec);
        Sleep(2000);


        /* display the result */
        // dprintf_i("command done, ec = %d\n", ec);
    }
}


/* device descriptor */
esp_dev_t dev = { .usart = &usart2 };

/* initialize the test */
err_t TestESP_Init(void)
{

    /* initialize the device */
    ESP_DevInit(&dev);
    /* start the testing task */
    Yield_Task(TestESP_Poll, &dev, 1324);


    return EOK;
}