#include "thincfg.h"
#include "options.h"

static void read_config() 
{
	if (persist_exists(CONFIG_KEY_INVERTMODE)) 
	{
		int mode = persist_read_int(CONFIG_KEY_INVERTMODE);
		set_invert_mode_value(mode);		

		#ifdef ENABLE_LOGGING
		char *output = "read_config: invert_mode=XXX";
		snprintf(output, strlen(output), "read_config: invert_mode=%d", mode);
		APP_LOG(APP_LOG_LEVEL_DEBUG, output);
		#endif
	}
	else
	{
		set_invert_mode_value(1); //default value

		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: invert_mode not configured. default=1");
		#endif
	}
	
	if (persist_exists(CONFIG_KEY_BTNOTIFICATION)) 
	{
		bool bt = (persist_read_int(CONFIG_KEY_BTNOTIFICATION == 1));
		set_bt_notification_value(bt);

		#ifdef ENABLE_LOGGING
		if(bt == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: bt_notification=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: bt_notification=false");
		#endif
	} 
	else
	{
		set_bt_notification_value(true); //default value
		
		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: bt_notification not configured. default=true");
		#endif
	}
}

static void apply_config() 
{
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "apply_config: done");
	#endif
}

static void in_dropped_handler(AppMessageResult reason, void *context) 
{
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "in_dropped_handler: done");
	#endif
}

static void in_received_handler(DictionaryIterator *received, void *context) 
{
	Tuple *mode = dict_find(received, CONFIG_KEY_INVERTMODE);
	if(mode) 
	{
		persist_write_int(CONFIG_KEY_INVERTMODE, mode->value->int32);
		int inv_mode = mode->value->int32;
		set_invert_mode_value(inv_mode);

		#ifdef ENABLE_LOGGING
		char *output = "in_received_handler: invert_mode=XXX";
		snprintf(output, strlen(output), "in_received_handler: invert_mode=%d", inv_mode);
		APP_LOG(APP_LOG_LEVEL_DEBUG, output);
		#endif
	}

	Tuple *bt = dict_find(received, CONFIG_KEY_BTNOTIFICATION);
	if(bt) 
	{
		persist_write_int(CONFIG_KEY_BTNOTIFICATION, bt->value->int32);
		bool bt_ntf = (bt->value->int32 == 1);
		set_bt_notification_value(bt_ntf);
		
		#ifdef ENABLE_LOGGING
		if(bt_ntf == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=false");
		#endif
	}

	if(mode || bt) apply_config();
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
	read_config();

	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "thincfg_init: done");
	#endif
}

void thincfg_deinit()
{
	app_message_deregister_callbacks();
	
	#ifdef ENABLE_LOGGING
	APP_LOG(APP_LOG_LEVEL_DEBUG, "thincfg_deinit: done");
	#endif
}
