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
#include "sys/yield.h"
#include "sys/sleep.h"

#include "sys/queue.h"
#include "util/string.h"

#define DEBUG DLVL_DEBUG
#include "debug.h"

typedef struct esp_dev {
    /* usart device */
    usart_dev_t *usart;
    /* reception queue */
    queue_t *rxq;


} esp_dev_t;


static void ESP_Input(void *ptr, size_t size)
{
    dprintf_i("sentence = %s\n", ptr);
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
    enum { RX_LINE, RX_IPD_LINE, RX_IPD_DATA, RX_PROMPT } mode = RX_LINE;

    size_t ipd_line_len, ipd_data_len, ipd_ch;

    /* endless loop of frame polling */
    for (;; Yield()) {
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
                if (e - s >= 1 && *s = '>') {
                    mode = RX_PROMPT; goto again;
                /* if we have at least four characters in the buffer, then
                 * maybe we are dealing with +IPD notification? */
                } else if (e - s >= 4 && memcmp(s, "+IPD", 4) == 0) {
                    mode = RX_IPD_LINE; goto again;
                /* no newline found */
                } else if (e == end) {
                    goto end;
                }
                /* swallow the trailing space  by going back */
                for (; e >= s && isspace(*e); e--);
                /* zero- terminate */
                *(++e) = 0;
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
                if (snscanf(s, e - s, "+IPD, %d, %d", &ipd_ch,
                    &ipd_data_len) != 2) {
                    /* unable to parse, restart in line mode */
                    mode = RX_LINE; goto again;
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

            /* unknown mode */
            default: assert (0, "unsupported mode");
            }

            /* no data left to be processed */
            if (s == e)
                break;

            /* process the chunk of data and go again */
            ESP_Input(s, e - s); s = e + 1;
        }

        /* all the chunks have been processed rewind the buffer */
        end: buf_size = s >= end ? 0 : end - s;
        /* still got some bytes in the buffer that we need to bring back
         * to the beginning? */
        if (buf_size && s != buf)
            memcpy(buf, s, buf_size);
    }
}


/* initialize esp device */
err_t ESP_DevInit(esp_dev_t *dev)
{
    /* start the task */
    err_t ec = Yield_Task(ESP_RxPoll, dev, 3000);
    /* sanity check */
    assert(ec >= EOK, "unable create a task for handling rx ");

    /* report success */
    return EOK;
}





/* test the esp connection */
static void TestESP_Poll(void *arg)
{
    /* enable signal */
    gpio_signal_t en = GPIO_SIGNAL_BLACKPILL_A4;
    // uint8_t buf[100];s

    GPIOSig_CfgOutput(en, GPIO_OTYPE_PP, 1);
    // GPIOSig_Set(en, 1);

    for (;; Sleep(1000)) {
        /* send the data */
        USART_Send(&usart2, "AT+GMR\r\n", 8, 0);
        // /* wait for the response */
        // for (;; Yield()) {
        //     err_t ec = USART_Recv(&usart2, buf, sizeof(buf)-1, 4000);
        //     if (ec <= EOK)
        //         break;
        //     buf[ec] = 0;
        //     dprintf_d("data received: %s\n", buf);
        // }

        // dprintf_d("end of loop\n", buf);
    }
}
/* device descriptor */
esp_dev_t dev = { .usart = &usart2 };
/* initialize the test */
err_t TestESP_Init(void)
{

    /* initialize the device */
    ESP_DevInit(&dev);
    Yield_Task(TestESP_Poll, 0, 512);

    return EOK;
}