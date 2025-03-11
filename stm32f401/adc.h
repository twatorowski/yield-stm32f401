/**
 * @file adc.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef STM32F401_ADC_H
#define STM32F401_ADC_H

#include "stm32f401.h"


/* instance base addresses */
#define ADC1_BASE                                            0x40012000
#define ADC_COMMON_BASE                                      0x40012300

/* peripheral instances */
#define ADC1                                                ((adc_t *) ADC1_BASE)
#define ADC_COMMON                                          ((adc_com_t *) ADC_COMMON_BASE)

/* register bank */
typedef struct {
    reg32_t SR;
    reg32_t CR1;
    reg32_t CR2;
    reg32_t SMPR1;
    reg32_t SMPR2;
    reg32_t JOFR1;
    reg32_t JOFR2;
    reg32_t JOFR3;
    reg32_t JOFR4;
    reg32_t HTR;
    reg32_t LTR;
    reg32_t SQR1;
    reg32_t SQR2;
    reg32_t SQR3;
    reg32_t JSQR;
    reg32_t JDR1;
    reg32_t JDR2;
    reg32_t JDR3;
    reg32_t JDR4;
    reg32_t DR;
} adc_t;


/********************  Bit definition for ADC_SR register  ********************/
#define ADC_SR_AWD                                           0x00000001
#define ADC_SR_EOC                                           0x00000002
#define ADC_SR_JEOC                                          0x00000004
#define ADC_SR_JSTRT                                         0x00000008
#define ADC_SR_STRT                                          0x00000010
#define ADC_SR_OVR                                           0x00000020

/*******************  Bit definition for ADC_CR1 register  ********************/
#define ADC_CR1_AWDCH                                        0x0000001f
#define ADC_CR1_AWDCH_0                                      0x00000001
#define ADC_CR1_AWDCH_1                                      0x00000002
#define ADC_CR1_AWDCH_2                                      0x00000004
#define ADC_CR1_AWDCH_3                                      0x00000008
#define ADC_CR1_AWDCH_4                                      0x00000010
#define ADC_CR1_EOCIE                                        0x00000020
#define ADC_CR1_AWDIE                                        0x00000040
#define ADC_CR1_JEOCIE                                       0x00000080
#define ADC_CR1_SCAN                                         0x00000100
#define ADC_CR1_AWDSGL                                       0x00000200
#define ADC_CR1_JAUTO                                        0x00000400
#define ADC_CR1_DISCEN                                       0x00000800
#define ADC_CR1_JDISCEN                                      0x00001000
#define ADC_CR1_DISCNUM                                      0x0000e000
#define ADC_CR1_DISCNUM_0                                    0x00002000
#define ADC_CR1_DISCNUM_1                                    0x00004000
#define ADC_CR1_DISCNUM_2                                    0x00008000
#define ADC_CR1_JAWDEN                                       0x00400000
#define ADC_CR1_AWDEN                                        0x00800000
#define ADC_CR1_RES                                          0x03000000
#define ADC_CR1_RES_0                                        0x01000000
#define ADC_CR1_RES_1                                        0x02000000
#define ADC_CR1_OVRIE                                        0x04000000

/*******************  Bit definition for ADC_CR2 register  ********************/
#define ADC_CR2_ADON                                         0x00000001
#define ADC_CR2_CONT                                         0x00000002
#define ADC_CR2_DMA                                          0x00000100
#define ADC_CR2_DDS                                          0x00000200
#define ADC_CR2_EOCS                                         0x00000400
#define ADC_CR2_ALIGN                                        0x00000800
#define ADC_CR2_JEXTSEL                                      0x000f0000
#define ADC_CR2_JEXTSEL_0                                    0x00010000
#define ADC_CR2_JEXTSEL_1                                    0x00020000
#define ADC_CR2_JEXTSEL_2                                    0x00040000
#define ADC_CR2_JEXTSEL_3                                    0x00080000
#define ADC_CR2_JEXTEN                                       0x00300000
#define ADC_CR2_JEXTEN_0                                     0x00100000
#define ADC_CR2_JEXTEN_1                                     0x00200000
#define ADC_CR2_JSWSTART                                     0x00400000
#define ADC_CR2_EXTSEL                                       0x0f000000
#define ADC_CR2_EXTSEL_0                                     0x01000000
#define ADC_CR2_EXTSEL_1                                     0x02000000
#define ADC_CR2_EXTSEL_2                                     0x04000000
#define ADC_CR2_EXTSEL_3                                     0x08000000
#define ADC_CR2_EXTEN                                        0x30000000
#define ADC_CR2_EXTEN_0                                      0x10000000
#define ADC_CR2_EXTEN_1                                      0x20000000
#define ADC_CR2_SWSTART                                      0x40000000

