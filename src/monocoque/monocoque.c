#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <basedir_fs.h>
#include <libconfig.h>

#include "gameloop/gameloop.h"
#include "gameloop/tachconfig.h"
#include "devices/simdevice.h"
#include "devices/sound.h"
#include "helper/parameters.h"
#include "helper/dirhelper.h"
#include "helper/confighelper.h"
#include "simulatorapi/simapi/simapi/simdata.h"
#include "slog/slog.h"

#define PROGRAM_NAME "monocoque"

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

int main(int argc, char** argv)
{
    display_banner();

    char* home_dir_str = gethome();
    if(home_dir_str == NULL)
    {
        fprintf(stderr, "You need a home directory");
        return 0;
    }
    Parameters* p = NULL;
    p = malloc(sizeof(Parameters));
    MonocoqueSettings* ms = malloc(sizeof(MonocoqueSettings));;

    ConfigError ppe = getParameters(argc, argv, p);
    if (ppe == E_SUCCESS_AND_EXIT || ppe == E_SOMETHING_BAD)
    {
        printf("invalid parameters\n");
        goto cleanup_final;
    }

    xdgHandle xdg;
    if(!xdgInitHandle(&xdg))
    {
        fprintf(stderr, "Function xdgInitHandle() failed, is $HOME unset?\n");
        goto cleanup_final;
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
        goto cleanup_final;
    }
    else
    {
        slogi("Opened and validated monocoque configuration file");
    }
    config_destroy(&cfg);



    if (ms->program_action == A_CONFIG_TACH)
    {
        int error = 0;
        SimDevice* tachdev = malloc(sizeof(SimDevice));
        tachdev->initialized = false;
        SimData* sdata = malloc(sizeof(SimData));
        DeviceSettings* ds = malloc(sizeof(DeviceSettings));

        error = devsetup("USB", "Tachometer", "None", ms, ds, NULL);

        if (error != MONOCOQUE_ERROR_NONE)
        {
            sloge("Could not proceed with tachometer configuration due to error: %i", error);
        }
        else
        {
            int devices = 0;

            devices = devinit(tachdev, 1, ds, ms);
            if(devices < 1)
            {
                error = MONOCOQUE_ERROR_INVALID_DEV;
            }
        }

        if (error != MONOCOQUE_ERROR_NONE)
        {
            sloge("Could not proceed with tachometer configuration due to error: %i", error);
        }
        else
        {
            slogi("configuring tachometer with max revs: %i, granularity: %i, saving to %s", p->max_revs, p->granularity, p->save_file);
            config_tachometer(p->max_revs, p->granularity, p->save_file, tachdev, sdata);
        }

        slogt("freeing devices if necessary");
        if(tachdev->initialized == true)
        {
            slogt("freeing tachmoeter device");
            tachdev->free(tachdev);
        }
        //free(tachdev);
        free(sdata);
        free(ds);
    }
    else
    {

        int error = 0;
        error = MONOCOQUE_ERROR_NONE;
        //setupsound();
        bool pulseaudio = false;

        if (ms->program_action == A_PLAY)
        {
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
            error = monocoque_mainloop(ms);
            if (error == MONOCOQUE_ERROR_NONE)
            {
                slogi("Game loop exited succesfully with error code: %i", error);
            }
            else
            {
                sloge("Game loop exited with error code: %i", error);
            }
            if(ms->disable_audio == false)
            {
                freesound();
            }
        }
        else
        {
            slogi("running monocoque in test mode...");

            if(ms->disable_audio == false)
            {
                setupsound();
            }
            ms->useconfig = 0;

            int configs = getNumberOfConfigs(ms->config_str);
            int confignum = getconfigtouse(ms->config_str, "default", configs-1);
            int configureddevices;
            configcheck(ms->config_str, confignum, &configureddevices);

            DeviceSettings* ds = malloc(configureddevices * sizeof(DeviceSettings));
            slogd("loading confignum %i, with %i devices.", confignum, configureddevices);

            int numdevices = uiloadconfig(ms->config_str, confignum, configureddevices, ms, ds);
            SimDevice* simdevices = malloc(numdevices * sizeof(SimDevice));
            int initdevices = devinit(simdevices, configureddevices, ds, ms);
            for( int i = 0; i < configureddevices; i++)
            {
                settingsfree(ds[i]);
            }
            free(ds);
            error = tester(simdevices, numdevices);
            if (error == MONOCOQUE_ERROR_NONE)
            {
                slogi("Test exited succesfully with error code: %i", error);
            }
            else
            {
                sloge("Test exited with error code: %i", error);
            }
            for (int x = 0; x < numdevices; x++)
            {
                if (simdevices[x].initialized == true)
                {
                    simdevices[x].free(&simdevices[x]);
                }
            }
            if(ms->disable_audio == false)
            {
                freesound();
            }
        }
        slog_destroy();
    }



cleanup_final:
    monocoquesettingsfree(ms);
    free(ms);
    freeparams(p);
    free(p);
    exit(0);
}
