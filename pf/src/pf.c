/**
 * @file pf.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-04
 * 
 * @copyright Copyright (c) 2025
 */

#include "pf/pf.h"
#include "dev/keyboard.h"
#include "dev/display.h"
#include "dev/pumps.h"
#include "dev/pressure_sense.h"
#include "dev/batt.h"
#include "dev/stepup.h"
#include "dev/charger.h"
#include "dev/valve.h"
#include "dev/husb238.h"
#include "sys/yield.h"
#include "sys/sleep.h"
#include "dev/vusb_detect.h"
#include "dev/standby.h"
#include "util/elems.h"
#include "util/stdio.h"

#define DEBUG DLVL_INFO
#include "debug.h"

/* convert from mmHG to kPa */
#define MMHG_TO_KPA(x)                      ((x) * 0.133f)
/* convert different time units to milliseconds */
#define S_TO_MS(x)                          ((x) * 1000)
#define M_TO_MS(x)                          ((x) * 60 * 1000)

/* modes of operation */
typedef enum mode {
    MODE_NO_COMP,
    MODE_LOW,
    MODE_MID,
    MODE_HIGH
} mode_t;

/* mode parameters */
static struct {
    float pressure;
    /* time which pressure is applied and when it's not applied */
    dtime_t pon_time, poff_time;
    /* whole procedure time */
    dtime_t proc_time;
    /* displayed name */
    const char *disp_name;
} modes[] = {
    /* no compression mode */
    [MODE_NO_COMP] = {
        .proc_time = M_TO_MS(0.1),
        .disp_name = "P  A",
    },
    /* low compression mode */
    [MODE_LOW] = {
        .pressure = MMHG_TO_KPA(50), .pon_time = S_TO_MS(5),
        .poff_time = S_TO_MS(3), .proc_time = M_TO_MS(15),
        .disp_name = "P  b",
    },
    /* mid compression mode */
    [MODE_MID] = {
        .pressure = MMHG_TO_KPA(50), .pon_time = S_TO_MS(55),
        .poff_time = S_TO_MS(45), .proc_time = M_TO_MS(15),
        .disp_name = "P  c",
    },
    /* high compression mode */
    [MODE_HIGH] = {
        .pressure = MMHG_TO_KPA(50), .pon_time = S_TO_MS(60),
        .poff_time = S_TO_MS(65), .proc_time = M_TO_MS(15),
        .disp_name = "P  d",
    },
};

/* battery capacity */
static int batt_cap = -1;

/* check whether we are connected to usb, what current can we draw, can we
 * charge the battery with it, blah blah... */
static void PF_PowerGovenorTask(void *arg)
{
    /* time of the last battery capacity measurement */
    time_t last_batt_cap_check = 0;
    /* usb check up */
    time_t last_time_usb_check = 0;

    /* get device descriptor */
    husb238_dev_t husb = { .swi2c = &swi2c_husb };
    /* get the current current capabilities ;-)*/
    husb328_amps_t amps = HUSB_AMPS_0A5;

    int was_usb_connected = 0;
    time_t usb_insertion_ts;


    /* power monitoring loop */
    for (;; Yield()) {
        /* check the usb bus */
        if (!last_time_usb_check || dtime_now(last_time_usb_check) > 500) {
            /* is the usb cable plugged in */
            int is_usb_connected = VUSBDet_IsConnected();
            /* usb inserted */
            if (!was_usb_connected && is_usb_connected)
                amps = HUSB_AMPS_0A5, usb_insertion_ts = time(0);

            /* are we connected for more than one second? */
            if (is_usb_connected && dtime_now(usb_insertion_ts) > 1000) {
                /* get the contract from the chip */
                if (HUSB238_GetCurrentContract(&husb, 0, &amps) < EOK)
                    amps = HUSB_AMPS_0A5;
                /* oh please, give me at least half an amp */
                if (amps == HUSB_AMPS_UNKNOWN)
                    amps = HUSB_AMPS_0A5;

            }
            /* kick the timestamp */
            last_time_usb_check = time(0);
            was_usb_connected = is_usb_connected;
        }

        /* derive the charging current */
        charger_current_t cc;
        if (amps <= HUSB_AMPS_0A5) {
            cc = CHARGER_CURRENT_515MA;
        } else if (amps <= HUSB_AMPS_1A) {
            cc = CHARGER_CURRENT_515MA;
        } else if (amps <= HUSB_AMPS_1A5) {
            cc = CHARGER_CURRENT_1103MA;
        } else if (amps <= HUSB_AMPS_2A) {
            cc = CHARGER_CURRENT_1394MA;
        } else if (amps <= HUSB_AMPS_2A5) {
            cc = CHARGER_CURRENT_1727MA;
        } else {
            cc = CHARGER_CURRENT_2316MA;
        }
        /* set the charging current */
        Charger_SetChargingCurrent(cc);


        /* time to check the battery capacity */
        if (!last_batt_cap_check ||
            dtime_now(last_batt_cap_check) > 5000) {
            /* disable the charger */
            Charger_Enable(0); Sleep(300);

            /* battery voltage expressed in millivolts */
            float batt_mv;
            /* get the battery voltage */
            Batt_GetVoltage(&batt_mv);
            /* convert to capacity */
            batt_cap = BATT_VoltageToCap(batt_mv);
            dprintf_i("batt_cap = %d, usb = %d\n", batt_cap,
                VUSBDet_IsConnected());

            /* restore charging */
            Charger_Enable(1);
            /* kick the timestamp */
            last_batt_cap_check = time(0);
        }
    }
}

