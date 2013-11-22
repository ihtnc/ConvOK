/*
#ifndef GESTURECFG
#define GESTURECFG

typedef void(*ShowConfigHandler)(void);
typedef void(*SelectConfigHandler)(int selected_index);
typedef void(*SaveConfigHandler)(int selected_index, int value_index);
typedef void(*CloseConfigHandler)(void);

typedef struct {
        ShowConfigHandler show_config;
		SelectConfigHandler select_config;
		SaveConfigHandler save_config;
        CloseConfigHandler close_config;
} GestureCFGCallbacks;

void gesturecfg_subscribe(GestureCFGCallbacks callback);
void gesturecfg_init();
void gesturecfg_set_menu_items(int count, ...);
void gesturecfg_set_initial_value(int menu_item_index, int value_index);
void gesturecfg_set_valid_values(int menu_item_index, int count, ...);
void gesturecfg_deinit();

#endif
*/
