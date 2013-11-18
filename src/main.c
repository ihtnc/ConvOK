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
	APP_LOG(APP_LOG_LEVEL_DEBUG, "determine_invert_status start...");
	bool invert;
	
	if(INVERT_MODE == INVERT_ON_AM)
		invert = (tick_time->tm_hour < 12);
	else if(INVERT_MODE == INVERT_ALWAYS)
		invert = true;
	else
		invert = false;
	
	Layer *inv_layer = inverter_layer_get_layer(inverter);
	layer_set_frame(inv_layer, GRect(0, 0, SCREEN_WIDTH, (invert ? SCREEN_HEIGHT : 0)));
	APP_LOG(APP_LOG_LEVEL_DEBUG, "determine_invert_status end.");
}

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

void slot_splash_animation_stopped(Animation *animation, void *data)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "slot_splash_animation_stopped start...");
	(void)animation;
	(void)data;
		
	show_splash = false;	

	time_t t = time(NULL);
	struct tm *local = localtime(&t);
	display_time(local);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "slot_splash_animation_stopped end.");
}

void splash_slot_init(int slot_number)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "splash_slot_init start...");
	Layer *inv_layer = inverter_layer_get_layer(inverter);
	
	if(slot_number == SLOT_TOP)
		splashimages[slot_number] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TOP_SPLASH);
	else if(slot_number == SLOT_MID)
		splashimages[slot_number] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MID_SPLASH);
	else if(slot_number == SLOT_BOT)
		splashimages[slot_number] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BOT_SPLASH);
	
	BitmapLayer *bmp_layer = bitmap_layer_create(slot_rectangles[slot_number]);
	bitmap_layer_set_bitmap(bmp_layer, splashimages[slot_number]);
	
	Layer *spl_layer = bitmap_layer_get_layer(bmp_layer);
	GRect spl_frame = layer_get_frame(spl_layer);
	spl_frame.origin.x = SLOT_XOFFSET;
	spl_frame.origin.y = SLOT_YOFFSETS[slot_number];
	layer_set_frame(spl_layer, spl_frame);
	layer_insert_below_sibling(spl_layer, inv_layer);
	
	PropertyAnimation *anim =
		property_animation_create_layer_frame(spl_layer, 
											  &slot_splash_rectangles[slot_number], 
											  &slot_rectangles[slot_number]);
	
	animation_set_duration(&anim->animation, SLOT_SPLASH_ANIMATION_DURATIONS[slot_number]);
	animation_set_curve(&anim->animation, AnimationCurveEaseInOut);
	animation_set_handlers(&anim->animation,
						   (AnimationHandlers)
						   {
							   .stopped = (AnimationStoppedHandler)slot_splash_animation_stopped
						   }, 
						   NULL);
	
	slot_splash_animations[slot_number] = anim;
	splash_containers[slot_number] = bmp_layer;
	
	image_slot_state[slot_number] = SLOT_SPLASH;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "splash_slot_init end.");
}

void splash_slot_deinit(int slot_number)
{
	animation_destroy(&slot_splash_animations[slot_number]->animation);
	property_animation_destroy(slot_splash_animations[slot_number]);
	
	Layer *spl_layer = bitmap_layer_get_layer(splash_containers[slot_number]);
	layer_remove_from_parent(spl_layer);
	bitmap_layer_destroy(splash_containers[slot_number]);
	gbitmap_destroy(splashimages[slot_number]);
}

void animate_splash(int slot_number)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "animate_splash start...");
	if(show_splash == false) { return; }
	if(image_slot_state[slot_number] != SLOT_SPLASH) { return; }
	
	animation_schedule(&slot_splash_animations[slot_number]->animation);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "animat_splash end.");
}

void splash_init() 
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "splash_init start...");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "retrieve time start...");	
	time_t t = time(NULL);
	struct tm *local = localtime(&t);
	determine_invert_status(local);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "retrieve time end.");

	show_splash = true;
	
	splash_slot_init(SLOT_TOP);
	splash_slot_init(SLOT_MID);
	splash_slot_init(SLOT_BOT);
	
	animate_splash(SLOT_TOP);
	animate_splash(SLOT_MID);
	animate_splash(SLOT_BOT);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "splash_init end.");
}

void splash_deinit()
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "splash_deinit start...");
	splash_slot_deinit(SLOT_TOP);
	splash_slot_deinit(SLOT_MID);
	splash_slot_deinit(SLOT_BOT);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "splash_deinit end.");
}

void inverter_init()
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter_init start...");
	inverter = inverter_layer_create(GRect(0, 0, SCREEN_WIDTH, 0));
	Layer *inv_layer = inverter_layer_get_layer(inverter);
	Layer *win_layer = window_get_root_layer(window);
	layer_add_child(win_layer, inv_layer);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter_init end.");
}

