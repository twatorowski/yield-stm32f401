/**
 * @file rcc.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_RCC_H
#define STM32F401_RCC_H

#include "stm32f401.h"

/* base addresses */
#define RCC_BASE                                             0x40023800

/* instances */
#define RCC                                                  ((rcc_t *)RCC_BASE)

/* register bank */
typedef struct {
    reg32_t CR;
    reg32_t PLLCFGR;
    reg32_t CFGR;
    reg32_t CIR;
    reg32_t AHB1RSTR;
    reg32_t AHB2RSTR;
    reg32_t AHB3RSTR;
    reg32_t RESERVED0;
    reg32_t APB1RSTR;
    reg32_t APB2RSTR;
    reg32_t RESERVED1[2];
    reg32_t AHB1ENR;
    reg32_t AHB2ENR;
    reg32_t AHB3ENR;
    reg32_t RESERVED2;
    reg32_t APB1ENR;
    reg32_t APB2ENR;
    reg32_t RESERVED3[2];
    reg32_t AHB1LPENR;
    reg32_t AHB2LPENR;
    reg32_t AHB3LPENR;
    reg32_t RESERVED4;
    reg32_t APB1LPENR;
    reg32_t APB2LPENR;
    reg32_t RESERVED5[2];
    reg32_t BDCR;
    reg32_t CSR;
    reg32_t RESERVED6[2];
    reg32_t SSCGR;
    reg32_t PLLI2SCFGR;
    reg32_t RESERVED7[1];
    reg32_t DCKCFGR;
} rcc_t;


/********************  Bit definition for RCC_CR register  ********************/
#define RCC_CR_HSION                                         0x00000001
#define RCC_CR_HSIRDY                                        0x00000002
#define RCC_CR_HSITRIM                                       0x000000F8
#define RCC_CR_HSITRIM_0                                     0x00000008
#define RCC_CR_HSITRIM_1                                     0x00000010
#define RCC_CR_HSITRIM_2                                     0x00000020
#define RCC_CR_HSITRIM_3                                     0x00000040
#define RCC_CR_HSITRIM_4                                     0x00000080
#define RCC_CR_HSICAL                                        0x0000FF00
#define RCC_CR_HSICAL_0                                      0x00000100
#define RCC_CR_HSICAL_1                                      0x00000200
#define RCC_CR_HSICAL_2                                      0x00000400
#define RCC_CR_HSICAL_3                                      0x00000800
#define RCC_CR_HSICAL_4                                      0x00001000
#define RCC_CR_HSICAL_5                                      0x00002000
#define RCC_CR_HSICAL_6                                      0x00004000
#define RCC_CR_HSICAL_7                                      0x00008000
#define RCC_CR_HSEON                                         0x00010000
#define RCC_CR_HSERDY                                        0x00020000
#define RCC_CR_HSEBYP                                        0x00040000
#define RCC_CR_CSSON                                         0x00080000
#define RCC_CR_PLLON                                         0x01000000
#define RCC_CR_PLLRDY                                        0x02000000

/********************  Bit definition for RCC_PLLCFGR register  ***************/
#define RCC_PLLCFGR_PLLM                                     0x0000003F
#define RCC_PLLCFGR_PLLM_0                                   0x00000001
#define RCC_PLLCFGR_PLLM_1                                   0x00000002
#define RCC_PLLCFGR_PLLM_2                                   0x00000004
#define RCC_PLLCFGR_PLLM_3                                   0x00000008
#define RCC_PLLCFGR_PLLM_4                                   0x00000010
#define RCC_PLLCFGR_PLLM_5                                   0x00000020
#define RCC_PLLCFGR_PLLN                                     0x00007FC0
#define RCC_PLLCFGR_PLLN_0                                   0x00000040
#define RCC_PLLCFGR_PLLN_1                                   0x00000080
#define RCC_PLLCFGR_PLLN_2                                   0x00000100
#define RCC_PLLCFGR_PLLN_3                                   0x00000200
#define RCC_PLLCFGR_PLLN_4                                   0x00000400
#define RCC_PLLCFGR_PLLN_5                                   0x00000800
#define RCC_PLLCFGR_PLLN_6                                   0x00001000
#define RCC_PLLCFGR_PLLN_7                                   0x00002000
#define RCC_PLLCFGR_PLLN_8                                   0x00004000
#define RCC_PLLCFGR_PLLP                                     0x00030000
#define RCC_PLLCFGR_PLLP_0                                   0x00010000
#define RCC_PLLCFGR_PLLP_1                                   0x00020000
#define RCC_PLLCFGR_PLLSRC                                   0x00400000
#define RCC_PLLCFGR_PLLSRC_HSE                               0x00400000
#define RCC_PLLCFGR_PLLSRC_HSI                               0x00000000
#define RCC_PLLCFGR_PLLQ                                     0x00400000
#define RCC_PLLCFGR_PLLQ_0                                   0x01000000
#define RCC_PLLCFGR_PLLQ_1                                   0x02000000
#define RCC_PLLCFGR_PLLQ_2                                   0x04000000
#define RCC_PLLCFGR_PLLQ_3                                   0x08000000

