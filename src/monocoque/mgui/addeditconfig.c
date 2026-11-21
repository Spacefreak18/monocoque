#include <string.h>

#include <nappgui.h>
#include <gui/guiall.h>

#include "addeditconfig.h"
#include "mainwindow.h"

#include "uiconfighelper.h"
#include "../helper/confighelper.h"
#include "../gameloop/gameloop.h"
#include "../gameloop/loopdata.h"
#include "../slog/slog.h"

/*---------------------------------------------------------------------------*/

static Layout *i_modal_layout(AddConfigData *data, App* app, MonocoqueSettings* ms, int confignum);

/*---------------------------------------------------------------------------*/

static AddConfigData *i_modal_data(Window *parent)
{
    AddConfigData *data = heap_new0(AddConfigData);
    data->parent = parent;
    data->type = UINT32_MAX;
    return data;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_modal_data(AddConfigData **data)
{
    AddConfigData* d = *data;
    heap_delete(data, AddConfigData);
}


/*---------------------------------------------------------------------------*/

static void i_OnTestClose(App *app, Event *e)
{

}

/*---------------------------------------------------------------------------*/

static void i_OnSaveConfig(AddConfigData *data, Event *e)
{

    window_stop_modal(data->parent, 0);
}

static void populateAPIAndSim(Combo* combo_api, Combo* combo_sim)
{
    combo_add_elem(combo_api, "Any", NULL);
    combo_add_elem(combo_api, "Assetto Corsa", NULL);
    combo_add_elem(combo_api, "RFactor2", NULL);
    combo_add_elem(combo_api, "Project Cars 2", NULL);
    combo_add_elem(combo_api, "SCSTrucks", NULL);
    combo_add_elem(combo_api, "OutSimOutGauge", NULL);
    combo_add_elem(combo_api, "Dirt Rally 2", NULL);
    combo_add_elem(combo_api, "F1 2018", NULL);
    combo_add_elem(combo_api, "Race Room", NULL);
    combo_add_elem(combo_api, "Forza", NULL);
    combo_add_elem(combo_api, "LeMans Ultimate", NULL);
    combo_add_elem(combo_api, "WreckFest 2", NULL);
    combo_add_elem(combo_api, "Richard Burns Rally", NULL);

    combo_add_elem(combo_sim, "Any", NULL);
    combo_add_elem(combo_sim, "Assetto Corsa", NULL);
    combo_add_elem(combo_sim, "RFactor2", NULL);
    combo_add_elem(combo_sim, "Automobilista2", NULL);
    combo_add_elem(combo_sim, "Automobilista2 (demo)", NULL);
    combo_add_elem(combo_sim, "EuroTrucks2", NULL);
    combo_add_elem(combo_sim, "American Truck Simulator", NULL);
    combo_add_elem(combo_sim, "Assetto Corsa Competizione", NULL);
    combo_add_elem(combo_sim, "Assetto Corsa Evo", NULL);
    combo_add_elem(combo_sim, "Assetto Corsa Rally", NULL);
    combo_add_elem(combo_sim, "LeMans Ultimate", NULL);
    combo_add_elem(combo_sim, "BeamNG", NULL);
    combo_add_elem(combo_sim, "Live For Speed", NULL);
    combo_add_elem(combo_sim, "Dirt Rally 2", NULL);
    combo_add_elem(combo_sim, "F1 2022", NULL);
    combo_add_elem(combo_sim, "Race Room", NULL);
    combo_add_elem(combo_sim, "Forza Horizon 5", NULL);
    combo_add_elem(combo_sim, "Forza Horizon 6", NULL);
    combo_add_elem(combo_sim, "Wreckfest 2", NULL);
    combo_add_elem(combo_sim, "Richard Burns Rally", NULL);
}

static Layout *i_modal_layout(AddConfigData *data, App* app, MonocoqueSettings* ms, int confignum)
{
    struct _add_config_t* subwindow = data;

    subwindow->main_outer_layout = layout_create(1,2);
    subwindow->general_layout = layout_create(6,2);

    layout_layout(subwindow->main_outer_layout, subwindow->general_layout, 0, 0);

    subwindow->combo_select_simapi = combo_create();
    subwindow->combo_select_sim = combo_create();
    subwindow->edit_car = edit_create();
    subwindow->label_select_sim = label_create();
    subwindow->label_select_simapi = label_create();
    subwindow->label_select_car = label_create();
    label_text(subwindow->label_select_simapi, "Api: ");
    label_text(subwindow->label_select_sim, "Sim: ");
    label_text(subwindow->label_select_car, "Car: ");
    layout_label(subwindow->general_layout, subwindow->label_select_simapi, 0, 0);
    layout_combo(subwindow->general_layout, subwindow->combo_select_simapi, 1, 0);
    layout_label(subwindow->general_layout, subwindow->label_select_sim, 2, 0);
    layout_combo(subwindow->general_layout, subwindow->combo_select_sim, 3, 0);
    layout_label(subwindow->general_layout, subwindow->label_select_car, 4, 0);
    layout_edit(subwindow->general_layout, subwindow->edit_car, 5, 0);

    subwindow->save_button = button_push(); 
    button_text(subwindow->save_button, "Save");
    layout_button(subwindow->general_layout, subwindow->save_button, 0, 1);
    //button_OnClick(subwindow->save_button, listener(data, i_OnSaveDevice, AddDeviceData));

    populateAPIAndSim(subwindow->combo_select_simapi, subwindow->combo_select_sim);
    
    combo_selected(subwindow->combo_select_simapi, 0);
    combo_selected(subwindow->combo_select_sim, 0);
    return subwindow->main_outer_layout;
}

/*---------------------------------------------------------------------------*/

Panel *addconfig_window(Window *parent, App* app, MonocoqueSettings* ms, int confignum)
{
    Panel *panel = panel_create();
    AddConfigData *data = i_modal_data(parent);
    data->panel = panel;

    data->confignum = confignum;
    data->ms = ms;
    data->apppointer = app;

    Layout *layout = i_modal_layout(data, app, ms, confignum);
    panel_layout(panel, layout);
    panel_data(panel, &data, i_destroy_modal_data, AddConfigData);
    return panel;
}
