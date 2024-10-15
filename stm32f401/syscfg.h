/**
 * @file syscfg.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_SYSCFG_H
#define STM32F401_SYSCFG_H

#include "stm32f401.h"

/* base addresses */
#define SYSCFG_BASE                                          0x40013800

/* instances */
#define SYSCFG                                              ((syscfg_t *)SYSCFG_BASE)

/* register bank */
typedef struct {
    reg32_t MEMRMP;
    reg32_t PMC;
    reg32_t EXTICR[4];
    reg32_t RESERVED[2];
    reg32_t CMPCR;
} syscfg_t;


/******************  Bit definition for SYSCFG_MEMRMP register  ***************/
#define SYSCFG_MEMRMP_MEM_MODE                               0x00000003
#define SYSCFG_MEMRMP_MEM_MODE_0                             0x00000001
#define SYSCFG_MEMRMP_MEM_MODE_1                             0x00000002
/******************  Bit definition for SYSCFG_PMC register  ******************/
#define SYSCFG_PMC_ADC1DC2                                   0x00010000

/*****************  Bit definition for SYSCFG_EXTICR1 register  ***************/
#define SYSCFG_EXTICR1_EXTI0                                 0x0000000f
#define SYSCFG_EXTICR1_EXTI1                                 0x000000f0
#define SYSCFG_EXTICR1_EXTI2                                 0x00000f00
#define SYSCFG_EXTICR1_EXTI3                                 0x0000f000
#define SYSCFG_EXTICR1_EXTI0_PA                              0x00000000
#define SYSCFG_EXTICR1_EXTI0_PB                              0x00000001
#define SYSCFG_EXTICR1_EXTI0_PC                              0x00000002
#define SYSCFG_EXTICR1_EXTI0_PD                              0x00000003
#define SYSCFG_EXTICR1_EXTI0_PE                              0x00000004
#define SYSCFG_EXTICR1_EXTI0_PH                              0x00000007
#define SYSCFG_EXTICR1_EXTI1_PA                              0x00000000
#define SYSCFG_EXTICR1_EXTI1_PB                              0x00000010
#define SYSCFG_EXTICR1_EXTI1_PC                              0x00000020
#define SYSCFG_EXTICR1_EXTI1_PD                              0x00000030
#define SYSCFG_EXTICR1_EXTI1_PE                              0x00000040
#define SYSCFG_EXTICR1_EXTI1_PH                              0x00000070
#define SYSCFG_EXTICR1_EXTI2_PA                              0x00000000
#define SYSCFG_EXTICR1_EXTI2_PB                              0x00000100
#define SYSCFG_EXTICR1_EXTI2_PC                              0x00000200
#define SYSCFG_EXTICR1_EXTI2_PD                              0x00000300
#define SYSCFG_EXTICR1_EXTI2_PE                              0x00000400
#define SYSCFG_EXTICR1_EXTI2_PH                              0x00000700
#define SYSCFG_EXTICR1_EXTI3_PA                              0x00000000
#define SYSCFG_EXTICR1_EXTI3_PB                              0x00001000
#define SYSCFG_EXTICR1_EXTI3_PC                              0x00002000
#define SYSCFG_EXTICR1_EXTI3_PD                              0x00003000
#define SYSCFG_EXTICR1_EXTI3_PE                              0x00004000
#define SYSCFG_EXTICR1_EXTI3_PH                              0x00007000

/*****************  Bit definition for SYSCFG_EXTICR2 register  ***************/
#define SYSCFG_EXTICR2_EXTI4                                 0x0000000f
#define SYSCFG_EXTICR2_EXTI5                                 0x000000f0
#define SYSCFG_EXTICR2_EXTI6                                 0x00000f00
#define SYSCFG_EXTICR2_EXTI7                                 0x0000f000
#define SYSCFG_EXTICR2_EXTI4_PA                              0x00000000
#define SYSCFG_EXTICR2_EXTI4_PB                              0x00000001
#define SYSCFG_EXTICR2_EXTI4_PC                              0x00000002
#define SYSCFG_EXTICR2_EXTI4_PD                              0x00000003
#define SYSCFG_EXTICR2_EXTI4_PE                              0x00000004
#define SYSCFG_EXTICR2_EXTI4_PH                              0x00000007
#define SYSCFG_EXTICR2_EXTI5_PA                              0x00000000
#define SYSCFG_EXTICR2_EXTI5_PB                              0x00000010
#define SYSCFG_EXTICR2_EXTI5_PC                              0x00000020
#define SYSCFG_EXTICR2_EXTI5_PD                              0x00000030
#define SYSCFG_EXTICR2_EXTI5_PE                              0x00000040
#define SYSCFG_EXTICR2_EXTI5_PH                              0x00000070
#define SYSCFG_EXTICR2_EXTI6_PA                              0x00000000
#define SYSCFG_EXTICR2_EXTI6_PB                              0x00000100
#define SYSCFG_EXTICR2_EXTI6_PC                              0x00000200
#define SYSCFG_EXTICR2_EXTI6_PD                              0x00000300
#define SYSCFG_EXTICR2_EXTI6_PE                              0x00000400
#define SYSCFG_EXTICR2_EXTI6_PH                              0x00000700
#define SYSCFG_EXTICR2_EXTI7_PA                              0x00000000
#define SYSCFG_EXTICR2_EXTI7_PB                              0x00001000
#define SYSCFG_EXTICR2_EXTI7_PC                              0x00002000
#define SYSCFG_EXTICR2_EXTI7_PD                              0x00003000
#define SYSCFG_EXTICR2_EXTI7_PE                              0x00004000
#define SYSCFG_EXTICR2_EXTI7_PH                              0x00007000