/********************  Bit definition for RCC_CFGR register  ******************/
#define RCC_CFGR_SW                                          0x00000003
#define RCC_CFGR_SW_0                                        0x00000001
#define RCC_CFGR_SW_1                                        0x00000002
#define RCC_CFGR_SW_HSI                                      0x00000000
#define RCC_CFGR_SW_HSE                                      0x00000001
#define RCC_CFGR_SW_PLL                                      0x00000002
#define RCC_CFGR_SWS                                         0x0000000C
#define RCC_CFGR_SWS_0                                       0x00000004
#define RCC_CFGR_SWS_1                                       0x00000008
#define RCC_CFGR_SWS_HSI                                     0x00000000
#define RCC_CFGR_SWS_HSE                                     0x00000004
#define RCC_CFGR_SWS_PLL                                     0x00000008
#define RCC_CFGR_HPRE                                        0x000000F0
#define RCC_CFGR_HPRE_0                                      0x00000010
#define RCC_CFGR_HPRE_1                                      0x00000020
#define RCC_CFGR_HPRE_2                                      0x00000040
#define RCC_CFGR_HPRE_3                                      0x00000080
#define RCC_CFGR_HPRE_DIV1                                   0x00000000
#define RCC_CFGR_HPRE_DIV2                                   0x00000080
#define RCC_CFGR_HPRE_DIV4                                   0x00000090
#define RCC_CFGR_HPRE_DIV8                                   0x000000A0
#define RCC_CFGR_HPRE_DIV16                                  0x000000B0
#define RCC_CFGR_HPRE_DIV64                                  0x000000C0
#define RCC_CFGR_HPRE_DIV128                                 0x000000D0
#define RCC_CFGR_HPRE_DIV256                                 0x000000E0
#define RCC_CFGR_HPRE_DIV512                                 0x000000F0
#define RCC_CFGR_PPRE1                                       0x00001C00
#define RCC_CFGR_PPRE1_0                                     0x00000400
#define RCC_CFGR_PPRE1_1                                     0x00000800
#define RCC_CFGR_PPRE1_2                                     0x00001000
#define RCC_CFGR_PPRE1_DIV1                                  0x00000000
#define RCC_CFGR_PPRE1_DIV2                                  0x00001000
#define RCC_CFGR_PPRE1_DIV4                                  0x00001400
#define RCC_CFGR_PPRE1_DIV8                                  0x00001800
#define RCC_CFGR_PPRE1_DIV16                                 0x00001C00
#define RCC_CFGR_PPRE2                                       0x0000E000
#define RCC_CFGR_PPRE2_0                                     0x00002000
#define RCC_CFGR_PPRE2_1                                     0x00004000
#define RCC_CFGR_PPRE2_2                                     0x00008000
#define RCC_CFGR_PPRE2_DIV1                                  0x00000000
#define RCC_CFGR_PPRE2_DIV2                                  0x00008000
#define RCC_CFGR_PPRE2_DIV4                                  0x0000A000
#define RCC_CFGR_PPRE2_DIV8                                  0x0000C000
#define RCC_CFGR_PPRE2_DIV16                                 0x0000E000
#define RCC_CFGR_RTCPRE                                      0x001F0000
#define RCC_CFGR_RTCPRE_0                                    0x00010000
#define RCC_CFGR_RTCPRE_1                                    0x00020000
#define RCC_CFGR_RTCPRE_2                                    0x00040000
#define RCC_CFGR_RTCPRE_3                                    0x00080000
#define RCC_CFGR_RTCPRE_4                                    0x00100000
#define RCC_CFGR_MCO1                                        0x00600000
#define RCC_CFGR_MCO1_0                                      0x00200000
#define RCC_CFGR_MCO1_1                                      0x00400000
#define RCC_CFGR_I2SSRC                                      0x00800000
#define RCC_CFGR_MCO1PRE                                     0x07000000
#define RCC_CFGR_MCO1PRE_0                                   0x01000000
#define RCC_CFGR_MCO1PRE_1                                   0x02000000
#define RCC_CFGR_MCO1PRE_2                                   0x04000000
#define RCC_CFGR_MCO2PRE                                     0x38000000
#define RCC_CFGR_MCO2PRE_0                                   0x08000000
#define RCC_CFGR_MCO2PRE_1                                   0x10000000
#define RCC_CFGR_MCO2PRE_2                                   0x20000000
#define RCC_CFGR_MCO2                                        0xC0000000
#define RCC_CFGR_MCO2_0                                      0x40000000
#define RCC_CFGR_MCO2_1                                      0x80000000

