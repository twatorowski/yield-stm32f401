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

#include "dev/beep.h"
#include "dev/aip650e.h"
#include "dev/display.h"
#include "dev/stepup.h"
#include "dev/keyboard.h"
#include "dev/vusb_detect.h"
#include "dev/charger.h"
#include "dev/pumps.h"
#include "dev/valve.h"
#include "dev/batt.h"
#include "dev/pressure_sense.h"
#include "dev/eeprom.h"
#include "dev/eeprom_dev.h"
#include "dev/husb238.h"
#include "dev/flash.h"
#include "boot/boot.h"
#include "dev/watchdog.h"
#include "dev/standby.h"
#include "pf/pf.h"

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
    /* initialize exception vector array */
    Vectors_Init();
    /* initialize reset source */
    Reset_Init();

    /* initialize dynamic memory */
    Heap_Init();
    /* initialize system timer */
    Time_Init();
    /* get the debugging going if in development mode */
    Debug_Init();
    /* start the context switcher */
    Yield_Init();

    /* kick the dog before jumping to main functions */
    Watchdog_Kick();

    /* create the entry task */
    Yield_Task(Main, 0, 2048);
    /* this shall initialize the scheduler */
    Yield_Start();
}

/* program main function */
void Main(void *arg)
{
    /* kick the dog */
    Watchdog_Kick();

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
    /* initialize flash driver */
    Flash_Init();

    /* initialize usart driver */
    USART_Init();
    /* initialize usart devices */
    USARTDev_Init();

    /* initialize leds */
    Led_Init();
    /* drive the led */
    Led_SetState(1, LED_RED);

    /* initialize i2c */
    SwI2C_Init();
    /* initialize particular i2c ports */
    SwI2CDev_Init();

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
    HTTPSrvWebsite_Init();

    /* initialize step up converter control */
    StepUp_Init();
    /* initialie display drivers */
    Display_Init();
    /* initialize pump drivers */
    Pumps_Init();
    /* initialize keyboard support */
    Kbd_Init();
    /* initialize pressure sensor */
    PressureSense_Init();
    /* initialize valve control */
    Valve_Init();
    /* initialize charger */
    Charger_Init();
    /* initialize Battery Measurement */
    Batt_Init();
    /* usb detection */
    VUSBDet_Init();
    /* usb pd negotiation chip driver */
    HUSB238_Init();
    /* standby mode support */
    StandBy_Init();

    /* print a welcome message */
    dprintf(DLVL_INFO, "Welcome to Yield OS (rst = %x)\n",
        Reset_GetLastResetSource());
    /* print the coredump if prGesent */
    CoreDump_PrintDump(1);

    /* initialize booloader logic */
    Boot_Init();


    /* infinite loop */
    for (;; Yield()) {
        /* kick the dog */
        Watchdog_Kick();
    }
}
