/**
 * @file display.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#include "dev/display.h"
#include "dev/aip650e.h"

/* driver descriptor */
static aip650e_dev_t aip650e = { .swi2c = &swi2c_disp };

/* initialize display */
err_t Display_Init(void)
{
    /* return status of the driver initialization */
    return AIP650E_DevInit(&aip650e);
}

/* enable/disable the display */
err_t Display_Enable(int enable)
{
    /* configure the chip */
    return AIP650E_Configure(&aip650e,
        enable ? AIP650E_CFG_DISPLAY_ON : AIP650E_CFG_DISPLAY_OFF);
}

/* sets the digit to be displayed on given position */
err_t Display_SetSegments(int position, display_segments_t segments)
{
    /* decoded digit */
    aip650e_digit_t d;

    /* decode digit */
    switch (position) {
    case 0: d = AIP650E_DIGIT_1; break;
    case 1: d = AIP650E_DIGIT_2; break;
    case 2: d = AIP650E_DIGIT_3; break;
    case 3: d = AIP650E_DIGIT_4; break;
    /* weird digit code */
    default: return EARGVAL;
    }

    /* set the segments */
    return AIP650E_SetSegments(&aip650e, d, segments);
}

/* set's the digit to be displayed  */
err_t Display_SetChar(int position, char c, int decimal_point)
{
    /* this will hold the mask of segments that shall be lit */
    display_segments_t value;

    /* encode char with segments */
    switch (c) {
    /* decimal numbers */
    case '0': value = DISP_SEGMENT_A | DISP_SEGMENT_B | DISP_SEGMENT_C |
        DISP_SEGMENT_D | DISP_SEGMENT_E | DISP_SEGMENT_F; break;
    case '1': value = DISP_SEGMENT_B | DISP_SEGMENT_C; break;
    case '2': value = DISP_SEGMENT_A | DISP_SEGMENT_B | DISP_SEGMENT_D |
        DISP_SEGMENT_E | DISP_SEGMENT_G; break;
    case '3': value = DISP_SEGMENT_A | DISP_SEGMENT_B | DISP_SEGMENT_C |
        DISP_SEGMENT_D | DISP_SEGMENT_G; break;
    case '4': value = DISP_SEGMENT_B | DISP_SEGMENT_C | DISP_SEGMENT_F |
        DISP_SEGMENT_G; break;
    case '5': value = DISP_SEGMENT_A | DISP_SEGMENT_C | DISP_SEGMENT_D |
        DISP_SEGMENT_F | DISP_SEGMENT_G; break;
    case '6': value = DISP_SEGMENT_A | DISP_SEGMENT_C | DISP_SEGMENT_D |
        DISP_SEGMENT_E | DISP_SEGMENT_F | DISP_SEGMENT_G; break;
    case '7': value =  DISP_SEGMENT_A | DISP_SEGMENT_B | DISP_SEGMENT_C;
        break;
    case '8': value = DISP_SEGMENT_A | DISP_SEGMENT_B | DISP_SEGMENT_C |
        DISP_SEGMENT_D | DISP_SEGMENT_E | DISP_SEGMENT_F | DISP_SEGMENT_G;
        break;
    case '9': value = DISP_SEGMENT_A | DISP_SEGMENT_B | DISP_SEGMENT_C |
        DISP_SEGMENT_D | DISP_SEGMENT_F | DISP_SEGMENT_G;
        break;
    case ' ': value = DISP_SEGMENT_NONE;
    /* unsupported character */
    default: return EARGVAL;
    }

    /* decimal point shall be lit? */
    if (decimal_point)
        value |= DISP_SEGMENT_DP;

    /* drive the segments */
    return Display_SetSegments(position, value);
}




#include "sys/sleep.h"
/* initialize particular device */
void Display_Test(void)
{

    for (int i = 0;; i++, Sleep(1000)) {
        // err_t ec = AIP650E_ReadKeays(&dev);
        Display_Enable(0);
        Display_SetChar(0, '0' + (i + 0) % 10, i & 1);
        Display_SetChar(1, '0' + (i + 0) % 10, i & 1);
        Display_SetChar(2, '0' + (i + 0) % 10, i & 1);
        Display_SetChar(3, '0' + (i + 0) % 10, i & 1);
        Display_Enable(1);
        // dprintf_i("reading status = %d\n", ec);s    }
    }
}