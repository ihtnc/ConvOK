#include "thincfg.h"
#include "options.h"

static void readConfig() 
{
	if (persist_exists(CONFIG_KEY_STRAIGHT)) 
	{
		straight_digits = persist_read_int(CONFIG_KEY_STRAIGHT);
	}
	else
	{
		straight_digits = 0;
	}
	
	if (persist_exists(CONFIG_KEY_SEMICOLON)) 
	{
		blinking_semicolon = persist_read_int(CONFIG_KEY_SEMICOLON);
	} 
	else
	{
		blinking_semicolon = 0;
	}
}

static void applyConfig() 
{
	
}

static void in_dropped_handler(AppMessageResult reason, void *context) 
{
}

static void in_received_handler(DictionaryIterator *received, void *context) 
{
	Tuple *straight = dict_find(received, CONFIG_KEY_STRAIGHT);
	Tuple *semicolon = dict_find(received, CONFIG_KEY_SEMICOLON);
	
	if (straight && semicolon) 
	{
		persist_write_int(CONFIG_KEY_STRAIGHT, straight->value->int32);
		persist_write_int(CONFIG_KEY_SEMICOLON, semicolon->value->int32);
		
		straight_digits = straight->value->int32;
		blinking_semicolon = semicolon->value->int32;
		
		applyConfig();
	}
}

static void app_message_init(void) 
{
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_open(64, 64);
}

void thincfg_init() 
{
    app_message_init();
	readConfig();
}

void thincfg_deinit()
{
	app_message_deregister_callbacks();
}