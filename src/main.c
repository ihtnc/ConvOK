#include "main.h"
#include "btmonitor.h"
#include "options.h"

/*
  This watchface has 3 slots: the fuzzy value, the hour value, and the quarter value.
  
  Slot on-screen layout:
     0 - Fuzzy value   - "It is", "A little after", "A bit before", "Almost"
     1 - Hour value    - Text value of the hour
     2 - Quarter value - Text value of the quarter-hour
  
  Each slot animates when changed. 
  The animation runs like this: the previous image in the slot will scroll out to the left of the screen and the current image in the slot will scroll in from the right of the screen to replace the previous image.
  
  Time formula:
  X:00      = It is X o'Clock           X:30      = It is X thirty
  X:01-X:06 = A little after X o'Clock  X:31-X:36 = A little after X thirty
  X:07-X:11 = A bit before X fifteen    X:37-X:41 = A bit before X fortyfive
  X:12-X:14 = Almost X fifteen          X:42-X:44 = Almost X fortyfive
  X:15      = It is  X fifteen          X:45      = It is  X fortyfive
  X:16-X:21 = A little after X fifteen  X:46-X:51 = A little after X fortyfive
  X:22-X:26 = A bit before X thirty     X:52-X:56 = A bit before X+1 o'Clock
  X:27-X:29 = Almost X thirty           X:57-X:59 = Almost X+1 o'Clock
*/
	
void determine_invert_status(struct tm *tick_time)
{
	bool invert;
	
	if(invert_mode == INVERT_ON_AM)
	{
		invert = (tick_time->tm_hour < 12);
	}
	else if(invert_mode == INVERT_ALWAYS)
	{
		invert = true;
	}
	else
	{
		invert = false;
	}
	
	layer_set_frame(inverter_layer_get_layer(inverter), GRect(0, 0, SCREEN_WIDTH, (invert ? SCREEN_HEIGHT : 0)));
	
	#ifdef ENABLE_LOGGING
	if(invert == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "determine_invert_status: inverted");
	else APP_LOG(APP_LOG_LEVEL_DEBUG, "determine_invert_status: not inverted");
	#endif
}

int resource_id_get_from_state(int slot_number)
{
	if(slot_number >= SLOTS_COUNT || slot_number < 0)
	{
		#ifdef ENABLE_LOGGING
		char *error = "resource_id_get_from_state: invalid value; slot_number=XXX";
		snprintf(error, strlen(error), "resource_id_get_from_state: invalid value; slot_number=%d", slot_number);
		APP_LOG(APP_LOG_LEVEL_DEBUG, error);
		#endif

		return RESOURCE_ID_ERROR;
	}
	
	#ifdef ENABLE_LOGGING
	char *output = "resource_id_get_from_state: slot_number=XXX; slot_state=XXX";
	snprintf(output, strlen(output), "resource_id_get_from_state: slot_number=%d; slot_state=%d", slot_number, slots[slot_number].state);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
		
	if(slots[slot_number].state != SLOT_STATE_SPLASH && slots[slot_number].state != SLOT_STATE_EMPTY)
	{
		if (slot_number == SLOT_TOP) { return IMAGE_RESOURCE_TOP_IDS[slots[slot_number].state]; }
		else if (slot_number == SLOT_MID) { return IMAGE_RESOURCE_MID_IDS[slots[slot_number].state]; }
		else if (slot_number == SLOT_BOT) { return IMAGE_RESOURCE_BOT_IDS[slots[slot_number].state]; }
	}
	
	return IMAGE_RESOURCE_SPLASH_IDS[slot_number];
}

