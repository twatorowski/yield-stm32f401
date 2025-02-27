/**
 * @file cb.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-03
 *
 * @copyright Copyright (c) 2024
 */

#ifndef SYS_SRC_EV_C
#define SYS_SRC_EV_C

#include "sys/ev.h"
#include "sys/yield.h"
#include "util/string.h"

/* subscribe to any given event */
err_t Ev_Subscribe(ev_t *event, cb_t callback)
{
    /* look for a slot within the callbacks array and try to
     * register the callback */
    for (int i = 0; i < SYS_EV_MAX_CBS; i++)
        if (!event->cb[i]) {
            event->cb[i] = callback; return EOK;
        }

    /* unable to register more callbacks */
    return EFATAL;
}

/* unsubscribe from the event */
err_t Ev_Unsubscribe(ev_t *event, cb_t callback)
{
    /* look for a slot within the callbacks array and try to
     * register the callback */
    for (int i = 0; i < SYS_EV_MAX_CBS; i++)
        if (event->cb[i] == callback) {
            event->cb[i] = 0; return EOK;
        }

    /* unable to unregister the event */
    return EFATAL;
}

/* notify the listeners of event that occured */
void Ev_Notify(ev_t *event, void *arg, dtime_t timeout)
{
    /* set the argument */
    event->arg = arg;
    /* bump up the id of the event call */
    event->id++;

    /* notify all of the subscribed listeners */
    for (int i = 0; i < SYS_EV_MAX_CBS; i++)
        if (event->cb[i])
            event->cb[i](arg);

    /* current time */
    time_t ts = time(0);
    /* wait this much for someone to register */
    for (uint32_t wait_cnt = event->wait_cnt;
        timeout && wait_cnt == event->wait_cnt; Yield()) {
            if (dtime_now(ts) > timeout)
                break;
    }

    /* spin here as long as someone is processing this event */
    for (uint32_t wait_cnt = event->wait_cnt; event->ack_cnt != wait_cnt;
        Yield());
}

/* wait for an event to occur */
err_t Ev_Wait(ev_t *event, void **arg, dtime_t timeout)
{
    /* this id will get incremented as soon as the event is triggered */
    uint32_t curr_id = event->id; err_t ec = EOK;
    /* register awaiter */
    event->wait_cnt++;

    /* waiting loop */
    for (time_t ts = time(0); ec == EOK; Yield()) {
        /* event has been emitted */
        if (curr_id != event->id)
            break;
        /* a timeout has occured */
        if (timeout && dtime_now(ts) > timeout)
            ec = ETIMEOUT;
        /* task on which we wait is cancelled */
        if (Yield_IsCancelled())
            ec = ECANCEL;
    }

    /* waiting failed */
    if (ec != EOK) {
        event->ack_cnt++;
    /* store the pointer to the event */
    } else {
        *arg = event->arg;
    }

    /* return status */
    return ec;
}

/* acknowledge the consumption of an event */
err_t Ev_Ack(ev_t *event)
{
    /* acknowledge the event */
    event->ack_cnt++;
    /* report status */
    return EOK;
}


#endif /* SYS_SRC_EV_C */