/********************  Bit definition for RCC_CIR register  *******************/
#define RCC_CIR_LSIRDYF                                      0x00000001
#define RCC_CIR_LSERDYF                                      0x00000002
#define RCC_CIR_HSIRDYF                                      0x00000004
#define RCC_CIR_HSERDYF                                      0x00000008
#define RCC_CIR_PLLRDYF                                      0x00000010
#define RCC_CIR_PLLI2SRDYF                                   0x00000020
#define RCC_CIR_CSSF                                         0x00000080
#define RCC_CIR_LSIRDYIE                                     0x00000100
#define RCC_CIR_LSERDYIE                                     0x00000200
#define RCC_CIR_HSIRDYIE                                     0x00000400
#define RCC_CIR_HSERDYIE                                     0x00000800
#define RCC_CIR_PLLRDYIE                                     0x00001000
#define RCC_CIR_PLLI2SRDYIE                                  0x00002000
#define RCC_CIR_LSIRDYC                                      0x00010000
#define RCC_CIR_LSERDYC                                      0x00020000
#define RCC_CIR_HSIRDYC                                      0x00040000
#define RCC_CIR_HSERDYC                                      0x00080000
#define RCC_CIR_PLLRDYC                                      0x00100000
#define RCC_CIR_PLLI2SRDYC                                   0x00200000
#define RCC_CIR_CSSC                                         0x00800000

/********************  Bit definition for RCC_AHB1RSTR register  **************/
#define RCC_AHB1RSTR_GPIOARST                                0x00000001
#define RCC_AHB1RSTR_GPIOBRST                                0x00000002
#define RCC_AHB1RSTR_GPIOCRST                                0x00000004
#define RCC_AHB1RSTR_GPIODRST                                0x00000008
#define RCC_AHB1RSTR_GPIOERST                                0x00000010
#define RCC_AHB1RSTR_GPIOHRST                                0x00000080
#define RCC_AHB1RSTR_CRCRST                                  0x00001000
#define RCC_AHB1RSTR_DMA1RST                                 0x00200000
#define RCC_AHB1RSTR_DMA2RST                                 0x00400000

/********************  Bit definition for RCC_AHB2RSTR register  **************/
#define RCC_AHB2RSTR_OTGFSRST                                0x00000080


/********************  Bit definition for RCC_APB1RSTR register  **************/
#define RCC_APB1RSTR_TIM2RST                                 0x00000001
#define RCC_APB1RSTR_TIM3RST                                 0x00000002
#define RCC_APB1RSTR_TIM4RST                                 0x00000004
#define RCC_APB1RSTR_TIM5RST                                 0x00000008
#define RCC_APB1RSTR_WWDGRST                                 0x00000800
#define RCC_APB1RSTR_SPI2RST                                 0x00004000
#define RCC_APB1RSTR_SPI3RST                                 0x00008000
#define RCC_APB1RSTR_USART2RST                               0x00020000
#define RCC_APB1RSTR_I2C1RST                                 0x00200000
#define RCC_APB1RSTR_I2C2RST                                 0x00400000
#define RCC_APB1RSTR_I2C3RST                                 0x00800000
#define RCC_APB1RSTR_PWRRST                                  0x10000000

/********************  Bit definition for RCC_APB2RSTR register  **************/
#define RCC_APB2RSTR_TIM1RST                                 0x00000001
#define RCC_APB2RSTR_USART1RST                               0x00000010
#define RCC_APB2RSTR_USART6RST                               0x00000020
#define RCC_APB2RSTR_ADCRST                                  0x00000100
#define RCC_APB2RSTR_SDIORST                                 0x00000800
#define RCC_APB2RSTR_SPI1RST                                 0x00001000
#define RCC_APB2RSTR_SPI4RST                                 0x00002000
#define RCC_APB2RSTR_SYSCFGRST                               0x00004000
#define RCC_APB2RSTR_TIM9RST                                 0x00010000
#define RCC_APB2RSTR_TIM10RST                                0x00020000
#define RCC_APB2RSTR_TIM11RST                                0x00040000
#define RCC_APB2RSTR_SPI1                                    RCC_APB2RSTR_SPI1RST

