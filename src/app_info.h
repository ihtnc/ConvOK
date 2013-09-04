#include "app_options.h"

#define APP_VER_MAJOR 1	
#define APP_VER_MINOR 7

#ifndef DEBUG
	#ifdef PHONE_HAS_HTTPPEBBLE
		#ifdef ANDROID
			#if INVERT_MODE == INVERT_ON_AM
				#define APP_NAME "Orange Kid [AM]"
			#elif INVERT_MODE == INVERT_ALWAYS
				#define APP_NAME "Orange Kid [AV]"
			#else
				#define APP_NAME "Orange Kid [A]"
			#endif
		#else
			#if INVERT_MODE == INVERT_ON_AM
				#define APP_NAME "Orange Kid [IM]"
			#elif INVERT_MODE == INVERT_ALWAYS
				#define APP_NAME "Orange Kid [IV]"
			#else
				#define APP_NAME "Orange Kid [I]"
			#endif
		#endif
	#else
		#if INVERT_MODE == INVERT_ON_AM
			#define APP_NAME "Orange Kid [M]"
		#elif INVERT_MODE == INVERT_ALWAYS
			#define APP_NAME "Orange Kid [V]"
		#else
			#define APP_NAME "Orange Kid"
		#endif
	#endif
#else
	#ifdef PHONE_HAS_HTTPPEBBLE
		#ifdef ANDROID
			#if INVERT_MODE == INVERT_ON_AM
				#define APP_NAME "Debug: Orange Kid [AM]"
			#elif INVERT_MODE == INVERT_ALWAYS
				#define APP_NAME "Debug: Orange Kid [AV]"
			#else
				#define APP_NAME "Debug: OrangeKid [A]"
			#endif
		#else
			#if INVERT_MODE == INVERT_ON_AM
				#define APP_NAME "Debug: Orange Kid [IM]"
			#elif INVERT_MODE == INVERT_ALWAYS
				#define APP_NAME "Debug: Orange Kid [IV]"
			#else
				#define APP_NAME "Debug: OrangeKid [I]"
			#endif
		#endif
	#else
		#if INVERT_MODE == INVERT_ON_AM
			#define APP_NAME "Debug: Orange Kid [M]"
		#elif INVERT_MODE == INVERT_ALWAYS
			#define APP_NAME "Debug: Orange Kid [V]"
		#else
			#define APP_NAME "Debug: OrangeKid"
		#endif
	#endif
#endif

#define APP_AUTHOR "ihopethisnamecounts"