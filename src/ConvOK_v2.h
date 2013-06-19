#include "pebble_os.h"

//Uncomment this for debug mode
//Debug mode treats the watch's minute value as the hour value and the watch's second value as the minute value.
//This is to speed up the transition of the animations
//#define DEBUG

#define SLOTS_COUNT 3
#define SLOT_STATUS_EMPTY -1

#define SLOT_TOP 0
#define SLOT_MID 1
#define SLOT_BOT  2

#define SLOT_XOFFSET 0
#define SLOT_TOP_YOFFSET 0
#define SLOT_MID_YOFFSET 30
#define SLOT_BOT_YOFFSET 104

//Duration and delay are in ms
//Note: In debug mode, be aware that every 15 seconds, the top slot will change again after just 1000ms.
//With this, ensure that the total animation duration of the top slot (out delay + out duration + in delay + in duration) does not reach 1000ms.
//Otherwise the animation after every 15 seconds will get clunky.
#define SLOT_TOP_OUT_DURATION 200
#define SLOT_MID_OUT_DURATION 500
#define SLOT_BOT_OUT_DURATION 400

#define SLOT_TOP_OUT_DELAY 100
#define SLOT_MID_OUT_DELAY 0
#define SLOT_BOT_OUT_DELAY 300
	
#define SLOT_TOP_IN_DURATION 300
#define SLOT_MID_IN_DURATION 500
#define SLOT_BOT_IN_DURATION 200

#define SLOT_TOP_IN_DELAY SLOT_TOP_OUT_DELAY + SLOT_TOP_OUT_DURATION + 200
#define SLOT_MID_IN_DELAY SLOT_MID_OUT_DELAY + SLOT_MID_OUT_DURATION + 0
#define SLOT_BOT_IN_DELAY SLOT_BOT_OUT_DELAY + SLOT_BOT_OUT_DURATION + 100

#define SCREEN_HEIGHT 168
#define SCREEN_WIDTH 144

void load_image_to_slot(int slot_number, int hour_value, int minute_value);
int determine_image_from_value(int slot_number, int slot_value);
void unload_image_from_slot(int slot_number);

void display_time(PblTm *tick_time);
void animate_slot(int slot_number, bool in);
