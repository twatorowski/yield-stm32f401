/**
 * @file usart.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_USART_H
#define STM32F401_USART_H


#include "stm32f401.h"

/* base addresses */
#define USART1_BASE                                          0x40011000
#define USART2_BASE                                          0x40004400
#define USART6_BASE                                          0x40011400

/* instances */
#define USART1                                              ((usart_t *)USART1_BASE)
#define USART2                                              ((usart_t *)USART2_BASE)
#define USART6                                              ((usart_t *)USART6_BASE)

/* register bank  */
typedef struct {
  reg32_t SR;
  reg32_t DR;
  reg32_t BRR;
  reg32_t CR1;
  reg32_t CR2;
  reg32_t CR3;
  reg32_t GTPR;
} usart_t;


/*******************  Bit definition for USART_SR register  *******************/
#define USART_SR_PE                                          0x00000001
#define USART_SR_FE                                          0x00000002
#define USART_SR_NE                                          0x00000004
#define USART_SR_ORE                                         0x00000008
#define USART_SR_IDLE                                        0x00000010
#define USART_SR_RXNE                                        0x00000020
#define USART_SR_TC                                          0x00000040
#define USART_SR_TXE                                         0x00000080
#define USART_SR_LBD                                         0x00000100
#define USART_SR_CTS                                         0x00000200

/*******************  Bit definition for USART_DR register  *******************/
#define USART_DR_DR                                          0x000001ff

/******************  Bit definition for USART_BRR register  *******************/
#define USART_BRR_DIV_Fraction                               0x0000000f
#define USART_BRR_DIV_Mantissa                               0x0000fff0

/******************  Bit definition for USART_CR1 register  *******************/
#define USART_CR1_SBK                                        0x00000001
#define USART_CR1_RWU                                        0x00000002
#define USART_CR1_RE                                         0x00000004
#define USART_CR1_TE                                         0x00000008
#define USART_CR1_IDLEIE                                     0x00000010
#define USART_CR1_RXNEIE                                     0x00000020
#define USART_CR1_TCIE                                       0x00000040
#define USART_CR1_TXEIE                                      0x00000080
#define USART_CR1_PEIE                                       0x00000100
#define USART_CR1_PS                                         0x00000200
#define USART_CR1_PCE                                        0x00000400
#define USART_CR1_WAKE                                       0x00000800
#define USART_CR1_M                                          0x00001000
#define USART_CR1_UE                                         0x00002000
#define USART_CR1_OVER8                                      0x00008000

/******************  Bit definition for USART_CR2 register  *******************/
#define USART_CR2_ADD                                        0x0000000f
#define USART_CR2_LBDL                                       0x00000020
#define USART_CR2_LBDIE                                      0x00000040
#define USART_CR2_LBCL                                       0x00000100
#define USART_CR2_CPHA                                       0x00000200
#define USART_CR2_CPOL                                       0x00000400
#define USART_CR2_CLKEN                                      0x00000800
#define USART_CR2_STOP                                       0x00003000
#define USART_CR2_STOP_0                                     0x00001000
#define USART_CR2_STOP_1                                     0x00002000
#define USART_CR2_LINEN                                      0x00004000

/******************  Bit definition for USART_CR3 register  *******************/
#define USART_CR3_EIE                                        0x00000001
#define USART_CR3_IREN                                       0x00000002
#define USART_CR3_IRLP                                       0x00000004
#define USART_CR3_HDSEL                                      0x00000008
#define USART_CR3_NACK                                       0x00000010
#define USART_CR3_SCEN                                       0x00000020
#define USART_CR3_DMAR                                       0x00000040
#define USART_CR3_DMAT                                       0x00000080
#define USART_CR3_RTSE                                       0x00000100
#define USART_CR3_CTSE                                       0x00000200
#define USART_CR3_CTSIE                                      0x00000400
#define USART_CR3_ONEBIT                                     0x00000800

/******************  Bit definition for USART_GTPR register  ******************/
#define USART_GTPR_PSC                                       0x000000ff
#define USART_GTPR_PSC_0                                     0x00000001
#define USART_GTPR_PSC_1                                     0x00000002
#define USART_GTPR_PSC_2                                     0x00000004
#define USART_GTPR_PSC_3                                     0x00000008
#define USART_GTPR_PSC_4                                     0x00000010
#define USART_GTPR_PSC_5                                     0x00000020
#define USART_GTPR_PSC_6                                     0x00000040
#define USART_GTPR_PSC_7                                     0x00000080
#define USART_GTPR_GT                                        0x0000ff00


#endif /* STM32F401_USART_H */
