#include "pebble.h"

#ifndef THINCFG
#define THINCFG

//If INVERT_MODE is INVERT_ALWAYS, then the color will be inverted (white on black text)
//If INVERT_MODE is INVERT_IN_AM, then the color will only be inverted from 12:00AM to 11:59AM
//Otherwise the color will not be inverted.
#define INVERT_NEVER 0
#define INVERT_ON_AM 1
#define INVERT_ALWAYS 2

enum 
{
	CONFIG_KEY_INVERTMODE = 29010,
	CONFIG_KEY_BTNOTIFICATION = 29011,
	CONFIG_KEY_ACTION = 29012
};

typedef void(*ThinCFGFieldChangedHandler)(const uint32_t key, const void *old_value, const void *new_value);

typedef struct {
        ThinCFGFieldChangedHandler field_changed;
} ThinCFGCallbacks;

int get_invert_mode_value(void);
void set_invert_mode_value(int value);

bool get_bt_notification_value(void);
void set_bt_notification_value(bool value);

void thincfg_init(void);
void thincfg_deinit(void);
void thincfg_subscribe(ThinCFGCallbacks callback);
void thincfg_unsubscribe(void);
	
#endif
