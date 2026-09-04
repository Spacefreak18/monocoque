#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <basedir_fs.h>
#include <libconfig.h>

#include "mgui/uiconfighelper.h"
#include "mgui/addeditaction.h"
#include "mgui/mainwindow.h"

#include "gameloop/gameloop.h"
#include "gameloop/loopdata.h"
#include "gameloop/tachconfig.h"
#include "devices/simdevice.h"
#include "devices/sound.h"
#include "helper/parameters.h"
#include "helper/dirhelper.h"
#include "helper/confighelper.h"
#include "simulatorapi/simapi/simapi/simdata.h"
#include "slog/slog.h"

#include <nappgui.h>
#include <gui/guiall.h>


#define PROGRAM_NAME "monocoque"
MonocoqueSettings ms;
Parameters* p;

char **m_argv;
int m_argc;
int appstate = 0;

void SetSettingsFromParameters(Parameters* p, MonocoqueSettings* ms, char* configdir_str, char* cachedir_str)
{

    if(p->user_specified_config_file == true && does_file_exist(p->config_filepath))
    {
        ms->config_str = strdup(p->config_filepath);
    }
    else
    {
        if(p->user_specified_config_dir == true && does_directory_exist(p->config_dirpath))
        {
            asprintf(&ms->config_str, "%s/%s", p->config_dirpath, "monocoque.config");
        }
        else
        {
            asprintf(&ms->config_str, "%s%s", configdir_str, "monocoque.config");
        }
    }

    if(p->user_specified_log_file == true && does_file_exist(p->log_fullfilename_str))
    {
        ms->log_dirname_str = strdup(p->log_dirname_str);
        ms->log_filename_str = strdup(p->log_filename_str);
    }
    else
    {
        ms->log_dirname_str = strdup(cachedir_str);
        ms->log_filename_str = strdup("monocoque.log");
    }

    ms->fps = p->fps;

    ms->verbosity_count = p->verbosity_count;
    ms->program_action = A_TEST;
    if (p->program_action == A_PLAY)
    {
        ms->program_action = A_PLAY;
    }
    if (p->program_action == A_CONFIG_TACH)
    {
        ms->program_action = A_CONFIG_TACH;
    }

    ms->force_udp_mode = false;
    ms->disable_audio = p->disable_audio;
}
/*---------------------------------------------------------------------------*/

int monocoque_initialize(int argc, char** argv)
{

    char* home_dir_str = gethome();
    if(home_dir_str == NULL)
    {
        fprintf(stderr, "You need a home directory");
        return 0;
    }

    p = NULL;
    // calloc, not malloc: freeparams() walks every pointer in this struct
    // on the early-exit paths, before getParameters has set them. See the
    // same change in monocoque-cli.c.
    p = calloc(1, sizeof(Parameters));
    p->config_dirpath = NULL;
    p->config_filepath = NULL;
    p->log_filename_str = NULL;
    p->log_fullfilename_str = NULL;
    p->log_dirname_str = NULL;

    ConfigError ppe = getParameters(argc, argv, p);
    if (ppe == E_SUCCESS_AND_EXIT || ppe == E_SOMETHING_BAD)
    {
        printf("invalid parameters\n");
        //goto cleanup_final;
    }

    xdgHandle xdg;
    if(!xdgInitHandle(&xdg))
    {
        fprintf(stderr, "Function xdgInitHandle() failed, is $HOME unset?\n");
        //goto cleanup_final;
    }
    const char* config_home_str = xdgConfigHome(&xdg);
    const char* cache_home_str = xdgCacheHome(&xdg);

    char* cachedir_str = NULL;
    char* configdir_str = NULL;

    if(p->user_specified_config_file == false && p->user_specified_config_dir == false)
    {
        create_xdg_dir(config_home_str);
        configdir_str = create_user_dir(home_dir_str, ".config", PROGRAM_NAME);
    }
    if(p->user_specified_log_file == false)
    {
        create_xdg_dir(cache_home_str);
        cachedir_str = create_user_dir(home_dir_str, ".cache", PROGRAM_NAME);
    }

    SetSettingsFromParameters(p, &ms, configdir_str, cachedir_str);

    if(cachedir_str != NULL)
    {
        free(cachedir_str);
    }
    if(configdir_str != NULL)
    {
        free(configdir_str);
    }
  
    slog_init("monocoque", SLOG_FLAGS_ALL, 1);
    slog_config_t slgCfg;
    slog_config_get(&slgCfg);
    slgCfg.eColorFormat = SLOG_COLORING_TAG;
    slgCfg.eDateControl = SLOG_TIME_ONLY;
    strcpy(slgCfg.sFileName, ms.log_filename_str);
    strcpy(slgCfg.sFilePath, ms.log_dirname_str);
    slgCfg.nTraceTid = 0;
    slgCfg.nToScreen = 1;
    slgCfg.nUseHeap = 0;
    slgCfg.nToFile = 1;
    slgCfg.nFlush = 0;
    slgCfg.nFlags = SLOG_FLAGS_ALL;
    slog_config_set(&slgCfg);
    if (ms.verbosity_count < 2)
    {
        slog_disable(SLOG_TRACE);
    }
    if (ms.verbosity_count < 1)
    {
        slog_disable(SLOG_DEBUG);
    }
    xdgWipeHandle(&xdg);




    if(ms.disable_audio == false)
    {
        setupsound();
    }
}
/*---------------------------------------------------------------------------*/