/* task for ui processing */
static void PF_UITask(void *arg)
{
    /* key masks */
    kbd_mask_t prev_keys = 0, curr_keys = 0, key_presssed = 0;
    /* timestamp since last keypress */
    time_t keypress_ts = 0;

    /* state machine */
    enum { STATE_OFF, STATE_IDLE, STATE_SELECT, STATE_BATT, STATE_END,
        STATE_RUNNING, STATE_ERR } state = STATE_OFF, next_state = STATE_BATT,
        end_follow_up_state;
    /* state update timestamp */
    time_t state_ts;
    /* is this a fresh start? */
    int fresh_start = 1;

    /* curretnly displayed chars */
    char display[5] = { 0 }, prev_display[4] = { 0 };
    /* separating dot display */
    int display_dot = 0, prev_display_dot = 0;
    /* last time the display got updated */
    time_t display_update_ts = 0;

    /* current mode */
    mode_t mode = MODE_NO_COMP;
    /* enumeration for the error code */
    enum {
        ERR_PRESSURE_SENSOR
    } err_code;

    /* pressure baseline as measured when sensor is first initialized */
    float pressure_baseline;

    /* ui event processing loop */
    for (;; Yield()) {
        /* get keyboard buttons state */
        curr_keys = Kbd_GetState();
        /* check which keys were pressed */
        key_presssed = (~prev_keys) & curr_keys;
        /* update the timestamp */
        if (key_presssed)
            keypress_ts = time(0);

        /* switch on current state */
        switch (state) {
        /* device is completely off */
        case STATE_OFF: {
            if (key_presssed & KBD_MASK_UP)
                next_state = STATE_BATT;
            /* render the writing on the display */
            snprintf(display, sizeof(display), "    ");

            /* we are not powered by the usb, go to standby mode */
            if (!fresh_start && !VUSBDet_IsConnected()) {
                StandBy_Enter();
            }
            /* clear the fresh start flag */
            if (next_state != STATE_OFF)
                fresh_start = 0;
        } break;
        /* idle state */
        case STATE_IDLE: {
            /* disable the device */
            if (key_presssed & KBD_MASK_UP || dtime_now(state_ts) > 1000)
                next_state = STATE_OFF;
            /* activate selection mode */
            if (key_presssed & KBD_MASK_MID)
                next_state = STATE_SELECT;
            /* render the writing on the display */
            snprintf(display, sizeof(display), "____");
        } break;
        /* device is actively running */
        case STATE_RUNNING: {
            /* how long are we acvice */
            dtime_t active_ms = dtime_now(state_ts);
            /* how many milliseconds are left */
            dtime_t ms_left = modes[mode].proc_time - active_ms;
            /* the program has ended */
            if (ms_left <= 0 || (key_presssed & KBD_MASK_UP)) {
                next_state = STATE_END; display_dot = 0;
                end_follow_up_state = STATE_OFF; break;
            }

            /* program ended with mode selection */
            if (key_presssed & KBD_MASK_MID) {
                next_state = STATE_END; display_dot = 0;
                end_follow_up_state = STATE_SELECT; break;
            }

            /* prepare the time display */
            int seconds_left = ms_left / 1000;
            int minutes_left = seconds_left / 60;
            /* set the display */
            snprintf(display, sizeof(display), "%2d%02d",
                minutes_left, seconds_left % 60);
            /* display the dot? */
            display_dot = ms_left % 1000 > 500;

            /* enable the fluid pump */
            Pumps_SetPumpDutyCycle(PUMPS_PUMP_FLUID, PUMPS_DIR_FWD, 1.0);

            /* whole pumping cycle time */
            int cycle_dur = modes[mode].pon_time + modes[mode].poff_time;
            /* milliseconds into current cycle */
            int ms_into_cycle = cycle_dur ? active_ms % cycle_dur : 0;
            /* are we in discharging cycle? */
            if (!cycle_dur || ms_into_cycle > modes[mode].pon_time) {
                /* enable the valve */
                Valve_Enable(1);
                /* disable the air pump */
                Pumps_SetPumpDutyCycle(PUMPS_PUMP_AIR, PUMPS_DIR_FWD, 0);
            /* pumpung air */
            } else {
                /* close the valve */
                Valve_Enable(0);
                /* current pressure value */
                float pressure;
                /* obtain the pressure readout */
                if (PressureSense_GetReadout(&pressure) < EOK) {
                    next_state = STATE_END; display_dot = 0;
                    end_follow_up_state = STATE_ERR;
                    err_code = ERR_PRESSURE_SENSOR;
                    break;
                }
                /* subract the sensor bias */
                pressure -= pressure_baseline;
                /* pump the air until pressure is reached */
                Pumps_SetPumpDutyCycle(PUMPS_PUMP_AIR, PUMPS_DIR_FWD,
                    pressure < modes[mode].pressure ? 1 : 0);
            }

        } break;
        /* end of running */
        case STATE_END: {
            /* display the writing */
            if ((dtime_now(state_ts) > 5000) ||
                (dtime_now(state_ts) > 2000 && key_presssed)) {
                /* close the valve */
                Valve_Enable(0); next_state = end_follow_up_state; break;
            }

            /* enable the valve to discharge the air */
            Valve_Enable(1);
            /* disable both pumps */
            Pumps_SetPumpDutyCycle(PUMPS_PUMP_AIR, PUMPS_DIR_FWD, 0);
            Pumps_SetPumpDutyCycle(PUMPS_PUMP_FLUID, PUMPS_DIR_FWD, 0);
            /* set the displayed text */
            snprintf(display, sizeof(display), "end ");
        } break;
        /* program selection mode */
        case STATE_SELECT: {
            /* initial "func" display */
            if (dtime_now(state_ts) < 1000) {
                /* render the writing on the display */
                snprintf(display, sizeof(display), "func");
                /* prevent further processing */
                break;
            }

            /* next mdoe */
            mode_t next_mode = mode;

            /* go back in mode selection */
            if (key_presssed & KBD_MASK_LEFT)
                next_mode = mode == 0 ? elems(modes) - 1 : mode - 1;
            /* advance the mode selection */
            if (key_presssed & KBD_MASK_RIGHT)
                next_mode = mode == elems(modes) - 1 ? 0 : mode + 1;
            /* end of mode seleciton */
            if (key_presssed & KBD_MASK_MID)
                next_state = STATE_RUNNING;
            /* disable the device */
            if ((key_presssed & KBD_MASK_UP) ||
                 dtime_now(keypress_ts) > 10 * 1000)
                next_state = STATE_OFF;
            /* mode was updated? */
            if (mode != next_mode)
                mode = next_mode;
            /* render the writing on the display */
            snprintf(display, sizeof(display), "%s",
                modes[next_mode].disp_name);
        } break;
        /* battery voltage */
        case STATE_BATT: {
            /* disable the device */
            if (key_presssed & KBD_MASK_UP) {
                next_state = STATE_OFF;
            /* go to idle state after 2 seconds */
            } else if (dtime_now(state_ts) > 2000 || key_presssed) {
                /* enable the pressure sensor */
                Valve_Enable(1);
                PressureSense_Enable(1); Sleep(300);
                /* time to obtain the baseline pressure */
                float pressure;
                err_t ec = PressureSense_GetReadout(&pressure);
                /* disable the valve */
                Valve_Enable(0);
                /* were we successfull in obtaining the pressure? */
                if (ec >= EOK) {
                    pressure_baseline = pressure;
                    next_state = STATE_SELECT;
                /* unable to initialize the pressure sensor */
                } else {
                    next_state = STATE_ERR;
                    err_code = ERR_PRESSURE_SENSOR;
                }
            }
            /* battery capacity has been measured? */
            if (batt_cap >= 0) {
                /* render the writing on the display */
                snprintf(display, sizeof(display), "b%3d",
                    batt_cap);
            } else {
                snprintf(display, sizeof(display), "    ");
            }
        } break;
        /* display error code */
        case STATE_ERR : {
            /* go to idle state after 2 seconds */
            if (dtime_now(state_ts) < 2000) {
                snprintf(display, sizeof(display), "Err "); break;
            }

            /* error code -> error string to be displayed */
            const char *err_str;
            switch (err_code) {
            case ERR_PRESSURE_SENSOR: err_str = "PrES"; break;
            default: err_str = "____"; break;
            }

            /* render the writing on the display */
            snprintf(display, sizeof(display), "%4s", err_str);
            /* advance the state machine */
            if (dtime_now(state_ts) > 4000)
                next_state = STATE_OFF;
        } break;
        }

        /* device was turned on */
        if (state == STATE_OFF && next_state != STATE_OFF) {
            dprintf_i("powering on\n", 0);
            /* enable power and the display */
            StepUp_Enable(1); Display_Enable(1);
            /* reset timestamp to force the update of the display */
            display_update_ts = 0;
        /* device was turned off */
        } else if (state != STATE_OFF && next_state == STATE_OFF) {
            dprintf_i("powering off\n", 0);
            /* disable power and the display */
            Display_Enable(0); StepUp_Enable(0);
            /* disable the pressure sensor */
            PressureSense_Enable(0);
            /* disable both pumps */
            Pumps_SetPumpDutyCycle(PUMPS_PUMP_AIR, PUMPS_DIR_FWD, 0);
            Pumps_SetPumpDutyCycle(PUMPS_PUMP_FLUID, PUMPS_DIR_FWD, 0);
        /* device is enabled  */
        } else if (state != STATE_OFF) {
            /* shall we update the display? */
            int update_display = dtime_now(display_update_ts) > 1000;
            /* update the display */
            for (size_t i = 0; i < elems(display) - 1; i++) {
                /* update display on digit change but not rarer that once
                 * per second */
                if (prev_display_dot != display_dot ||
                    prev_display[i] != display[i] || update_display) {
                    /* set the character */
                    Display_SetChar(i, prev_display[i] = display[i],
                        i == 1 && display_dot);
                    /* set this flag to indicate that the display was updated */
                    update_display = 1;
                    prev_display_dot = display_dot;
                }
            }
            /* kick the timestmap if the display was in fact updated */
            if (update_display)
                display_update_ts = time(0);
        }

        /* update key mask */
        prev_keys = curr_keys;
        /* advance the state machine */
        if (state != next_state)
            state = next_state, state_ts = time(0);
    }
}

/* initialize pain freeze logic */
err_t PF_Init(void)
{
    /* start battery govenor task */
    Yield_Task(PF_PowerGovenorTask, 0, 2048);
    /* create the user interface task */
    Yield_Task(PF_UITask, 0, 2048);

    /* report status */
    return EOK;
}