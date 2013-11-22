#include "gesturecfg.h"
#include "options.h"

static GestureCFGCallbacks cfgcallbacks;
static Window *config;
static MenuLayer *menu;

void gesturecfg_subscribe(GestureCFGCallbacks callback)
{
	cfgcallbacks = callbacks;
	
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "gesturecfg_subscribe: done");
	#endif
}

void gesturecfg_set_initial_value(int menu_item_index, int value_index)
{
}

void gesturecfg_set_valid_values(int menu_item_index, int count, ...)
{
}

void gesturecfg_set_menu_items(int count, ...)
{
	va_list args;
    va_start(args, count);
	for(unsigned int i = 0; i < count; i++)
    {
        char *arg = va_arg(args, char*);
    }
    va_end(args);
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
	/*
	switch (cell_index->row)
	{
    // This is the menu item with the cycling icon
    case 1:
      // Cycle the icon
      current_icon = (current_icon + 1) % NUM_MENU_ICONS;
      // After changing the icon, mark the layer to have it updated
      layer_mark_dirty(menu_layer_get_layer(menu_layer));
      break;
	}
*/
}

static void window_load(Window *window) 
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
	menu = menu_layer_create(bounds);
	
	menu_layer_set_callbacks(menu_layer, NULL, 
							 (MenuLayerCallbacks)
							 {
								 .get_num_sections = menu_get_num_sections_callback,
								 .get_num_rows = menu_get_num_rows_callback,
								 .get_header_height = menu_get_header_height_callback,
								 .draw_header = menu_draw_header_callback,
								 .draw_row = menu_draw_row_callback,
								 .select_click = menu_select_callback,
							 });
	
	layer_add_child(window_layer, menu_layer_get_layer(menu));
}

void gesturecfg_deinit()
{
	window_destroy(config);
}

void gesturecfg_init()
{
	config = window_create();
	window_set_background_color(config, GColorWhite);
	
	window_set_window_handlers(window, 
							   (WindowHandlers)
							   {
								   .load = window_load,
								   .unload = window_unload,
							   });
	
	//window_stack_push(window, true);
}
