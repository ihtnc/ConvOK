#include "pebble.h"
	
#define SLOTS_COUNT 3
	
#define SLOT_STATE_NORMAL 0
#define SLOT_STATE_EMPTY -1
#define SLOT_STATE_SPLASH -2
	
#define SLOT_TOP 0
#define SLOT_MID 1
#define SLOT_BOT  2

#define SCREEN_HEIGHT 168
#define SCREEN_WIDTH 144
	
Window *window;
InverterLayer *inverter;

int invert_mode;

typedef struct
{
	GBitmap *image;
	BitmapLayer *layer;
	
	PropertyAnimation *animation_in;
	GRect animation_in_from_frame;
	GRect animation_in_to_frame;
	
	PropertyAnimation *animation_out;
	GRect animation_out_from_frame;
	GRect animation_out_to_frame;
	
	int state;
	int slot_number;
} Slot;
Slot slots[SLOTS_COUNT];

typedef struct
{
	int animation_duration_in;
	int animation_duration_out;
	int animation_duration_splash;
	int animation_delay_in;
	int animation_delay_out;
	int offset_x;
	int offset_y;
	int offset_splash_x;
	int offset_splash_y;
} SlotInfo;
SlotInfo info[SLOTS_COUNT] = 
{
	{
		300, //animation_duration_in
		200, //animation_duration_out
		1200, //animation_duration_splash
		200, //animation_delay_in
		100, //animation_delay_out
		0, //offset_x
		0, //offset_y
		10, //offset_splash_x
		0 //offset_splash_y
	},
	{
		500, //animation_duration_in
		500, //animation_duration_out
		1200, //animation_duration_splash
		0, //animation_delay_in
		0, //animation_delay_out
		0, //offset_x
		30, //offset_y
		20, //offset_splash_x
		0 //offset_splash_y
	},
	{
		200, //animation_duration_in
		400, //animation_duration_out
		1200, //animation_duration_splash
		100, //animation_delay_in
		300, //animation_delay_out
		0, //offset_x
		104, //offset_y
		0, //offset_splash_x
		0 //offset_splash_y
	}
};

const int IMAGE_RESOURCE_SPLASH_IDS[SLOTS_COUNT] = 
{
	RESOURCE_ID_IMAGE_TOP_SPLASH,
	RESOURCE_ID_IMAGE_MID_SPLASH,
	RESOURCE_ID_IMAGE_BOT_SPLASH
};

const int IMAGE_RESOURCE_TOP_IDS[4] = 
{
	RESOURCE_ID_IMAGE_TOP_EXACT,
	RESOURCE_ID_IMAGE_TOP_AFTER,
	RESOURCE_ID_IMAGE_TOP_BEFORE, 
	RESOURCE_ID_IMAGE_TOP_ALMOST
};

const int IMAGE_RESOURCE_MID_IDS[12] = 
{
	RESOURCE_ID_IMAGE_MID_12, RESOURCE_ID_IMAGE_MID_01, 
	RESOURCE_ID_IMAGE_MID_02, RESOURCE_ID_IMAGE_MID_03, 
	RESOURCE_ID_IMAGE_MID_04, RESOURCE_ID_IMAGE_MID_05, 
	RESOURCE_ID_IMAGE_MID_06, RESOURCE_ID_IMAGE_MID_07,
	RESOURCE_ID_IMAGE_MID_08, RESOURCE_ID_IMAGE_MID_09, 
	RESOURCE_ID_IMAGE_MID_10, RESOURCE_ID_IMAGE_MID_11
};

const int IMAGE_RESOURCE_BOT_IDS[4] = 
{
 	 RESOURCE_ID_IMAGE_BOT_00, RESOURCE_ID_IMAGE_BOT_15,
 	 RESOURCE_ID_IMAGE_BOT_30, RESOURCE_ID_IMAGE_BOT_45
};
