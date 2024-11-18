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
#include "dev/spi_dev.h"
#include "dev/spi.h"
#include "dev/swi2c_dev.h"
#include "dev/swi2c.h"
#include "dev/usart_dev.h"
#include "dev/usart.h"
#include "net/tcpip/tcpip.h"
#include "sys/heap.h"
#include "sys/sem.h"
#include "sys/sleep.h"
#include "sys/queue.h"
#include "sys/yield.h"
#include "util/string.h"

#include "dev/usb.h"
#include "dev/usb_core.h"
#include "dev/usb_vcp.h"
#include "dev/usb_eem.h"

#include "net/tcpip/dhcp_srv.h"


#define DEBUG DLVL_INFO
#include "debug.h"

// TODO:
/*
1. MDNS
2. DHCP Server
3. Web Server
4. Test on Windows
5. USB Connected function (based on sof counts) - DONE
6. USB VCP locks ETHERNET for some reason - debug  DONE

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

    /* start the server */
    TCPIPDhcpSrv_Init();

    /* print a welcome message */
    dprintf(DLVL_INFO, "Welcome to Yield OS\n", 0);




    /* infinite loop */
    for (;; Yield());
}
