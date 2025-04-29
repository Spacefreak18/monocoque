#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#include "usbhapticdevice.h"
#include "../../helper/confighelper.h"
#include "../../simulatorapi/simapi/simapi/simdata.h"
#include "../../simulatorapi/simapi/simapi/simmapper.h"
#include "../../slog/slog.h"

#define kmhtoms      0.277778
#define minspeedinms 0.5
#define minvelocity  50
#define maxbrake     0
#define maxthrottle  0
#define maxXvelocity 0.001


bool hasTyreDiameter(SimData* simdata)
{
    if (simdata->tyrediameter[0] == -1 || simdata->tyrediameter[1] == -1 || simdata->tyrediameter[2] == -1 || simdata->tyrediameter[3] == -1)
    {
        slogt("failed to find tyre diameter data");
        return false;
    }
    slogt("tyre diameter data found");
    return true;
}

int loadtyreconfig(SimData* simdata, char* configfile)
{
    config_t cfg;
    config_init(&cfg);
    config_setting_t* config_cars = NULL;

    if (!config_read_file(&cfg, configfile))
    {
        slogw("Could not open diameters save file.");
        config_destroy(&cfg);
        return 1;
    }

    config_cars = config_lookup(&cfg, "cars");

    if(config_cars == NULL)
    {
        slogd("diameters config file corrupted");
        config_destroy(&cfg);
        return 1;
    }
    slogt("parsing diameters config file");

    int cars = config_setting_length(config_cars);

    int i = 0;
    while (i<cars)
    {
        DeviceSettings settings;

        config_setting_t* config_car = config_setting_get_elem(config_cars, i);
        if(config_car != NULL)
        {
            const char* car;
            const char* simstr;
            int sim = 0;
            double tyre0;
            double tyre1;
            double tyre2;
            double tyre3;
            config_setting_lookup_string(config_car, "car", &car);
            config_setting_lookup_float(config_car, "tyre0", &tyre0);
            config_setting_lookup_float(config_car, "tyre1", &tyre1);
            config_setting_lookup_float(config_car, "tyre2", &tyre2);
            config_setting_lookup_float(config_car, "tyre3", &tyre3);
            int found = config_setting_lookup_string(config_car, "sim", &simstr);
            if(found == CONFIG_FALSE)
            {
                int found = config_setting_lookup_int(config_car, "sim", &sim);
            }
            else
            {
                sim = simapi_strtogame(simstr);
            }


            if(simdata->car != NULL && car != NULL)
            {
                if(simdata->car[0] != '\0' && car[0] != '\0')
                {
                    slogt("%s %s %i %i", simdata->car, car, simdata->simexe, sim);
                    if (strcicmp(car, simdata->car) == 0 && sim == simdata->simexe)
                    {
                        slogi("found saved car %s with tyre diameters %f %f %f %f", car, tyre0, tyre1, tyre2, tyre3);
                        simdata->tyrediameter[0] = tyre0;
                        simdata->tyrediameter[1] = tyre1;
                        simdata->tyrediameter[2] = tyre2;
                        simdata->tyrediameter[3] = tyre3;
                        break;
                    }
                }
            }
        }
        else
        {
            slogw("Possible corruption in config file on entry %i attempting to continue", i+1);
        }
        i++;
    }


    config_destroy(&cfg);

    return 0;
}

