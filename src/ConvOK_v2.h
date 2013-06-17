#include "pebble_os.h"

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
	
void load_image_to_slot(int slot_number, int hour_value, int minute_value);
int determine_image_from_value(int slot_number, int slot_value);
void unload_image_from_slot(int slot_number);

void display_time(PblTm *tick_time);
int get_display_hour(int hour);