void inverter_deinit()
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter_deinit start...");
	Layer *inv_layer = inverter_layer_get_layer(inverter);
	layer_remove_from_parent(inv_layer);
	inverter_layer_destroy(inverter);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "inverter_deinit end.");
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) 
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_tick start...");
	display_time(tick_time);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_tick end.");
}

void handle_init()
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_init start...");
	memset(&image_containers, 0, sizeof(image_containers));
	memset(&previmage_containers, 0, sizeof(previmage_containers));
	memset(&splash_containers, 0, sizeof(splash_containers));
	memset(&images, 0, sizeof(images));
	memset(&previmages, 0, sizeof(previmages));
	memset(&splashimages, 0, sizeof(splashimages));
	memset(&slot_out_animations, 0, sizeof(slot_out_animations));
	memset(&slot_in_animations, 0, sizeof(slot_in_animations));
	memset(&slot_splash_animations, 0, sizeof(slot_splash_animations));

	window = window_create();
	window_stack_push(window, true);	
	window_set_background_color(window, GColorBlack);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_timer_service_subscribe start...");
	#ifndef DEBUG
		tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	#else
		tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
	#endif
	APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_timer_service_subscribe end.");
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "initialize rectangles start...");
	slot_rectangles[SLOT_TOP] = GRect(SLOT_XOFFSET, SLOT_TOP_YOFFSET, SCREEN_WIDTH, SLOT_MID_YOFFSET - SLOT_TOP_YOFFSET);
	slot_rectangles[SLOT_MID] = GRect(SLOT_XOFFSET, SLOT_MID_YOFFSET, SCREEN_WIDTH, SLOT_BOT_YOFFSET - SLOT_MID_YOFFSET);
	slot_rectangles[SLOT_BOT] = GRect(SLOT_XOFFSET, SLOT_BOT_YOFFSET, SCREEN_WIDTH, SCREEN_HEIGHT - SLOT_BOT_YOFFSET);
	  
	slot_in_rectangles[SLOT_TOP] = GRect(SLOT_XOFFSET + SCREEN_WIDTH, SLOT_TOP_YOFFSET, SCREEN_WIDTH, SLOT_MID_YOFFSET - SLOT_TOP_YOFFSET);
	slot_in_rectangles[SLOT_MID] = GRect(SLOT_XOFFSET + SCREEN_WIDTH, SLOT_MID_YOFFSET, SCREEN_WIDTH, SLOT_BOT_YOFFSET - SLOT_MID_YOFFSET);
	slot_in_rectangles[SLOT_BOT] = GRect(SLOT_XOFFSET + SCREEN_WIDTH, SLOT_BOT_YOFFSET, SCREEN_WIDTH, SCREEN_HEIGHT - SLOT_BOT_YOFFSET);
	
	slot_out_rectangles[SLOT_TOP] = GRect(SLOT_XOFFSET - SCREEN_WIDTH, SLOT_TOP_YOFFSET, SCREEN_WIDTH, SLOT_MID_YOFFSET - SLOT_TOP_YOFFSET);
	slot_out_rectangles[SLOT_MID] = GRect(SLOT_XOFFSET - SCREEN_WIDTH, SLOT_MID_YOFFSET, SCREEN_WIDTH, SLOT_BOT_YOFFSET - SLOT_MID_YOFFSET);
	slot_out_rectangles[SLOT_BOT] = GRect(SLOT_XOFFSET - SCREEN_WIDTH, SLOT_BOT_YOFFSET, SCREEN_WIDTH, SCREEN_HEIGHT - SLOT_BOT_YOFFSET);
	  
	slot_splash_rectangles[SLOT_TOP] = GRect(SLOT_XOFFSET + SLOT_TOP_SPLASH_XOFFSET, SLOT_TOP_YOFFSET, SCREEN_WIDTH, SLOT_MID_YOFFSET - SLOT_TOP_YOFFSET);
	slot_splash_rectangles[SLOT_MID] = GRect(SLOT_XOFFSET - SLOT_MID_SPLASH_XOFFSET, SLOT_MID_YOFFSET, SCREEN_WIDTH, SLOT_BOT_YOFFSET - SLOT_MID_YOFFSET);
	slot_splash_rectangles[SLOT_BOT] = GRect(SLOT_XOFFSET + SLOT_BOT_SPLASH_XOFFSET, SLOT_BOT_YOFFSET, SCREEN_WIDTH, SCREEN_HEIGHT - SLOT_BOT_YOFFSET);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "initialize rectangles end.");

	//btmonitor_init(true);
	inverter_init();
	splash_init();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_init end.");
}

void handle_deinit() 
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_deinit start...");
	for (int i = 0; i < SLOTS_COUNT; i++) 
	{
		unload_image_from_slot(i);
	}
	
	//btmonitor_deinit();
	inverter_deinit();
	splash_deinit();

	tick_timer_service_unsubscribe();
	window_destroy(window);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handle_deinit end.");
}

int main(void) 
{
	handle_init();
	app_event_loop();
	handle_deinit();
}
