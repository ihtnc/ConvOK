#include "pebble.h"
	
#define SLOTS_COUNT 3
#define SLOT_STATUS_EMPTY -1
#define SLOT_SPLASH -2
	
#define SLOT_TOP 0
#define SLOT_MID 1
#define SLOT_BOT  2

#define SLOT_XOFFSET 0
#define SLOT_TOP_YOFFSET 0
#define SLOT_MID_YOFFSET 30
#define SLOT_BOT_YOFFSET 104

#define SLOT_TOP_SPLASH_XOFFSET 10
#define SLOT_MID_SPLASH_XOFFSET 20
#define SLOT_BOT_SPLASH_XOFFSET 0
	
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

#define SLOT_TOP_SPLASH_DURATION 1200
#define SLOT_MID_SPLASH_DURATION 1200
#define SLOT_BOT_SPLASH_DURATION 1200

#define SCREEN_HEIGHT 168
#define SCREEN_WIDTH 144
	
Window *window;
InverterLayer *inverter;
bool show_splash;

GBitmap *images[SLOTS_COUNT];
GBitmap *previmages[SLOTS_COUNT];
GBitmap *splashimages[SLOTS_COUNT];

//These 2 image containers are needed for animation (1 for the current image and 1 for the previous image).
BitmapLayer *image_containers[SLOTS_COUNT];
BitmapLayer *previmage_containers[SLOTS_COUNT];
BitmapLayer *splash_containers[SLOTS_COUNT]; //Image containers for the splash screen

GRect slot_rectangles[SLOTS_COUNT];        //Current frame
GRect slot_out_rectangles[SLOTS_COUNT];    //Target frame of the scroll out animation
GRect slot_in_rectangles[SLOTS_COUNT];     //Source frame of the scroll in animation
GRect slot_splash_rectangles[SLOTS_COUNT]; //Source frame of the splash animation

//These 2 are used to animate the images (the current image is used on the in animation and the previous image is used in the out animation)
PropertyAnimation *slot_out_animations[SLOTS_COUNT];
PropertyAnimation *slot_in_animations[SLOTS_COUNT];

PropertyAnimation *slot_splash_animations[SLOTS_COUNT]; //Used to animate the splash screen

//Animation timings
const int SLOT_OUT_ANIMATION_DURATIONS[SLOTS_COUNT] = {SLOT_TOP_OUT_DURATION, SLOT_MID_OUT_DURATION, SLOT_BOT_OUT_DURATION};
const int SLOT_OUT_ANIMATION_DELAYS[SLOTS_COUNT] = {SLOT_TOP_OUT_DELAY, SLOT_MID_OUT_DELAY, SLOT_BOT_OUT_DELAY};
const int SLOT_IN_ANIMATION_DURATIONS[SLOTS_COUNT] = {SLOT_TOP_IN_DURATION, SLOT_MID_IN_DURATION, SLOT_BOT_IN_DURATION};
const int SLOT_IN_ANIMATION_DELAYS[SLOTS_COUNT] = {SLOT_TOP_IN_DELAY, SLOT_MID_IN_DELAY, SLOT_BOT_IN_DELAY};
const int SLOT_SPLASH_ANIMATION_DURATIONS[SLOTS_COUNT] = {SLOT_TOP_SPLASH_DURATION, SLOT_MID_SPLASH_DURATION, SLOT_BOT_SPLASH_DURATION};

//Position of the slots along the Y-axis
const int SLOT_YOFFSETS[SLOTS_COUNT] = {SLOT_TOP_YOFFSET, SLOT_MID_YOFFSET, SLOT_BOT_YOFFSET};

//The state of the slot can either be "empty" or the image currently in the slot.
//This is to prevent the slot from unnecessarily reloading the image.
int image_slot_state[SLOTS_COUNT] = {SLOT_STATUS_EMPTY, SLOT_STATUS_EMPTY, SLOT_STATUS_EMPTY};
int previmage_slot_state[SLOTS_COUNT] = {SLOT_STATUS_EMPTY, SLOT_STATUS_EMPTY, SLOT_STATUS_EMPTY};

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
