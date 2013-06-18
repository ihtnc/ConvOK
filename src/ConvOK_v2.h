#include "pebble_os.h"

//#define DEBUG
	
// Slot on-screen layout:
//     0
//     1
//     2

#define SLOTS_COUNT 3
#define SLOT_STATUS_EMPTY -1

#define SLOT_TOP 0
#define SLOT_MID 1
#define SLOT_BOT  2

#define SLOT_XOFFSET 0
#define SLOT_TOP_YOFFSET 0
#define SLOT_MID_YOFFSET 30
#define SLOT_BOT_YOFFSET 104
	
#define SLOT_TOP_OUT_DURATION 200
#define SLOT_MID_OUT_DURATION 500
#define SLOT_BOT_OUT_DURATION 400

#define SLOT_TOP_OUT_DELAY 100
#define SLOT_MID_OUT_DELAY 0
#define SLOT_BOT_OUT_DELAY 50
	
#define SLOT_TOP_IN_DURATION 300
#define SLOT_MID_IN_DURATION 500
#define SLOT_BOT_IN_DURATION 400

#define SLOT_TOP_IN_DELAY SLOT_TOP_OUT_DELAY + SLOT_TOP_OUT_DURATION + 100
#define SLOT_MID_IN_DELAY SLOT_MID_OUT_DELAY + SLOT_MID_OUT_DURATION + 0
#define SLOT_BOT_IN_DELAY SLOT_BOT_OUT_DELAY + SLOT_BOT_OUT_DURATION + 50

#define SCREEN_HEIGHT 168
#define SCREEN_WIDTH 144

void load_image_to_slot(int slot_number, int hour_value, int minute_value);
int determine_image_from_value(int slot_number, int slot_value);
void unload_image_from_slot(int slot_number);

void display_time(PblTm *tick_time);
void animate_slot(int slot_number, bool in);