/******************  Bit definition for ADC_SMPR1 register  *******************/
#define ADC_SMPR1_SMP10                                      0x00000007
#define ADC_SMPR1_SMP10_0                                    0x00000001
#define ADC_SMPR1_SMP10_1                                    0x00000002
#define ADC_SMPR1_SMP10_2                                    0x00000004
#define ADC_SMPR1_SMP11                                      0x00000038
#define ADC_SMPR1_SMP11_0                                    0x00000008
#define ADC_SMPR1_SMP11_1                                    0x00000010
#define ADC_SMPR1_SMP11_2                                    0x00000020
#define ADC_SMPR1_SMP12                                      0x000001c0
#define ADC_SMPR1_SMP12_0                                    0x00000040
#define ADC_SMPR1_SMP12_1                                    0x00000080
#define ADC_SMPR1_SMP12_2                                    0x00000100
#define ADC_SMPR1_SMP13                                      0x00000e00
#define ADC_SMPR1_SMP13_0                                    0x00000200
#define ADC_SMPR1_SMP13_1                                    0x00000400
#define ADC_SMPR1_SMP13_2                                    0x00000800
#define ADC_SMPR1_SMP14                                      0x00007000
#define ADC_SMPR1_SMP14_0                                    0x00001000
#define ADC_SMPR1_SMP14_1                                    0x00002000
#define ADC_SMPR1_SMP14_2                                    0x00004000
#define ADC_SMPR1_SMP15                                      0x00038000
#define ADC_SMPR1_SMP15_0                                    0x00008000
#define ADC_SMPR1_SMP15_1                                    0x00010000
#define ADC_SMPR1_SMP15_2                                    0x00020000
#define ADC_SMPR1_SMP16                                      0x001c0000
#define ADC_SMPR1_SMP16_0                                    0x00040000
#define ADC_SMPR1_SMP16_1                                    0x00080000
#define ADC_SMPR1_SMP16_2                                    0x00100000
#define ADC_SMPR1_SMP17                                      0x00e00000
#define ADC_SMPR1_SMP17_0                                    0x00200000
#define ADC_SMPR1_SMP17_1                                    0x00400000
#define ADC_SMPR1_SMP17_2                                    0x00800000
#define ADC_SMPR1_SMP18                                      0x07000000
#define ADC_SMPR1_SMP18_0                                    0x01000000
#define ADC_SMPR1_SMP18_1                                    0x02000000
#define ADC_SMPR1_SMP18_2                                    0x04000000

/******************  Bit definition for ADC_SMPR2 register  *******************/
#define ADC_SMPR2_SMP0                                       0x00000007
#define ADC_SMPR2_SMP0_0                                     0x00000001
#define ADC_SMPR2_SMP0_1                                     0x00000002
#define ADC_SMPR2_SMP0_2                                     0x00000004
#define ADC_SMPR2_SMP1                                       0x00000038
#define ADC_SMPR2_SMP1_0                                     0x00000008
#define ADC_SMPR2_SMP1_1                                     0x00000010
#define ADC_SMPR2_SMP1_2                                     0x00000020
#define ADC_SMPR2_SMP2                                       0x000001c0
#define ADC_SMPR2_SMP2_0                                     0x00000040
#define ADC_SMPR2_SMP2_1                                     0x00000080
#define ADC_SMPR2_SMP2_2                                     0x00000100
#define ADC_SMPR2_SMP3                                       0x00000e00
#define ADC_SMPR2_SMP3_0                                     0x00000200
#define ADC_SMPR2_SMP3_1                                     0x00000400
#define ADC_SMPR2_SMP3_2                                     0x00000800
#define ADC_SMPR2_SMP4                                       0x00007000
#define ADC_SMPR2_SMP4_0                                     0x00001000
#define ADC_SMPR2_SMP4_1                                     0x00002000
#define ADC_SMPR2_SMP4_2                                     0x00004000
#define ADC_SMPR2_SMP5                                       0x00038000
#define ADC_SMPR2_SMP5_0                                     0x00008000
#define ADC_SMPR2_SMP5_1                                     0x00010000
#define ADC_SMPR2_SMP5_2                                     0x00020000
#define ADC_SMPR2_SMP6                                       0x001c0000
#define ADC_SMPR2_SMP6_0                                     0x00040000
#define ADC_SMPR2_SMP6_1                                     0x00080000
#define ADC_SMPR2_SMP6_2                                     0x00100000
#define ADC_SMPR2_SMP7                                       0x00e00000
#define ADC_SMPR2_SMP7_0                                     0x00200000
#define ADC_SMPR2_SMP7_1                                     0x00400000
#define ADC_SMPR2_SMP7_2                                     0x00800000
#define ADC_SMPR2_SMP8                                       0x07000000
#define ADC_SMPR2_SMP8_0                                     0x01000000
#define ADC_SMPR2_SMP8_1                                     0x02000000
#define ADC_SMPR2_SMP8_2                                     0x04000000
#define ADC_SMPR2_SMP9                                       0x38000000
#define ADC_SMPR2_SMP9_0                                     0x08000000
#define ADC_SMPR2_SMP9_1                                     0x10000000
#define ADC_SMPR2_SMP9_2                                     0x20000000

