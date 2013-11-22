#include "btmonitor.h"
#include "options.h"

static bool needs_vibrate;
static bool is_connected;
static uint8_t vibe_freq_count = 13;
static uint8_t vibe_freq[] = {1, 1, 1, 1, 5, 5, 5, 5, 5, 30, 30, 30, 60};
static uint8_t vibe_index = 0;
static AppTimer *timer;
static BTMonitorCallbacks btcallbacks;
	
const VibePattern vibes_disconnect_pattern = 
{
	.durations = (uint32_t []) {200, 100, 200, 100, 400, 200},
	.num_segments = 6
};

const VibePattern vibes_connect_pattern = 
{
	.durations = (uint32_t []) {100, 100, 100, 100},
	.num_segments = 4
};

static void app_timer_callback(void *context)
{
	vibe_index++;
	is_connected = bluetooth_connection_service_peek();
	
	#ifdef ENABLE_LOGGING
	if(is_connected) APP_LOG(APP_LOG_LEVEL_DEBUG, "app_timer_callback: is_connected=true");
	else APP_LOG(APP_LOG_LEVEL_DEBUG, "app_timer_callback: is_connected=false");
	#endif
	
	if(is_connected == false)
	{
		//if still disconnected, the vibrate frequency gets reduced over time, until it no longer vibrates 
		if(vibe_index < vibe_freq_count) 
		{
			if(needs_vibrate == true)
			{
				#ifdef ENABLE_LOGGING
				APP_LOG(APP_LOG_LEVEL_DEBUG, "app_timer_callback: disconnect vibrate pattern");
				#endif
				
				vibes_enqueue_custom_pattern(vibes_disconnect_pattern);
			}
			
			#ifdef ENABLE_LOGGING
			char *output = "app_timer_callback: callback on=XXX minute(s)";
			snprintf(output, strlen(output), "app_timer_callback: callback on=%d minute(s)", vibe_freq[vibe_index]);
			APP_LOG(APP_LOG_LEVEL_DEBUG, output);
			#endif
				
			timer = app_timer_register(vibe_freq[vibe_index] * 60 * 1000, app_timer_callback, NULL);
		}
		
		if(btcallbacks.ping)
		{
			#ifdef ENABLE_LOGGING
			APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth_connection_callback: run ping callback");
			#endif
			
			btcallbacks.ping();
		}
	}
}	
	
static void bluetooth_connection_callback(bool connected)
{
	is_connected = connected;
	vibe_index = 0;
	
	#ifdef ENABLE_LOGGING
	if(connected) APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth_connection_callback: connected=true");
	else APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth_connection_callback: connected=false");
	#endif
		
	if(connected == false)
	{
		if(needs_vibrate == true) 
		{
			#ifdef ENABLE_LOGGING
			APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth_connection_callback: disconnect vibrate pattern");
			#endif
			
			vibes_enqueue_custom_pattern(vibes_disconnect_pattern);
		}
		
		#ifdef ENABLE_LOGGING
		char *output = "bluetooth_connection_callback: callback on=XXX minute(s)";
		snprintf(output, strlen(output), "bluetooth_connection_callback: callback on=%d minute(s)", vibe_freq[vibe_index]);
		APP_LOG(APP_LOG_LEVEL_DEBUG, output);
		#endif
			
		timer = app_timer_register(vibe_freq[vibe_index] * 60 * 1000, 
								   (AppTimerCallback)
								   {
									   app_timer_callback
								   }, 
								   NULL);
	}
	else
	{
		if(needs_vibrate == true) 
		{
			#ifdef ENABLE_LOGGING
			APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth_connection_callback: connect vibrate pattern");
			#endif
				
			vibes_enqueue_custom_pattern(vibes_connect_pattern);
		}
	}
	
	if(needs_vibrate == false)
	{
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth_connection_callback: will not vibrate");
		#endif
	}
	
	if(btcallbacks.status_changed) 
	{
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth_connection_callback: run status_changed callback");
		#endif
		
		btcallbacks.status_changed(is_connected);
	}
}

void btmonitor_unsubscribe()
{
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "btmonitor_unsubscribe: done");
	#endif
}

void btmonitor_subscribe(BTMonitorCallbacks callbacks)
{
	btcallbacks = callbacks;
	
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "btmonitor_subscribe: done");
	#endif
}

void btmonitor_init(bool enable_vibrate) 
{
	needs_vibrate = enable_vibrate;
	
	#ifdef ENABLE_LOGGING
	if(needs_vibrate == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "btmonitor_init: enable_vibrate=true");
	else APP_LOG(APP_LOG_LEVEL_DEBUG, "btmonitor_init: enable_vibrate=false");
	#endif
	
	bluetooth_connection_callback(bluetooth_connection_service_peek());	
	bluetooth_connection_service_subscribe(bluetooth_connection_callback);
}

void btmonitor_deinit() 
{
	bluetooth_connection_service_unsubscribe();
	free(timer);	
	
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "btmonitor_deinit: done");
	#endif
}
