//Debug mode treats the watch's minute value as the hour value and the watch's second value as the minute value.
//This is to speed up the transition of the animations
//#define DEBUG

//#define PHONE_HAS_HTTPPEBBLE
//#define ANDROID

#define SLOT_XOFFSET 0
#define SLOT_TOP_YOFFSET 0
#define SLOT_MID_YOFFSET 30
#define SLOT_BOT_YOFFSET 104

#define SLOT_TOP_SPLASH_XOFFSET 10
#define SLOT_MID_SPLASH_XOFFSET 20
#define SLOT_BOT_SPLASH_XOFFSET 0

//If INVERT_MODE is INVERT_ALWAYS, then the color will be inverted (white on black text)
//If INVERT_MODE is INVERT_IN_AM, then the color will only be inverted from 12:00AM to 11:59AM
//Otherwise the color will not be inverted.
#define INVERT_NEVER 0
#define INVERT_ON_AM 1
#define INVERT_ALWAYS 2
#define INVERT_MODE INVERT_NEVER

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