int state_determine_value(int slot_number, struct tm *time) 
{
	#ifndef DEBUG
		int hour_value = time->tm_hour % 12;
		int minute_value = time->tm_min;
	#else
		int hour_value = time->tm_min % 12;
		int minute_value = time->tm_sec;
	#endif
		
	if(slot_number >= SLOTS_COUNT || slot_number < 0)
	{
		#ifdef ENABLE_LOGGING
		char *error = "state_determine_value: invalid value; slot_number=XXX";
		snprintf(error, strlen(error), "state_determine_value: invalid value; slot_number=%d", slot_number);
		APP_LOG(APP_LOG_LEVEL_DEBUG, error);
		#endif

		return SLOT_STATE_ERROR;
	}
	
	int state_value = -1;
	int quarter_value = minute_value / 15;
	int quarter_remainder = minute_value % 15;
	
	//Determine the slot value (the index of the correct image resource)
	if (slot_number == SLOT_TOP)
	{
		if (quarter_remainder == 0)
		{
		  	state_value = 0; //Index of "It is"
		}
		else if(quarter_remainder >= 1 && quarter_remainder <= 6)
		{
		 	state_value = 1; //Index of "A little after"
		}
		else if(quarter_remainder >= 7 && quarter_remainder <= 11)
		{
			state_value = 2; //Index of "A bit before"
		}
		else if(quarter_remainder >= 12 && quarter_remainder <= 14)
		{
		  	state_value = 3; //Index of "Almost"
		}
	}
	else if (slot_number == SLOT_MID)
	{
		//After 6 minutes, the fuzzy value will now pertain to the next quarter value.
		//So, if it's currently the third quarter, the hour should also move forward.
		if (quarter_remainder >= 7 && quarter_value == 3) { state_value = hour_value + 1; }
		else { state_value = hour_value; }
	  
		//Normalize the value (should only be 0-11)
		state_value = state_value % 12;
	}
	else if (slot_number == SLOT_BOT) 
	{
		//After 6 minutes, the fuzzy value will now pertain to the next quarter value.
		if (quarter_remainder >= 7) { state_value = quarter_value + 1; }
		else { state_value = quarter_value; }
	
		//Normalize the value (should only be 0-3)
		state_value = state_value % 4; 
	}
	
	#ifdef ENABLE_LOGGING
	char *output = "state_determine_value: slot_number=XXX; state_value=XXX";
	snprintf(output, strlen(output), "state_determine_value: slot_number=%d; state_value=%d", slot_number, state_value);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
	
	return state_value;
}

