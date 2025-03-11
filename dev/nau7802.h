/**
 * @file nau7802.h
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2023-05-31
 *
 * @brief Driver for the 24-bit strain gauge adc
 */

#ifndef DEV_NAU7802_H
#define DEV_NAU7802_H

#include "err.h"
#include "dev/gpio_signals.h"
#include "dev/swi2c_dev.h"

/** device descriptor */
typedef struct nau7802_dev {
    /** guarding semaphore */
    sem_t sem;
    /** swi2c device */
    swi2c_dev_t *swi2c;
    /* drdy pin */
    gpio_signal_t drdy;
} nau7802_dev_t;

/** maximal value reported by the adc  */
#define NAU7802_MAX_VAL                                 ((int32_t)0x7fffff)

/** device address */
#define NAU7802_ADDR                                    0x2A

/** registers */
#define NAU7802_REG_PU_CTRL                             0x00
#define NAU7802_REG_CTRL1                               0x01
#define NAU7802_REG_CTRL2                               0x02
#define NAU7802_REG_ADCO_B2                             0x12
#define NAU7802_REG_ADC                                 0x15
#define NAU7802_REG_PGA                                 0x1B
#define NAU7802_REG_POWER                               0x1C
#define NAU7802_REG_REVISION_ID                         0x1F


/** power up control */
#define NAU7802_PU_CTRL_RR                              0x01
#define NAU7802_PU_CTRL_PUD                             0x02
#define NAU7802_PU_CTRL_PUA                             0x04
#define NAU7802_PU_CTRL_PUR                             0x08
#define NAU7802_PU_CTRL_CS                              0x10
#define NAU7802_PU_CTRL_CR                              0x20
#define NAU7802_PU_CTRL_OSCS                            0x40
#define NAU7802_PU_CTRL_AVDDS                           0x80

/** control 1 register */
#define NAU7802_CTRL1_GAINS                             0x07
#define NAU7802_CTRL1_GAINS_0                           0x01
#define NAU7802_CTRL1_GAINS_1                           0x02
#define NAU7802_CTRL1_GAINS_2                           0x04
#define NAU7802_CTRL1_VLDO                              0x38
#define NAU7802_CTRL1_VLDO_0                            0x08
#define NAU7802_CTRL1_VLDO_1                            0x10
#define NAU7802_CTRL1_VLDO_2                            0x20
#define NAU7802_CTRL1_DRDY_SEL                          0x40
#define NAU7802_CTRL1_CRP                               0x80


/** control 2 register */
#define NAU7802_CTRL2_CALMOD                            0x03
#define NAU7802_CTRL2_CALMOD_0                          0x01
#define NAU7802_CTRL2_CALMOD_1                          0x02
#define NAU7802_CTRL2_CALS                              0x03
#define NAU7802_CTRL2_CAL_ERR                           0x04
#define NAU7802_CTRL2_CRS                               0x70
#define NAU7802_CTRL2_CRS_0                             0x10
#define NAU7802_CTRL2_CRS_1                             0x20
#define NAU7802_CTRL2_CRS_2                             0x40
#define NAU7802_CTRL2_CHS                               0x80


/** adc registers bits */
#define NAU7802_ADC_REG_CHP                             0x03
#define NAU7802_ADC_REG_CHP_0                           0x01
#define NAU7802_ADC_REG_CHP_1                           0x02
#define NAU7802_ADC_ADC_VCM                             0x0C
#define NAU7802_ADC_ADC_VCM_0                           0x04
#define NAU7802_ADC_ADC_VCM_1                           0x08
#define NAU7802_ADC_REG_CHPS                            0x30
#define NAU7802_ADC_REG_CHPS_0                          0x10
#define NAU7802_ADC_REG_CHPS_1                          0x20


/** pga registers bits */
#define NAU7802_PGA_PGACHPDIS                           0x01
#define NAU7802_PGA_PGAINV                              0x08
#define NAU7802_PGA_BYPEN                               0x10
#define NAU7802_PGA_BUFEN                               0x20
#define NAU7802_PGA_LDOMODE                             0x40
#define NAU7802_PGA_RD_OTP_SEL                          0x80


/** power control register bits */
#define NAU7802_POWER_PGA_CURR                          0x03
#define NAU7802_POWER_PGA_CURR_0                        0x01
#define NAU7802_POWER_PGA_CURR_1                        0x02
#define NAU7802_POWER_ADC_CURR                          0x0C
#define NAU7802_POWER_ADC_CURR_0                        0x04
#define NAU7802_POWER_ADC_CURR_1                        0x08
#define NAU7802_POWER_MASTER_BIAS_CURR                  0x70
#define NAU7802_POWER_MASTER_BIAS_CURR_0                0x10
#define NAU7802_POWER_MASTER_BIAS_CURR_1                0x20
#define NAU7802_POWER_MASTER_BIAS_CURR_2                0x40
#define NAU7802_POWER_PGA_CAP_EN                        0x80

