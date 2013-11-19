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

/*
int determine_image_from_value(int slot_number, int slot_value)
{
	//If the value is invalid, just show the splash image
	//This is applicable only when the watchface initially loads
	if (slot_number == SLOT_TOP)
	{
		if(slot_value != SLOT_SPLASH && slot_value != SLOT_STATUS_EMPTY) { return IMAGE_RESOURCE_TOP_IDS[slot_value]; }
		else { return RESOURCE_ID_IMAGE_TOP_SPLASH; }
	}
	else if (slot_number == SLOT_MID)
	{
		if(slot_value != SLOT_SPLASH && slot_value != SLOT_STATUS_EMPTY) { return IMAGE_RESOURCE_MID_IDS[slot_value]; }
		else { return RESOURCE_ID_IMAGE_MID_SPLASH; }
	}
	else if (slot_number == SLOT_BOT) 
	{
		if(slot_value != SLOT_SPLASH && slot_value != SLOT_STATUS_EMPTY) { return IMAGE_RESOURCE_BOT_IDS[slot_value]; }
		else { return RESOURCE_ID_IMAGE_BOT_SPLASH; }
	}
	else 
	{
		return -1;
	}
}

void slot_out_animation_stopped(Animation *animation, void *data)
{
	(void)animation;
	(void)data;
}

void slot_in_animation_stopped(Animation *animation, void *data)
{
	(void)animation;
	(void)data;
}

void animate_main(int slot_number)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "animate_main start...");
	//Do not run the animation if there are no images.
	if(image_slot_state[slot_number] == SLOT_STATUS_EMPTY) { return; }
	animation_schedule(&slot_in_animations[slot_number]->animation);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "animate_main end.");
}

void main_init(int slot_number)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "main_init start...");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "out animation init start...");
	Layer *prev_layer = bitmap_layer_get_layer(previmage_containers[slot_number]);
	slot_out_animations[slot_number] = 
		property_animation_create_layer_frame(prev_layer,
											  &slot_rectangles[slot_number], 
											  &slot_out_rectangles[slot_number]);
	animation_set_duration(&slot_out_animations[slot_number]->animation, 
						   SLOT_OUT_ANIMATION_DURATIONS[slot_number]);
	animation_set_curve(&slot_out_animations[slot_number]->animation,
						AnimationCurveEaseIn);
	animation_set_handlers(&slot_out_animations[slot_number]->animation,
						   (AnimationHandlers)
						   {
							   .stopped = (AnimationStoppedHandler)slot_out_animation_stopped
						   },
						   NULL);
	animation_set_delay(&slot_out_animations[slot_number]->animation, 
						SLOT_OUT_ANIMATION_DELAYS[slot_number]);
	animation_schedule(&slot_out_animations[slot_number]->animation);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "out animation init end.");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "in animation init start...");
	Layer *in_layer = bitmap_layer_get_layer(image_containers[slot_number]);
	slot_in_animations[slot_number] = 
		property_animation_create_layer_frame(in_layer,
											  &slot_in_rectangles[slot_number], 
											  &slot_rectangles[slot_number]);
	  
	animation_set_duration(&slot_in_animations[slot_number]->animation, 
						   SLOT_IN_ANIMATION_DURATIONS[slot_number]);
	animation_set_curve(&slot_in_animations[slot_number]->animation, 
						AnimationCurveEaseOut);
	animation_set_handlers(&slot_in_animations[slot_number]->animation,
						   (AnimationHandlers)
						   {
							   .stopped = (AnimationStoppedHandler)slot_in_animation_stopped
						   },
						   NULL);
	animation_set_delay(&slot_in_animations[slot_number]->animation,
						SLOT_IN_ANIMATION_DELAYS[slot_number]);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "in animation init end.");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "main_init end.");
}

void main_deinit(int slot_number)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "main_deinit start...");
	animation_destroy(&slot_in_animations[slot_number]->animation);
	property_animation_destroy(slot_in_animations[slot_number]);
	animation_destroy(&slot_out_animations[slot_number]->animation);
	property_animation_destroy(slot_out_animations[slot_number]);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "main_deinit end.");
}

void unload_image_from_slot(int slot_number) 
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "unload_image_from_slot start...");
	//Removes the images from the display and unloads the resource to free up RAM.
	//Can handle being called on an already empty slot.
	
	if (image_slot_state[slot_number] != SLOT_STATUS_EMPTY && image_slot_state[slot_number] != SLOT_SPLASH) 
	{
		Layer *prv_layer = bitmap_layer_get_layer(previmage_containers[slot_number]);
		layer_remove_from_parent(prv_layer);
		bitmap_layer_destroy(previmage_containers[slot_number]);
		gbitmap_destroy(images[slot_number]);

		Layer *main_layer = bitmap_layer_get_layer(image_containers[slot_number]);
		layer_remove_from_parent(main_layer);
		bitmap_layer_destroy(image_containers[slot_number]);
		gbitmap_destroy(previmages[slot_number]);
	
		image_slot_state[slot_number] = SLOT_STATUS_EMPTY;
	}
	else if (image_slot_state[slot_number] != SLOT_STATUS_EMPTY && image_slot_state[slot_number] == SLOT_SPLASH) 
	{
		Layer *spl_layer = bitmap_layer_get_layer(splash_containers[slot_number]);
		layer_remove_from_parent(spl_layer);
		bitmap_layer_destroy(splash_containers[slot_number]);
		gbitmap_destroy(splashimages[slot_number]);
	}

	main_deinit(slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "unload_image_from_slot end.");
}

void load_image_to_slot(int slot_number, int hour_value, int minute_value) 
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "load_image_to_slot start...");
	//Loads the digit image from the application's resources and displays it on-screen in the correct location.
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "validation start...");
	//Validations
	if (slot_number < 0 || slot_number > SLOTS_COUNT) { return; }
	if (hour_value < 0 || hour_value > 11) { return; }
	if (minute_value < 0 || minute_value > 59) { return; }
	APP_LOG(APP_LOG_LEVEL_DEBUG, "validation end.");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "determine image start...");
	int slot_value = -1;
	int quarter_value = minute_value / 15;
	int quarter_remainder = minute_value % 15;
	
	//Determine the slot value (the index of the correct image resource)
	if (slot_number == SLOT_TOP)
	{
		if (quarter_remainder == 0)
		{
		  	slot_value = 0; //Index of "It is"
		}
		else if(quarter_remainder >= 1 && quarter_remainder <= 6)
		{
		 	 slot_value = 1; //Index of "A little after"
		}
		else if(quarter_remainder >= 7 && quarter_remainder <= 11)
		{
		 	 slot_value = 2; //Index of "A bit before"
		}
		else if(quarter_remainder >= 12 && quarter_remainder <= 14)
		{
		  	slot_value = 3; //Index of "Almost"
		}
	}
	else if (slot_number == SLOT_MID)
	{
		//After 6 seconds, the fuzzy value will now pertain to the next quarter value.
		//So, if it's currently the third quarter, the hour should also move forward.
		if (quarter_remainder >= 7 && quarter_value == 3) { slot_value = hour_value + 1; }
		else { slot_value = hour_value; }
	  
		//Normalize the value (should only be 0-11)
		slot_value = slot_value % 12;
	}
	else if (slot_number == SLOT_BOT) 
	{
			//After 6 seconds, the fuzzy value will now pertain to the next quarter value.
		if (quarter_remainder >= 7) { slot_value = quarter_value + 1; }
		else { slot_value = quarter_value; }
	
		//Normalize the value (should only be 0-3)
		slot_value = slot_value % 4; 
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG, "determine image end.");

	//Do not reload the image if slot_state value did not change
	if (image_slot_state[slot_number] == slot_value) { return; }
	
	//The state is about to change so the current state will become the previous state
	int prevslot_state = image_slot_state[slot_number];
	
	unload_image_from_slot(slot_number);
	
	Layer *inv_layer = inverter_layer_get_layer(inverter);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "create gbitmap start...");
	//Load the image based on the current state
	int resourceid = determine_image_from_value(slot_number, slot_value);
	images[slot_number] = gbitmap_create_with_resource(resourceid);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "create gbitmap end.");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "create bitmaplayer start...");
	image_containers[slot_number] = bitmap_layer_create(slot_rectangles[slot_number]);
	bitmap_layer_set_bitmap(image_containers[slot_number], images[slot_number]);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "create bitmaplayer end.");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "add to window start...");
	Layer *bmp_layer = bitmap_layer_get_layer(image_containers[slot_number]);
	GRect bmp_frame = layer_get_frame(bmp_layer);
	bmp_frame.origin.x = SLOT_XOFFSET + SCREEN_WIDTH;
	bmp_frame.origin.y = SLOT_YOFFSETS[slot_number];
	layer_set_frame(bmp_layer, bmp_frame);
	layer_insert_below_sibling(bmp_layer, inv_layer);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "add to window end.");
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "create prev gbitmap start...");
	//Load the image based on the previous state
	int prevresourceid = determine_image_from_value(slot_number, prevslot_state);
	previmages[slot_number] = gbitmap_create_with_resource(prevresourceid);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "create prev gbitmap end.");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "create prev bitmaplayer start...");
	previmage_containers[slot_number] = bitmap_layer_create(slot_rectangles[slot_number]);
	bitmap_layer_set_bitmap(previmage_containers[slot_number], previmages[slot_number]);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "create prev bitmaplayer end.");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "add to window start...");
	Layer *prevbmp_layer = bitmap_layer_get_layer(image_containers[slot_number]);
	GRect prevbmp_frame = layer_get_frame(prevbmp_layer);
	prevbmp_frame.origin.x = SLOT_XOFFSET;
	prevbmp_frame.origin.y =  SLOT_YOFFSETS[slot_number];
	layer_set_frame(prevbmp_layer, prevbmp_frame);
	layer_insert_below_sibling(prevbmp_layer, inv_layer);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "add to window end.");

	image_slot_state[slot_number] = slot_value;
	main_init(slot_number);
	animate_main(slot_number);
}

void display_time(struct tm *tick_time) 
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "display_time start...");
	determine_invert_status(tick_time);
		
	if(show_splash == true) { return; }

	#ifndef DEBUG
		int normalized_hour = tick_time->tm_hour % 12;
		int normalized_minute = tick_time->tm_min;
	#else
		int normalized_hour = tick_time->tm_min % 12;
		int normalized_minute = tick_time->tm_sec;
	#endif
	
	load_image_to_slot(SLOT_TOP, normalized_hour, normalized_minute);
	load_image_to_slot(SLOT_MID, normalized_hour, normalized_minute);
	load_image_to_slot(SLOT_BOT, normalized_hour, normalized_minute);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "display_time end.");
}
*/


