#ifndef BTMONITOR
#define BTMONITOR
	
#include "pebble.h"

typedef void(*BTStatusChangedHandler)(bool connected);
typedef void(*BTPingHandler)(void);

typedef struct {
        BTStatusChangedHandler status_changed;
        BTPingHandler ping;
} BTMonitorCallbacks;

void set_btmonitor_vibrate(bool value);
void btmonitor_subscribe(BTMonitorCallbacks callback);
void btmonitor_init();
void btmonitor_deinit(void);
void btmonitor_unsubscribe(void);

#endif
