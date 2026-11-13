#include <string.h>

#include <nappgui.h>
#include <gui/guiall.h>

#include "addeditaction.h"
#include "testaction.h"
#include "mainwindow.h"

#include "uiconfighelper.h"
#include "../helper/confighelper.h"
#include "../gameloop/gameloop.h"
#include "../gameloop/loopdata.h"
#include "../slog/slog.h"


test_loop_args* test_loop_data;

/*---------------------------------------------------------------------------*/

static Layout *i_modal_layout(AddDeviceData *data, App* app, MonocoqueSettings* ms, enum device_action dev_action, int confignum, int devicenum);

/*---------------------------------------------------------------------------*/

static AddDeviceData *i_modal_data(Window *parent)
{
    AddDeviceData *data = heap_new0(AddDeviceData);
    data->parent = parent;
    data->type = UINT32_MAX;
    return data;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_modal_data(AddDeviceData **data)
{
    AddDeviceData* d = *data;
    DeviceSettings ds = *d->ds;
    settingsfree(ds);
    free(d->ds);
    heap_delete(data, AddDeviceData);
}


/*---------------------------------------------------------------------------*/

static void i_OnTestClose(App *app, Event *e)
{

}

static void update_device_data_from_controls(AddDeviceData* data)
{
    data->ds->dev_type = combo_get_selected(data->combo_select_device_class);
    data->ds->dev_subtype = combo_get_selected(data->combo_select_device_subtype);
    uint32_t device_selected = combo_get_selected(data->combo_select_device);
    data->ds->dev = strdup(combo_get_text(data->combo_select_device, device_selected));
    if(data->ds->dev_type == SIMDEV_SERIAL)
    {
        data->ds->dev_subtype = combo_get_selected(data->combo_select_device_subtype) + SerialDevicesOffset;
    }
    if(data->ds->dev_type == SIMDEV_USB)
    {
        data->ds->dev_subtype = combo_get_selected(data->combo_select_device_subtype) + USBDevicesOffset;
    }
    data->ds->hapticsettings.tyre = combo_get_selected(data->combo_select_tyre);
    data->ds->hapticsettings.effect_type = combo_get_selected(data->combo_select_effect);
    data->ds->hapticsettings.modulation = combo_get_selected(data->combo_select_modulation);
}


static void i_OnTestDevice(AddDeviceData *data, Event *e)
{
    update_device_data_from_controls(data);

    Panel *panel = NULL;
    
    data->apppointer->window3 = window_create(ekWINDOW_STDRES);
    
    SimData* t_simdata = malloc(sizeof(SimData));
    
    set_basic_simdata(t_simdata);
    set_wheel_spin_simdata(t_simdata);
    if(data->ds->has_haptic_effects == true)
    {
        if(data->ds->hapticsettings.effect_type == EFFECT_TYRELOCK)
        {
            set_wheel_lock_simdata(t_simdata);
        }
    }

    
    test_loop_data = malloc(sizeof(test_loop_args));
    test_loop_data->ms = data->ms;
    test_loop_data->confignum = data->confignum;
    test_loop_data->devicenum = data->devicenum;
    test_loop_data->ds = data->ds;
    test_loop_data->simdata = t_simdata;
    
    start_test(test_loop_data);
    panel = test_window(data->apppointer->window3, data->apppointer);
    
    window_panel(data->apppointer->window3, panel);
    window_origin(data->apppointer->window3, v2df(800, 200));
    window_title(data->apppointer->window3, "monocoque");
    window_OnClose(data->apppointer->window3, listener(data->apppointer, i_OnTestClose, App));
    
    window_modal(data->apppointer->window3, data->apppointer->window);
    
    window_destroy(&data->apppointer->window3);
    
    free(t_simdata);
    free(test_loop_data);
}

static void addHapticDeviceLayout(AddDeviceData* data)
{
    struct _add_device_t* subwindow = data;
    subwindow->haptic_effect_sublayout = layout_create(8, 3);
    
    subwindow->label_select_effect = label_create();
    subwindow->combo_select_effect = combo_create();
    subwindow->label_select_modulation = label_create();
    subwindow->combo_select_modulation = combo_create();
    subwindow->label_select_tyre = label_create();
    subwindow->combo_select_tyre = combo_create();
    subwindow->label_motorposition = label_create();
    subwindow->edit_motorposition = edit_create();
    
    subwindow->label_frequency = label_create();
    subwindow->edit_frequency = edit_create();
    subwindow->label_amplitude = label_create();
    subwindow->edit_amplitude = edit_create();

    subwindow->label_frequencyMax = label_create();
    subwindow->edit_frequencyMax = edit_create();
    subwindow->label_amplitudeMax = label_create();
    subwindow->edit_amplitudeMax = edit_create();
    subwindow->label_threshold = label_create();
    subwindow->edit_threshold = edit_create();
    subwindow->label_duration = label_create();
    subwindow->edit_duration = edit_create();
    
    label_text(subwindow->label_select_effect, "Effect:");
    label_text(subwindow->label_select_modulation, "Modulation:");
    label_text(subwindow->label_select_tyre, "Tyre:");
    label_text(subwindow->label_motorposition, "Motor Position:");
    label_text(subwindow->label_frequency, "Frequency:");
    label_text(subwindow->label_amplitude, "Amplitude:");
    label_text(subwindow->label_frequencyMax, "Max Frequency:");
    label_text(subwindow->label_amplitudeMax, "Max Amplitude:");
    label_text(subwindow->label_threshold, "Threshold:");
    label_text(subwindow->label_duration, "Duration:");
    
    layout_label(subwindow->haptic_effect_sublayout, subwindow->label_select_effect, 0, 0);
    layout_combo(subwindow->haptic_effect_sublayout, subwindow->combo_select_effect, 1, 0);
    layout_label(subwindow->haptic_effect_sublayout, subwindow->label_select_modulation, 2, 0);
    layout_combo(subwindow->haptic_effect_sublayout, subwindow->combo_select_modulation, 3, 0);
    layout_label(subwindow->haptic_effect_sublayout, subwindow->label_select_tyre, 4, 0);
    layout_combo(subwindow->haptic_effect_sublayout, subwindow->combo_select_tyre, 5, 0);
    layout_label(subwindow->haptic_effect_sublayout, subwindow->label_motorposition, 6, 0);
    layout_edit(subwindow->haptic_effect_sublayout, subwindow->edit_motorposition, 7, 0);

    combo_add_elem(subwindow->combo_select_effect, "Engine", NULL);
    combo_add_elem(subwindow->combo_select_effect, "Gear", NULL);
    combo_add_elem(subwindow->combo_select_effect, "ABS", NULL);
    combo_add_elem(subwindow->combo_select_effect, "TyreSlip", NULL);
    combo_add_elem(subwindow->combo_select_effect, "TyreLock", NULL);
    combo_add_elem(subwindow->combo_select_effect, "Suspension", NULL);

    combo_add_elem(subwindow->combo_select_modulation, "None", NULL);
    combo_add_elem(subwindow->combo_select_modulation, "Frequency", NULL);
    combo_add_elem(subwindow->combo_select_modulation, "Amplitude", NULL);

    combo_add_elem(subwindow->combo_select_tyre, "FrontLeft", NULL);
    combo_add_elem(subwindow->combo_select_tyre, "FrontRight", NULL);
    combo_add_elem(subwindow->combo_select_tyre, "RearLeft", NULL);
    combo_add_elem(subwindow->combo_select_tyre, "RearRight", NULL);
    combo_add_elem(subwindow->combo_select_tyre, "Fronts", NULL);
    combo_add_elem(subwindow->combo_select_tyre, "Rears", NULL);
    combo_add_elem(subwindow->combo_select_tyre, "All", NULL);

    layout_label(subwindow->haptic_effect_sublayout, subwindow->label_frequency, 0, 1);
    layout_edit(subwindow->haptic_effect_sublayout, subwindow->edit_frequency, 1, 1);
    layout_label(subwindow->haptic_effect_sublayout, subwindow->label_amplitude, 2, 1);
    layout_edit(subwindow->haptic_effect_sublayout, subwindow->edit_amplitude, 3, 1);
    layout_label(subwindow->haptic_effect_sublayout, subwindow->label_frequencyMax, 6, 1);
    layout_edit(subwindow->haptic_effect_sublayout, subwindow->edit_frequencyMax, 7, 1);
    
    layout_label(subwindow->haptic_effect_sublayout, subwindow->label_amplitudeMax, 0, 2);
    layout_edit(subwindow->haptic_effect_sublayout, subwindow->edit_amplitudeMax, 1, 2);
    layout_label(subwindow->haptic_effect_sublayout, subwindow->label_threshold, 2, 2);
    layout_edit(subwindow->haptic_effect_sublayout, subwindow->edit_threshold, 3, 2);
    layout_label(subwindow->haptic_effect_sublayout, subwindow->label_duration, 4, 2);
    layout_edit(subwindow->haptic_effect_sublayout, subwindow->edit_duration, 5, 2);



    cell_dbind(layout_cell(subwindow->haptic_effect_sublayout, 1, 1), HapticEffectSettings, uint32_t, frequency);
    cell_dbind(layout_cell(subwindow->haptic_effect_sublayout, 3, 1), HapticEffectSettings, uint32_t, amplitude);
    cell_dbind(layout_cell(subwindow->haptic_effect_sublayout, 7, 1), HapticEffectSettings, uint32_t, frequencyMax);
    cell_dbind(layout_cell(subwindow->haptic_effect_sublayout, 1, 2), HapticEffectSettings, uint32_t, amplitudeMax);
    cell_dbind(layout_cell(subwindow->haptic_effect_sublayout, 7, 0), HapticEffectSettings, uint32_t, motorposition);
    cell_dbind(layout_cell(subwindow->haptic_effect_sublayout, 3, 2), HapticEffectSettings, real64_t, threshold);
    cell_dbind(layout_cell(subwindow->haptic_effect_sublayout, 5, 2), HapticEffectSettings, real64_t, duration);
    
    cell_dbind(layout_cell(subwindow->haptic_effect_sublayout, 3, 0), HapticEffectSettings, uint32_t, modulation);
    cell_dbind(layout_cell(subwindow->haptic_effect_sublayout, 5, 0), HapticEffectSettings, uint32_t, tyre);
    cell_dbind(layout_cell(subwindow->haptic_effect_sublayout, 1, 0), HapticEffectSettings, uint32_t, effect_type);
    layout_dbind(subwindow->haptic_effect_sublayout, NULL, HapticEffectSettings);
    layout_dbind_obj(subwindow->haptic_effect_sublayout, &data->ds->hapticsettings, HapticEffectSettings);

    // can i get this from the dbind
    combo_selected(subwindow->combo_select_tyre, data->ds->hapticsettings.tyre);
    combo_selected(subwindow->combo_select_effect, data->ds->hapticsettings.effect_type);
    combo_selected(subwindow->combo_select_modulation, data->ds->hapticsettings.modulation);

    for (uint32_t row = 0; row < 3; row++)
    {
        for (uint32_t col = 1; col < 8; col += 2)
        {
            layout_halign(subwindow->haptic_effect_sublayout, col, row, ekJUSTIFY);
        }
    }

}

static void addSerialDeviceLayout(AddDeviceData* data)
{
    struct _add_device_t* subwindow = data;
    subwindow->serial_device_sublayout = layout_create(4, 1);    
    subwindow->edit_baud = edit_create();
    subwindow->label_baud = label_create();
    label_text(subwindow->label_baud, "baud: ");
    layout_label(subwindow->serial_device_sublayout, subwindow->label_baud, 0, 0);
    layout_edit(subwindow->serial_device_sublayout, subwindow->edit_baud, 1, 0);

    cell_dbind(layout_cell(subwindow->serial_device_sublayout, 1, 0), SerialDeviceSettings, uint32_t, baud);
    layout_dbind(subwindow->serial_device_sublayout, NULL, SerialDeviceSettings);
    layout_dbind_obj(subwindow->serial_device_sublayout, &data->ds->serialdevsettings, SerialDeviceSettings);
    
    layout_update(subwindow->serial_device_sublayout);
}

static void addSoundDeviceLayout(AddDeviceData* data)
{
    struct _add_device_t* subwindow = data;

    subwindow->sound_device_sublayout = layout_create(6, 1);
    
    subwindow->edit_channel = edit_create();
    subwindow->edit_channels = edit_create();
    subwindow->label_channel = label_create();
    subwindow->label_channels = label_create();
    subwindow->label_volume = label_create();
    subwindow->edit_volume = edit_create();
    
    label_text(subwindow->label_channel, "Channel: ");
    label_text(subwindow->label_channels, "Channels: ");
    label_text(subwindow->label_volume, "Volume:");
    
    layout_label(subwindow->sound_device_sublayout, subwindow->label_channel, 0, 0);
    layout_label(subwindow->sound_device_sublayout, subwindow->label_channels, 2, 0);
    layout_label(subwindow->sound_device_sublayout, subwindow->label_volume, 4, 0);
    
    layout_edit(subwindow->sound_device_sublayout, subwindow->edit_channel, 1, 0);
    layout_edit(subwindow->sound_device_sublayout, subwindow->edit_channels, 3, 0);
    layout_edit(subwindow->sound_device_sublayout, subwindow->edit_volume, 5, 0);
    

    cell_dbind(layout_cell(subwindow->sound_device_sublayout, 1, 0), SoundDeviceSettings, uint32_t, pan);
    cell_dbind(layout_cell(subwindow->sound_device_sublayout, 3, 0), SoundDeviceSettings, uint32_t, channels);
    cell_dbind(layout_cell(subwindow->sound_device_sublayout, 5, 0), SoundDeviceSettings, uint32_t, volume);
    layout_dbind(subwindow->sound_device_sublayout, NULL, SoundDeviceSettings);
    layout_dbind_obj(subwindow->sound_device_sublayout, &data->ds->sounddevsettings, SoundDeviceSettings);
    

    layout_update(subwindow->sound_device_sublayout);
}

static void addLedDeviceLayout(AddDeviceData* data)
{
    struct _add_device_t* subwindow = data;

    subwindow->led_device_sublayout = layout_create(6,1);
    
    subwindow->edit_num_leds = edit_create();
    subwindow->edit_start_led = edit_create();
    subwindow->edit_end_led = edit_create();
    
    subwindow->label_num_leds = label_create();
    subwindow->label_start_led = label_create();
    subwindow->label_end_led = label_create();
    
    
    label_text(subwindow->label_num_leds, "Leds: ");
    label_text(subwindow->label_start_led, "Start Led: ");
    label_text(subwindow->label_end_led, "End Led: ");
    
    layout_label(subwindow->led_device_sublayout, subwindow->label_num_leds, 0, 0);
    layout_label(subwindow->led_device_sublayout, subwindow->label_start_led, 2, 0);
    layout_label(subwindow->led_device_sublayout, subwindow->label_end_led, 4, 0);
    
    layout_edit(subwindow->led_device_sublayout, subwindow->edit_num_leds, 1, 0);
    layout_edit(subwindow->led_device_sublayout, subwindow->edit_start_led, 3, 0);
    layout_edit(subwindow->led_device_sublayout, subwindow->edit_end_led, 5, 0);
    
    

    
    
    cell_dbind(layout_cell(subwindow->led_device_sublayout, 1, 0), SerialDeviceSettings, uint32_t, numleds);
    cell_dbind(layout_cell(subwindow->led_device_sublayout, 3, 0), SerialDeviceSettings, uint32_t, startled);
    cell_dbind(layout_cell(subwindow->led_device_sublayout, 5, 0), SerialDeviceSettings, uint32_t, endled);
    layout_dbind(subwindow->led_device_sublayout, NULL, SerialDeviceSettings);
    layout_dbind_obj(subwindow->led_device_sublayout, &data->ds->serialdevsettings, SerialDeviceSettings);
    

    layout_update(subwindow->led_device_sublayout);
}

/*---------------------------------------------------------------------------*/

static void i_OnSaveDevice(AddDeviceData *data, Event *e)
{
    update_device_data_from_controls(data);
    slogd("Attempting save of device class %i device type %i device %s", data->ds->dev_type, data->ds->dev_subtype, data->ds->dev);
    if(data->ds->has_haptic_effects == true) {
        fprintf(stderr, "it does\n");
}
    save_device_config(data->ms->cfg, data->ms->config_str, data->confignum, data->devicenum, data->ds);


    window_stop_modal(data->parent, 0);
}

static void populateUSBTachometerTypesCombo(Combo* combo)
{
    combo_add_elem(combo, "RevBurner", NULL);
}

static void populateUSBWheelTypesCombo(Combo* combo)
{
    combo_add_elem(combo, "CammusC5", NULL);
    combo_add_elem(combo, "CammusC12", NULL);
    combo_add_elem(combo, "MozaR5", NULL);
    combo_add_elem(combo, "CSLEliteV3Pedals", NULL);
    combo_add_elem(combo, "SimMagicP1000Pedals", NULL);
    combo_add_elem(combo, "SimMagicGTNEO", NULL);
    combo_add_elem(combo, "Moza_New", NULL);
    combo_add_elem(combo, "LogitechG29", NULL);
    combo_add_elem(combo, "Moza_KS_PRO_Wheel", NULL);
    combo_add_elem(combo, "SimNetPedals", NULL);
}

static void populateSerialSubTypesCombo(Combo* combo)
{
    combo_add_elem(combo, "ShiftLights", NULL);
    combo_add_elem(combo, "SimWind", NULL);
    combo_add_elem(combo, "Haptic", NULL);
    combo_add_elem(combo, "Wheel", NULL);
    combo_add_elem(combo, "SimLed", NULL);
    combo_add_elem(combo, "Custom", NULL);
}

static void populateUSBSubTypesCombo(Combo* combo)
{
    combo_add_elem(combo, "Tachometer", NULL);
    combo_add_elem(combo, "Haptic", NULL);
    combo_add_elem(combo, "Wheel", NULL);
}

static void i_OnDeviceSubtypeSelect(AddDeviceData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    cell_visible(data->cell_led_device_layout, false);
    cell_visible(data->cell_haptic_effect_layout, false);
    data->ds->has_led_effects = false;
    data->ds->has_haptic_effects = false;

    uint32_t class_selected = combo_get_selected(data->combo_select_device_class);
    const char_t *device_class = combo_get_text(data->combo_select_device_class, class_selected);
    uint32_t selected = combo_get_selected(data->combo_select_device_subtype);
    const char_t *text = combo_get_text(data->combo_select_device_subtype, selected);
    
    if (text != NULL && strcicmp(text, "SIMLED") == 0)
    {
        cell_visible(data->cell_led_device_layout, true);
        data->ds->has_led_effects = true;
    }
    if (text != NULL && strcicmp(text, "HAPTIC") == 0)
    {
        cell_visible(data->cell_haptic_effect_layout, true);
        data->ds->has_haptic_effects = true;
    }
    if(device_class != NULL && strcicmp(device_class, "USB") == 0)
    {
        combo_clear(data->combo_select_device);
        combo_text(data->combo_select_device, "");
        if (text != NULL && (strcicmp(text, "WHEEL") == 0 || strcicmp(text, "USBWHEEL") == 0) )
        {
            populateUSBWheelTypesCombo(data->combo_select_device);
        }
        if (text != NULL && (strcicmp(text, "TACHOMETER") == 0 || strcicmp(text, "USBTACHOMETER") == 0) )
        {
            populateUSBTachometerTypesCombo(data->combo_select_device);
        }
    }
    panel_update(data->panel);
}



static void i_OnDeviceClassSelect(AddDeviceData *data, Event *e)
{
    data->ds->has_led_effects = false;
    data->ds->has_haptic_effects = false;

    const EvButton *p = event_params(e, EvButton);

    panel_visible_layout(data->device_specific_panel, 0);
    cell_visible(data->cell_led_device_layout, false);
    cell_visible(data->cell_haptic_effect_layout, false);
    panel_update(data->device_specific_panel);
    combo_clear(data->combo_select_device);
    combo_text(data->combo_select_device, "");
    combo_clear(data->combo_select_device_subtype);
    combo_text(data->combo_select_device_subtype, "");
    uint32_t selected = combo_get_selected(data->combo_select_device_class);
    const char_t *text = combo_get_text(data->combo_select_device_class, selected);
    if (text != NULL && strcicmp(text, "SERIAL") == 0)
    {
        add_serial_devices_to_combo(data->combo_select_device);
        populateSerialSubTypesCombo(data->combo_select_device_subtype);
        panel_visible_layout(data->device_specific_panel, 1);
        // since SIMLED will be selected we will show led settings
        cell_visible(data->cell_led_device_layout, true);
        data->ds->has_led_effects = true;
    }
    if (text != NULL && strcicmp(text, "SOUND") == 0)
    {
        add_sound_devices_to_combo(data->combo_select_device);
        combo_add_elem(data->combo_select_device_subtype, "Haptic", NULL);
        panel_visible_layout(data->device_specific_panel, 2);
        // since HAPTIC will be selected we will show haptic settings
        cell_visible(data->cell_haptic_effect_layout, true);
        data->ds->has_haptic_effects = true;
    }
    if (text != NULL && strcicmp(text, "USB") == 0)
    {
        //TODO: eventually add device detection or a different approach, this drop down will become untenable
        populateUSBSubTypesCombo(data->combo_select_device_subtype);
        populateUSBTachometerTypesCombo(data->combo_select_device);
    }
    combo_selected(data->combo_select_device, 0);
    combo_selected(data->combo_select_device_subtype, 0);
    panel_update(data->device_specific_panel);
    panel_update(data->panel);
}

static Panel* device_specific_panel_create(AddDeviceData *data)
{
    Panel *panel = panel_create();
    data->empty_device_sublayout = layout_create(4, 1);

    addSerialDeviceLayout(data);
    addSoundDeviceLayout(data);

    panel_layout(panel, data->empty_device_sublayout);
    panel_layout(panel, data->serial_device_sublayout);
    panel_layout(panel, data->sound_device_sublayout);
    return panel;
}


static Layout *i_modal_layout(AddDeviceData *data, App* app, MonocoqueSettings* ms, enum device_action dev_action, int confignum, int devicenum)
{

    struct _add_device_t* subwindow = data;

    subwindow->main_outer_layout = layout_create(1,11);
    subwindow->top_selection_layout = layout_create(4,1);
    subwindow->general_settings_layout = layout_create(4, 1);


    // device class specific settings ( max 1 row so far )
    // possible led specific settings ( 1 row )
    // possible haptic specific settings ( 3 rows )

    subwindow->add_save_test_layout = layout_create(2,1);

    subwindow->combo_select_device_class = combo_create();
    subwindow->combo_select_device_subtype = combo_create();
    subwindow->combo_select_device = combo_create();
    subwindow->label_select_device_class = label_create();
    subwindow->label_select_device_subtype = label_create();
    subwindow->label_select_device = label_create();
    label_text(subwindow->label_select_device_class, "Device Class: ");
    label_text(subwindow->label_select_device_subtype, "Device SubType: ");
    label_text(subwindow->label_select_device, "Device: ");
    layout_label(subwindow->top_selection_layout, subwindow->label_select_device_class, 0, 0);
    layout_combo(subwindow->top_selection_layout, subwindow->combo_select_device_class, 1, 0);
    layout_label(subwindow->top_selection_layout, subwindow->label_select_device_subtype, 2, 0);
    layout_combo(subwindow->top_selection_layout, subwindow->combo_select_device_subtype, 3, 0);



    layout_label(subwindow->general_settings_layout, subwindow->label_select_device, 2, 0);
    layout_combo(subwindow->general_settings_layout, subwindow->combo_select_device, 3, 0);
    subwindow->edit_fps = edit_create();
    subwindow->label_fps = label_create();
    label_text(subwindow->label_fps, "fps: ");
    layout_label(subwindow->general_settings_layout, subwindow->label_fps, 0, 0);
    layout_edit(subwindow->general_settings_layout, subwindow->edit_fps, 1, 0);


    cell_dbind(layout_cell(subwindow->top_selection_layout, 1, 0), DeviceSettings, uint32_t, dev_type);
    cell_dbind(layout_cell(subwindow->top_selection_layout, 3, 0), DeviceSettings, uint32_t, dev_subtype);
    cell_dbind(layout_cell(subwindow->general_settings_layout, 1, 0), DeviceSettings, uint32_t, fps);
    layout_dbind(subwindow->general_settings_layout, NULL, DeviceSettings);
    layout_dbind(subwindow->top_selection_layout, NULL, DeviceSettings);
    layout_dbind_obj(subwindow->general_settings_layout, data->ds, DeviceSettings);

    subwindow->device_specific_panel = device_specific_panel_create(data);
    panel_visible_layout(data->device_specific_panel, 0);

    addLedDeviceLayout(data);
    addHapticDeviceLayout(data);


    subwindow->test_button = button_push();
    subwindow->save_button = button_push(); 
    button_text(subwindow->test_button, "Test");
    button_text(subwindow->save_button, "Save");
    layout_button(subwindow->add_save_test_layout, subwindow->test_button, 0, 0);
    layout_button(subwindow->add_save_test_layout, subwindow->save_button, 1, 0);
    button_OnClick(subwindow->test_button, listener(data, i_OnTestDevice, AddDeviceData));
    button_OnClick(subwindow->save_button, listener(data, i_OnSaveDevice, AddDeviceData));



    layout_layout(subwindow->main_outer_layout, subwindow->top_selection_layout, 0, 0);
    layout_layout(subwindow->main_outer_layout, subwindow->general_settings_layout, 0, 1);
    layout_panel(subwindow->main_outer_layout, subwindow->device_specific_panel, 0, 2);
    layout_layout(subwindow->main_outer_layout, subwindow->led_device_sublayout, 0, 3);
    layout_layout(subwindow->main_outer_layout, subwindow->haptic_effect_sublayout, 0, 4);
    layout_layout(subwindow->main_outer_layout, subwindow->add_save_test_layout, 0, 7);

    subwindow->cell_led_device_layout = layout_cell(subwindow->main_outer_layout, 0, 3);
    subwindow->cell_haptic_effect_layout = layout_cell(subwindow->main_outer_layout, 0, 4);
    cell_visible(subwindow->cell_led_device_layout, false);
    cell_visible(subwindow->cell_haptic_effect_layout, false);
    

    if(dev_action == DEVICE_ACTION_ADD)
    {
        combo_add_elem(subwindow->combo_select_device_class, "USB", NULL);
        combo_add_elem(subwindow->combo_select_device_class, "Sound", NULL);
        combo_add_elem(subwindow->combo_select_device_class, "Serial", NULL);


        combo_OnSelect(subwindow->combo_select_device_class, listener(data, i_OnDeviceClassSelect, AddDeviceData));
        combo_OnSelect(subwindow->combo_select_device_subtype, listener(data, i_OnDeviceSubtypeSelect, AddDeviceData));

        combo_selected(data->combo_select_device_class, 1);
        add_sound_devices_to_combo(data->combo_select_device);
        combo_add_elem(data->combo_select_device_subtype, "Haptic", NULL);
        data->ds->has_haptic_effects = true;
        combo_selected(subwindow->combo_select_device_subtype, data->ds->dev_subtype);
        panel_visible_layout(data->device_specific_panel, 2);
        cell_visible(data->cell_haptic_effect_layout, true);

        return subwindow->main_outer_layout;
    }

    char str0[255];
    sprintf(str0, "%s", data->ds->dev);
    combo_add_elem(subwindow->combo_select_device, str0, NULL);
    

    subwindow->cell_top_selection_layout = layout_cell(subwindow->main_outer_layout, 0, 0);
    cell_enabled(subwindow->cell_top_selection_layout, false);

    if(data->ds->dev_type == SIMDEV_SERIAL)
    {
        combo_add_elem(subwindow->combo_select_device_class, "Serial", NULL);
        add_serial_devices_to_combo(data->combo_select_device);
        populateSerialSubTypesCombo(data->combo_select_device_subtype);
        combo_selected(subwindow->combo_select_device_subtype, data->ds->dev_subtype - SerialDevicesOffset);
        panel_visible_layout(data->device_specific_panel, 1);
    }
    if(data->ds->dev_type == SIMDEV_SOUND)
    {
        combo_add_elem(subwindow->combo_select_device_class, "Sound", NULL);
        add_sound_devices_to_combo(data->combo_select_device);
        combo_add_elem(data->combo_select_device_subtype, "Haptic", NULL);
        combo_selected(subwindow->combo_select_device_subtype, data->ds->dev_subtype);
        panel_visible_layout(data->device_specific_panel, 2);
        cell_visible(data->cell_haptic_effect_layout, true);
    }
    if(data->ds->dev_type == SIMDEV_USB)
    {
        populateUSBSubTypesCombo(data->combo_select_device_subtype);
        populateUSBTachometerTypesCombo(data->combo_select_device);
        //TODO: eventually add device detection or a different approach, this drop down will become untenable
        combo_selected(subwindow->combo_select_device_subtype, data->ds->dev_subtype - USBDevicesOffset);
        //panel_visible_layout(data->device_specific_panel, 2);
    }

    if(data->ds->dev_subtype == SIMDEVTYPE_SIMLED) {
        cell_visible(data->cell_led_device_layout, true);
    }
    
    combo_selected(subwindow->combo_select_device_class, 0);
    return subwindow->main_outer_layout;
}

/*---------------------------------------------------------------------------*/

Panel *adddevice_window(Window *parent, App* app, MonocoqueSettings* ms, enum device_action dev_action, int confignum, int devicenum)
{
    Panel *panel = panel_create();
    AddDeviceData *data = i_modal_data(parent);
    data->panel = panel;

    data->devicenum = devicenum;
    data->confignum = confignum;
    data->ds = malloc(sizeof(DeviceSettings));

    // move this somewhere better or use dbind to set this stuff
    data->ds->dev_type = 0;
    data->ds->dev_subtype = 0;
    data->ds->dev_subsubtype = 0;
    data->ds->dev = NULL;
    data->ds->fps = 60;
    data->ds->specific_config_file = NULL;
    data->ds->has_config = false;
    data->ds->serialdevsettings.baud = 115200;
    data->ds->serialdevsettings.startled = 0;
    data->ds->serialdevsettings.endled = 0;
    data->ds->serialdevsettings.numleds = 0;
    data->ds->sounddevsettings.channels = 2;
    data->ds->sounddevsettings.pan = 0;
    data->ds->sounddevsettings.volume = 50;
    data->ds->sounddevsettings.noise = 0;
    data->ds->hapticsettings.frequency = 440;
    data->ds->hapticsettings.amplitude = 50;
    data->ds->hapticsettings.frequencyMax = 440;
    data->ds->hapticsettings.amplitudeMax = 50;
    data->ds->hapticsettings.motorposition = 0;
    data->ds->hapticsettings.threshold = 0.0;
    data->ds->hapticsettings.duration = 0.0;
    data->ds->hapticsettings.modulation = 1;
    data->ds->hapticsettings.tyre = 0;
    data->ds->hapticsettings.effect_type = 0;
    data->ds->has_led_effects = false;
    data->ds->has_haptic_effects = false;

    data->ms = ms;
    data->apppointer = app;
    if(dev_action == DEVICE_ACTION_EDIT)
    {
        int numdevices = getsingledevice(ms->config_str, confignum, devicenum, ms, data->ds);
    }

    Layout *layout = i_modal_layout(data, app, ms, dev_action, confignum, devicenum);
    panel_layout(panel, layout);
    panel_data(panel, &data, i_destroy_modal_data, AddDeviceData);
    return panel;
}