/********************  Bit definition for RCC_AHB1ENR register  ***************/
#define RCC_AHB1ENR_GPIOAEN                                  0x00000001
#define RCC_AHB1ENR_GPIOBEN                                  0x00000002
#define RCC_AHB1ENR_GPIOCEN                                  0x00000004
#define RCC_AHB1ENR_GPIODEN                                  0x00000008
#define RCC_AHB1ENR_GPIOEEN                                  0x00000010
#define RCC_AHB1ENR_GPIOHEN                                  0x00000080
#define RCC_AHB1ENR_CRCEN                                    0x00001000
#define RCC_AHB1ENR_DMA1EN                                   0x00200000
#define RCC_AHB1ENR_DMA2EN                                   0x00400000

/********************  Bit definition for RCC_AHB2ENR register  ***************/
#define RCC_AHB2_SUPPORT
#define RCC_AHB2ENR_OTGFSEN                                  0x00000080

/********************  Bit definition for RCC_APB1ENR register  ***************/
#define RCC_APB1ENR_TIM2EN                                   0x00000001
#define RCC_APB1ENR_TIM3EN                                   0x00000002
#define RCC_APB1ENR_TIM4EN                                   0x00000004
#define RCC_APB1ENR_TIM5EN                                   0x00000008
#define RCC_APB1ENR_WWDGEN                                   0x00000800
#define RCC_APB1ENR_SPI2EN                                   0x00004000
#define RCC_APB1ENR_SPI3EN                                   0x00008000
#define RCC_APB1ENR_USART2EN                                 0x00020000
#define RCC_APB1ENR_I2C1EN                                   0x00200000
#define RCC_APB1ENR_I2C2EN                                   0x00400000
#define RCC_APB1ENR_I2C3EN                                   0x00800000
#define RCC_APB1ENR_PWREN                                    0x10000000

/********************  Bit definition for RCC_APB2ENR register  ***************/
#define RCC_APB2ENR_TIM1EN                                   0x00000001
#define RCC_APB2ENR_USART1EN                                 0x00000010
#define RCC_APB2ENR_USART6EN                                 0x00000020
#define RCC_APB2ENR_ADC1EN                                   0x00000100
#define RCC_APB2ENR_SDIOEN                                   0x00000800
#define RCC_APB2ENR_SPI1EN                                   0x00001000
#define RCC_APB2ENR_SPI4EN                                   0x00002000
#define RCC_APB2ENR_SYSCFGEN                                 0x00004000
#define RCC_APB2ENR_TIM9EN                                   0x00010000
#define RCC_APB2ENR_TIM10EN                                  0x00020000
#define RCC_APB2ENR_TIM11EN                                  0x00040000

/********************  Bit definition for RCC_AHB1LPENR register  *************/
#define RCC_AHB1LPENR_GPIOALPEN                              0x00000001
#define RCC_AHB1LPENR_GPIOBLPEN                              0x00000002
#define RCC_AHB1LPENR_GPIOCLPEN                              0x00000004
#define RCC_AHB1LPENR_GPIODLPEN                              0x00000008
#define RCC_AHB1LPENR_GPIOELPEN                              0x00000010
#define RCC_AHB1LPENR_GPIOHLPEN                              0x00000080
#define RCC_AHB1LPENR_CRCLPEN                                0x00001000
#define RCC_AHB1LPENR_FLITFLPEN                              0x00008000
#define RCC_AHB1LPENR_SRAM1LPEN                              0x00010000
#define RCC_AHB1LPENR_DMA1LPEN                               0x00200000
#define RCC_AHB1LPENR_DMA2LPEN                               0x00400000


/********************  Bit definition for RCC_AHB2LPENR register  *************/
#define RCC_AHB2LPENR_OTGFSLPEN                              0x00000080


/********************  Bit definition for RCC_APB1LPENR register  *************/
#define RCC_APB1LPENR_TIM2LPEN                               0x00000001
#define RCC_APB1LPENR_TIM3LPEN                               0x00000002
#define RCC_APB1LPENR_TIM4LPEN                               0x00000004
#define RCC_APB1LPENR_TIM5LPEN                               0x00000008
#define RCC_APB1LPENR_WWDGLPEN                               0x00000800
#define RCC_APB1LPENR_SPI2LPEN                               0x00004000
#define RCC_APB1LPENR_SPI3LPEN                               0x00008000
#define RCC_APB1LPENR_USART2LPEN                             0x00020000
#define RCC_APB1LPENR_I2C1LPEN                               0x00200000
#define RCC_APB1LPENR_I2C2LPEN                               0x00400000
#define RCC_APB1LPENR_I2C3LPEN                               0x00800000
#define RCC_APB1LPENR_PWRLPEN                                0x10000000

