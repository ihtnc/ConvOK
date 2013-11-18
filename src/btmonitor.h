#ifndef BTMONITOR
#define BTMONITOR
	
#include "pebble.h"

typedef void(*BTStatusChangedHandler)(bool connected);
typedef void(*BTPingHandler)(void);

typedef struct {
        BTStatusChangedHandler status_changed;
        BTPingHandler ping;
} BTMonitorCallbacks;

void btmonitor_subscribe(BTMonitorCallbacks callback);
void btmonitor_init(bool enable_vibrate);
void btmonitor_deinit();

#endif