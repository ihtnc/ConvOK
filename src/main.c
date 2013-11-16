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
	
	if(INVERT_MODE == INVERT_ON_AM)
		invert = (tick_time->tm_hour < 12);
	else if(INVERT_MODE == INVERT_ALWAYS)
		invert = true;
	else
		invert = false;
	
	Layer *inv_layer = inverter_layer_get_layer(inverter);
	layer_set_frame(inv_layer, GRect(0, 0, SCREEN_WIDTH, (invert ? SCREEN_HEIGHT : 0)));
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

void animate_main(int slot_number)
{
	//Do not run the animation if there are no images.
	if(image_slot_state[slot_number] == SLOT_STATUS_EMPTY) { return; }
	animation_schedule(&slot_in_animations[slot_number]->animation);
}

void unload_image_from_slot(int slot_number) 
{
	//Removes the images from the display and unloads the resource to free up RAM.
	//Can handle being called on an already empty slot.
	
	if (image_slot_state[slot_number] != SLOT_STATUS_EMPTY && image_slot_state[slot_number] != SLOT_SPLASH) 
	{
		Layer *prv_layer = bitmap_layer_get_layer(previmage_containers[slot_number]);
		layer_remove_from_parent(prv_layer);
		bitmap_layer_destroy(previmage_containers[slot_number]);
		
		Layer *main_layer = bitmap_layer_get_layer(image_containers[slot_number]);
		layer_remove_from_parent(main_layer);
		bitmap_layer_destroy(image_containers[slot_number]);
	
		image_slot_state[slot_number] = SLOT_STATUS_EMPTY;
	}
	else if (image_slot_state[slot_number] != SLOT_STATUS_EMPTY && image_slot_state[slot_number] == SLOT_SPLASH) 
	{
		Layer *spl_layer = bitmap_layer_get_layer(splash_containers[slot_number]);
		layer_remove_from_parent(spl_layer);
		bitmap_layer_destroy(splash_containers[slot_number]);
	}
}

void load_image_to_slot(int slot_number, int hour_value, int minute_value) 
{
	//Loads the digit image from the application's resources and displays it on-screen in the correct location.
	  
	//Validations
	if (slot_number < 0 || slot_number > SLOTS_COUNT) { return; }
	if (hour_value < 0 || hour_value > 11) { return; }
	if (minute_value < 0 || minute_value > 59) { return; }
	
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
	
	//Do not reload the image if slot_state value did not change
	if (image_slot_state[slot_number] == slot_value) { return; }
	
	//The state is about to change so the current state will become the previous state
	int prevslot_state = image_slot_state[slot_number];
	
	unload_image_from_slot(slot_number);
	
	Layer *inv_layer = inverter_layer_get_layer(inverter);
	
	//Load the image based on the current state
	int resourceid = determine_image_from_value(slot_number, slot_value);
	GBitmap *bmp = gbitmap_create_with_resource(resourceid);
	
	image_containers[slot_number] = bitmap_layer_create(bmp->bounds);
	bitmap_layer_set_bitmap(image_containers[slot_number], bmp);
	
	Layer *bmp_layer = bitmap_layer_get_layer(image_containers[slot_number]);
	GRect bmp_frame = layer_get_frame(bmp_layer);
	bmp_frame.origin.x = SLOT_XOFFSET + SCREEN_WIDTH;
	bmp_frame.origin.y = SLOT_YOFFSETS[slot_number];
	layer_set_frame(bmp_layer, bmp_frame);
	layer_insert_below_sibling(bmp_layer, inv_layer);

	//Load the image based on the previous state
	int prevresourceid = determine_image_from_value(slot_number, prevslot_state);
	GBitmap *prevbmp = gbitmap_create_with_resource(prevresourceid);
	
	previmage_containers[slot_number] = bitmap_layer_create(prevbmp->bounds);
	bitmap_layer_set_bitmap(previmage_containers[slot_number], prevbmp);
	
	Layer *prevbmp_layer = bitmap_layer_get_layer(image_containers[slot_number]);
	GRect prevbmp_frame = layer_get_frame(prevbmp_layer);
	prevbmp_frame.origin.x = SLOT_XOFFSET;
	prevbmp_frame.origin.y =  SLOT_YOFFSETS[slot_number];
	layer_set_frame(prevbmp_layer, prevbmp_frame);
	layer_insert_below_sibling(prevbmp_layer, inv_layer);
	
	image_slot_state[slot_number] = slot_value;
	animate_main(slot_number);
}

