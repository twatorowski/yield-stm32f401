/**
 * @file main.c
 *
 * @date 23.06.2019
 * @author twatorowski (tw@mightydevices.com)
 *
 * @brief main application file
 */

#include "compiler.h"
#include "vectors.h"

#include "dev/analog.h"
#include "dev/cpuclock.h"
#include "dev/dma.h"
#include "dev/fpu.h"
#include "dev/gpio.h"
#include "dev/led.h"
#include "dev/seed.h"
#include "dev/spi_dev.h"
#include "dev/spi.h"
#include "dev/swi2c_dev.h"
#include "dev/swi2c.h"
#include "dev/usart_dev.h"
#include "dev/usart.h"
#include "dev/usb_core.h"
#include "dev/usb_eem.h"
#include "dev/usb_vcp.h"
#include "dev/usb.h"
#include "net/dhcp/server.h"
#include "net/mdns/server.h"
#include "net/tcpip/tcpip.h"
#include "net/uhttpsrv/uhttpsrv.h"
#include "sys/heap.h"
#include "sys/queue.h"
#include "sys/sem.h"
#include "sys/sleep.h"
#include "sys/yield.h"
#include "util/jenkins.h"
#include "util/string.h"
#include "www/api.h"
#include "www/website.h"
#include "www/ws.h"

#define DEBUG DLVL_INFO
#include "debug.h"
#include "coredump.h"


#include "test/esp.h"

// TODO:
/*
 * 1. dhcp client
 * 2. dns client
 * 3. mqtt
*/


/* program main function, must return int so that gcc does not complain in
 * pedantic mode (-Wmain) */
void Main(void *arg);

/* program init function, called before main (with interrupts disabled) */
void Init(void)
{
    /* initialize dynamic memory */
    Heap_Init();
    /* initialize system timer */
    Time_Init();
    /* get the debugging going if in development mode */
    Debug_Init();
    /* start the context switcher */
    Yield_Init();

    /* create the entry task */
    Yield_Task(Main, 0, 2048);
    /* this shall initialize the scheduler */
    Yield_Start();
}

/* program main function */
void Main(void *arg)
{
    /* start the fpu */
    FPU_Init();
    /* configure the system clock */
    CpuClock_Init();

    /* initialize gpio */
    GPIO_Init();
    /* initialize dma controller */
    DMA_Init();
    /* initialize adc */
    Analog_Init();
    /* initialize pseudo random number generator */
    Seed_Init();

    /* initialize usart driver */
    USART_Init();
    /* initialize usart devices */
    USARTDev_Init();

    /* initialize leds */
    Led_Init();
    /* drive the led */
    Led_SetState(1, LED_BLU);

    /* initialize usb status */
    USB_Init();
    /* initialize core logic */
    USBCore_Init();
    /* start the serial port */
    USBVCP_Init();
    /* and the network interface */
    USBEEM_Init();

    /* initialize tcp/ip stack */
    TCPIP_Init();

    /* start the dhcp server */
    DHCPSrv_Init();
    /* start the mdns server */
    MDNSSrv_Init();

    /* initialize common logic to all http servers */
    UHTTPSrv_Init();

    /* initialize http website server */
    // HTTPSrvWebsite_Init();
    // /* initialize http api server */
    // HTTPSrvApi_Init();
    // /* start the websocket server */
    // WebSocketSrv_Init();

    /* print a welcome message */
    dprintf(DLVL_INFO, "Welcome to Yield OS\n", 0);
    /* print the coredump if present */
    CoreDump_PrintDump(1);

    /* start the esp test */
    TestESP_Init();


    /* infinite loop */
    for (;; Yield());
}
