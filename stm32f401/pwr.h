/**
 * @file pwr.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_PWR_H
#define STM32F401_PWR_H

#include "stm32f401.h"

/* instance base addresses */
#define PWR_BASE                                             0x40007000


/* peripheral instances */
#define PWR                                                 ((pwr_t *) PWR_BASE)


/* register bank */
typedef struct {
    reg32_t CR;
    reg32_t CSR;
} pwr_t;

/********************  Bit definition for PWR_CR register  ********************/
#define PWR_CR_LPDS                                          0x00000001
#define PWR_CR_PDDS                                          0x00000002
#define PWR_CR_CWUF                                          0x00000004
#define PWR_CR_CSBF                                          0x00000008
#define PWR_CR_PVDE                                          0x00000010
#define PWR_CR_PLS                                           0x000000e0
#define PWR_CR_PLS_0                                         0x00000020
#define PWR_CR_PLS_1                                         0x00000040
#define PWR_CR_PLS_2                                         0x00000080

/*!< PVD level configuration */
#define PWR_CR_PLS_LEV0                                      0x00000000
#define PWR_CR_PLS_LEV1                                      0x00000020
#define PWR_CR_PLS_LEV2                                      0x00000040
#define PWR_CR_PLS_LEV3                                      0x00000060
#define PWR_CR_PLS_LEV4                                      0x00000080
#define PWR_CR_PLS_LEV5                                      0x000000a0
#define PWR_CR_PLS_LEV6                                      0x000000c0
#define PWR_CR_PLS_LEV7                                      0x000000e0
#define PWR_CR_DBP                                           0x00000100
#define PWR_CR_FPDS                                          0x00000200
#define PWR_CR_LPLVDS                                        0x00000400
#define PWR_CR_MRLVDS                                        0x00000800
#define PWR_CR_ADCDC1                                        0x00002000
#define PWR_CR_VOS                                           0x0000c000
#define PWR_CR_VOS_0                                         0x00004000
#define PWR_CR_VOS_1                                         0x00008000

/* Legacy define */
#define  PWR_CR_PMODE                        PWR_CR_VOS

/*******************  Bit definition for PWR_CSR register  ********************/
#define PWR_CSR_WUF                                          0x00000001
#define PWR_CSR_SBF                                          0x00000002
#define PWR_CSR_PVDO                                         0x00000004
#define PWR_CSR_BRR                                          0x00000008
#define PWR_CSR_EWUP                                         0x00000100
#define PWR_CSR_BRE                                          0x00000200
#define PWR_CSR_VOSRDY                                       0x00004000

/* Legacy define */
#define  PWR_CSR_REGRDY                      PWR_CSR_VOSRDY



#endif /* STM32F401_PWR_H */