/** revision id */
#define NAU7802_REVISION_ID_ID                          0x0F
#define NAU7802_REVISION_ID_ID_0                        0x00
#define NAU7802_REVISION_ID_ID_1                        0x00
#define NAU7802_REVISION_ID_ID_2                        0x00
#define NAU7802_REVISION_ID_ID_3                        0x00


/** applicable ldo voltages */
typedef enum nau7802_ldo {
    NAU7802_LDO_4V5,
    NAU7802_LDO_4V2,
    NAU7802_LDO_3V9,
    NAU7802_LDO_3V6,
    NAU7802_LDO_3V3,
    NAU7802_LDO_3V0,
    NAU7802_LDO_2V7,
    NAU7802_LDO_2V4,
    NAU7802_LDO_EXTERNAL,
} nau7802_ldo_t;

/** The possible gains */
typedef enum nau7802_gain {
    NAU7802_GAIN_1,
    NAU7802_GAIN_2,
    NAU7802_GAIN_4,
    NAU7802_GAIN_8,
    NAU7802_GAIN_16,
    NAU7802_GAIN_32,
    NAU7802_GAIN_64,
    NAU7802_GAIN_128,
} nau7802_gain_t;

/** The possible sample rates */
typedef enum nau7802_sample_rate {
    NAU7802_RATE_10SPS = 0,
    NAU7802_RATE_20SPS = 1,
    NAU7802_RATE_40SPS = 2,
    NAU7802_RATE_80SPS = 3,
    NAU7802_RATE_320SPS = 7,
} nau7802_sample_rate_t;

/** the possible calibration modes */
typedef enum nau7802_calib_mode {
    NAU7802_CALMOD_INTERNAL = 0,
    NAU7802_CALMOD_OFFSET = 2,
    NAU7802_CALMOD_GAIN = 3,
} nau7802_calib_mode_t;


/**
 * @brief initialize driver
 *
 * @return err_t error code
 */
err_t NAU7802_Init(void);

/**
 * @brief initialize particular device
 *
 * @param dev device descriptor
 *
 * @return err_t initialization error code
 */
err_t NAU7802_DevInit(nau7802_dev_t *dev);

/**
 * @brief perform device reset procedure
 *
 * @param dev device descriptor
 *
 * @return err_t status code
 */
err_t NAU7802_Reset(nau7802_dev_t *dev);

/**
 * @brief reads the revision id
 *
 * @param dev device descriptor
 * @param rev_id placeholder for revision id
 *
 * @return err_t error code
 */
err_t NAU7802_GetRevisionID(nau7802_dev_t *dev, int *rev_id);

/**
 * @brief enable/disable the conversions
 *
 * @param dev device descriptor
 * @param enable 1 - enable, 0 - disable
 *
 * @return err_t error code
 */
err_t NAU7802_Enable(nau7802_dev_t *dev, int enable);

/**
 * @brief set ldo voltage used to power the bridge
 *
 * @param dev device descriptor
 * @param mode voltage level
 *
 * @return err_t error code
 */
err_t NAU7802_SetLDO(nau7802_dev_t *dev, nau7802_ldo_t mode);

/**
 * @brief set the PGA gain
 *
 * @param dev device descriptor
 * @param gain gain setting to be applied
 *
 * @return err_t error code
 */
err_t NAU7802_SetGain(nau7802_dev_t *dev, nau7802_gain_t gain);

/**
 * @brief set the sampling rate
 *
 * @param dev device descriptor
 * @param rate sampling rate
 *
 * @return err_t error code
 */
err_t NAU7802_SetSamplingRate(nau7802_dev_t *dev, nau7802_sample_rate_t rate);

/**
 * @brief perform calibration
 *
 * @param dev device descriptor
 * @param mode mode of calibration
 *
 * @return err_t error code
 */
err_t NAU7802_Calibrate(nau7802_dev_t *dev, nau7802_calib_mode_t mode);

/**
 * @brief reads adc conversion result
 *
 * @param dev device descriptor
 * @param value value read
 *
 * @return err_t error code
 */
err_t NAU7802_Read(nau7802_dev_t *dev, int32_t *value);

/**
 * @brief check if the data is ready for reading
 *
 * @param dev device descriptor
 * @param ready ready placeholder
 *
 * @return err_t error code
 */
err_t NAU7802_DataReady(nau7802_dev_t *dev, int *ready);


#endif /* DEV_NAU7802_H */
