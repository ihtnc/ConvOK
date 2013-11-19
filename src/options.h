//Debug mode treats the watch's minute value as the hour value and the watch's second value as the minute value.
//This is to speed up the transition of the animations
//#define DEBUG
#define ENABLE_LOGGING

//#define ANDROID

//If INVERT_MODE is INVERT_ALWAYS, then the color will be inverted (white on black text)
//If INVERT_MODE is INVERT_IN_AM, then the color will only be inverted from 12:00AM to 11:59AM
//Otherwise the color will not be inverted.
#define INVERT_NEVER 0
#define INVERT_ON_AM 1
#define INVERT_ALWAYS 2
