/**
 * @file ws.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-05
 * 
 * @copyright Copyright (c) 2025
 */

#include "dev/led.h"
#include "net/websocket/websocket.h"
#include "sys/yield.h"
#include "util/string.h"

#define DEBUG DLVL_INFO
#include "debug.h"


/* serve the api */
static void WebSocketSrv_Serve(void *arg)
{
    /* error code */
    err_t ec; uint8_t buf[64]; const char *rsp;
    /* create the socket */
    websocket_t *ws = WebSocket_Create();

    /* data type placeholder */
    websocket_data_type_t dtype;

    /* poll the websocket */
    for (;; Yield()) {
        /* listen to the socket */
        if ((ec = WebSocket_Listen(ws, 42069, 0, 0)) < EOK)
            continue;

        dprintf_i("we are now connected\n", 0);

        /* process frames as they come */
        for (;; Yield()) {
            /* read the frame */
            if ((ec = WebSocket_Recv(ws, &dtype, buf, sizeof(buf) - 1, 0)) < EOK)
                break;

            /* zero terminate the received string of data */
            buf[ec] = 0; dprintf_i("data received: %s\n", buf);
            /* get the current state of the led */
            int led_state = Led_GetState(LED_RED);

            /* caller wants to toggle the led? */
            if (memcmp(buf, "toggle", 6) == 0) {
                Led_SetState(!led_state, LED_RED); rsp = "led was TOGGLED";
            /* enable led */
            } else if (memcmp(buf, "on", 2) == 0) {
                Led_SetState(1, LED_RED); rsp = "led is now ON\n";
            /* disable led */
            } else if (memcmp(buf, "off", 3) == 0) {
                Led_SetState(0, LED_RED); rsp = "led is now OFF\n";
            /* keep-alive */
            } else if (memcmp(buf, "ping", 4) == 0) {
                rsp = "pong";
            /* unknown command */
            } else {
                rsp = "unknown command :-(\n";
            }

            /* send the response back */
            if ((ec = WebSocket_Send(ws, WS_DATA_TYPE_TEXT, rsp,
                strlen(rsp))) < EOK)
                break;
        }

        /* close the connection */
        WebSocket_Close(ws);
        dprintf_i("websocket disconnected\n", 0);
    }
}


/* initialize the websocket based api */
err_t WebSocketSrv_Init(void)
{
    /* report status */
    return Yield_Task(WebSocketSrv_Serve, 0, 2500);
}