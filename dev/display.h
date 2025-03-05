/**
 * @file display.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 *
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_DISPLAY_H
#define DEV_DISPLAY_H

#include "err.h"
#include "dev/aip650e.h"

/* connection mapping between the aip650 and the display */
typedef enum display_segments {
    DISP_SEGMENT_NONE = AIP650E_SEGMENT_NONE,
    /* mapping between the driver and the display itself */
    DISP_SEGMENT_A = AIP650E_SEGMENT_C,
    DISP_SEGMENT_B = AIP650E_SEGMENT_A,
    DISP_SEGMENT_C = AIP650E_SEGMENT_E,
    DISP_SEGMENT_D = AIP650E_SEGMENT_G,
    DISP_SEGMENT_E = AIP650E_SEGMENT_DP,
    DISP_SEGMENT_F = AIP650E_SEGMENT_B,
    DISP_SEGMENT_G = AIP650E_SEGMENT_D,
    DISP_SEGMENT_DP = AIP650E_SEGMENT_F,
    DISP_SEGMENT_ALL = AIP650E_SEGMENT_ALL,
} display_segments_t;

/**
 * @brief initialize the display
 *
 * @return err_t error code
 */
err_t Display_Init(void);

/**
 * @brief enable or disable the display
 *
 * @param enable 0 - disable, 1 - enable
 *
 * @return err_t error code
 */
err_t Display_Enable(int enable);

/**
 * @brief set segments on any given display position
 *
 * @param position position 0 - 3
 * @param segments bit-mask of the segments to be set
 *
 * @return err_t error code
 */
err_t Display_SetSegments(int position, display_segments_t segments);

/**
 * @brief set character on any given position
 *
 * @param position position 0 - 3
 * @param c char to be displayed (if possible)
 * @param decimal_point shall the decimal point be lit?
 *
 * @return err_t error code
 */
err_t Display_SetChar(int position, char c, int decimal_point);


/**
 * @brief clear the display
 *
 * @return err_t error code
 */
err_t Display_Clear(void);


/**
 * @brief set the characters on the display
 *
 * @param position offset from the left
 * @param c array of characters
 * @param size size of the array
 *
 * @return err_t error code
 */
err_t Display_SetChars(size_t offs, const char *c, size_t size);

#endif /* DEV_DISPLAY_H */
