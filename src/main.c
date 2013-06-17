#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "ConvOK_v2.h"

#define MY_UUID { 0x58, 0x99, 0xC3, 0xF9, 0x3E, 0x66, 0x4B, 0xC9, 0xA0, 0x86, 0xA7, 0xD3, 0x97, 0xE4, 0xDB, 0xFD }

PBL_APP_INFO(MY_UUID,
             "Conversation with Orange Kid v2", "ihopethisnamecounts",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

BmpContainer image_containers[SLOTS_COUNT];

// The state is either "empty" or the image currently in the slot.
// This is to prevent the slot from unnecessarily reloading the image.
int image_slot_state[SLOTS_COUNT] = {SLOT_STATUS_EMPTY, SLOT_STATUS_EMPTY, SLOT_STATUS_EMPTY};

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

Window window;

void load_image_to_slot(int slot_number, int hour_value, int minute_value) 
{
  //Loads the digit image from the application's resources and displays it on-screen in the correct location.
  
  //Time formula:
  //X:00      = It is X o'Clock           X:30      = It is X thirty
  //X:01-X:06 = A little after X o'Clock  X:31-X:36 = A little after X thirty
  //X:07-X:11 = A bit before X fifteen    X:37-X:41 = A bit before X fortyfive
  //X:12-X:14 = Almost X fifteen          X:42-X:44 = Almost X fortyfive
  //X:15      = It is  X fifteen          X:45      = It is  X fortyfive
  //X:16-X:21 = A little after X fifteen  X:46-X:51 = A little after X fortyfive
  //X:22-X:26 = A bit before X thirty     X:52-X:56 = A bit before X+1 o'Clock
  //X:27-X:29 = Almost X thirty           X:57-X:59 = Almost X+1 o'Clock
  
  //TODO: Signal these error(s)?
  if (slot_number < 0 || slot_number > SLOTS_COUNT) { return; }
  if (hour_value < 1 || hour_value > 12) { return; }
  if (minute_value < 0 || minute_value > 59) { return; }
  
  int slot_y = 0;
  int slot_value = -1;
  int quarter_value = minute_value / 15;
  int quarter_remainder = minute_value % 15;

  if (slot_number == SLOT_TOP)
  {
    if (quarter_remainder == 0)
	{
      slot_value = 0;
    }
    else if(quarter_remainder >= 1 && quarter_remainder <= 6)
	{
      slot_value = 1;
    }
    else if(quarter_remainder >= 7 && quarter_remainder <= 11)
	{
      slot_value = 2;
    }
    else if(quarter_remainder >= 12 && quarter_remainder <= 14)
	{
      slot_value = 3;
    }
    
    slot_y = SLOT_TOP_YOFFSET;
  }
  else if (slot_number == SLOT_MID)
  {
	if (quarter_remainder >= 7 && quarter_value == 3) { slot_value = hour_value + 1; }
    else { slot_value = hour_value; }
	  
	//Normalize value (should only be 0-11)
	slot_value = slot_value % 12;
	slot_y = SLOT_MID_YOFFSET;  
  }
  else if (slot_number == SLOT_BOT) 
  {
	if (quarter_remainder >= 7) { slot_value = quarter_value + 1; }
    else { slot_value = quarter_value; }
    
    //Normalize value (should only be 0-3)
	slot_value = slot_value % 4; 
    slot_y = SLOT_BOT_YOFFSET;
  }
  
  //Do not reload the image if slot_state value did not change
  if (image_slot_state[slot_number] == slot_value) { return; }
  
  int resourceid = determine_image_from_value(slot_number, slot_value);
  
  unload_image_from_slot(slot_number);
  bmp_init_container(resourceid, &image_containers[slot_number]);
  
  image_containers[slot_number].layer.layer.frame.origin.x = SLOT_XOFFSET;
  image_containers[slot_number].layer.layer.frame.origin.y = slot_y;
  
  layer_add_child(&window.layer, &image_containers[slot_number].layer.layer);
  
  image_slot_state[slot_number] = slot_value;
}

int determine_image_from_value(int slot_number, int slot_value)
{
  if (slot_number == SLOT_TOP)
  {
    return IMAGE_RESOURCE_TOP_IDS[slot_value];
  }
  else if (slot_number == SLOT_MID)
  {
	return IMAGE_RESOURCE_MID_IDS[slot_value];
  }
  else if (slot_number == SLOT_BOT) 
  {
    return IMAGE_RESOURCE_BOT_IDS[slot_value];
  }
  else 
  {
    return -1;
  }
}
									 
void unload_image_from_slot(int slot_number) 
{
  //Removes the digit from the display and unloads the image resource to free up RAM.
  //Can handle being called on an already empty slot.

  if (image_slot_state[slot_number] != SLOT_STATUS_EMPTY) 
  {
	layer_remove_from_parent(&image_containers[slot_number].layer.layer);
    bmp_deinit_container(&image_containers[slot_number]);
    image_slot_state[slot_number] = SLOT_STATUS_EMPTY;
  }
}

void display_time(PblTm *tick_time) 
{
  int normalized_hour = tick_time->tm_hour % 12;
  int normalized_minute = tick_time->tm_min;
  
  load_image_to_slot(SLOT_TOP, normalized_hour, normalized_minute);
  load_image_to_slot(SLOT_MID, normalized_hour, normalized_minute);
  load_image_to_slot(SLOT_BOT, normalized_hour, normalized_minute);
}

void handle_init(AppContextRef ctx)
{
  (void)ctx;

  window_init(&window, "Watchface");
  window_stack_push(&window, true);	
  window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&APP_RESOURCES);
	
  // Avoids a blank screen on watch start.
  PblTm tick_time;
  get_time(&tick_time); 
  display_time(&tick_time);
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
      .tick_units = MINUTE_UNIT
    }
  };
    
  app_event_loop(params, &handlers);
}