/******************  Bit definition for ADC_JOFR1 register  *******************/
#define ADC_JOFR1_JOFFSET1                                   0x00000fff

/******************  Bit definition for ADC_JOFR2 register  *******************/
#define ADC_JOFR2_JOFFSET2                                   0x00000fff

/******************  Bit definition for ADC_JOFR3 register  *******************/
#define ADC_JOFR3_JOFFSET3                                   0x00000fff

/******************  Bit definition for ADC_JOFR4 register  *******************/
#define ADC_JOFR4_JOFFSET4                                   0x00000fff

/*******************  Bit definition for ADC_HTR register  ********************/
#define ADC_HTR_HT                                           0x00000fff

/*******************  Bit definition for ADC_LTR register  ********************/
#define ADC_LTR_LT                                           0x00000fff

/*******************  Bit definition for ADC_SQR1 register  *******************/
#define ADC_SQR1_SQ13                                        0x0000001f
#define ADC_SQR1_SQ13_0                                      0x00000001
#define ADC_SQR1_SQ13_1                                      0x00000002
#define ADC_SQR1_SQ13_2                                      0x00000004
#define ADC_SQR1_SQ13_3                                      0x00000008
#define ADC_SQR1_SQ13_4                                      0x00000010
#define ADC_SQR1_SQ14                                        0x000003e0
#define ADC_SQR1_SQ14_0                                      0x00000020
#define ADC_SQR1_SQ14_1                                      0x00000040
#define ADC_SQR1_SQ14_2                                      0x00000080
#define ADC_SQR1_SQ14_3                                      0x00000100
#define ADC_SQR1_SQ14_4                                      0x00000200
#define ADC_SQR1_SQ15                                        0x00007c00
#define ADC_SQR1_SQ15_0                                      0x00000400
#define ADC_SQR1_SQ15_1                                      0x00000800
#define ADC_SQR1_SQ15_2                                      0x00001000
#define ADC_SQR1_SQ15_3                                      0x00002000
#define ADC_SQR1_SQ15_4                                      0x00004000
#define ADC_SQR1_SQ16                                        0x000f8000
#define ADC_SQR1_SQ16_0                                      0x00008000
#define ADC_SQR1_SQ16_1                                      0x00010000
#define ADC_SQR1_SQ16_2                                      0x00020000
#define ADC_SQR1_SQ16_3                                      0x00040000
#define ADC_SQR1_SQ16_4                                      0x00080000
#define ADC_SQR1_L                                           0x00f00000
#define ADC_SQR1_L_0                                         0x00100000
#define ADC_SQR1_L_1                                         0x00200000
#define ADC_SQR1_L_2                                         0x00400000
#define ADC_SQR1_L_3                                         0x00800000

