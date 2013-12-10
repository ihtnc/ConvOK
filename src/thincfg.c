#include "thincfg.h"
#include "btmonitor.h"
#include "options.h"

static int invert_mode;
static bool bt_notification;

static int old_invert_mode;
static bool old_bt_notification;

static ThinCFGCallbacks cfgcallbacks;

int get_invert_mode_value(void) { return invert_mode; }
bool get_bt_notification_value(void) { return bt_notification; }

static void read_config() 
{
	if (persist_exists(CONFIG_KEY_INVERTMODE)) 
	{
		invert_mode = persist_read_int(CONFIG_KEY_INVERTMODE);

		#ifdef ENABLE_LOGGING
		char *output = "read_config: invert_mode=XXX";
		snprintf(output, strlen(output), "read_config: invert_mode=%d", invert_mode);
		APP_LOG(APP_LOG_LEVEL_DEBUG, output);
		#endif
	}
	else
	{
		invert_mode = 0; //default value

		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: invert_mode not configured. default=0");
		#endif
	}
	
	if (persist_exists(CONFIG_KEY_BTNOTIFICATION)) 
	{
		bool bt = persist_read_bool(CONFIG_KEY_BTNOTIFICATION);
		bt_notification = bt;

		#ifdef ENABLE_LOGGING
		if(bt == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: bt_notification=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: bt_notification=false");
		#endif
	} 
	else
	{
		bt_notification = false; //default value

		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "read_config: bt_notification not configured. default=false");
		#endif
	}

	old_bt_notification = bt_notification;
	old_invert_mode = invert_mode;
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
		invert_mode = (int)mode->value->int32;
	
		#ifdef ENABLE_LOGGING
		char *output = "in_received_handler: invert_mode=XXX";
		snprintf(output, strlen(output), "in_received_handler: invert_mode=%d", (int)mode->value->int32);
		APP_LOG(APP_LOG_LEVEL_DEBUG, output);
		#endif
	}

	if(old_invert_mode != invert_mode && cfgcallbacks.field_changed)
	{
		cfgcallbacks.field_changed(CONFIG_KEY_INVERTMODE, (void *)&old_invert_mode, (void *)&invert_mode);
	}
	old_invert_mode = invert_mode;

	Tuple *bt = dict_find(received, CONFIG_KEY_BTNOTIFICATION);
	if(bt) 
	{
		persist_write_int(CONFIG_KEY_BTNOTIFICATION, bt->value->int32);
		bt_notification = (bt->value->int32 == 1);

		#ifdef ENABLE_LOGGING
		if(bt_notification == true) APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=true");
		else APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=false");
		#endif
	}
	else
	{
		//since thinCFG won't pass fields that are not selected, we set the bt_notification to false if its key is not returned
		persist_write_bool(CONFIG_KEY_BTNOTIFICATION, false);
		bt_notification = false;

		#ifdef ENABLE_LOGGING
		APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler: bt_notification=false");
		#endif
	}

	if(old_bt_notification != bt_notification && cfgcallbacks.field_changed)
	{
		cfgcallbacks.field_changed(CONFIG_KEY_BTNOTIFICATION, (void *)&old_bt_notification, (void *)&bt_notification);
	}
	old_bt_notification = bt_notification;
}

void thincfg_unsubscribe()
{
	//
}

void thincfg_subscribe(ThinCFGCallbacks callbacks)
{
	cfgcallbacks = callbacks;
}

static void app_message_init(void) 
{
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void thincfg_init() 
{
    app_message_init();
	read_config();
}

void thincfg_deinit()
{
	app_message_deregister_callbacks();
}
