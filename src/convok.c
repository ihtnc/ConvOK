/*

A Conversation with OrangeKid
 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "resource_ids.auto.h"

#define MY_UUID {0xae, 0x49, 0xd8, 0x2c, 0x8f, 0x4e, 0xc1, 0xa3, 0x3f, 0x4e, 0xc3, 0xb8, 0x8c, 0x8f, 0x1d, 0x97}
PBL_APP_INFO(MY_UUID, "Conversation with Orange Kid", "Ice2097", 0x5, 0x0, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;
//TextLayer text_date_layer;
//
// There's only enough memory to load about 6 of 10 required images
// so we have to swap them in & out...
//
// We have one "slot" per digit location on screen.
//
// Because layers can only have one parent we load a digit for each
// slot--even if the digit image is already in another slot.
//
// Slot on-screen layout:
//     0 1
//     2 3
//
#define TOTAL_IMAGE_SLOTS 3

#define NUMBER_OF_IMAGES 19 

// These images are 30,74,64 X 144 pixels ,
// black and white words
const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_TOP_ALM, RESOURCE_ID_IMAGE_NUM_1, RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3, RESOURCE_ID_IMAGE_NUM_4, RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6, RESOURCE_ID_IMAGE_NUM_7, RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9, RESOURCE_ID_IMAGE_NUM_10, RESOURCE_ID_IMAGE_NUM_11, RESOURCE_ID_IMAGE_NUM_12,
  RESOURCE_ID_IMAGE_TOP_BEF, RESOURCE_ID_IMAGE_TOP_AFT,
  RESOURCE_ID_IMAGE_BOT_00, RESOURCE_ID_IMAGE_BOT_15, RESOURCE_ID_IMAGE_BOT_30, RESOURCE_ID_IMAGE_BOT_45
};

BmpContainer image_containers[TOTAL_IMAGE_SLOTS];

#define EMPTY_SLOT -1

// The state is either "empty" or the digit of the image currently in
// the slot--which was going to be used to assist with de-duplication
// but we're not doing that due to the one parent-per-layer
// restriction mentioned above.
int image_slot_state[TOTAL_IMAGE_SLOTS] = {EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT};


void load_digit_image_into_slot(int slot_number, int digit_value) {
  /*

     Loads the digit image from the application's resources and
     displays it on-screen in the correct location.

     Each slot is a quarter of the screen.

   */

  // TODO: Signal these error(s)?

  if ((slot_number < 0) || (slot_number >= TOTAL_IMAGE_SLOTS)) {
    return;
  }

  if ((digit_value < 0) || (digit_value > 19)) {
    return;
  }

  if (image_slot_state[slot_number] != EMPTY_SLOT) {
    return;
  }

  image_slot_state[slot_number] = digit_value;
  bmp_init_container(IMAGE_RESOURCE_IDS[digit_value], &image_containers[slot_number]);
  image_containers[slot_number].layer.layer.frame.origin.x = 0;
  if (slot_number==0) {
    image_containers[slot_number].layer.layer.frame.origin.y = 0;
  }
  else if (slot_number==1) {
    image_containers[slot_number].layer.layer.frame.origin.y = 30;
  }else{
    image_containers[slot_number].layer.layer.frame.origin.y = 104;
  }
  layer_add_child(&window.layer, &image_containers[slot_number].layer.layer);

}


void unload_digit_image_from_slot(int slot_number) {
  /*

     Removes the digit from the display and unloads the image resource
     to free up RAM.

     Can handle being called on an already empty slot.

   */

  if (image_slot_state[slot_number] != EMPTY_SLOT) {
    layer_remove_from_parent(&image_containers[slot_number].layer.layer);
    bmp_deinit_container(&image_containers[slot_number]);
    image_slot_state[slot_number] = EMPTY_SLOT;
  }

}




unsigned short get_display_hour(unsigned short hour) {

  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}


void display_time(PblTm *tick_time) {
 load_digit_image_into_slot(1, get_display_hour(tick_time->tm_hour));
int minhand = tick_time->tm_min;
int minrem = 0;
if (minhand <= 7){
 //   unload_digit_image_from_slot(1);
 //   load_digit_image_into_slot(1, get_display_hour(tick_time->tm_hour));
	unload_digit_image_from_slot(2);
    load_digit_image_into_slot(2, 15);
	minrem=minhand;
}
else if (minhand >= 8 && minhand <= 20) {
	unload_digit_image_from_slot(2);
    load_digit_image_into_slot(2, 16);
	minrem=minhand-15;
}
else if (minhand >= 21 && minhand <= 37) {
	unload_digit_image_from_slot(2);
    load_digit_image_into_slot(2, 17);
	minrem=minhand-30;
}
else if (minhand >= 38 && minhand <= 51) {
	unload_digit_image_from_slot(2);
    load_digit_image_into_slot(2, 18);
	minrem=minhand-45;
}
else{
    unload_digit_image_from_slot(1);
    load_digit_image_into_slot(1, get_display_hour(tick_time->tm_hour+1));
	unload_digit_image_from_slot(2);
    load_digit_image_into_slot(2, 15);
	minrem=minhand-60;
}
	
	

if (minrem <= -4){
	unload_digit_image_from_slot(0);
    load_digit_image_into_slot(0, 13);

}
else if (minrem <= -2){
	unload_digit_image_from_slot(0);
    load_digit_image_into_slot(0, 0);
}
else if (minrem <= 2){
	unload_digit_image_from_slot(0);
    //load_digit_image_into_slot(0, 13);
}
else {
	unload_digit_image_from_slot(0);
    load_digit_image_into_slot(0, 14);
}
  

    // unload_digit_image_from_slot(1);
    // load_digit_image_into_slot(1, get_display_hour(tick_time->tm_hour));

  // display_value(get_display_hour(tick_time->tm_hour), 1);
  // display_value(tick_time->tm_min, 2);
}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)t;
  (void)ctx;
  display_time(t->tick_time);
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "ConvOK");
  window_stack_push(&window, true);
  window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&APP_RESOURCES);

  
  // Avoids a blank screen on watch start.
  PblTm tick_time;

  get_time(&tick_time);
 
  display_time(&tick_time);
}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  for (int i = 0; i < TOTAL_IMAGE_SLOTS; i++) {
    unload_digit_image_from_slot(i);
  }

}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }

  };
  app_event_loop(params, &handlers);
}
