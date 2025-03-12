/**
 * @file ws.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-02
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "net/websocket/websocket.h"
#include "sys/yield.h"
#include "sys/sleep.h"

#define DEBUG DLVL_INFO
#include "debug.h"

#if 1
/* task */
static void TestWS_Test(void *arg)
{
    /* error code */
    err_t ec; uint8_t buf[256];
    /* create the socket */
    websocket_t *ws = WebSocket_Create();
    /* data type */
    websocket_data_type_t data_type;

    /* */
    for (;; Yield()) {
        /* listen for connections */
        ec = WebSocket_Listen(ws, 42069, 0, 0);
        dprintf_i("WS CONNECTED, ec = %d\n", ec);
        /* error during connection? */
        if (ec < EOK)
            continue;

        /* frame echoing loop */
        for (;; Yield()) {
            /* read the data */
            ec = WebSocket_Recv(ws, &data_type, buf, sizeof(buf), 3000);
            dprintf_i("WS RX, ec = %d\n", ec);
            /* error during reception */
            if (ec < EOK)
                break;


            /* send the data back */
            ec = WebSocket_Send(ws, data_type, "urmom", 5);
            dprintf_i("WS SENT, ec = %d\n", ec);
            if (ec < EOK)
                break;
        }

        /* close the connection */
        WebSocket_Close(ws);
        dprintf_i("WS CLOSE 2\n", 0);
    }
}
#endif

#if 0
/* task */
static void TestWS_TestCli(void *arg)
{
    /* error code */
    err_t ec; uint8_t buf[256];
    /* create the socket */
    websocket_t *ws = WebSocket_Create();
    /* data type */
    websocket_data_type_t data_type;

    /* */
    for (;; Sleep(1000)) {
        /* listen for connections */
        ec = WebSocket_Connect(ws,
            (tcpip_ip_addr_t)TCPIP_IP_ADDR(192, 168, 50, 101), 6969, 0);
        dprintf_i("WS CONNECTED, ec = %d\n", ec);
        /* error during connection? */
        if (ec < EOK)
            continue;

        /* frame echoing loop */
        for (;; Yield()) {
            /* send the data back */
            ec = WebSocket_Send(ws, data_type, "urmom", 5);
            dprintf_i("WS SENT, ec = %d\n", ec);
            if (ec < EOK)
                break;

            /* read the data */
            ec = Websocket_Recv(ws, &data_type, buf, sizeof(buf));
            dprintf_i("WS RX, ec = %d\n", ec);
            /* error during reception */
            if (ec < EOK)
                break;
        }

        /* close the connection */
        WebSocket_Close(ws);
        dprintf_i("WS CLOSE 2\n", 0);
    }
}
#endif
#if 0
/* task */
static void TestWS_TestTCP(void *arg)
{
    /* error code */
    err_t ec = EOK;
    /* create the tcp socket */
    tcpip_tcp_sock_t *sock = TCPIPTcpSock_Create(128, 128);
    /* buffer */
    uint8_t buf[256];


    for (;; Sleep(1000)) {
        /* connect to the tcpip socket */
        ec = TCPIPTcpSock_Connect(sock,
            (tcpip_ip_addr_t)TCPIP_IP_ADDR(192, 168, 50, 101), 6969, 0);
        dprintf_i("TCP CONNECTED, ec = %d\n", ec);


        /* do the exchange */
        ec = TCPIPTcpSock_Send(sock, "tomek", 5, 0);
        dprintf_i("TCP SENT, ec = %d\n", ec);

        ec = TCPIPTcpSock_Recv(sock, buf, sizeof(buf) - 1, 0);
        dprintf_i("TCP RECV, ec = %d\n", ec);

        /* close the socket */
        ec = TCPIPTcpSock_Close(sock, 0);
        dprintf_i("TCP CLOSED, ec = %d\n", ec);

    }
}
#endif

/* test for the websockets */
err_t TestWS_Init(void)
{
    /* start the test task */
    return Yield_Task(TestWS_Test, 0, 3000);
}