#include "../helper/confighelper.h"

#pragma once

enum device_action { 
    DEVICE_ACTION_ADD, DEVICE_ACTION_EDIT, DEVICE_ACTION_DELETE
};

typedef struct _app_t App;
struct _app_t
{
    bool_t is_running;
    
    Panel *main_panel;
    Panel *subpanel;


    Layout *layout1;
    Layout *layout1a;
    Layout *layout2;
    Layout *layout3;
    Layout *layout4;
    Layout *sub_layout;


    Window *window;
    Window *window2;
    Window *window3;
    Layout* layout;

    Combo* combo_configurations;
    Button* button_autostart;
    Edit* edit_forceconfig;
    ListBox* listbox_devices;

    Button *button_startstop;
    Button *button_add;
    Button *button_edit;
    Button *button_del;
    Button *button_test;

    Label *label_autostart;
    Label *label_forceconfig;
    Label *label_configurations;
    Label *label_game_status;
    Label *label_simd_status;

    Cell* devices_subpanel;
    Cell* cell_config_dropdown;
};
