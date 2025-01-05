/**
 * @file config.h
 *
 * @date 29.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * Main configuration file
 */

#ifndef CONFIG
#define CONFIG

#include <stdint.h>



/** Development configuration */
#ifndef DEVELOPMENT
/** Development flag. Used to generate the code with all of the printf
 * debugging enabled */
#define DEVELOPMENT                                 1
#endif


/** System Clock configuration */
/** mcu frequency */
#define CPUCLOCK_HZ                                 84000000
/** ahb frequency */
#define AHBCLOCK_HZ                                 (CPUCLOCK_HZ / 1)
/** apb1 bus frequency */
#define APB1CLOCK_HZ                                (CPUCLOCK_HZ / 2)
/** apb2 bus frequency */
#define APB2CLOCK_HZ                                (CPUCLOCK_HZ / 1)


/** Debug configuration */
/** default debug level  */
#define DEBUG_DEFAULT_LEVEL                         DLVL_WARN
/** maximal length of the debug line string  */
#define DEBUG_MAX_LINE_LEN                          256



/** DMA assignment */
/** usart1 tx dma peripheral */
#define DMA_USART1_TX_PERIPH                        DMA_2
/** usart1 tx dma stream */
#define DMA_USART1_TX_STREAM                        DMA_STREAM_7
/** usart1 tx dma channel */
#define DMA_USART1_TX_CHANNEL                       DMA2_S7_USART1_TX
/** usart1 rx dma peripheral */
#define DMA_USART1_RX_PERIPH                        DMA_2
/** usart1 rx dma stream */
#define DMA_USART1_RX_STREAM                        DMA_STREAM_2
/** usart1 rx dma channel */
#define DMA_USART1_RX_CHANNEL                       DMA2_S2_USART1_RX

/** spi tx dma peripheral */
#define DMA_SPI1_TX_PERIPH                          DMA_2
/** spi tx dma stream */
#define DMA_SPI1_TX_STREAM                          DMA_STREAM_5
/** spi tx dma channel */
#define DMA_SPI1_TX_CHANNEL                         DMA2_S5_SPI1_TX
/** spi rx dma peripheral */
#define DMA_SPI1_RX_PERIPH                          DMA_2
/** spi rx dma stream */
#define DMA_SPI1_RX_STREAM                          DMA_STREAM_0
/** spi rx dma channel */
#define DMA_SPI1_RX_CHANNEL                         DMA2_S0_SPI1_RX


/** Yield configuration */
/** Memory for the tasks */
#define SYS_HEAP_SIZE                               (36 * 1024)
/** coroutine stack size */
#define SYS_CORO_STACK_SIZE                         256
/** maximal number of concurrently running coroutines */
#define SYS_CORO_MAX_NUM                            4
/** sys max event callback subscribers */
#define SYS_EV_MAX_CBS                              8


/** Interrupt priorities */
/** systick overflow */
#define INT_PRI_SYSTICK                             0x00
/** context switcher */
#define INT_PRI_YIELD                               0xf0



/** USART1 Configuration */
/** baudrate */
#define USART1_BAURDRATE                            115200


/** USB Configuration */
/* usb uses common fifo for reception so we need to set it's size to
 * hold the largest packet possible */
#define USB_RX_FIFO_SIZE                            384
/** control endpoint max packet size */
#define USB_CTRLEP_SIZE                             64
/** virtual com port interrupt endpoint frame size */
#define USB_VCP_INT_SIZE                            8
/** virtual com port transmission frame size */
#define USB_VCP_TX_SIZE                             32
/** virtual com port reception frame size */
#define USB_VCP_RX_SIZE                             32
/** virtual ethernet transmission frame size */
#define USB_EEM_TX_SIZE                             64
/** virtual ethernet reception frame size */
#define USB_EEM_RX_SIZE                             64


/** USBCore Configuration */
/* maximal number of interfaces */
#define USBCORE_MAX_IFACE_NUM                       10


/** USBEEM Configuration */
/* maximal size of the ethernet frame */
#define USBEEM_MAX_ETH_FRAME_LEN                    1518
/* size of rx buffer expressed in number of ethernet frames */
#define USBEEM_RX_BUF_CAPACITY                      2
/* size of tx buffer expressed in number of ethernet frames */
#define USBEEM_TX_BUF_CAPACITY                      2


/** TCP/IP Stack configuration: Rx/Tx */
/* reception buffer size - must facilitate one full ethernet frame */
#define TCPIP_RXTX_BUF_SIZE                         1528


/** TCP/IP Stack configuration: Ethetnet */
/* mac address */
#define TCPIP_ETH_ADDRESS                           \
    TCPIP_ETH_ADDR(0x00, 0x01, 0x02, 0x03, 0x04, 100)


/** TCP/IP Stack configuration: Address Resolution Protocol */
/* number of records stored in arp table */
#define TCPIP_ARP_TABLE_SIZE                        5
/* number of attempts that arp engine uses to look for the hardware
 * address within the arp table and issue requests when none is found */
#define TCPIP_ARP_ATTEMPTS                          5


/** TCP/IP Stack configuration: IP */
/* local ip address */
#define TCPIP_IP_ADDRESS                            \
    TCPIP_IP_ADDR(192, 168, 50, 124)
/* sub-network mask */
#define TCPIP_IP_NETMASK                            \
    TCPIP_IP_ADDR(255, 255, 255, 0)
/* gateway address */
#define TCPIP_IP_GATEWAY                            \
    TCPIP_IP_ADDR(192, 168, 50, 124)


/** TCP/IP Stack configuration: TCP */
#define TCPIP_TCP_SOCK_NUM                          4

/** TCP/IP Stack configuration: UDP */
#define TCPIP_UDP_SOCK_NUM                          4



/** DHCP Server configuration */
/* dhcp default port */
#define DHCP_SRV_PORT                               67
/* start of the ip range that we can assign */
#define DHCP_SRV_IP_RANGE_START                     \
    TCPIP_IP_ADDR(192, 168, 50, 100)
/* end of the ip range (exclusive) that we can assign */
#define DHCP_SRV_IP_RANGE_END                       \
    TCPIP_IP_ADDR(192, 168, 50, 110)
/* size of the recordbook (basically the number of clients that lease an ip) */
#define DHCP_SRV_RECORDBOOK_CAPACITY                4


/** MDNS Server configuration */
/* mdns default port */
#define MDNS_SRV_PORT                               5353
/* mdns multicast ip address */
#define MDNS_SRV_MCAST_IP                           \
    TCPIP_IP_ADDR(224, 0, 0, 251)
/* device name */
#define MDNS_SRV_DEVICE_NAME                        "stm32.local"



/**  HTTPSrv configuration */
/* maximal line length in the http request */
#define UHTTPSRV_MAX_LINE_LEN                       256


/**  Websocket configuration */
/* maximal length of the line of text in http header that is being
 * send/received during connection establishment */
#define WEBSOCKETS_MAX_LINE_LEN                     256


#endif /* CONFIG_H_ */