int monocoque_gameloop_start()
{
    start_loop(&ms);
}

/*---------------------------------------------------------------------------*/

static void i_OnStart(App *app, Event *e)
{
    unref(e);
    app->is_running = TRUE;
  
    int rc = 0;
    if(appstate == 0)
    {
        rc = monocoque_gameloop_start();
    }
    else
    {
        monocoque_mainloop_stop(&ms);
    }
}
/*---------------------------------------------------------------------------*/

static void i_OnStop(App *app, Event *e)
{
    monocoque_mainloop_stop(&ms);
}

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    close_monocoque_config(ms.cfg);
    monocoquesettingsfree(&ms);
    
    freeparams(p);
    free(p);

    for (int i = 0; i < (uint32_t)m_argc; ++i)
        free(m_argv[i]);
    free(m_argv);

    osapp_finish();
    unref(app);
    unref(e);
}
/*---------------------------------------------------------------------------*/

static void i_OnAddClose(App *app, Event *e)
{


}

static void i_OnDelete(App* app, Event *e)
{
    int devicenum = listbox_get_selected(app->listbox_devices);
    if(devicenum == UINT32_MAX)
    {
        return;
    }
    delete_device_config(ms.cfg, ms.config_str, 0, devicenum);

    listbox_clear(app->listbox_devices);
    combo_clear(app->combo_configurations);
    populate_device_list(app->listbox_devices, ms.cfg);
    populate_combo_box(app->combo_configurations, ms.cfg);
}
/*---------------------------------------------------------------------------*/

static void i_OnAddEdit(App* app, Event *e, enum device_action device_action)
{
    Panel *panel = NULL;

    int devicenum = listbox_get_selected(app->listbox_devices);
    if(devicenum == UINT32_MAX && device_action == DEVICE_ACTION_EDIT)
    {
        return;
    }
    if(device_action == DEVICE_ACTION_ADD)
    {
        devicenum = listbox_count(app->listbox_devices);
    }
    app->window2 = window_create(ekWINDOW_STDRES);
    
    panel = adddevice_window(app->window2, app, &ms, device_action, 0, devicenum);
    
    window_panel(app->window2, panel);
    window_origin(app->window2, v2df(800, 200));
    window_title(app->window2, "monocoque");
    window_OnClose(app->window2, listener(app, i_OnAddClose, App));
    
    window_modal(app->window2, app->window);

    window_destroy(&app->window2);

    listbox_clear(app->listbox_devices);
    combo_clear(app->combo_configurations);
    populate_device_list(app->listbox_devices, ms.cfg);
    populate_combo_box(app->combo_configurations, ms.cfg);
}
/*---------------------------------------------------------------------------*/

static void i_OnAdd(App *app, Event *e)
{
    i_OnAddEdit(app, e, DEVICE_ACTION_ADD);
}
/*---------------------------------------------------------------------------*/

static void i_OnEdit(App *app, Event *e)
{
    i_OnAddEdit(app, e, DEVICE_ACTION_EDIT);
}

/*---------------------------------------------------------------------------*/


