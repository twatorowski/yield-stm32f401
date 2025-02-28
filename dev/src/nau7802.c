/**
 * @file nau7802.c
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2023-05-31
 *
 * @brief Driver for the 24-bit strain gauge adc
 */

#include "err.h"
#include "dev/nau7802.h"
#include "sys/sleep.h"

#define DEBUG DLVL_INFO
#include "debug.h"

/* write data to the chip at given register address */
static err_t NAU7802_I2CWrite(nau7802_dev_t *dev, int reg_addr,
    const void *ptr, size_t size, dtime_t timeout)
{
    /* error code, register address placeholder */
    err_t ec; uint8_t reg_addr_u8 = reg_addr;

    /* try to lock the device in given time */
    if (Sem_Lock(&(dev->swi2c->sem), timeout) != EOK)
        return ETIMEOUT;

    /* start with the addressing phase */
    if ((ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_ADDR | SWI2C_OPER_WR,
        NAU7802_ADDR, &reg_addr_u8, 1)) != EOK)
        goto end;

    /* finish up with data write  */
    if ((ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_WR | SWI2C_OPER_STOP,
        NAU7802_ADDR, (void *)ptr, size)) != EOK)
        goto end;

    /* release the i2c */
    end: Sem_Release(&(dev->swi2c->sem)); return ec;
}

/* read data from the chip starting from the reg_address */
static err_t NAU7802_I2CRead(nau7802_dev_t *dev, int reg_addr, void *ptr,
    size_t size, dtime_t timeout)
{
    /* error code, register address placeholder */
    err_t ec; uint8_t reg_addr_u8 = reg_addr;

    /* try to lock the device in given time */
    if (Sem_Lock(&(dev->swi2c->sem), timeout) != EOK)
        return ETIMEOUT;

    /* start with the addressing phase */
    if ((ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_ADDR | SWI2C_OPER_WR,
        NAU7802_ADDR, &reg_addr_u8, 1)) != EOK)
        goto end;

    /* re-start with data read phase */
    if ((ec = SwI2C_Transfer(dev->swi2c, SWI2C_OPER_ADDR | SWI2C_OPER_RD |
        SWI2C_OPER_STOP, NAU7802_ADDR, ptr, size)) != EOK)
        goto end;

    /* release the i2c */
    end: Sem_Release(&(dev->swi2c->sem)); return ec;
}

/* write register */
static err_t NAU7802_RegisterWrite(nau7802_dev_t *dev, int reg_addr, uint8_t val)
{
    /* commence i2c transfer */
    return NAU7802_I2CWrite(dev, reg_addr, &val, 1, 0);
}

/* read register */
static err_t NAU7802_RegisterRead(nau7802_dev_t *dev, int reg_addr, uint8_t *val)
{
    /* commence i2c transfer */
    return NAU7802_I2CRead(dev, reg_addr, val, 1, 0);
}

/* modifyu register contents register */
static err_t NAU7802_RegisterModify(nau7802_dev_t *dev, int reg_addr,
    uint8_t mask, uint8_t val)
{
    /* register value placeholder and error code */
    uint8_t reg; err_t ec;

    /* read the register value */
    if ((ec = NAU7802_RegisterRead(dev, reg_addr, &reg)) != EOK)
        return ec;
    /* update the value */
    reg = (reg & ~mask) | val;
    /* write the value back */
    if ((ec =  NAU7802_RegisterWrite(dev, reg_addr, reg) != EOK))
        return ec;

    /* report status */
    return EOK;
}

/* initialize driver */
err_t NAU7802_Init(void)
{
    /* return status */
    return EOK;
}

/* initialize particular device */
err_t NAU7802_DevInit(nau7802_dev_t *dev)
{
    int rev_id; err_t ec = EOK;

    /* user specified the drdy pin? */
    if (dev->drdy.gpio != 0) {
        /* configure pull down on drdy pin  */
        GPIOSig_CfgPull(dev->drdy, GPIO_PULL_DN);
        /* set as input */
        GPIOSig_CfgInput(dev->drdy);
    }

    /* reset the device */
    if ((ec = NAU7802_Reset(dev)) != EOK)
        goto end;
    /* get the revision id */
    if ((ec = NAU7802_GetRevisionID(dev, &rev_id)) != EOK || rev_id != 0xf)
        goto end;

    /* enable sampling */
    if (!ec) ec = NAU7802_Enable(dev, 1);

    /* configure ldo and gain */
    if (!ec) ec = NAU7802_SetLDO(dev, NAU7802_LDO_EXTERNAL);
    if (!ec) ec = NAU7802_SetGain(dev, NAU7802_GAIN_128);
    if (!ec) ec = NAU7802_SetSamplingRate(dev, NAU7802_RATE_40SPS);

    /* disable ADC chopper clock */
    if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_ADC,
        NAU7802_ADC_REG_CHPS, NAU7802_ADC_REG_CHPS);
    /* use low ESR caps */
    if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_PGA,
        NAU7802_PGA_LDOMODE, 0);
    /* PGA stabilizer cap on output */
    if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_POWER,
        NAU7802_POWER_PGA_CAP_EN, NAU7802_POWER_PGA_CAP_EN);

    /* do the calibration */
    if (!ec) ec = NAU7802_Calibrate(dev, NAU7802_CALMOD_INTERNAL);
    if (!ec) ec = NAU7802_Calibrate(dev, NAU7802_CALMOD_OFFSET);

    /* report initialization status */
    end: return ec;
}

