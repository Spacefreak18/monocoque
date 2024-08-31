#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <jansson.h>
#include <math.h>

#include "usbhapticdevice.h"
#include "../../helper/confighelper.h"
#include "../../simulatorapi/simapi/simapi/simdata.h"
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
        const char* car;
        double tyre0;
        double tyre1;
        double tyre2;
        double tyre3;
        config_setting_lookup_string(config_car, "car", &car);
        config_setting_lookup_float(config_car, "tyre0", &tyre0);
        config_setting_lookup_float(config_car, "tyre1", &tyre1);
        config_setting_lookup_float(config_car, "tyre2", &tyre2);
        config_setting_lookup_float(config_car, "tyre3", &tyre3);

        if(simdata->car != NULL)
        {
            if (strcicmp(car, simdata->car) == 0)
            {
                simdata->tyrediameter[0] = tyre0;
                simdata->tyrediameter[1] = tyre1;
                simdata->tyrediameter[2] = tyre2;
                simdata->tyrediameter[3] = tyre3;
                break;
            }
        }
        i++;
    }


    config_destroy(&cfg);

    return 0;
}

//void loadtyreconfig(SimData* simdata, char* configfile)
//{
//    json_t *root;
//    json_error_t error;
//
//    root = json_load_file(configfile, 0, NULL);
//
//    if(!root)
//    {
//        slogw("could not open config file for tyre diameters");
//        return;
//    }
//
//    if(!json_is_object(root))
//    {
//        slogw("Malformed content (1) in tyre diameters config");
//        json_decref(root);
//        return;
//    }
//
//    json_t* cararray = json_object_get(root, "cars");
//    if(!json_is_array(cararray))
//    {
//        slogw("Malformed content (2) in tyre diameters config");
//        json_decref(root);
//        json_decref(cararray);
//        return;
//    }
//
//    for(int i = 0; i < json_array_size(cararray); i++)
//    {
//        json_t *data, *sha, *commit, *message;
//        const char *message_text;
//
//        data = json_array_get(cararray, i);
//        if(!json_is_object(data))
//        {
//            slogw("Malformed content (3) in tyre diameters config");
//            break;
//        }
//        json_t* jcar = json_object_get(data, "car");
//
//        if(!json_is_string(jcar))
//        {
//            slogw("Malformed content (4) in tyre diameters config");
//
//            json_decref(data);
//            json_decref(jcar);
//            break;
//        }
//        const char* car = json_string_value(jcar);
//
//        json_t* jtyre0 = json_object_get(data, "tyre0");
//        json_t* jtyre1 = json_object_get(data, "tyre1");
//        json_t* jtyre2 = json_object_get(data, "tyre2");
//        json_t* jtyre3 = json_object_get(data, "tyre3");
//
//        double diameter0 = json_real_value(jtyre0);
//        double diameter1 = json_real_value(jtyre1);
//        double diameter2 = json_real_value(jtyre2);
//        double diameter3 = json_real_value(jtyre3);
//
//        slogt("car is: %s", car);
//        slogt("diameter 0 is %f", diameter0);
//        slogt("diameter 1 is %f", diameter1);
//        slogt("diameter 2 is %f", diameter2);
//        slogt("diameter 3 is %f", diameter3);
//
//        json_decref(data);
//        json_decref(jcar);
//        json_decref(jtyre0);
//        json_decref(jtyre1);
//        json_decref(jtyre2);
//        json_decref(jtyre3);
//
//        if(simdata->car != NULL)
//        {
//            if (strcicmp(car, simdata->car) == 0)
//            {
//                simdata->tyrediameter[0] = diameter0;
//                simdata->tyrediameter[1] = diameter1;
//                simdata->tyrediameter[2] = diameter2;
//                simdata->tyrediameter[3] = diameter3;
//                break;
//            }
//        }
//    }
//
//    json_decref(root);
//    json_decref(cararray);
//}

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

//void savetyreconfig(SimData* simdata, char* configfile)
//{
//
//    json_t* object = json_object();
//    json_t* array = json_array();
//    json_t* car = json_string(simdata->car);
//    json_t* tyre0 = json_real(simdata->tyrediameter[0]);
//    json_t* tyre1 = json_real(simdata->tyrediameter[1]);
//    json_t* tyre2 = json_real(simdata->tyrediameter[2]);
//    json_t* tyre3 = json_real(simdata->tyrediameter[3]);
//    json_object_set_new(object, "car", car);
//    json_object_set_new(object, "tyre0", tyre0);
//    json_object_set_new(object, "tyre1", tyre1);
//    json_object_set_new(object, "tyre2", tyre2);
//    json_object_set_new(object, "tyre3", tyre3);
//
//    json_array_append(array, object);
//
//    json_t* file = json_load_file(configfile, 0, NULL);
//
//    if(!file)
//    {
//        slogw("could not open config file for tyre diameters");
//        return;
//    }
//
//    if(!json_is_object(file))
//    {
//        json_object_set_new(file, "cars", array);
//    }
//    else
//    {
//        json_t* cararray = json_object_get(file, "cars");
//        json_array_append(cararray, array);
//    }
//
//    json_dump_file(file, configfile, 0);
//
//    json_decref(tyre0);
//    json_decref(tyre1);
//    json_decref(tyre2);
//    json_decref(tyre3);
//    json_decref(car);
//    json_decref(object);
//    json_decref(array);
//}

void getTyreDiameter(SimData* simdata)
{
    if(simdata->velocity > minvelocity && simdata->brake <= maxbrake && simdata->gas <= maxthrottle)
    {
        if (simdata->velocityX/simdata->velocity < maxXvelocity)
        {
            double Speedms = kmhtoms * simdata->velocity;
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

    slogt("wheel vibration calculation with wheel config set to %i configchecked %i configfile %s", useconfig, *configcheck, configfile);

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

            }
            break;
        default:
            slogd("Unknown effect type");
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
                }
            }
            if (tyre == FRONTRIGHT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[1] > threshold)
                {
                    play += wheelslip[1] - threshold;
                }
            }
            if (tyre == REARLEFT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[2] > threshold)
                {
                    play += wheelslip[2] - threshold;
                }
            }
            if (tyre == REARRIGHT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[3] > threshold)
                {
                    play += wheelslip[3] - threshold;
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
                }
            }
            if (tyre == FRONTRIGHT || tyre == FRONTS || tyre == ALLFOUR)
            {
                if(wheelslip[1] > threshold)
                {
                    play += wheelslip[1] - threshold;
                }
            }
            if (tyre == REARLEFT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[2] > threshold)
                {
                    play += wheelslip[2] - threshold;
                }
            }
            if (tyre == REARRIGHT || tyre == REARS || tyre == ALLFOUR)
            {
                if(wheelslip[3] > threshold)
                {
                    play += wheelslip[3] - threshold;
                }
            }
            if(simdata->abs <= 0)
            {
                play = 0;
            }
            break;
    }

    return play;
}