static void set_bottom_label_status(Label* label)
{
    char status_string[256];
    snprintf(status_string, 256, "Game: %s      Game Status: None      Simd: %s", get_simexe_name(), get_simd_onoff());
    label_text(label, status_string);
}
/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app, MonocoqueSettings* ms)
{
    app->main_panel = panel_create();
    app->subpanel = panel_scroll(FALSE, TRUE);

    app->layout = layout_create(8,5);


    app->listbox_devices = listbox_create();
    listbox_size(app->listbox_devices, s2df(50, 50));
    app->combo_configurations = combo_create();
    app->label_configurations = label_create();
    label_text(app->label_configurations, "Configuration: ");
    app->label_game_status = label_create();

    app->button_startstop = button_push();
    app->button_add = button_push();
    app->button_edit = button_push();
    app->button_del = button_push();

    app->layout1 = layout_create(8,1);
    app->layout2 = layout_create(8,1);
    app->layout3 = layout_create(1,1);
    app->layout4 = layout_create(1,2);

    layout_layout(app->layout, app->layout1, 0, 0);
    layout_layout(app->layout, app->layout2, 0, 1);
    layout_layout(app->layout, app->layout3, 0, 2);
    layout_layout(app->layout, app->layout4, 0, 3);

    appstate = 0;

    set_bottom_label_status(app->label_game_status);
    populate_device_list(app->listbox_devices, ms->cfg);
    listbox_select(app->listbox_devices, 0, true);
    populate_combo_box(app->combo_configurations, ms->cfg);


    button_text(app->button_startstop, "Start");
    button_text(app->button_add, "Add Device");
    button_text(app->button_del, "Delete Device");
    button_text(app->button_edit, "Edit Device");
    button_OnClick(app->button_startstop, listener(app, i_OnStart, App));
    button_OnClick(app->button_add, listener(app, i_OnAdd, App));
    button_OnClick(app->button_edit, listener(app, i_OnEdit, App));
    button_OnClick(app->button_del, listener(app, i_OnDelete, App));


    layout_label(app->layout1, app->label_configurations, 0, 0);
    layout_combo(app->layout1, app->combo_configurations, 1, 0);
    
    layout_button(app->layout2, app->button_add, 3, 0);
    layout_button(app->layout2, app->button_edit, 4, 0);
    layout_button(app->layout2, app->button_del, 5, 0);

    layout_listbox(app->layout3, app->listbox_devices, 0, 0);

    layout_button(app->layout4, app->button_startstop, 0, 0);
    layout_label(app->layout4, app->label_game_status, 0, 1);

    panel_layout(app->subpanel, app->layout3);
    layout_panel(app->layout, app->subpanel, 0, 2);
    
    panel_layout(app->main_panel, app->layout);


    app->devices_subpanel = layout_cell(app->layout, 0, 2);

    // spacing

    layout_valign(app->layout4, 0, 0, ekBOTTOM);
    layout_valign(app->layout4, 0, 1, ekBOTTOM);

    layout_halign(app->layout4, 0, 0, ekJUSTIFY);
    layout_halign(app->layout4, 0, 1, ekJUSTIFY);

    layout_hexpand(app->layout, 0);
    layout_hexpand(app->layout4, 0);

    layout_vexpand(app->layout, 0);
    layout_vexpand(app->layout, 2);
    layout_vexpand(app->layout4, 0);

    button_vpadding(app->button_startstop, 0);
    button_hpadding(app->button_startstop, 0);
    layout_hexpand(app->layout1, 1);

    return app->main_panel;
}