/* perform device reset procedure */
err_t NAU7802_Reset(nau7802_dev_t *dev)
{
    /* error code and readback placeholder */
    err_t ec = EOK; uint8_t reg;

    /* start the reset of the chip */
    if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_PU_CTRL,
        NAU7802_PU_CTRL_RR, NAU7802_PU_CTRL_RR);
    /* wait for the chip to settle */
    if (!ec) Sleep(10);
    /* clear the reset bit */
    if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_PU_CTRL,
        NAU7802_PU_CTRL_RR, 0);
    /* power up the digital section of the chip */
    if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_PU_CTRL,
        NAU7802_PU_CTRL_PUD, NAU7802_PU_CTRL_PUD);
    /* wait for the power to stabilize */
    if (!ec) Sleep(10);
    /* read the status of the power up */
    if (!ec) ec = NAU7802_RegisterRead(dev, NAU7802_REG_PU_CTRL, &reg);

    /* return status based on communication errors and power up status */
    return ec ? ec : (reg & NAU7802_PU_CTRL_PUR ? EOK : EFATAL);
}

/* reads the revision id */
err_t NAU7802_GetRevisionID(nau7802_dev_t *dev, int *rev_id)
{
    /* placeholder */
    uint8_t reg;
    /* try to obtain the revision */
    err_t ec = NAU7802_RegisterRead(dev, NAU7802_REG_REVISION_ID, &reg);
    /* unable to communicate with chip? */
    if (ec)
        return ec;

    /* get significant id */
    *rev_id = reg & NAU7802_REVISION_ID_ID;
    /* return status */
    return EOK;
}

/* enable/disable the conversions */
err_t NAU7802_Enable(nau7802_dev_t *dev, int enable)
{
    /* error code */
    err_t ec = EOK;

    /* disable procedure */
    if (!enable) {
        /* disable the analog part */
        if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_PU_CTRL,
            NAU7802_PU_CTRL_PUA, 0);
        /* and then disable the digital part */
        if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_PU_CTRL,
            NAU7802_PU_CTRL_PUD, 0);
    /* enable procedure */
    } else {
        /* enable the digital part */
        if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_PU_CTRL,
            NAU7802_PU_CTRL_PUD, NAU7802_PU_CTRL_PUD);
        /* enable the analog part */
        if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_PU_CTRL,
            NAU7802_PU_CTRL_PUA, NAU7802_PU_CTRL_PUA);

        /* wait for the analog supply to stabilize */
        Sleep(600);

        /* start the conversions */
        if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_PU_CTRL,
            NAU7802_PU_CTRL_CS, NAU7802_PU_CTRL_CS);
    }

    /* report operation status */
    return ec;
}

/* set ldo voltage used to power the bridge */
err_t NAU7802_SetLDO(nau7802_dev_t *dev, nau7802_ldo_t mode)
{
    /* register bits to be set */
    uint8_t vldo, avdds; err_t ec = EOK;

    /* translate to register bits */
    switch (mode) {
    case NAU7802_LDO_4V5: vldo = 0; avdds = 1; break;
    case NAU7802_LDO_4V2: vldo = 1; avdds = 1; break;
    case NAU7802_LDO_3V9: vldo = 2; avdds = 1; break;
    case NAU7802_LDO_3V6: vldo = 3; avdds = 1; break;
    case NAU7802_LDO_3V3: vldo = 4; avdds = 1; break;
    case NAU7802_LDO_3V0: vldo = 5; avdds = 1; break;
    case NAU7802_LDO_2V7: vldo = 6; avdds = 1; break;
    case NAU7802_LDO_2V4: vldo = 7; avdds = 1; break;
    case NAU7802_LDO_EXTERNAL: vldo = 0; avdds = 0; break;
    }

    /* configure voltage source (internal/external) */
    if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_PU_CTRL,
        NAU7802_PU_CTRL_AVDDS, avdds << LSB(NAU7802_PU_CTRL_AVDDS));
    /* configure voltage value (internal ldo) */
    if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_CTRL1,
        NAU7802_CTRL1_VLDO, vldo << LSB(NAU7802_CTRL1_VLDO));

    /* report status */
    return ec;
}