void splash_deinit(int slot_number)
{
	property_animation_destroy(slots[slot_number].animation);
	
	layer_remove_from_parent(bitmap_layer_get_layer(slots[slot_number].layer));
	bitmap_layer_destroy(slots[slot_number].layer);
	gbitmap_destroy(slots[slot_number].image);
	
	#ifdef ENABLE_LOGGING
	char *output = "splash_deinit: XXX";
	snprintf(output, strlen(output), "splash_deinit: %d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
}

void slot_splash_animation_out_stopped(Animation *animation, bool finished, void *data)
{
	(void)animation;
	int slot_number = *(int*)data;
	
	#ifdef ENABLE_LOGGING
	char *output = "slot_splash_animation_out_stopped: XXX";
	snprintf(output, strlen(output), "slot_splash_animation_out_stopped: %d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
		
	splash_deinit(slot_number);
}

void splash_animation_out_init(int slot_number)
{
	if(slot_number >= SLOTS_COUNT)
	{
		#ifdef ENABLE_LOGGING
		char *error = "splash_animation_out_init: invalid value; slot_number=XXX";
		snprintf(error, strlen(error), "splash_animation_out_init: invalid value; slot_number=%d", slot_number);
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
						   { .stopped = (AnimationStoppedHandler)slot_splash_animation_out_stopped }, 
						   &slots[slot_number].slot_number);

	#ifdef ENABLE_LOGGING
	char *output = "splash_animation_out_init: XXX";
	snprintf(output, strlen(output), "splash_animation_out_init: %d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
}

void splash_animate(int slot_number)
{
	if(slot_number >= SLOTS_COUNT)
	{
		#ifdef ENABLE_LOGGING
		char *error1 = "splash_animate: invalid value; slot_number=XXX";
		snprintf(error1, strlen(error1), "splash_animate: invalid value; slot_number=%d", slot_number);
		APP_LOG(APP_LOG_LEVEL_DEBUG, error1);
		#endif

		return;
	}

	if(slots[slot_number].state != SLOT_STATE_SPLASH) 
	{ 
		#ifdef ENABLE_LOGGING
		char *error2 = "splash_animate: invalid value; state=XXX";
		snprintf(error2, strlen(error2), "splash_animate: invalid value; state=%d", slots[slot_number].state);
		APP_LOG(APP_LOG_LEVEL_DEBUG, error2);
		#endif

		return; 
	}	

	animation_schedule((Animation*)slots[slot_number].animation);
	
	#ifdef ENABLE_LOGGING
	char *output = "splash_animate: XXX";
	snprintf(output, strlen(output), "splash_animate: %d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
}

void slot_splash_animation_in_stopped(Animation *animation, bool finished, void *data)
{
	(void)animation;
	int slot_number = *(int*)data;
	
	#ifdef ENABLE_LOGGING
	char *output = "slot_splash_animation_in_stopped: XXX";
	snprintf(output, strlen(output), "slot_splash_animation_in_stopped: %d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif

	splash_animation_out_init(slot_number);
	splash_animate(slot_number);
}

void splash_animation_in_init(int slot_number) 
{
	if(slot_number >= SLOTS_COUNT)
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
	slots[slot_number].image = gbitmap_create_with_resource(IMAGE_RESOURCE_SPLASH_IDS[slot_number]);
	
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
						   { .stopped = (AnimationStoppedHandler)slot_splash_animation_in_stopped }, 
						   &slots[slot_number].slot_number);

	#ifdef ENABLE_LOGGING
	char *output = "splash_animation_in_init: XXX";
	snprintf(output, strlen(output), "splash_animation_in_init: %d", slot_number);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
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

void inverter_deinit()
{
	layer_remove_from_parent(inverter_layer_get_layer(inverter));
	inverter_layer_destroy(inverter);
							 
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter_deinit: done");
	#endif
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) 
{
	#ifdef ENABLE_LOGGING
	char *output = "handle_tick: MM/dd/yyyy hh:mm:ss";
	strftime(output, strlen(output), "handle_tick: %D %T", tick_time);
	APP_LOG(APP_LOG_LEVEL_DEBUG, output);
	#endif
	
	//display_time(tick_time);
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
	
	splash_animate(SLOT_TOP);
	splash_animate(SLOT_MID);
	splash_animate(SLOT_BOT);

	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_init: done");
	#endif
}

void handle_deinit() 
{
	/*
	
	for (int i = 0; i < SLOTS_COUNT; i++) 
	{
		unload_image_from_slot(i);
	}
	
	//btmonitor_deinit();
	*/
	inverter_deinit();

	tick_timer_service_unsubscribe();
	window_destroy(window);
	
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_deinit: done");
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