static App *i_create(void)
{
    uint32_t i;
    int old_argc = (int)osapp_argc();
    m_argc = old_argc + 1;
    
    m_argv = malloc((m_argc+1) * sizeof(char *));
    m_argv[m_argc] = NULL;
    for (i = 0; i < (uint32_t)m_argc; ++i)
    {
        char buffer[128];
        m_argv[i] = NULL;
        if (i == 1)
        {
            m_argv[i] = strdup("play");
        }
        else
        {
            uint32_t old_i = (i > 1) ? i - 1 : i;
            osapp_argv(old_i, buffer, sizeof(buffer));
            m_argv[i] = strdup(buffer);
        }
    }

    // do some monocoque things here
    ms.tyre_diameter_config = NULL;
    ms.config_str = NULL;
    ms.log_filename_str = NULL;
    ms.log_dirname_str = NULL;

    monocoque_initialize(m_argc, m_argv);
    ms.cfg = open_monocoque_config(ms.config_str);

    // find a better spot for this
    dbind(SimData, uint32_t, velocity);
    dbind(SimData, uint32_t, rpms);
    dbind(SimData, uint32_t, gear);
    dbind(SimData, uint32_t, maxrpm);
    dbind(SimData, real64_t, tyreRPS[0]);
    dbind(SimData, real64_t, tyreRPS[1]);
    dbind(SimData, real64_t, tyreRPS[2]);
    dbind(SimData, real64_t, tyreRPS[3]);
    dbind(SimData, real64_t, tyrediameter[0]);
    dbind(SimData, real64_t, tyrediameter[1]);
    dbind(SimData, real64_t, tyrediameter[2]);
    dbind(SimData, real64_t, tyrediameter[3]);

    dbind(DeviceSettings, HapticEffectSettings, hapticsettings);
    dbind(HapticEffectSettings, uint32_t, frequency);
    dbind(HapticEffectSettings, uint32_t, frequencyMax);
    dbind(HapticEffectSettings, uint32_t, amplitude);
    dbind(HapticEffectSettings, uint32_t, amplitudeMax);
    dbind(HapticEffectSettings, uint32_t, motorposition);
    dbind(HapticEffectSettings, real64_t, threshold);
    dbind(HapticEffectSettings, real64_t, duration);
    dbind_enum(VibrationEffectType, EFFECT_ENGINERPM,  "Engine RPM");
    dbind_enum(VibrationEffectType, EFFECT_GEARSHIFT,  "Gear Shift");
    dbind_enum(VibrationEffectType, EFFECT_ABSBRAKES,  "ABS Brakes");
    dbind_enum(VibrationEffectType, EFFECT_TYRESLIP,   "Tyre Slip");
    dbind_enum(VibrationEffectType, EFFECT_TYRELOCK,   "Tyre Lock");
    dbind_enum(VibrationEffectType, EFFECT_SUSPENSION, "Suspension");
    dbind_enum(EffectModulationType, EFFECT_MODULATION_NONE, "None");
    dbind_enum(EffectModulationType, EFFECT_MODULATION_FREQUENCY, "Frequency");
    dbind_enum(EffectModulationType, EFFECT_MODULATION_AMPLIFY, "Amplify");
    dbind_enum(MonocoqueTyreIdentifier, FRONTLEFT, "Front Left");
    dbind_enum(MonocoqueTyreIdentifier, FRONTRIGHT, "Front Right");
    dbind_enum(MonocoqueTyreIdentifier, REARLEFT, "Rear Left");
    dbind_enum(MonocoqueTyreIdentifier, REARRIGHT, "Rear Right");
    dbind_enum(MonocoqueTyreIdentifier, FRONTS, "Fronts");
    dbind_enum(MonocoqueTyreIdentifier, REARS, "Rears");
    dbind_enum(MonocoqueTyreIdentifier, ALLFOUR, "All Four");
    dbind(HapticEffectSettings, uint32_t, modulation);
    dbind(HapticEffectSettings, uint32_t, tyre);
    dbind(HapticEffectSettings, uint32_t, effect_type);
    dbind(DeviceSettings, SerialDeviceSettings, serialdevsettings);
    dbind(SerialDeviceSettings, uint32_t, baud);
    dbind(SoundDeviceSettings, uint32_t, pan);
    dbind(SoundDeviceSettings, uint32_t, channels);
    dbind(SoundDeviceSettings, uint32_t, volume);
    dbind(DeviceSettings, SerialDeviceSettings, serialdevsettings);
    dbind(SerialDeviceSettings, uint32_t, startled);
    dbind(SerialDeviceSettings, uint32_t, numleds);
    dbind(SerialDeviceSettings, uint32_t, endled);
    dbind_enum(DeviceType, SIMDEV_USB, "USB");
    dbind_enum(DeviceType, SIMDEV_SOUND, "Sound");
    dbind_enum(DeviceType, SIMDEV_SERIAL, "Serial");
    dbind(DeviceSettings, DeviceType, dev_type);
    dbind(DeviceSettings, uint32_t, dev_subtype);
    dbind(DeviceSettings, uint32_t, fps);
    //gui stuff
    App *app = heap_new0(App);

    Panel *panel = i_panel(app, &ms);
    app->window = window_create(ekWINDOW_STDRES);
    window_panel(app->window, panel);
    window_origin(app->window, v2df(800, 200));
    window_title(app->window, "monocoque");
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);

    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    //monocoquesettingsfree(&ms);
    //freeparams(p);
    //free(p);
    osapp_finish();
    window_destroy(&(*app)->window);
    heap_delete(app, App);
}


/*---------------------------------------------------------------------------*/



static void i_update(App *app, const real64_t prtime, const real64_t ctime)
{
    if (app->is_running == TRUE)
    {
        if(appstate == 2)
        {
            button_text(app->button_startstop, "Playing... Press to Stop");
            cell_enabled(app->devices_subpanel, false);
        }
        if(appstate == 1)
        {
            button_text(app->button_startstop, "Searching for data... Press to Stop");
            cell_enabled(app->devices_subpanel, false);
        }
        if(appstate <= 0)
        {
            button_text(app->button_startstop, "Start");
            cell_enabled(app->devices_subpanel, true);
        }
        set_bottom_label_status(app->label_game_status);
    }

}

/*---------------------------------------------------------------------------*/

#include <osapp/osmain.h>
osmain_sync(.04, i_create, i_destroy, i_update, "", App)