/*****************  Bit definition for SYSCFG_EXTICR3 register  ***************/
#define SYSCFG_EXTICR3_EXTI8                                 0x0000000f
#define SYSCFG_EXTICR3_EXTI9                                 0x000000f0
#define SYSCFG_EXTICR3_EXTI10                                0x00000f00
#define SYSCFG_EXTICR3_EXTI11                                0x0000f000
#define SYSCFG_EXTICR3_EXTI8_PA                              0x00000000
#define SYSCFG_EXTICR3_EXTI8_PB                              0x00000001
#define SYSCFG_EXTICR3_EXTI8_PC                              0x00000002
#define SYSCFG_EXTICR3_EXTI8_PD                              0x00000003
#define SYSCFG_EXTICR3_EXTI8_PE                              0x00000004
#define SYSCFG_EXTICR3_EXTI8_PH                              0x00000007
#define SYSCFG_EXTICR3_EXTI9_PA                              0x00000000
#define SYSCFG_EXTICR3_EXTI9_PB                              0x00000010
#define SYSCFG_EXTICR3_EXTI9_PC                              0x00000020
#define SYSCFG_EXTICR3_EXTI9_PD                              0x00000030
#define SYSCFG_EXTICR3_EXTI9_PE                              0x00000040
#define SYSCFG_EXTICR3_EXTI9_PH                              0x00000070
#define SYSCFG_EXTICR3_EXTI10_PA                             0x00000000
#define SYSCFG_EXTICR3_EXTI10_PB                             0x00000100
#define SYSCFG_EXTICR3_EXTI10_PC                             0x00000200
#define SYSCFG_EXTICR3_EXTI10_PD                             0x00000300
#define SYSCFG_EXTICR3_EXTI10_PE                             0x00000400
#define SYSCFG_EXTICR3_EXTI10_PH                             0x00000700
#define SYSCFG_EXTICR3_EXTI11_PA                             0x00000000
#define SYSCFG_EXTICR3_EXTI11_PB                             0x00001000
#define SYSCFG_EXTICR3_EXTI11_PC                             0x00002000
#define SYSCFG_EXTICR3_EXTI11_PD                             0x00003000
#define SYSCFG_EXTICR3_EXTI11_PE                             0x00004000
#define SYSCFG_EXTICR3_EXTI11_PH                             0x00007000

/*****************  Bit definition for SYSCFG_EXTICR4 register  ***************/
#define SYSCFG_EXTICR4_EXTI12                                0x0000000f
#define SYSCFG_EXTICR4_EXTI13                                0x000000f0
#define SYSCFG_EXTICR4_EXTI14                                0x00000f00
#define SYSCFG_EXTICR4_EXTI15                                0x0000f000
#define SYSCFG_EXTICR4_EXTI12_PA                             0x00000000
#define SYSCFG_EXTICR4_EXTI12_PB                             0x00000001
#define SYSCFG_EXTICR4_EXTI12_PC                             0x00000002
#define SYSCFG_EXTICR4_EXTI12_PD                             0x00000003
#define SYSCFG_EXTICR4_EXTI12_PE                             0x00000004
#define SYSCFG_EXTICR4_EXTI12_PH                             0x00000007
#define SYSCFG_EXTICR4_EXTI13_PA                             0x00000000
#define SYSCFG_EXTICR4_EXTI13_PB                             0x00000010
#define SYSCFG_EXTICR4_EXTI13_PC                             0x00000020
#define SYSCFG_EXTICR4_EXTI13_PD                             0x00000030
#define SYSCFG_EXTICR4_EXTI13_PE                             0x00000040
#define SYSCFG_EXTICR4_EXTI13_PH                             0x00000070
#define SYSCFG_EXTICR4_EXTI14_PA                             0x00000000
#define SYSCFG_EXTICR4_EXTI14_PB                             0x00000100
#define SYSCFG_EXTICR4_EXTI14_PC                             0x00000200
#define SYSCFG_EXTICR4_EXTI14_PD                             0x00000300
#define SYSCFG_EXTICR4_EXTI14_PE                             0x00000400
#define SYSCFG_EXTICR4_EXTI14_PH                             0x00000700
#define SYSCFG_EXTICR4_EXTI15_PA                             0x00000000
#define SYSCFG_EXTICR4_EXTI15_PB                             0x00001000
#define SYSCFG_EXTICR4_EXTI15_PC                             0x00002000
#define SYSCFG_EXTICR4_EXTI15_PD                             0x00003000
#define SYSCFG_EXTICR4_EXTI15_PE                             0x00004000
#define SYSCFG_EXTICR4_EXTI15_PH                             0x00007000

/******************  Bit definition for SYSCFG_CMPCR register  ****************/
#define SYSCFG_CMPCR_CMP_PD                                  0x00000001
#define SYSCFG_CMPCR_READY                                   0x00000100



#endif /* STM32F401_SYSCFG_H */