void display_time(struct tm *tick_time) 
{
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

void main_init(int slot_number)
{
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
}

void slot_splash_animation_stopped(Animation *animation, void *data)
{
	(void)animation;
	(void)data;
		
	show_splash = false;
	
	time_t t;
	time(&t);
	struct tm *local = localtime(&t);
	display_time(local);
}

void splash_slot_init(int slot_number)
{
	Layer *inv_layer = inverter_layer_get_layer(inverter);
	
	GBitmap *bmp = NULL;
	
	if(slot_number == SLOT_TOP)
		bmp = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TOP_SPLASH);
	else if(slot_number == SLOT_MID)
		bmp = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MID_SPLASH);
	else if(slot_number == SLOT_BOT)
		bmp = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BOT_SPLASH);
	
	BitmapLayer *bmp_layer = bitmap_layer_create(bmp->bounds);
	bitmap_layer_set_bitmap(bmp_layer, bmp);
	
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
}

void splash_slot_deinit(int slot_number)
{
	property_animation_destroy(slot_splash_animations[slot_number]);
	
	Layer *spl_layer = bitmap_layer_get_layer(splash_containers[slot_number]);
	layer_remove_from_parent(spl_layer);
	bitmap_layer_destroy(splash_containers[slot_number]);
}

void animate_splash(int slot_number)
{
	if(show_splash == false) { return; }
	if(image_slot_state[slot_number] != SLOT_SPLASH) { return; }
	
	animation_schedule(&slot_splash_animations[slot_number]->animation);
}

void splash_init() 
{
	time_t t;
	time(&t);
	struct tm *local = localtime(&t);
	determine_invert_status(local);
	
	show_splash = true;
	
	splash_slot_init(SLOT_TOP);
	splash_slot_init(SLOT_MID);
	splash_slot_init(SLOT_BOT);
	
	animate_splash(SLOT_TOP);
	animate_splash(SLOT_MID);
	animate_splash(SLOT_BOT);
}

void splash_deinit()
{
	splash_slot_deinit(SLOT_TOP);
	splash_slot_deinit(SLOT_MID);
	splash_slot_deinit(SLOT_BOT);
}

void inverter_init()
{
	inverter = inverter_layer_create(GRect(0, 0, SCREEN_WIDTH, 0));
	Layer *inv_layer = inverter_layer_get_layer(inverter);
	Layer *win_layer = window_get_root_layer(window);
	layer_add_child(win_layer, inv_layer);
}

void inverter_deinit()
{
	Layer *inv_layer = inverter_layer_get_layer(inverter);
	layer_remove_from_parent(inv_layer);
	inverter_layer_destroy(inverter);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) 
{
	display_time(tick_time);
}

void handle_init()
{
	window = window_create();
	window_stack_push(window, true);	
	window_set_background_color(window, GColorBlack);
	
	#ifndef DEBUG
		tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	#else
		tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
	#endif
	
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
	
	btmonitor_init(true);
	inverter_init();
	splash_init();
}

void handle_deinit() 
{
	for (int i = 0; i < SLOTS_COUNT; i++) 
	{
		unload_image_from_slot(i);
	}
	
	btmonitor_deinit();
	inverter_deinit();
	splash_deinit();
	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main(void) 
{
	handle_init();
	app_event_loop();
	handle_deinit();
}
