/**
 * @file keyboard.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_KEYBOARD_H
#define DEV_KEYBOARD_H

#include "err.h"
#include "util/bit.h"

/* keyboard mask */
typedef enum kbd_mask {
    KBD_MASK_UP = BIT_VAL(0),
    KBD_MASK_MID = BIT_VAL(1),
    KBD_MASK_LEFT = BIT_VAL(2),
    KBD_MASK_RIGHT = BIT_VAL(3),
} kbd_mask_t;

/**
 * @brief initialize keyboard driver
 *
 * @return err_t error code
 */
err_t Kbd_Init(void);

/**
 * @brief returns the current state of the buttons
 *
 * @return kbd_keymask_t error code
 */
kbd_mask_t Kbd_GetState(void);

#endif /* DEV_KEYBOARD_H */