void slot_deinit(int slot_number)
{
	property_animation_destroy(slots[slot_number].animation);
	
	layer_remove_from_parent(bitmap_layer_get_layer(slots[slot_number].layer));
	bitmap_layer_destroy(slots[slot_number].layer);
	gbitmap_destroy(slots[slot_number].image);
	
	#ifdef ENABLE_LOGGING
	char *output = "slot_deinit: slot_number=XXX";
	snprintf(output, strlen(output), "slot_deinit: slot_number=%d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
}

void slot_animate(int slot_number)
{
	if(slot_number >= SLOTS_COUNT || slot_number < 0)
	{
		#ifdef ENABLE_LOGGING
		char *error1 = "slot_animate: invalid value; slot_number=XXX";
		snprintf(error1, strlen(error1), "slot_animate: invalid value; slot_number=%d", slot_number);
		APP_LOG(APP_LOG_LEVEL_DEBUG, error1);
		#endif

		return;
	}

	if((Animation*)slots[slot_number].animation == NULL)
	{
		#ifdef ENABLE_LOGGING
		char *error3 = "slot_animate: no animation; slot_number=XXX";
		snprintf(error3, strlen(error3), "slot_animate: no animation; slot_number=%d", slot_number);
		APP_LOG(APP_LOG_LEVEL_DEBUG, error3);
		#endif

		return;
	}
	
	animation_schedule((Animation*)slots[slot_number].animation);
	
	#ifdef ENABLE_LOGGING
	char *output = "slot_animate: slot_number=XXX";
	snprintf(output, strlen(output), "slot_animate: slot_number=%d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
}

void slot_animation_out_stopped(Animation *animation, bool finished, void *data)
{
	(void)animation;
	int slot_number = *(int*)data;
	
	#ifdef ENABLE_LOGGING
	char *output = "slot_animation_out_stopped: slot_number=XXX";
	snprintf(output, strlen(output), "slot_animation_out_stopped: slot_number=%d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
		
	slot_deinit(slot_number);
	main_animation_in_init(slot_number);
	slot_animate(slot_number);
}

void slot_animation_out_init(int slot_number)
{
	if(slot_number >= SLOTS_COUNT || slot_number < 0)
	{
		#ifdef ENABLE_LOGGING
		char *error = "slot_animation_out_init: invalid value; slot_number=XXX";
		snprintf(error, strlen(error), "slot_animation_out_init: invalid value; slot_number=%d", slot_number);
		APP_LOG(APP_LOG_LEVEL_DEBUG, error);
		#endif

		return;
	}

	property_animation_destroy(slots[slot_number].animation);

	GRect from_frame;
	GRect to_frame;

	if(slot_number == SLOT_TOP)
	{
		from_frame = GRect(info[SLOT_TOP].offset_x, info[SLOT_TOP].offset_y, SCREEN_WIDTH, info[SLOT_MID].offset_y - info[SLOT_TOP].offset_y);
		to_frame = GRect(info[SLOT_TOP].offset_x - SCREEN_WIDTH, info[SLOT_TOP].offset_y, SCREEN_WIDTH, info[SLOT_MID].offset_y - info[SLOT_TOP].offset_y);
	}
	else if(slot_number == SLOT_MID)
	{
		from_frame = GRect(info[SLOT_MID].offset_x, info[SLOT_MID].offset_y, SCREEN_WIDTH, info[SLOT_BOT].offset_y - info[SLOT_MID].offset_y);
		to_frame = GRect(info[SLOT_MID].offset_x - SCREEN_WIDTH, info[SLOT_MID].offset_y, SCREEN_WIDTH, info[SLOT_BOT].offset_y - info[SLOT_MID].offset_y);
	}
	else if(slot_number == SLOT_BOT)
	{
		from_frame = GRect(info[SLOT_BOT].offset_x, info[SLOT_BOT].offset_y, SCREEN_WIDTH, SCREEN_HEIGHT - info[SLOT_BOT].offset_y);
		to_frame = GRect(info[SLOT_BOT].offset_x - SCREEN_WIDTH, info[SLOT_BOT].offset_y, SCREEN_WIDTH, SCREEN_HEIGHT - info[SLOT_BOT].offset_y);
	}
	
	slots[slot_number].animation_from_frame = from_frame;
	slots[slot_number].animation_to_frame = to_frame;
	slots[slot_number].animation = property_animation_create_layer_frame(bitmap_layer_get_layer(slots[slot_number].layer), &slots[slot_number].animation_from_frame, &slots[slot_number].animation_to_frame);

	animation_set_duration((Animation*)slots[slot_number].animation, info[slot_number].animation_duration_out);
	animation_set_curve((Animation*)slots[slot_number].animation, AnimationCurveEaseInOut);
	animation_set_handlers((Animation*)slots[slot_number].animation,
						   (AnimationHandlers)
						   { .stopped = (AnimationStoppedHandler)slot_animation_out_stopped }, 
						   &slots[slot_number].slot_number);

	#ifdef ENABLE_LOGGING
	char *output = "slot_animation_out_init: slot_number=XXX";
	snprintf(output, strlen(output), "slot_animation_out_init: slot_number=%d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
}

void main_animation_in_stopped(Animation *animation, bool finished, void *data)
{
	(void)animation;
	int slot_number = *(int*)data;
	
	#ifdef ENABLE_LOGGING
	char *output = "main_animation_in_stopped: slot_number=XXX";
	snprintf(output, strlen(output), "main_animation_in_stopped: slot_number=%d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif

	slot_animation_out_init(slot_number);
}

void main_animation_in_init(int slot_number)
{
	if(slot_number >= SLOTS_COUNT || slot_number < 0)
	{
		#ifdef ENABLE_LOGGING
		char *error1 = "main_animation_in_init: invalid value: slot_number=XXX";
		snprintf(error1, strlen(error1), "main_animation_in_init: invalid value; slot_number=%d", slot_number);
		APP_LOG(APP_LOG_LEVEL_DEBUG, error1);
		#endif

		return;
	}
	
	time_t t = time(NULL);
	struct tm *local = localtime(&t);
	int state = state_determine_value(slot_number, local);
	
	if(state == SLOT_STATE_ERROR)
	{
		#ifdef ENABLE_LOGGING
		char *error2 = "main_animation_in_init: invalid state; slot_number=XXX";
		snprintf(error2, strlen(error2), "main_animation_in_init: invalid state; slot_number=%d", slot_number);
		APP_LOG(APP_LOG_LEVEL_DEBUG, error2);
		#endif
			
		return;
	}
	
	if(slots[slot_number].state == state)
	{
		#ifdef ENABLE_LOGGING
		char *info = "main_animation_in_init: state unchanged; slot_number=XXX";
		snprintf(info, strlen(info), "main_animation_in_init: state unchanged; slot_number=%d", slot_number);
		APP_LOG(APP_LOG_LEVEL_DEBUG, info);
		#endif
			
		return;
	}
	
	slots[slot_number].state = state;
	slots[slot_number].image = gbitmap_create_with_resource(resource_id_get_from_state(slot_number));
	
	GRect from_frame;
	GRect to_frame;

	if(slot_number == SLOT_TOP)
	{
		from_frame = GRect(info[SLOT_TOP].offset_x + SCREEN_WIDTH, info[SLOT_TOP].offset_y, SCREEN_WIDTH, info[SLOT_MID].offset_y - info[SLOT_TOP].offset_y);
		to_frame = GRect(info[SLOT_TOP].offset_x, info[SLOT_TOP].offset_y, SCREEN_WIDTH, info[SLOT_MID].offset_y - info[SLOT_TOP].offset_y);      	
	}
	else if(slot_number == SLOT_MID)
	{
		from_frame = GRect(info[SLOT_MID].offset_x + SCREEN_WIDTH, info[SLOT_MID].offset_y, SCREEN_WIDTH, info[SLOT_BOT].offset_y - info[SLOT_MID].offset_y);	
		to_frame = GRect(info[SLOT_MID].offset_x, info[SLOT_MID].offset_y, SCREEN_WIDTH, info[SLOT_BOT].offset_y - info[SLOT_MID].offset_y);		
	}
	else if(slot_number == SLOT_BOT)
	{
		from_frame = GRect(info[SLOT_BOT].offset_x + SCREEN_WIDTH, info[SLOT_BOT].offset_y, SCREEN_WIDTH, SCREEN_HEIGHT - info[SLOT_BOT].offset_y);
		to_frame = GRect(info[SLOT_BOT].offset_x, info[SLOT_BOT].offset_y, SCREEN_WIDTH, SCREEN_HEIGHT - info[SLOT_BOT].offset_y);
	}

	slots[slot_number].layer = bitmap_layer_create(from_frame);	
	
	bitmap_layer_set_bitmap(slots[slot_number].layer, slots[slot_number].image);
	layer_insert_below_sibling(bitmap_layer_get_layer(slots[slot_number].layer), inverter_layer_get_layer(inverter));

	slots[slot_number].animation_from_frame = from_frame;
	slots[slot_number].animation_to_frame = to_frame;
	slots[slot_number].animation = property_animation_create_layer_frame(bitmap_layer_get_layer(slots[slot_number].layer), &slots[slot_number].animation_from_frame, &slots[slot_number].animation_to_frame);

	animation_set_duration((Animation*)slots[slot_number].animation, info[slot_number].animation_duration_in);
	animation_set_curve((Animation*)slots[slot_number].animation, AnimationCurveEaseInOut);
	animation_set_handlers((Animation*)slots[slot_number].animation,
						   (AnimationHandlers)
						   { .stopped = (AnimationStoppedHandler)main_animation_in_stopped }, 
						   &slots[slot_number].slot_number);

	#ifdef ENABLE_LOGGING
	char *output = "main_animation_in_init: slot_number=XXX";
	snprintf(output, strlen(output), "main_animation_in_init: slot_number=%d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
}

void splash_animation_in_stopped(Animation *animation, bool finished, void *data)
{
	(void)animation;
	int slot_number = *(int*)data;
	
	#ifdef ENABLE_LOGGING
	char *output = "splash_animation_in_stopped: slot_number=XXX";
	snprintf(output, strlen(output), "splash_animation_in_stopped: slot_number=%d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif

	slot_animation_out_init(slot_number);
	slot_animate(slot_number);
}

void splash_animation_in_init(int slot_number) 
{
	if(slot_number >= SLOTS_COUNT || slot_number < 0)
	{
		#ifdef ENABLE_LOGGING
		char *error = "splash_animation_in_init: invalid value; slot_number=XXX";
		snprintf(error, strlen(error), "splash_animation_in_init: invalid value; slot_number=%d", slot_number);
		APP_LOG(APP_LOG_LEVEL_DEBUG, error);
		#endif

		return;
	}

	slots[slot_number].slot_number = slot_number;
	slots[slot_number].state = SLOT_STATE_SPLASH;
	slots[slot_number].image = gbitmap_create_with_resource(resource_id_get_from_state(slot_number));
	
	GRect from_frame;
	GRect to_frame;
	
	if(slot_number == SLOT_TOP)
	{
		from_frame = GRect(info[SLOT_TOP].offset_x + info[SLOT_TOP].offset_splash_x, info[SLOT_TOP].offset_splash_y, SCREEN_WIDTH, info[SLOT_MID].offset_y - info[SLOT_TOP].offset_y);
		to_frame = GRect(info[SLOT_TOP].offset_x, info[SLOT_TOP].offset_y, SCREEN_WIDTH, info[SLOT_MID].offset_y - info[SLOT_TOP].offset_y);
	}
	else if(slot_number == SLOT_MID)
	{
		from_frame = GRect(info[SLOT_MID].offset_x - info[SLOT_MID].offset_splash_x, info[SLOT_MID].offset_splash_y, SCREEN_WIDTH, info[SLOT_BOT].offset_y - info[SLOT_MID].offset_y);
		to_frame = GRect(info[SLOT_MID].offset_x, info[SLOT_MID].offset_y, SCREEN_WIDTH, info[SLOT_BOT].offset_y - info[SLOT_MID].offset_y);	
	}
	else if(slot_number == SLOT_BOT)
	{
		from_frame = GRect(info[SLOT_BOT].offset_x + info[SLOT_BOT].offset_splash_x, info[SLOT_BOT].offset_splash_y, SCREEN_WIDTH, SCREEN_HEIGHT - info[SLOT_BOT].offset_y);
		to_frame = GRect(info[SLOT_BOT].offset_x, info[SLOT_BOT].offset_y, SCREEN_WIDTH, SCREEN_HEIGHT - info[SLOT_BOT].offset_y);
	}

	slots[slot_number].layer = bitmap_layer_create(from_frame);	
	
	bitmap_layer_set_bitmap(slots[slot_number].layer, slots[slot_number].image);
	layer_insert_below_sibling(bitmap_layer_get_layer(slots[slot_number].layer), inverter_layer_get_layer(inverter));

	slots[slot_number].animation_from_frame = from_frame;
	slots[slot_number].animation_to_frame = to_frame;
	slots[slot_number].animation = property_animation_create_layer_frame(bitmap_layer_get_layer(slots[slot_number].layer), &slots[slot_number].animation_from_frame, &slots[slot_number].animation_to_frame);
	animation_set_duration((Animation*)slots[slot_number].animation, info[slot_number].animation_duration_splash);
	animation_set_curve((Animation*)slots[slot_number].animation, AnimationCurveEaseInOut);
	animation_set_handlers((Animation*)slots[slot_number].animation,
						   (AnimationHandlers)
						   { .stopped = (AnimationStoppedHandler)splash_animation_in_stopped }, 
						   &slots[slot_number].slot_number);

	#ifdef ENABLE_LOGGING
	char *output = "splash_animation_in_init: slot_number=XXX";
	snprintf(output, strlen(output), "splash_animation_in_init: slot_number=%d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
}

void inverter_deinit()
{
	layer_remove_from_parent(inverter_layer_get_layer(inverter));
	inverter_layer_destroy(inverter);
							 
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter_deinit: done");
	#endif
}

void inverter_init()
{
	invert_mode = INVERT_ON_AM;
	inverter = inverter_layer_create(GRect(0, 0, SCREEN_WIDTH, 0));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inverter));
	
	#ifdef ENABLE_LOGGING
	if(invert_mode == INVERT_ON_AM) APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter_init: INVERT_ON_AM");
	else if(invert_mode == INVERT_ALWAYS) APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter_init: INVERT_ALWAYS");
	else if(invert_mode == INVERT_NEVER) APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter_init: INVERT_NEVER");
	else APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter_init: invalid invert_mode; default=INVERT_NEVER");
	#endif
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) 
{
	#ifdef ENABLE_LOGGING
	char *output = "handle_tick: MM/dd/yyyy hh:mm:ss";
	strftime(output, strlen(output), "handle_tick: %D %T", tick_time);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif

	if(slots[SLOT_TOP].state == SLOT_STATE_SPLASH
	  || slots[SLOT_MID].state == SLOT_STATE_SPLASH
	  || slots[SLOT_BOT].state == SLOT_STATE_SPLASH)
	{
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_tick: ticked while showing splash");
		#endif
		return;
	}
		
	slot_animation_out_init(SLOT_TOP);
	slot_animation_out_init(SLOT_MID);
	slot_animation_out_init(SLOT_BOT);
	
	slot_animate(SLOT_TOP);
	slot_animate(SLOT_MID);
	slot_animate(SLOT_BOT);
}

void handle_deinit() 
{
	//btmonitor_deinit();
	inverter_deinit();

	tick_timer_service_unsubscribe();
	
	slot_deinit(SLOT_TOP);
	slot_deinit(SLOT_MID);
	slot_deinit(SLOT_BOT);
	
	window_destroy(window);
	
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_deinit: done");
	#endif
}

void handle_init()
{
	window = window_create();
	window_stack_push(window, true);	
	window_set_background_color(window, GColorBlack);

	#ifndef DEBUG
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_timer_service_subscribe: MINUTE_UNIT");
		#endif
	#else
		tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
		#ifdef ENABLE_LOGGING
			APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_timer_service_subscribe: SECOND_UNIT");
		#endif
	#endif
	
	//btmonitor_init(true);
	inverter_init();

	time_t t = time(NULL);
	struct tm *local = localtime(&t);
	determine_invert_status(local);
	
	splash_animation_in_init(SLOT_TOP);
	splash_animation_in_init(SLOT_MID);
	splash_animation_in_init(SLOT_BOT);
	
	slot_animate(SLOT_TOP);
	slot_animate(SLOT_MID);
	slot_animate(SLOT_BOT);

	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_init: done");
	#endif
}

int main(void) 
{
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "main: start");
	#endif
		
	handle_init();
	app_event_loop();
	handle_deinit();
}