/* set the PGA gain */
err_t NAU7802_SetGain(nau7802_dev_t *dev, nau7802_gain_t gain)
{
    /* register bits to be set */
    uint8_t gain_bits;

    /* translate to bit settings */
    switch (gain) {
    case NAU7802_GAIN_1: gain_bits = 0; break;
    case NAU7802_GAIN_2: gain_bits = 1; break;
    case NAU7802_GAIN_4: gain_bits = 2; break;
    case NAU7802_GAIN_8: gain_bits = 3; break;
    case NAU7802_GAIN_16: gain_bits = 4; break;
    case NAU7802_GAIN_32: gain_bits = 5; break;
    case NAU7802_GAIN_64: gain_bits = 6; break;
    case NAU7802_GAIN_128: gain_bits = 7; break;
    }

    /* modify the gain setting */
    return NAU7802_RegisterModify(dev, NAU7802_REG_CTRL1, NAU7802_CTRL1_GAINS,
        gain_bits << LSB(NAU7802_CTRL1_GAINS));
}

/* set the sampling rate */
err_t NAU7802_SetSamplingRate(nau7802_dev_t *dev, nau7802_sample_rate_t rate)
{
    /* register bits to be set */
    uint8_t rate_bits;

    /* translate to bit settings */
    switch (rate) {
    case NAU7802_RATE_10SPS: rate_bits = 0; break;
    case NAU7802_RATE_20SPS: rate_bits = 1; break;
    case NAU7802_RATE_40SPS: rate_bits = 2; break;
    case NAU7802_RATE_80SPS: rate_bits = 3; break;
    case NAU7802_RATE_320SPS: rate_bits = 7; break;
    }

    /* modify the sampling rate setting */
    return NAU7802_RegisterModify(dev, NAU7802_REG_CTRL2, NAU7802_CTRL2_CRS,
        rate_bits << LSB(NAU7802_CTRL2_CRS));
}

/* perform calibration */
err_t NAU7802_Calibrate(nau7802_dev_t *dev, nau7802_calib_mode_t mode)
{
    /* register bits */
    uint8_t mode_bits, ctrl2; err_t ec = EOK;

    /* translate to the register bits */
    switch (mode) {
    case NAU7802_CALMOD_INTERNAL: mode_bits = 0; break;
    case NAU7802_CALMOD_OFFSET: mode_bits = 2; break;
    case NAU7802_CALMOD_GAIN: mode_bits = 3; break;
    }

    /* set the calibration mode */
    if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_CTRL2,
        NAU7802_CTRL2_CALMOD, mode_bits << LSB(NAU7802_CTRL2_CALMOD));
    /* start the calibration */
    if (!ec) ec = NAU7802_RegisterModify(dev, NAU7802_REG_CTRL2,
        NAU7802_CTRL2_CALS, NAU7802_CTRL2_CALS);

    /* poll until the calibration is done */
    for (; ec == EOK; Sleep(10)) {
        /* read the register */
        ec = NAU7802_RegisterRead(dev, NAU7802_REG_CTRL2, &ctrl2);
        /* communication error or calibration done? */
        if (ec != EOK || (ctrl2 & NAU7802_CTRL2_CALS))
            break;
    }

    /* return status based on communication error or calibration error flag */
    return ec ? ec : (ctrl2 & NAU7802_CTRL2_CAL_ERR ? EFATAL : EOK);
}

/* reads adc conversion result */
err_t NAU7802_Read(nau7802_dev_t *dev, int32_t *value)
{
    /* data placeholder */
    uint8_t buf[3];

    /* obtain adc conversion result */
    err_t ec = NAU7802_I2CRead(dev, NAU7802_REG_ADCO_B2, buf, 3, 0);
    /* unable to obtain the readout */
    if (ec)
        return ec;

    /* reconstruct the value */
    *value = buf[0] << 16 | buf[1] << 8 | buf[2] << 0;
    /* sign extend */
    *value = (*value << 8) >> 8;

    /* report status */
    return EOK;
}

/* check if the data is ready for reading */
err_t NAU7802_DataReady(nau7802_dev_t *dev, int *ready)
{
    err_t ec; uint8_t reg;

    /* device uses drdy pin? */
    if (dev->drdy.gpio != 0) {
        *ready = GPIOSig_Get(dev->drdy); return EOK;
    }

    /* read the data ready bit holding register */
    if ((ec = NAU7802_RegisterRead(dev, NAU7802_REG_PU_CTRL, &reg)) != EOK)
        return ec;
    /* extract the conversion ready bit */
    *ready = !!(reg & NAU7802_PU_CTRL_CR);

    /* report status */
    return EOK;
}