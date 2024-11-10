/**
 * @file ev.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-03
 *
 * @copyright Copyright (c) 2024
 */

#ifndef SYS_EV_H
#define SYS_EV_H

#include <stdint.h>

#include "config.h"
#include "err.h"
#include "sys/time.h"

/** callback function type */
typedef void (*cb_t) (void *);

/** callback based event */
typedef struct ev {
    /* current callback argument */
    void *arg;
    /* event call id */
    uint32_t id, someone_waits;
    /** array of callback */
	cb_t cb[SYS_EV_MAX_CBS];
} ev_t;

/**
 * @brief subscribe to any given event with given callback
 *
 * @param event event you want to subscribe to
 * @param callback callback that shall be called when event is triggered
 *
 * @return err_t error code
 */
err_t Ev_Subscribe(ev_t *event, cb_t callback);


/**
 * @brief unsubscribe from the event
 *
 * @param event event you want to unsubscribe from
 * @param callback callback that shall be unregistered
 *
 * @return err_t error code
 */
err_t Ev_Unsubscribe(ev_t *event, cb_t callback);

/**
 * @brief notify the listeners of event that occured
 *
 * @param event event to be triggered
 * @param arg argument to be passed to all of the listeners and awaiters
 */
void Ev_Notify(ev_t *event, void *arg);

/**
 * @brief wait for an event to occur
 *
 * @param event event that we wait for to be tiggered
 * @param arg placehodler to store the pointer to the event argument
 * @param timeout maximal waiting time or 0 if infinite
 *
 * @return err_t waiting status
 */
err_t Ev_Wait(ev_t *event, void **arg, dtime_t timeout);


#endif /* SYS_EV_H */
