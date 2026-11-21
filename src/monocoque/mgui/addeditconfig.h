#include <gui/gui.hxx>
#include "mainwindow.h"

Panel *addconfig_window(Window *parent, App* app, MonocoqueSettings* ms, int confignum);

typedef struct _add_config_t AddConfigData;

struct _add_config_t
{
    Layout* main_outer_layout;
    Layout* general_layout;

    Label* label_select_simapi;
    Label* label_select_sim;
    Label* label_select_car;
    Combo* combo_select_simapi;
    Combo* combo_select_sim;
    //Combo* combo_select_car;
    Edit* edit_car;

    Button* save_button;

    int confignum;
    MonocoqueSettings* ms;
    Panel* panel;
    App* apppointer;
    Window *parent;
    uint32_t type;
};
