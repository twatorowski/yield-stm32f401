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
#include <stddef.h>

#include "config.h"
#include "err.h"
#include "sys/time.h"

/** callback function type */
typedef void (*cb_t) (void *);

/** listener structure */
typedef struct ev_listener {
    /* listener task */
    uint32_t task_id;
    /* id of an event when we started to listen */
    uint32_t ev_id;
    /* the event itself */
    struct ev *ev;
} ev_listener_t;

/** callback based event */
typedef struct ev {
    /* current callback argument */
    void *arg;
    /* event call id */
    uint32_t id;
    /** array of callback */
	cb_t cb[SYS_EV_MAX_CBS];

    /* all the listeners */
    ev_listener_t listeners[4];
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
 * @brief Waits for an event to happen
 *
 * @param event event that we wait for
 * @param timeout timeout in ms
 *
 * @return err_t result code
**/
err_t Ev_Wait(ev_t *event, dtime_t timeout);


/**
 * @brief start listening to events
 *
 * @param event event that we want to listen for
 * @return ev_listener_t* listener descriptor
**/
ev_listener_t * Ev_Listen(ev_t *event);

/**
 * @brief wait and capture the event that we are listening to
 *
 * @param lst listener descriptor
 * @param arg pointer to which we will store the pointer to event's argument
 * @param timeout max timeout
 *
 * @return err_t status of waiting for the event
**/
err_t Ev_Capture(ev_listener_t *lst, void **arg, dtime_t timeout);

/**
 * @brief ack the event that was captured. After acking the pointer to the
 * argument that user got with Ev_Capture is no longer valid
 *
 * @param lst listener descriptor
**/
void Ev_Ack(ev_listener_t *lst);

/**
 * @brief stop listening to an event
 *
 * @param lst listener descriptor
**/
void Ev_ListenEnd(ev_listener_t *lst);

/* convenienve macro for listeing to events */
#define ev_listen(l, ev)        \
    for (ev_listener_t * CLEANUP(ev_listen_cleanup) (l) = Ev_Listen(ev); ; Ev_Ack(l), Yield())

/* cleanup logic for the event listen macro */
static inline void ev_listen_cleanup(ev_listener_t **l) { if (*l) Ev_ListenEnd(*l); }

#endif /* SYS_EV_H */