int savetyreconfig(SimData* simdata, char* configfile)
{
    config_t cfg;
    config_setting_t* root;
    config_setting_t* array;
    config_setting_t* carobject;
    config_setting_t* setting;


    config_init(&cfg);
    if (!config_read_file(&cfg, configfile))
    {
        slogw("Could not open diameters save file, creating new.");
        root = config_root_setting(&cfg);
        array = config_setting_add(root, "cars", CONFIG_TYPE_LIST);
    }
    else
    {
        array = config_lookup(&cfg, "cars");
    }

    // TODO add check to not add same car-sim combination twice
    carobject = config_setting_add(array, "cars", CONFIG_TYPE_GROUP);

    setting = config_setting_add(carobject, "car", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, simdata->car);
    setting = config_setting_add(carobject, "sim", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, simapi_gametostr(simdata->simexe));
    setting = config_setting_add(carobject, "tyre0", CONFIG_TYPE_FLOAT);
    config_setting_set_float(setting, simdata->tyrediameter[0]);
    setting = config_setting_add(carobject, "tyre1", CONFIG_TYPE_FLOAT);
    config_setting_set_float(setting, simdata->tyrediameter[1]);
    setting = config_setting_add(carobject, "tyre2", CONFIG_TYPE_FLOAT);
    config_setting_set_float(setting, simdata->tyrediameter[2]);
    setting = config_setting_add(carobject, "tyre3", CONFIG_TYPE_FLOAT);
    config_setting_set_float(setting, simdata->tyrediameter[3]);

    /* Write out the new configuration. */
    if(! config_write_file(&cfg, configfile))
    {
      slogi("Error while writing file.");
      config_destroy(&cfg);
    }

    slogi("New configuration successfully written to: %s\n", configfile);

    config_destroy(&cfg);

    return 0;
}

void getTyreDiameter(SimData* simdata)
{
    if(simdata->velocity > minvelocity && simdata->brake <= maxbrake && simdata->gas <= maxthrottle)
    {
        double Speedms = kmhtoms * simdata->velocity;
        if (simdata->Xvelocity/Speedms < maxXvelocity)
        {
            for(int i = 0; i < 4; i++)
            {
                simdata->tyrediameter[i] = Speedms / simdata->tyreRPS[i] * 2;
            }
            slogi("Successfully set tyre diameters for wheel slip effects.");
        }

    }
}


