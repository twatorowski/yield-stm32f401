/**
 * @file i2c.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_I2C_H
#define STM32F401_I2C_H

#include "stm32f401.h"

/* base address */
#define I2C1_BASE                                           (0x40005400L)
#define I2C2_BASE                                           (0x40005800L)
#define I2C3_BASE                                           (0x40005C00L)

/* register base */
#define I2C1                                                ((i2c_t *) I2C1_BASE)
#define I2C2                                                ((i2c_t *) I2C2_BASE)
#define I2C3                                                ((i2c_t *) I2C3_BASE)

/* register bank */
typedef struct {
    reg32_t CR1;
    reg32_t CR2;
    reg32_t OAR1;
    reg32_t OAR2;
    reg32_t DR;
    reg32_t SR1;
    reg32_t SR2;
    reg32_t CCR;
    reg32_t TRISE;
    reg32_t FLTR;
} i2c_t;


/*******************  Bit definition for I2C_CR1 register  ********************/
#define I2C_CR1_PE                                           0x00000001
#define I2C_CR1_SMBUS                                        0x00000002
#define I2C_CR1_SMBTYPE                                      0x00000008
#define I2C_CR1_ENARP                                        0x00000010
#define I2C_CR1_ENPEC                                        0x00000020
#define I2C_CR1_ENGC                                         0x00000040
#define I2C_CR1_NOSTRETCH                                    0x00000080
#define I2C_CR1_START                                        0x00000100
#define I2C_CR1_STOP                                         0x00000200
#define I2C_CR1_ACK                                          0x00000400
#define I2C_CR1_POS                                          0x00000800
#define I2C_CR1_PEC                                          0x00001000
#define I2C_CR1_ALERT                                        0x00002000
#define I2C_CR1_SWRST                                        0x00008000

/*******************  Bit definition for I2C_CR2 register  ********************/
#define I2C_CR2_FREQ                                         0x0000003f
#define I2C_CR2_FREQ_0                                       0x00000001
#define I2C_CR2_FREQ_1                                       0x00000002
#define I2C_CR2_FREQ_2                                       0x00000004
#define I2C_CR2_FREQ_3                                       0x00000008
#define I2C_CR2_FREQ_4                                       0x00000010
#define I2C_CR2_FREQ_5                                       0x00000020
#define I2C_CR2_ITERREN                                      0x00000100
#define I2C_CR2_ITEVTEN                                      0x00000200
#define I2C_CR2_ITBUFEN                                      0x00000400
#define I2C_CR2_DMAEN                                        0x00000800
#define I2C_CR2_LAST                                         0x00001000

/*******************  Bit definition for I2C_OAR1 register  *******************/
#define I2C_OAR1_ADD1_7                                      0x000000fe
#define I2C_OAR1_ADD8_9                                      0x00000300
#define I2C_OAR1_ADD0                                        0x00000001
#define I2C_OAR1_ADD1                                        0x00000002
#define I2C_OAR1_ADD2                                        0x00000004
#define I2C_OAR1_ADD3                                        0x00000008
#define I2C_OAR1_ADD4                                        0x00000010
#define I2C_OAR1_ADD5                                        0x00000020
#define I2C_OAR1_ADD6                                        0x00000040
#define I2C_OAR1_ADD7                                        0x00000080
#define I2C_OAR1_ADD8                                        0x00000100
#define I2C_OAR1_ADD9                                        0x00000200
#define I2C_OAR1_ADDMODE                                     0x00008000

/*******************  Bit definition for I2C_OAR2 register  *******************/
#define I2C_OAR2_ENDUAL                                      0x00000001
#define I2C_OAR2_ADD2                                        0x000000fe

/********************  Bit definition for I2C_DR register  ********************/
#define I2C_DR_DR                                            0x000000ff

/*******************  Bit definition for I2C_SR1 register  ********************/
#define I2C_SR1_SB                                           0x00000001
#define I2C_SR1_ADDR                                         0x00000002
#define I2C_SR1_BTF                                          0x00000004
#define I2C_SR1_ADD10                                        0x00000008
#define I2C_SR1_STOPF                                        0x00000010
#define I2C_SR1_RXNE                                         0x00000040
#define I2C_SR1_TXE                                          0x00000080
#define I2C_SR1_BERR                                         0x00000100
#define I2C_SR1_ARLO                                         0x00000200
#define I2C_SR1_AF                                           0x00000400
#define I2C_SR1_OVR                                          0x00000800
#define I2C_SR1_PECERR                                       0x00001000
#define I2C_SR1_TIMEOUT                                      0x00004000
#define I2C_SR1_SMBALERT                                     0x00008000

/*******************  Bit definition for I2C_SR2 register  ********************/
#define I2C_SR2_MSL                                          0x00000001
#define I2C_SR2_BUSY                                         0x00000002
#define I2C_SR2_TRA                                          0x00000004
#define I2C_SR2_GENCALL                                      0x00000010
#define I2C_SR2_SMBDEFAULT                                   0x00000020
#define I2C_SR2_SMBHOST                                      0x00000040
#define I2C_SR2_DUALF                                        0x00000080
#define I2C_SR2_PEC                                          0x0000ff00

/*******************  Bit definition for I2C_CCR register  ********************/
#define I2C_CCR_CCR                                          0x00000fff
#define I2C_CCR_DUTY                                         0x00004000
#define I2C_CCR_FS                                           0x00008000

/******************  Bit definition for I2C_TRISE register  *******************/
#define I2C_TRISE_TRISE                                      0x0000003f

/******************  Bit definition for I2C_FLTR register  *******************/
#define I2C_FLTR_DNF                                         0x0000000f
#define I2C_FLTR_ANOFF                                       0x00000010


#endif /* STM32F401_I2C_H */
