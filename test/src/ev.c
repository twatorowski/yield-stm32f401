/**
 * @file ev.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-12
 * 
 * @copyright Copyright (c) 2025
**/


#include "err.h"
#include "sys/yield.h"
#include "sys/sleep.h"
#include "sys/ev.h"
#include "util/forall.h"
#include "compiler.h"

#define DEBUG DLVL_INFO
#include "debug.h"


/* event that we use for tests */
static ev_t ev;
/* event argument type */
typedef struct evarg {
    int value;
} evarg_t;

static void TestEV_Callback(void *ptr)
{
    /* true form of the event argument */
    evarg_t *arg = ptr;

    dprintf_i("callback, value = %d\n", arg->value);
}

/* initialize test */
static void TestEV_Producer(void *arg)
{
    /* value that we will broadcast */
    int value = 0;

    for (;; Sleep(1000)) {
        /* notify to all listeners */
        Ev_Notify(&ev, &(evarg_t){.value = value});
        dprintf_i("notified of %d\n", value);
        value++;
    }
}

/* listener task */
static void TestEV_Listener(void *arg)
{
    for (;; Sleep(4000)) {

        /* listen to events */
        ev_listen (l, &ev) {
            Sleep(2000);
            evarg_t *a;
            if (Ev_Capture(l, (void **)&a, 300) < EOK) {
                dprintf_i("timeout\n", 0);
                continue;
            }

            dprintf_i("a = %d\n", a->value);
            if (a->value > 10)
                break;
        }
    }
}


/* initialize event test */
err_t TestEV_Init(void)
{
    Yield_Task(TestEV_Producer, 0, 1024);
    Yield_Task(TestEV_Listener, 0, 1024);

    Ev_Subscribe(&ev, TestEV_Callback);

    return EOK;
}