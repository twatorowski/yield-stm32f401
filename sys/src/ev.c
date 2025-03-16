/**
 * @file cb.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-03
 * 
 * @copyright Copyright (c) 2024
**/

#include "sys/ev.h"
#include "sys/yield.h"
#include "util/string.h"
#include "util/elems.h"
#include "util/forall.h"

/* subscribe to any given event */
err_t Ev_Subscribe(ev_t *event, cb_t callback)
{

    /* look for a slot within the callbacks array and try to
     * register the callback */
    for (int i = 0; i < SYS_EV_MAX_CBS; i++) {
        if (!event->cb[i]) {
            event->cb[i] = callback; return EOK;
        }
    }
    /* unable to register more callbacks */
    return EFATAL;
}

/* unsubscribe from the event */
err_t Ev_Unsubscribe(ev_t *event, cb_t callback)
{
    /* look for a slot within the callbacks array and try to
     * register the callback */
    for (int i = 0; i < SYS_EV_MAX_CBS; i++) {
        if (event->cb[i] == callback) {
            event->cb[i] = 0; return EOK;
        }
    }

    /* unable to unregister the event */
    return EFATAL;
}

/* notify the listeners of event that occured */
void Ev_Notify(ev_t *event, void *arg)
{
    /* pointer to the listener entry */
    ev_listener_t *lst;

    /* set the argument */
    event->arg = arg;
    /* bump up the id of the event call */
    event->id++;

    /* notify all of the subscribed listeners */
    for (int i = 0; i < SYS_EV_MAX_CBS; i++)
        if (event->cb[i])
            event->cb[i](arg);

    /* loop until all listeners have acked the reception of the event */
    for (int all_done = 1; ; all_done = 1, Yield()) {
        /* look for an listener that still did not finish it's job */
        forall (lst, event->listeners)
            if (lst->task_id != 0 && lst->ev_id != event->id) {
                all_done = 0; break;
            }
        /* all listeners are done? */
        if (all_done)
            break;
    }
}

/* wait for envent to happen */
err_t Ev_Wait(ev_t *event, dtime_t timeout)
{
    /* get current event id */
    uint32_t id = event->id;

    /* wait as long as the event id is not equal to id */
    for (time_t ts = time(0); id == event->id; Yield())
        if (timeout && dtime_now(ts) > timeout)
            return ETIMEOUT;
    /* return success */
    return EOK;
}

/* start listening to events */
ev_listener_t * Ev_Listen(ev_t *event)
{
    /* pointer to the listener entry */
    ev_listener_t *lst;

    /* look for a free entry */
    forall (lst, event->listeners)
        if (lst->task_id == 0)
            break;

    /* not found */
    if (lst == arrend(event->listeners))
        return 0;

    /* store information about the listener */
    lst->task_id = Yield_GetTaskID();
    lst->ev_id = event->id;
    lst->ev = event;

    /* report success */
    return lst;
}

/* receive an event during listening */
err_t Ev_Capture(ev_listener_t *lst, void **arg, dtime_t timeout)
{
    /* wait as long as the event id is not equal to id */
    for (time_t ts = time(0); lst->ev->id == lst->ev_id; Yield())
        if (timeout && dtime_now(ts) > timeout)
            return ETIMEOUT;
    /* return the event argument */
    if (arg)
        *arg = lst->ev->arg;

    /* return the success code */
    return EOK;
}

/* ack the event that was captured */
void Ev_Ack(ev_listener_t *lst)
{
    lst->ev_id = lst->ev->id;
}

/* we are done listening */
void Ev_ListenEnd(ev_listener_t *lst)
{
    /* clear the record */
    lst->task_id = 0;
}

