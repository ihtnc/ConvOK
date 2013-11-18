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

void app_timer_callback()
{
	vibe_index++;
	is_connected = bluetooth_connection_service_peek();
	
	if(is_connected == false)
	{
		//if still disconnected, the vibrate frequency gets reduced over time, until it no longer vibrates 
		if(vibe_index > vibe_freq_count) 
		{
			if(needs_vibrate) vibes_enqueue_custom_pattern(vibes_disconnect_pattern);
			
			timer = app_timer_register(vibe_freq[vibe_index] * 60 * 1000, app_timer_callback, NULL);
		}
		
		if(btcallbacks.ping) btcallbacks.ping();
	}
}	
	
void bluetooth_connection_callback(bool connected)
{
	is_connected = connected;
	vibe_index = 0;
	
	if(connected == false)
	{
		if(needs_vibrate) vibes_enqueue_custom_pattern(vibes_disconnect_pattern);
		
		timer = app_timer_register(vibe_freq[vibe_index] * 60 * 1000, 
								   (AppTimerCallback)
								   {
									   app_timer_callback
								   }, 
								   NULL);
	}
	else
	{
		if(needs_vibrate) vibes_enqueue_custom_pattern(vibes_connect_pattern);
	}
	
	if(btcallbacks.status_changed) btcallbacks.status_changed(is_connected);
}

void btmonitor_subscribe(BTMonitorCallbacks callbacks)
{
	btcallbacks = callbacks;
}

void btmonitor_init(bool enable_vibrate) 
{
	needs_vibrate = enable_vibrate;
	bluetooth_connection_callback(bluetooth_connection_service_peek());	
	bluetooth_connection_service_subscribe(bluetooth_connection_callback);
}

void btmonitor_deinit() 
{
	bluetooth_connection_service_unsubscribe();
}