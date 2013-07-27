#include "app_options.h"

#define APP_VER_MAJOR 1	
#define APP_VER_MINOR 6

#ifndef DEBUG
	#ifdef PHONE_HAS_HTTPPEBBLE
		#ifdef ANDROID
			#define APP_NAME "Orange Kid [A]"
		#else
			#define APP_NAME "Orange Kid [I]"
		#endif
	#else
		#define APP_NAME "Orange Kid"
	#endif
#else
	#ifdef PHONE_HAS_HTTPPEBBLE
		#ifdef ANDROID
			#define APP_NAME "Debug: OrangeKid [A]"
		#else
			#define APP_NAME "Debug: OrangeKid [I]"
		#endif
	#else
		#define APP_NAME "Debug: OrangeKid"
	#endif
#endif

#define APP_AUTHOR "ihopethisnamecounts"