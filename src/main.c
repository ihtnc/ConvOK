#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "ConvOK_v2.h"

#define MY_UUID { 0x58, 0x99, 0xC3, 0xF9, 0x3E, 0x66, 0x4B, 0xC9, 0xA0, 0x86, 0xA7, 0xD3, 0x97, 0xE4, 0xDB, 0xFD }

PBL_APP_INFO(MY_UUID,
             #ifndef DEBUG
               "Conversation with Orange Kid v2",
             #else
               "ConvOKv2-debug",
             #endif
             "ihopethisnamecounts",
             1, 3, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

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

Window window;
bool show_splash;

//These 2 image containers are needed for animation (1 for the current image and 1 for the previous image).
BmpContainer image_containers[SLOTS_COUNT];
BmpContainer previmage_containers[SLOTS_COUNT];

BmpContainer splash_containers[SLOTS_COUNT]; //Image containers for the splash screen

GRect slot_rectangles[SLOTS_COUNT];        //Current frame
GRect slot_out_rectangles[SLOTS_COUNT];    //Target frame of the scroll out animation
GRect slot_in_rectangles[SLOTS_COUNT];     //Source frame of the scroll in animation
GRect slot_splash_rectangles[SLOTS_COUNT]; //Source frame of the splash animation

//These 2 are used to animate the images (the current image is used on the in animation and the previous image is used in the out animation)
PropertyAnimation slot_out_animations[SLOTS_COUNT];
PropertyAnimation slot_in_animations[SLOTS_COUNT];

PropertyAnimation slot_splash_animations[SLOTS_COUNT]; //Used to animate the splash screen

//Animation timings
const int SLOT_OUT_ANIMATION_DURATIONS[SLOTS_COUNT] = {SLOT_TOP_OUT_DURATION, SLOT_MID_OUT_DURATION, SLOT_BOT_OUT_DURATION};
const int SLOT_OUT_ANIMATION_DELAYS[SLOTS_COUNT] = {SLOT_TOP_OUT_DELAY, SLOT_MID_OUT_DELAY, SLOT_BOT_OUT_DELAY};
const int SLOT_IN_ANIMATION_DURATIONS[SLOTS_COUNT] = {SLOT_TOP_IN_DURATION, SLOT_MID_IN_DURATION, SLOT_BOT_IN_DURATION};
const int SLOT_IN_ANIMATION_DELAYS[SLOTS_COUNT] = {SLOT_TOP_IN_DELAY, SLOT_MID_IN_DELAY, SLOT_BOT_IN_DELAY};
const int SLOT_SPLASH_ANIMATION_DURATIONS[SLOTS_COUNT] = {SLOT_TOP_SPLASH_DURATION, SLOT_MID_SPLASH_DURATION, SLOT_BOT_SPLASH_DURATION};

//Position of the slots along the Y-axis
const int SLOT_YOFFSETS[SLOTS_COUNT] = {SLOT_TOP_YOFFSET, SLOT_MID_YOFFSET, SLOT_BOT_YOFFSET};

//The state of the slot can either be "empty" or the image currently in the slot.
//This is to prevent the slot from unnecessarily reloading the image.
int image_slot_state[SLOTS_COUNT] = {SLOT_STATUS_EMPTY, SLOT_STATUS_EMPTY, SLOT_STATUS_EMPTY};
int previmage_slot_state[SLOTS_COUNT] = {SLOT_STATUS_EMPTY, SLOT_STATUS_EMPTY, SLOT_STATUS_EMPTY};

const int IMAGE_RESOURCE_TOP_IDS[4] = 
{
  RESOURCE_ID_IMAGE_TOP_EXACT,
  RESOURCE_ID_IMAGE_TOP_AFTER,
  RESOURCE_ID_IMAGE_TOP_BEFORE, 
  RESOURCE_ID_IMAGE_TOP_ALMOST
};

const int IMAGE_RESOURCE_MID_IDS[12] = 
{
  RESOURCE_ID_IMAGE_MID_12, RESOURCE_ID_IMAGE_MID_01, 
  RESOURCE_ID_IMAGE_MID_02, RESOURCE_ID_IMAGE_MID_03, 
  RESOURCE_ID_IMAGE_MID_04, RESOURCE_ID_IMAGE_MID_05, 
  RESOURCE_ID_IMAGE_MID_06, RESOURCE_ID_IMAGE_MID_07,
  RESOURCE_ID_IMAGE_MID_08, RESOURCE_ID_IMAGE_MID_09, 
  RESOURCE_ID_IMAGE_MID_10, RESOURCE_ID_IMAGE_MID_11
};

const int IMAGE_RESOURCE_BOT_IDS[4] = 
{
  RESOURCE_ID_IMAGE_BOT_00, RESOURCE_ID_IMAGE_BOT_15,
  RESOURCE_ID_IMAGE_BOT_30, RESOURCE_ID_IMAGE_BOT_45
};

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
  
  //Load the image based on the current state
  int resourceid = determine_image_from_value(slot_number, slot_value);
  bmp_init_container(resourceid, &image_containers[slot_number]);
  image_containers[slot_number].layer.layer.frame.origin.x = SLOT_XOFFSET + SCREEN_WIDTH;
  image_containers[slot_number].layer.layer.frame.origin.y = SLOT_YOFFSETS[slot_number];
  layer_add_child(&window.layer, &image_containers[slot_number].layer.layer);
  
  //Load the image based on the previous state
  int prevresourceid = determine_image_from_value(slot_number, prevslot_state);
  bmp_init_container(prevresourceid, &previmage_containers[slot_number]);
  previmage_containers[slot_number].layer.layer.frame.origin.x = SLOT_XOFFSET;
  previmage_containers[slot_number].layer.layer.frame.origin.y =  SLOT_YOFFSETS[slot_number];
  layer_add_child(&window.layer, &previmage_containers[slot_number].layer.layer);
  
  image_slot_state[slot_number] = slot_value;
  animate_slot(slot_number);
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
									 
void unload_image_from_slot(int slot_number) 
{
  //Removes the images from the display and unloads the resource to free up RAM.
  //Can handle being called on an already empty slot.

  if (image_slot_state[slot_number] != SLOT_STATUS_EMPTY && image_slot_state[slot_number] != SLOT_SPLASH) 
  {
	layer_remove_from_parent(&previmage_containers[slot_number].layer.layer);
    layer_remove_from_parent(&image_containers[slot_number].layer.layer);
    bmp_deinit_container(&previmage_containers[slot_number]);
    bmp_deinit_container(&image_containers[slot_number]);

    image_slot_state[slot_number] = SLOT_STATUS_EMPTY;
  }
  else if (image_slot_state[slot_number] != SLOT_STATUS_EMPTY && image_slot_state[slot_number] == SLOT_SPLASH) 
  {
	layer_remove_from_parent(&splash_containers[slot_number].layer.layer);
    bmp_deinit_container(&splash_containers[slot_number]);
  }
}

void display_time(PblTm *tick_time) 
{
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

void animate_slot(int slot_number)
{
  //Do not run the animation if there are no images.
  if(image_slot_state[slot_number] == SLOT_STATUS_EMPTY) { return; }

  property_animation_init_layer_frame(&slot_out_animations[slot_number], 
                                      &previmage_containers[slot_number].layer.layer,
                                      &slot_rectangles[slot_number], &slot_out_rectangles[slot_number]);
    
  animation_set_duration(&slot_out_animations[slot_number].animation, SLOT_OUT_ANIMATION_DURATIONS[slot_number]);
  animation_set_curve(&slot_out_animations[slot_number].animation, AnimationCurveEaseIn);
  animation_set_handlers(&slot_out_animations[slot_number].animation,
                         (AnimationHandlers)
                         {
                           .stopped = (AnimationStoppedHandler)slot_out_animation_stopped
                         }, 
                         NULL);
  animation_set_delay(&slot_out_animations[slot_number].animation, SLOT_OUT_ANIMATION_DELAYS[slot_number]);
  animation_schedule(&slot_out_animations[slot_number].animation);
  
  property_animation_init_layer_frame(&slot_in_animations[slot_number], 
                                      &image_containers[slot_number].layer.layer,
                                      &slot_in_rectangles[slot_number], &slot_rectangles[slot_number]);
  
  animation_set_duration(&slot_in_animations[slot_number].animation, SLOT_IN_ANIMATION_DURATIONS[slot_number]);
  animation_set_curve(&slot_in_animations[slot_number].animation, AnimationCurveEaseOut);
  animation_set_handlers(&slot_in_animations[slot_number].animation,
                         (AnimationHandlers)
                         {
                           .stopped = (AnimationStoppedHandler)slot_in_animation_stopped
                         }, 
                         NULL);
  animation_set_delay(&slot_in_animations[slot_number].animation, SLOT_IN_ANIMATION_DELAYS[slot_number]);
  animation_schedule(&slot_in_animations[slot_number].animation);
}

void load_splash_screen() 
{
  show_splash = true;

  bmp_init_container(RESOURCE_ID_IMAGE_TOP_SPLASH, &splash_containers[SLOT_TOP]);
  bmp_init_container(RESOURCE_ID_IMAGE_MID_SPLASH, &splash_containers[SLOT_MID]);
  bmp_init_container(RESOURCE_ID_IMAGE_BOT_SPLASH, &splash_containers[SLOT_BOT]);
  
  splash_containers[SLOT_TOP].layer.layer.frame.origin.x = SLOT_XOFFSET;
  splash_containers[SLOT_TOP].layer.layer.frame.origin.y = SLOT_YOFFSETS[SLOT_TOP];
  layer_add_child(&window.layer, &splash_containers[SLOT_TOP].layer.layer);

  splash_containers[SLOT_MID].layer.layer.frame.origin.x = SLOT_XOFFSET;
  splash_containers[SLOT_MID].layer.layer.frame.origin.y = SLOT_YOFFSETS[SLOT_MID];
  layer_add_child(&window.layer, &splash_containers[SLOT_MID].layer.layer);
  
  splash_containers[SLOT_BOT].layer.layer.frame.origin.x = SLOT_XOFFSET;
  splash_containers[SLOT_BOT].layer.layer.frame.origin.y = SLOT_YOFFSETS[SLOT_BOT];
  layer_add_child(&window.layer, &splash_containers[SLOT_BOT].layer.layer);

  image_slot_state[SLOT_TOP] = SLOT_SPLASH;
  image_slot_state[SLOT_MID] = SLOT_SPLASH;
  image_slot_state[SLOT_BOT] = SLOT_SPLASH;
  
  animate_splash(SLOT_TOP);
  animate_splash(SLOT_MID);
  animate_splash(SLOT_BOT);
}

void slot_splash_animation_stopped(Animation *animation, void *data)
{
  (void)animation;
  (void)data;
	
  show_splash = false;
	
  PblTm tick_time;
  get_time(&tick_time); 
  display_time(&tick_time);
}

void animate_splash(int slot_number)
{
  if(show_splash == false) { return; }
  if(image_slot_state[slot_number] != SLOT_SPLASH) { return; }
  
  property_animation_init_layer_frame(&slot_splash_animations[slot_number], 
                                      &splash_containers[slot_number].layer.layer,
                                      &slot_splash_rectangles[slot_number], &slot_rectangles[slot_number]);
    
  animation_set_duration(&slot_splash_animations[slot_number].animation, SLOT_SPLASH_ANIMATION_DURATIONS[slot_number]);
  animation_set_curve(&slot_splash_animations[slot_number].animation, AnimationCurveEaseInOut);
  animation_set_handlers(&slot_splash_animations[slot_number].animation,
                         (AnimationHandlers)
                         {
                           .stopped = (AnimationStoppedHandler)slot_splash_animation_stopped
                         }, 
                         NULL);
  animation_schedule(&slot_splash_animations[slot_number].animation);
}

void handle_init(AppContextRef ctx)
{
  (void)ctx;

  window_init(&window, "Watchface");
  window_stack_push(&window, true);	
  window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&APP_RESOURCES);
  
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
  
  // Avoids a blank screen on watch start.
  load_splash_screen();
}

void handle_deinit(AppContextRef ctx) 
{
  (void)ctx;

  for (int i = 0; i < SLOTS_COUNT; i++) 
  {
    unload_image_from_slot(i);
  }
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) 
{
  (void)t;
  (void)ctx;

  display_time(t->tick_time);
}

void pbl_main(void *params) 
{
  PebbleAppHandlers handlers = 
  {
    .init_handler = &handle_init,
	.deinit_handler = &handle_deinit,
	.tick_info = 
	{
      .tick_handler = &handle_minute_tick,    
      
      #ifndef DEBUG
        .tick_units = MINUTE_UNIT
      #else
        .tick_units = SECOND_UNIT
      #endif
    }
  };
    
  app_event_loop(params, &handlers);
}