/*******************  Bit definition for ADC_SQR2 register  *******************/
#define ADC_SQR2_SQ7                                         0x0000001f
#define ADC_SQR2_SQ7_0                                       0x00000001
#define ADC_SQR2_SQ7_1                                       0x00000002
#define ADC_SQR2_SQ7_2                                       0x00000004
#define ADC_SQR2_SQ7_3                                       0x00000008
#define ADC_SQR2_SQ7_4                                       0x00000010
#define ADC_SQR2_SQ8                                         0x000003e0
#define ADC_SQR2_SQ8_0                                       0x00000020
#define ADC_SQR2_SQ8_1                                       0x00000040
#define ADC_SQR2_SQ8_2                                       0x00000080
#define ADC_SQR2_SQ8_3                                       0x00000100
#define ADC_SQR2_SQ8_4                                       0x00000200
#define ADC_SQR2_SQ9                                         0x00007c00
#define ADC_SQR2_SQ9_0                                       0x00000400
#define ADC_SQR2_SQ9_1                                       0x00000800
#define ADC_SQR2_SQ9_2                                       0x00001000
#define ADC_SQR2_SQ9_3                                       0x00002000
#define ADC_SQR2_SQ9_4                                       0x00004000
#define ADC_SQR2_SQ10                                        0x000f8000
#define ADC_SQR2_SQ10_0                                      0x00008000
#define ADC_SQR2_SQ10_1                                      0x00010000
#define ADC_SQR2_SQ10_2                                      0x00020000
#define ADC_SQR2_SQ10_3                                      0x00040000
#define ADC_SQR2_SQ10_4                                      0x00080000
#define ADC_SQR2_SQ11                                        0x01f00000
#define ADC_SQR2_SQ11_0                                      0x00100000
#define ADC_SQR2_SQ11_1                                      0x00200000
#define ADC_SQR2_SQ11_2                                      0x00400000
#define ADC_SQR2_SQ11_3                                      0x00800000
#define ADC_SQR2_SQ11_4                                      0x01000000
#define ADC_SQR2_SQ12                                        0x3e000000
#define ADC_SQR2_SQ12_0                                      0x02000000
#define ADC_SQR2_SQ12_1                                      0x04000000
#define ADC_SQR2_SQ12_2                                      0x08000000
#define ADC_SQR2_SQ12_3                                      0x10000000
#define ADC_SQR2_SQ12_4                                      0x20000000

/*******************  Bit definition for ADC_SQR3 register  *******************/
#define ADC_SQR3_SQ1                                         0x0000001f
#define ADC_SQR3_SQ1_0                                       0x00000001
#define ADC_SQR3_SQ1_1                                       0x00000002
#define ADC_SQR3_SQ1_2                                       0x00000004
#define ADC_SQR3_SQ1_3                                       0x00000008
#define ADC_SQR3_SQ1_4                                       0x00000010
#define ADC_SQR3_SQ2                                         0x000003e0
#define ADC_SQR3_SQ2_0                                       0x00000020
#define ADC_SQR3_SQ2_1                                       0x00000040
#define ADC_SQR3_SQ2_2                                       0x00000080
#define ADC_SQR3_SQ2_3                                       0x00000100
#define ADC_SQR3_SQ2_4                                       0x00000200
#define ADC_SQR3_SQ3                                         0x00007c00
#define ADC_SQR3_SQ3_0                                       0x00000400
#define ADC_SQR3_SQ3_1                                       0x00000800
#define ADC_SQR3_SQ3_2                                       0x00001000
#define ADC_SQR3_SQ3_3                                       0x00002000
#define ADC_SQR3_SQ3_4                                       0x00004000
#define ADC_SQR3_SQ4                                         0x000f8000
#define ADC_SQR3_SQ4_0                                       0x00008000
#define ADC_SQR3_SQ4_1                                       0x00010000
#define ADC_SQR3_SQ4_2                                       0x00020000
#define ADC_SQR3_SQ4_3                                       0x00040000
#define ADC_SQR3_SQ4_4                                       0x00080000
#define ADC_SQR3_SQ5                                         0x01f00000
#define ADC_SQR3_SQ5_0                                       0x00100000
#define ADC_SQR3_SQ5_1                                       0x00200000
#define ADC_SQR3_SQ5_2                                       0x00400000
#define ADC_SQR3_SQ5_3                                       0x00800000
#define ADC_SQR3_SQ5_4                                       0x01000000
#define ADC_SQR3_SQ6                                         0x3e000000
#define ADC_SQR3_SQ6_0                                       0x02000000
#define ADC_SQR3_SQ6_1                                       0x04000000
#define ADC_SQR3_SQ6_2                                       0x08000000
#define ADC_SQR3_SQ6_3                                       0x10000000
#define ADC_SQR3_SQ6_4                                       0x20000000

