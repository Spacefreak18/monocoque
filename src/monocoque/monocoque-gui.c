#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <basedir_fs.h>
#include <libconfig.h>

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

#define PROGRAM_NAME "monocoque"
MonocoqueSettings* ms;

int appstate = 0;

void display_banner()
{
    printf("______  ______________   ___________________________________  ___________\n");
    printf("___   |/  /_  __ \\__  | / /_  __ \\_  ____/_  __ \\_  __ \\_  / / /__  ____/\n");
    printf("__  /|_/ /_  / / /_   |/ /_  / / /  /    _  / / /  / / /  / / /__  __/   \n");
    printf("_  /  / / / /_/ /_  /|  / / /_/ // /___  / /_/ // /_/ // /_/ / _  /___   \n");
    printf("/_/  /_/  \\____/ /_/ |_/  \\____/ \\____/  \\____/ \\___\\_\\\\____/  /_____/   \n");
}

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

int monocoque_main(int argc, char** argv)
{
    char* home_dir_str = gethome();
    if(home_dir_str == NULL)
    {
        fprintf(stderr, "You need a home directory");
        return 0;
    }
    Parameters* p = NULL;
    p = malloc(sizeof(Parameters));
    ms = malloc(sizeof(MonocoqueSettings));;

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

    fprintf(stderr, "applying settings\n");
    SetSettingsFromParameters(p, ms, configdir_str, cachedir_str);

    if(cachedir_str != NULL)
    {
      free(cachedir_str);
    }
    if(configdir_str != NULL)
    {
      free(configdir_str);
    }
    fprintf(stderr, "settings applied\n");

  
    slog_init("monocoque", SLOG_FLAGS_ALL, 1);
    slog_config_t slgCfg;
    slog_config_get(&slgCfg);
    slgCfg.eColorFormat = SLOG_COLORING_TAG;
    slgCfg.eDateControl = SLOG_TIME_ONLY;
    strcpy(slgCfg.sFileName, ms->log_filename_str);
    strcpy(slgCfg.sFilePath, ms->log_dirname_str);
    slgCfg.nTraceTid = 0;
    slgCfg.nToScreen = 1;
    slgCfg.nUseHeap = 0;
    slgCfg.nToFile = 1;
    slgCfg.nFlush = 0;
    slgCfg.nFlags = SLOG_FLAGS_ALL;
    slog_config_set(&slgCfg);
    if (ms->verbosity_count < 2)
    {
        slog_disable(SLOG_TRACE);
    }
    if (ms->verbosity_count < 1)
    {
        slog_disable(SLOG_DEBUG);
    }
    xdgWipeHandle(&xdg);

    slogi("checking for diameters config");
    char* diameters_file_str;
    asprintf(&diameters_file_str, "%s/.config/monocoque/diameters.config", home_dir_str);
    ms->tyre_diameter_config = strdup(diameters_file_str);
    free(diameters_file_str);

    ms->useconfig = 0;
    ms->configcheck = 0;


    slogi("Testing monocoque config file: %s", ms->config_str);
    slogd("using diameters file %s %i", ms->tyre_diameter_config, ms->configcheck);
    config_t cfg;
    config_init(&cfg);
    config_setting_t* config_devices = NULL;
    if (!config_read_file(&cfg, ms->config_str))
    {
        sloge("Issue with monocoque config file: %s:%d - %s", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        fprintf(stderr, "Issue with monocoque config file: %s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        slog_destroy();
        //goto cleanup_final;
    }
    else
    {
        slogi("Opened and validated monocoque configuration file");
    }
    config_destroy(&cfg);



        int error = 0;
        error = MONOCOQUE_ERROR_NONE;
        //setupsound();
        bool pulseaudio = false;

        //if (ms->program_action == A_PLAY)
        //{
            ms->useconfig = 1;
            slogi("running monocoque in gameloop mode..");
//#ifdef USE_PULSEAUDIO
            //pa_threaded_mainloop_unlock(mainloop);
            pulseaudio = true;
//#endif

            if(ms->disable_audio == false)
            {
                setupsound();
            }
            //error = looper(devices, numdevices, p);
            error = monocoque_mainloop_start(ms);
            //if (error == MONOCOQUE_ERROR_NONE)
            //{
            //    slogi("Game loop exited succesfully with error code: %i", error);
            //}
            //else
            //{
            //    sloge("Game loop exited with error code: %i", error);
            //}
            //if(ms->disable_audio == false)
            //{
            //    freesound();
            //}
        //}



}


#include <nappgui.h>

#define NUM_BRICKS 40

Button *start_button;
Button *stop_button;

Label *game_status;
Label *simd_status;

typedef struct _app_t App;


struct _app_t
{
    bool_t is_running;
    Cell *button;
    Slider *slider;
    View *view;
    Window *window;
};



/*---------------------------------------------------------------------------*/

static void i_OnStart(App *app, Event *e)
{

    appstate = 0;
    unref(e);
    app->is_running = TRUE;
    cell_enabled(app->button, FALSE);
    uint32_t i;
    int argc = (int)osapp_argc();
    
    char **argv = malloc(argc * sizeof(char *));
    
    for (i = 0; i < (uint32_t)argc; ++i)
    {
        char buffer[128];
    
        osapp_argv(i, buffer, sizeof(buffer));
    
        argv[i] = strdup(buffer);  /* allocate a copy */
    }
    
    int rc = monocoque_main(argc, argv);
    fprintf(stderr, "monocoque mainloop started");
    /* cleanup */
    for (i = 0; i < (uint32_t)argc; ++i)
        free(argv[i]);
    
    free(argv);
}

static void i_OnStop(App *app, Event *e)
{
    monocoque_mainloop_stop(ms);
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 4);
    View *view = view_create();
    Slider *slider = slider_create();
    game_status = label_create();
    start_button = button_push();
    stop_button = button_push();
    view_size(view, s2df(800, 600));
    //view_OnDraw(view, listener(app, i_OnDraw, App));
    //slider_OnMoved(slider, listener(app, i_OnSlider, App));
    label_text(game_status, "Game:  None Detected      Simd: Not Detected");
    button_text(start_button, "Start");
    button_text(stop_button, "Stop");
    button_OnClick(start_button, listener(app, i_OnStart, App));
    button_OnClick(stop_button, listener(app, i_OnStop, App));
    layout_view(layout, view, 0, 0);
    //layout_slider(layout, slider, 0, 1);
    layout_label(layout, game_status, 0, 3);
    layout_button(layout, start_button, 0, 1);
    layout_button(layout, stop_button, 0, 2);
    layout_vexpand(layout, 0);
    layout_vmargin(layout, 0, 10);
    layout_vmargin(layout, 2, 10);
    layout_margin(layout, 10);
    panel_layout(panel, layout);
    app->view = view;
    app->slider = slider;
    app->button = layout_cell(layout, 0, 3);
    return panel;
}

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    osapp_finish();
    unref(app);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static App *i_create(void)
{
    App *app = heap_new0(App);
    Panel *panel = i_panel(app);
    app->window = window_create(ekWINDOW_STDRES);
    window_panel(app->window, panel);
    window_origin(app->window, v2df(200, 200));
    window_title(app->window, "monocoque");
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
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
            button_text(start_button, "Playing... Press to Stop");
        }
        if(appstate == 1)
        {
            button_text(start_button, "Searching for data... Press to Stop");
        }
        if(appstate == 0)
        {
            button_text(start_button, "Start");
        }
        label_text(game_status, "Game:  (Searching) None Detected      Simd: Not Detected");
    }

    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

#include <osapp/osmain.h>
osmain_sync(.04, i_create, i_destroy, i_update, "", App)
