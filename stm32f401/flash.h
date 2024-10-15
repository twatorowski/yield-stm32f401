/**
 * @file flash.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_FLASH_H
#define STM32F401_FLASH_H


#include "stm32f401.h"


/* instance base addresses */
#define FLASH_BASE                                           0x40023c00

/* peripheral instances */
#define FLASH                                                ((flash_t *) FLASH_BASE)

/* register bank */
typedef struct {
    reg32_t ACR;
    reg32_t KEYR;
    reg32_t OPTKEYR;
    reg32_t SR;
    reg32_t CR;
    reg32_t OPTCR;
    reg32_t OPTCR1;
} flash_t;


/*******************  Bits definition for FLASH_ACR register  *****************/
#define FLASH_ACR_LATENCY                                    0x00000007
#define FLASH_ACR_LATENCY_0WS                                0x00000000
#define FLASH_ACR_LATENCY_1WS                                0x00000001
#define FLASH_ACR_LATENCY_2WS                                0x00000002
#define FLASH_ACR_LATENCY_3WS                                0x00000003
#define FLASH_ACR_LATENCY_4WS                                0x00000004
#define FLASH_ACR_LATENCY_5WS                                0x00000005
#define FLASH_ACR_LATENCY_6WS                                0x00000006
#define FLASH_ACR_LATENCY_7WS                                0x00000007
#define FLASH_ACR_PRFTEN                                     0x00000100
#define FLASH_ACR_ICEN                                       0x00000200
#define FLASH_ACR_DCEN                                       0x00000400
#define FLASH_ACR_ICRST                                      0x00000800
#define FLASH_ACR_DCRST                                      0x00001000
#define FLASH_ACR_BYTE0_ADDRESS                              0x40023c00
#define FLASH_ACR_BYTE2_ADDRESS                              0x40023c03

/*******************  Bits definition for FLASH_SR register  ******************/
#define FLASH_SR_EOP                                         0x00000001
#define FLASH_SR_SOP                                         0x00000002
#define FLASH_SR_WRPERR                                      0x00000010
#define FLASH_SR_PGAERR                                      0x00000020
#define FLASH_SR_PGPERR                                      0x00000040
#define FLASH_SR_PGSERR                                      0x00000080
#define FLASH_SR_RDERR                                       0x00000100
#define FLASH_SR_BSY                                         0x00010000

/*******************  Bits definition for FLASH_CR register  ******************/
#define FLASH_CR_PG                                          0x00000001
#define FLASH_CR_SER                                         0x00000002
#define FLASH_CR_MER                                         0x00000004
#define FLASH_CR_SNB                                         0x000000f8
#define FLASH_CR_SNB_0                                       0x00000008
#define FLASH_CR_SNB_1                                       0x00000010
#define FLASH_CR_SNB_2                                       0x00000020
#define FLASH_CR_SNB_3                                       0x00000040
#define FLASH_CR_SNB_4                                       0x00000080
#define FLASH_CR_PSIZE                                       0x00000300
#define FLASH_CR_PSIZE_0                                     0x00000100
#define FLASH_CR_PSIZE_1                                     0x00000200
#define FLASH_CR_STRT                                        0x00010000
#define FLASH_CR_EOPIE                                       0x01000000
#define FLASH_CR_ERRIE_Msk                                   0x02000000
#define FLASH_CR_LOCK                                        0x80000000

/*******************  Bits definition for FLASH_OPTCR register  ***************/
#define FLASH_OPTCR_OPTLOCK                                  0x00000001
#define FLASH_OPTCR_OPTSTRT                                  0x00000002
#define FLASH_OPTCR_BOR_LEV_0                                0x00000004
#define FLASH_OPTCR_BOR_LEV_1                                0x00000008
#define FLASH_OPTCR_BOR_LEV                                  0x0000000c
#define FLASH_OPTCR_WDG_SW                                   0x00000020
#define FLASH_OPTCR_nRST_STOP                                0x00000040
#define FLASH_OPTCR_nRST_STDBY                               0x00000080
#define FLASH_OPTCR_RDP                                      0x0000ff00
#define FLASH_OPTCR_RDP_0                                    0x00000100
#define FLASH_OPTCR_RDP_1                                    0x00000200
#define FLASH_OPTCR_RDP_2                                    0x00000400
#define FLASH_OPTCR_RDP_3                                    0x00000800
#define FLASH_OPTCR_RDP_4                                    0x00001000
#define FLASH_OPTCR_RDP_5                                    0x00002000
#define FLASH_OPTCR_RDP_6                                    0x00004000
#define FLASH_OPTCR_RDP_7                                    0x00008000
#define FLASH_OPTCR_nWRP                                     0x0fff0000
#define FLASH_OPTCR_nWRP_0                                   0x00010000
#define FLASH_OPTCR_nWRP_1                                   0x00020000
#define FLASH_OPTCR_nWRP_2                                   0x00040000
#define FLASH_OPTCR_nWRP_3                                   0x00080000
#define FLASH_OPTCR_nWRP_4                                   0x00100000
#define FLASH_OPTCR_nWRP_5                                   0x00200000
#define FLASH_OPTCR_nWRP_6                                   0x00400000
#define FLASH_OPTCR_nWRP_7                                   0x00800000
#define FLASH_OPTCR_nWRP_8                                   0x01000000
#define FLASH_OPTCR_nWRP_9                                   0x02000000
#define FLASH_OPTCR_nWRP_10                                  0x04000000
#define FLASH_OPTCR_nWRP_11                                  0x08000000

/******************  Bits definition for FLASH_OPTCR1 register  ***************/
#define FLASH_OPTCR1_nWRP                                    0x0fff0000
#define FLASH_OPTCR1_nWRP_0                                  0x00010000
#define FLASH_OPTCR1_nWRP_1                                  0x00020000
#define FLASH_OPTCR1_nWRP_2                                  0x00040000
#define FLASH_OPTCR1_nWRP_3                                  0x00080000
#define FLASH_OPTCR1_nWRP_4                                  0x00100000
#define FLASH_OPTCR1_nWRP_5                                  0x00200000
#define FLASH_OPTCR1_nWRP_6                                  0x00400000
#define FLASH_OPTCR1_nWRP_7                                  0x00800000
#define FLASH_OPTCR1_nWRP_8                                  0x01000000
#define FLASH_OPTCR1_nWRP_9                                  0x02000000
#define FLASH_OPTCR1_nWRP_10                                 0x04000000
#define FLASH_OPTCR1_nWRP_11                                 0x08000000


#endif /* STM32F401_FLASH_H */