/*******************  Bit definition for ADC_JSQR register  *******************/
#define ADC_JSQR_JSQ1                                        0x0000001f
#define ADC_JSQR_JSQ1_0                                      0x00000001
#define ADC_JSQR_JSQ1_1                                      0x00000002
#define ADC_JSQR_JSQ1_2                                      0x00000004
#define ADC_JSQR_JSQ1_3                                      0x00000008
#define ADC_JSQR_JSQ1_4                                      0x00000010
#define ADC_JSQR_JSQ2                                        0x000003e0
#define ADC_JSQR_JSQ2_0                                      0x00000020
#define ADC_JSQR_JSQ2_1                                      0x00000040
#define ADC_JSQR_JSQ2_2                                      0x00000080
#define ADC_JSQR_JSQ2_3                                      0x00000100
#define ADC_JSQR_JSQ2_4                                      0x00000200
#define ADC_JSQR_JSQ3                                        0x00007c00
#define ADC_JSQR_JSQ3_0                                      0x00000400
#define ADC_JSQR_JSQ3_1                                      0x00000800
#define ADC_JSQR_JSQ3_2                                      0x00001000
#define ADC_JSQR_JSQ3_3                                      0x00002000
#define ADC_JSQR_JSQ3_4                                      0x00004000
#define ADC_JSQR_JSQ4                                        0x000f8000
#define ADC_JSQR_JSQ4_0                                      0x00008000
#define ADC_JSQR_JSQ4_1                                      0x00010000
#define ADC_JSQR_JSQ4_2                                      0x00020000
#define ADC_JSQR_JSQ4_3                                      0x00040000
#define ADC_JSQR_JSQ4_4                                      0x00080000
#define ADC_JSQR_JL                                          0x00300000
#define ADC_JSQR_JL_0                                        0x00100000
#define ADC_JSQR_JL_1                                        0x00200000

/*******************  Bit definition for ADC_JDR1 register  *******************/
#define ADC_JDR1_JDATA                                       0x0000ffff

/*******************  Bit definition for ADC_JDR2 register  *******************/
#define ADC_JDR2_JDATA                                       0x0000ffff

/*******************  Bit definition for ADC_JDR3 register  *******************/
#define ADC_JDR3_JDATA                                       0x0000ffff

/*******************  Bit definition for ADC_JDR4 register  *******************/
#define ADC_JDR4_JDATA                                       0x0000ffff

/********************  Bit definition for ADC_DR register  ********************/
#define ADC_DR_DATA                                          0x0000ffff
#define ADC_DR_ADC2DATA                                      0xffff0000


/* register bank */
typedef struct {
    reg32_t CSR;
    reg32_t CCR;
    reg32_t CDR;
} adc_com_t;


/*******************  Bit definition for ADC_CSR register  ********************/
#define ADC_CSR_AWD1                                         0x00000001
#define ADC_CSR_EOC1                                         0x00000002
#define ADC_CSR_JEOC1                                        0x00000004
#define ADC_CSR_JSTRT1                                       0x00000008
#define ADC_CSR_STRT1                                        0x00000010
#define ADC_CSR_OVR1                                         0x00000020

/* Legacy defines */
#define  ADC_CSR_DOVR1                        ADC_CSR_OVR1

/*******************  Bit definition for ADC_CCR register  ********************/
#define ADC_CCR_MULTI                                        0x0000001f
#define ADC_CCR_MULTI_0                                      0x00000001
#define ADC_CCR_MULTI_1                                      0x00000002
#define ADC_CCR_MULTI_2                                      0x00000004
#define ADC_CCR_MULTI_3                                      0x00000008
#define ADC_CCR_MULTI_4                                      0x00000010
#define ADC_CCR_DELAY                                        0x00000f00
#define ADC_CCR_DELAY_0                                      0x00000100
#define ADC_CCR_DELAY_1                                      0x00000200
#define ADC_CCR_DELAY_2                                      0x00000400
#define ADC_CCR_DELAY_3                                      0x00000800
#define ADC_CCR_DDS                                          0x00002000
#define ADC_CCR_DMA                                          0x0000c000
#define ADC_CCR_DMA_0                                        0x00004000
#define ADC_CCR_DMA_1                                        0x00008000
#define ADC_CCR_ADCPRE                                       0x00030000
#define ADC_CCR_ADCPRE_0                                     0x00010000
#define ADC_CCR_ADCPRE_1                                     0x00020000
#define ADC_CCR_VBATE                                        0x00400000
#define ADC_CCR_TSVREFE                                      0x00800000

/*******************  Bit definition for ADC_CDR register  ********************/
#define ADC_CDR_DATA1                                        0x0000ffff
#define ADC_CDR_DATA2                                        0xffff0000

/* Legacy defines */
#define ADC_CDR_RDATA_MST         ADC_CDR_DATA1
#define ADC_CDR_RDATA_SLV         ADC_CDR_DATA2



#endif /* STM32F401_ADC_H */
