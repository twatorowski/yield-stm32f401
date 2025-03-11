/**
 * @file aip650e.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_AIP650E_H
#define DEV_AIP650E_H

#include "err.h"

#include "dev/swi2c_dev.h"
#include "util/bit.h"

/* enumeration for the digits */
typedef enum aip650e_digit {
    AIP650E_DIGIT_1,
    AIP650E_DIGIT_2,
    AIP650E_DIGIT_3,
    AIP650E_DIGIT_4,
} aip650e_digit_t;

/* enumebration of segments */
typedef enum aip650e_segments {
    AIP650E_SEGMENT_NONE = 0,
    /* separate segments */
    AIP650E_SEGMENT_A = BIT_VAL(0),
    AIP650E_SEGMENT_B = BIT_VAL(1),
    AIP650E_SEGMENT_C = BIT_VAL(2),
    AIP650E_SEGMENT_D = BIT_VAL(3),
    AIP650E_SEGMENT_E = BIT_VAL(4),
    AIP650E_SEGMENT_F = BIT_VAL(5),
    AIP650E_SEGMENT_G = BIT_VAL(6),
    AIP650E_SEGMENT_DP = BIT_VAL(7),
    /* all of the segments */
    AIP650E_SEGMENT_ALL = AIP650E_SEGMENT_A | AIP650E_SEGMENT_B |
        AIP650E_SEGMENT_C | AIP650E_SEGMENT_D | AIP650E_SEGMENT_E |
        AIP650E_SEGMENT_F | AIP650E_SEGMENT_G | AIP650E_SEGMENT_DP
} aip650e_segments_t;

/* configuration bits */
typedef enum aip650e_config {
    /* display on/off */
    AIP650E_CFG_DISPLAY_OFF = 0,
    AIP650E_CFG_DISPLAY_ON = BIT_VAL(0),
    /* sleep mode */
    AIP650E_CFG_SLEEP_OFF = 0,
    AIP650E_CFG_SLEEP_ON = BIT_VAL(2),

    /* brightness levels */
    AIP650E_CFG_BRIGHTNESS =   7 << 4,
    AIP650E_CFG_BRIGHTNESS_1 = 1 << 4,
    AIP650E_CFG_BRIGHTNESS_2 = 2 << 4,
    AIP650E_CFG_BRIGHTNESS_3 = 3 << 4,
    AIP650E_CFG_BRIGHTNESS_4 = 4 << 4,
    AIP650E_CFG_BRIGHTNESS_5 = 5 << 4,
    AIP650E_CFG_BRIGHTNESS_6 = 6 << 4,
    AIP650E_CFG_BRIGHTNESS_7 = 7 << 4,
    AIP650E_CFG_BRIGHTNESS_8 = 0 << 4,
} aip650e_config_t;

/* key encoding */
typedef enum aip650e_key {
    AIP650E_KEY_1,
    AIP650E_KEY_2,
    AIP650E_KEY_3,
    AIP650E_KEY_4,
    AIP650E_KEY_5,
    AIP650E_KEY_6,
    AIP650E_KEY_7,
    AIP650E_KEY_1_2,
} aip650e_key_t;

/* device descriptor */
typedef struct aip650e_dev {
    swi2c_dev_t *swi2c;
} aip650e_dev_t;

/**
 * @brief initialize common part of the device driver
 *
 * @return err_t error code
 */
err_t AIP650E_Init();

/**
 * @brief initialize particular device
 *
 * @param dev device descriptor
 * @return err_t error code
 */
err_t AIP650E_DevInit(aip650e_dev_t *dev);

/**
 * @brief read the key states as reported by the chip
 *
 * @param dev device descriptor
 * @param digit digit (or row)
 * @param key key (or column)
 *
 * @return err_t error code
 */
err_t AIP650E_ReadKeys(aip650e_dev_t *dev, aip650e_digit_t *digit,
    aip650e_key_t *key);

/**
 * @brief configure the operation of the chip
 *
 * @param dev device descriptor
 * @param cfg confguration bit-mask
 *
 * @return err_t error code
 */
err_t AIP650E_Configure(aip650e_dev_t *dev, aip650e_config_t cfg);

/**
 * @brief set the segments for given digit
 *
 * @param dev device descriptor
 * @param digit digit for which we set the segments
 * @param segments segments bitmas
 *
 * @return err_t error code
 */
err_t AIP650E_SetSegments(aip650e_dev_t *dev, aip650e_digit_t digit,
    aip650e_segments_t segments);



// TODO:
err_t AIP650E_Test(void);


#endif /* DEV_AIP650E_H */