/********************  Bit definition for RCC_APB2LPENR register  *************/
#define RCC_APB2LPENR_TIM1LPEN                               0x00000001
#define RCC_APB2LPENR_USART1LPEN                             0x00000010
#define RCC_APB2LPENR_USART6LPEN                             0x00000020
#define RCC_APB2LPENR_ADC1LPEN                               0x00000100
#define RCC_APB2LPENR_SDIOLPEN                               0x00000800
#define RCC_APB2LPENR_SPI1LPEN                               0x00001000
#define RCC_APB2LPENR_SPI4LPEN                               0x00002000
#define RCC_APB2LPENR_SYSCFGLPEN                             0x00004000
#define RCC_APB2LPENR_TIM9LPEN                               0x00010000
#define RCC_APB2LPENR_TIM10LPEN                              0x00020000
#define RCC_APB2LPENR_TIM11LPEN                              0x00040000

/********************  Bit definition for RCC_BDCR register  ******************/
#define RCC_BDCR_LSEON                                       0x00000001
#define RCC_BDCR_LSERDY                                      0x00000002
#define RCC_BDCR_LSEBYP                                      0x00000004
#define RCC_BDCR_RTCSEL                                      0x00000300
#define RCC_BDCR_RTCSEL_0                                    0x00000100
#define RCC_BDCR_RTCSEL_1                                    0x00000200
#define RCC_BDCR_RTCEN                                       0x00008000
#define RCC_BDCR_BDRST                                       0x00010000

/********************  Bit definition for RCC_CSR register  *******************/
#define RCC_CSR_LSION                                        0x00000001
#define RCC_CSR_LSIRDY                                       0x00000002
#define RCC_CSR_RMVF                                         0x01000000
#define RCC_CSR_BORRSTF                                      0x02000000
#define RCC_CSR_PINRSTF                                      0x04000000
#define RCC_CSR_PORRSTF                                      0x08000000
#define RCC_CSR_SFTRSTF                                      0x10000000
#define RCC_CSR_IWDGRSTF                                     0x20000000
#define RCC_CSR_WWDGRSTF                                     0x40000000
#define RCC_CSR_LPWRRSTF                                     0x80000000
#define RCC_CSR_PADRSTF                                      RCC_CSR_PINRSTF
#define RCC_CSR_WDGRSTF                                      RCC_CSR_IWDGRSTF

/********************  Bit definition for RCC_SSCGR register  *****************/
#define RCC_SSCGR_MODPER                                     0x00001FFF
#define RCC_SSCGR_INCSTEP                                    0x0FFFE000
#define RCC_SSCGR_SPREADSEL                                  0x40000000
#define RCC_SSCGR_SSCGEN                                     0x80000000

/********************  Bit definition for RCC_PLLI2SCFGR register  ************/
#define RCC_PLLI2SCFGR_PLLI2SN                               0x00007FC0
#define RCC_PLLI2SCFGR_PLLI2SN_0                             0x00000040
#define RCC_PLLI2SCFGR_PLLI2SN_1                             0x00000080
#define RCC_PLLI2SCFGR_PLLI2SN_2                             0x00000100
#define RCC_PLLI2SCFGR_PLLI2SN_3                             0x00000200
#define RCC_PLLI2SCFGR_PLLI2SN_4                             0x00000400
#define RCC_PLLI2SCFGR_PLLI2SN_5                             0x00000800
#define RCC_PLLI2SCFGR_PLLI2SN_6                             0x00001000
#define RCC_PLLI2SCFGR_PLLI2SN_7                             0x00002000
#define RCC_PLLI2SCFGR_PLLI2SN_8                             0x00004000
#define RCC_PLLI2SCFGR_PLLI2SR                               0x70000000
#define RCC_PLLI2SCFGR_PLLI2SR_0                             0x10000000
#define RCC_PLLI2SCFGR_PLLI2SR_1                             0x20000000
#define RCC_PLLI2SCFGR_PLLI2SR_2                             0x40000000

/********************  Bit definition for RCC_DCKCFGR register  ***************/
#define RCC_DCKCFGR_TIMPRE                                   0x01000000




#endif /* STM32F401_RCC_H */