double slipeffect(SimData* simdata, int effecttype, int tyre, double threshold, int useconfig, int* configcheck, char* configfile)
{
    double play = 0;
    double wheelslip[4];
    wheelslip[0] = 0;
    wheelslip[1] = 0;
    wheelslip[2] = 0;
    wheelslip[3] = 0;

    if(simdata->car == NULL)
    {
        slogw("sim is not compatible with wheel and tyre effects, or car and session not loaded.");
        return 0;
    }
    if(simdata->car[0] == '\0')
    {
        slogw("sim is not compatible with wheel and tyre effects, or car not loaded.");
        return 0;
    }

    slogt("wheel vibration calculation with wheel config set to %i configchecked %i configfile %s car %s sim %i", useconfig, *configcheck, configfile, simdata->car, simdata->simexe);

    switch (effecttype)
    {
        case (EFFECT_TYRESLIP):
        case (EFFECT_TYRELOCK):
        case (EFFECT_ABSBRAKES):

            if(useconfig == 1 && configfile != NULL)
            {
                // check for saved tyre diameter in config file
                // if not saved version exists get tyre diameter and save it
                // use config check variable to track if the config check has been performed
                // avoid many opens of the same file
                int error = 0;
                if(hasTyreDiameter(simdata)==false && *configcheck == 0)
                {
                    slogi("attempting load of tyre diameter config");
                    error = loadtyreconfig(simdata, configfile);
                    *configcheck = 1;
                }

                if(hasTyreDiameter(simdata)==false)
                {
                    slogt("could not find tyre diameter in config file, attempting to calculate new");
                    getTyreDiameter(simdata);
                    if(hasTyreDiameter(simdata)==true)
                    {
                        slogi("saving tyre config");
                        error = savetyreconfig(simdata, configfile);
                    }
                }
            }
            if(hasTyreDiameter(simdata)==true)
            {
                double Speedms = kmhtoms * simdata->velocity;
                slogt("attempting wheel slip calculation");
                if (Speedms > minspeedinms)
                {
                    for(int i = 0; i < 4; i++)
                    {
                        wheelslip[i] = (Speedms - simdata->tyrediameter[i] * simdata->tyreRPS[i] / 2) / Speedms;
                    }
                }
                else
                {
                    for(int i = 0; i < 4; i++)
                    {
                        wheelslip[i] = 0;
                    }
                }
                slogt("wheelslip values are %f %f %f %f", wheelslip[0], wheelslip[1], wheelslip[2], wheelslip[3]);
                slogt("velocities (x,y,z) are %f %f %f", simdata->Xvelocity, simdata->Yvelocity, simdata->Zvelocity);
            }
            break;
        case EFFECT_SUSPENSION:
            break;
        default:
            slogd("Unknown effect type");
    }

    if(simdata->Yvelocity <= 0)
    {
        return 0;
    }
    if(simdata->Zvelocity > 1 || simdata->Zvelocity < -1)
    {
        return 0;
    }

    switch (effecttype)
    {
        case (EFFECT_TYRESLIP):

            if (tyre == FRONTLEFT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[0] < -threshold)
                {
                    play += fabs(wheelslip[0]) - fabs(threshold);
                    slogt("slip is %f", play);
                }
            }
            if (tyre == FRONTRIGHT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[1] < -threshold)
                {
                    play += fabs(wheelslip[1]) - fabs(threshold);
                    slogt("slip is %f", play);
                }
            }
            if (tyre == REARLEFT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[2] < -threshold)
                {
                    play += fabs(wheelslip[2]) - fabs(threshold);
                    slogt("slip is %f", play);
                }
            }
            if (tyre == REARRIGHT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[3] < -threshold)
                {
                    play += fabs(wheelslip[3]) - fabs(threshold);
                    slogt("slip is %f", play);
                }
            }
            break;

        case (EFFECT_TYRELOCK):
            if (tyre == FRONTLEFT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[0] > threshold)
                {
                    play += wheelslip[0] - threshold;
                    slogt("lock is %f", play);
                }
            }
            if (tyre == FRONTRIGHT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[1] > threshold)
                {
                    play += wheelslip[1] - threshold;
                    slogt("lock is %f", play);
                }
            }
            if (tyre == REARLEFT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[2] > threshold)
                {
                    play += wheelslip[2] - threshold;
                    slogt("lock is %f", play);
                }
            }
            if (tyre == REARRIGHT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[3] > threshold)
                {
                    play += wheelslip[3] - threshold;
                    slogt("lock is %f", play);
                }
            }

            break;
        case (EFFECT_ABSBRAKES):
            threshold = simdata->abs + threshold;
            if (tyre == FRONTLEFT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[0] > threshold)
                {
                    play += wheelslip[0] - threshold;
                    slogt("abs is %f", play);
                }
            }
            if (tyre == FRONTRIGHT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[1] > threshold)
                {
                    play += wheelslip[1] - threshold;
                    slogt("abs is %f", play);
                }
            }
            if (tyre == REARLEFT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[2] > threshold)
                {
                    play += wheelslip[2] - threshold;
                    slogt("abs is %f", play);
                }
            }
            if (tyre == REARRIGHT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[3] > threshold)
                {
                    play += wheelslip[3] - threshold;
                    slogt("abs is %f", play);
                }
            }
            if(simdata->abs <= 0)
            {
                play = 0;
            }
            break;

        case (EFFECT_SUSPENSION):

            if (tyre == FRONTLEFT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(simdata->suspension[0] > threshold)
                {
                    play += simdata->suspension[0] - threshold;
                    slogt("suspension is %f", play);
                }
            }
            if (tyre == FRONTRIGHT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(simdata->suspension[1] > threshold)
                {
                    play += simdata->suspension[1] - threshold;
                    slogt("suspension is %f", play);
                }
            }
            if (tyre == REARLEFT || tyre == REARS || tyre == ALLFOUR)
            {
                if(simdata->suspension[2] > threshold)
                {
                    play += simdata->suspension[2] - threshold;
                    slogt("suspension is %f", play);
                }
            }
            if (tyre == REARRIGHT || tyre == REARS || tyre == ALLFOUR)
            {
                if(simdata->suspension[3] > threshold)
                {
                    play += simdata->suspension[3] - threshold;
                    slogt("suspension is %f", play);
                }
            }
            break;
    }

    return play;
}

