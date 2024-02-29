#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>

#include "confighelper.h"

#include "../slog/slog.h"
#include "parameters.h"

#include <pulse/pulseaudio.h>

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}


int strtogame(const char* game, MonocoqueSettings* ms)
{
    slogd("Checking for %s in list of supported simulators.", game);
    if (strcicmp(game, "ac") == 0)
    {
        slogd("Setting simulator to Assetto Corsa");
        ms->sim_name = SIMULATOR_ASSETTO_CORSA;
    }
    else if (strcicmp(game, "rf2") == 0)
    {
        slogd("Setting simulator to RFactor 2");
        ms->sim_name = SIMULATOR_RFACTOR2;
    }
    else
        if (strcicmp(game, "test") == 0)
        {
            slogd("Setting simulator to Test Data");
            ms->sim_name = SIMULATOR_SIMAPI_TEST;
        }
        else
        {
            slogi("%s does not appear to be a supported simulator.", game);
            return MONOCOQUE_ERROR_INVALID_SIM;
        }
    return MONOCOQUE_ERROR_NONE;
}

int strtodevsubtype(const char* device_subtype, DeviceSettings* ds, int simdev)
{
    ds->is_valid = false;
    ds->dev_subtype = SIMDEVTYPE_UNKNOWN;

    switch (simdev) {
        case SIMDEV_USB:
            if (strcicmp(device_subtype, "Tachometer") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_TACHOMETER;
                break;
            }
        case SIMDEV_SERIAL:
            if (strcicmp(device_subtype, "ShiftLights") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_SHIFTLIGHTS;
                break;
            }
            if (strcicmp(device_subtype, "SimWind") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_SIMWIND;
                break;
            }
        case SIMDEV_SOUND:
            if (strcicmp(device_subtype, "Engine") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_ENGINESOUND;
                break;
            }
            if (strcicmp(device_subtype, "Gear") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_GEARSOUND;
                break;
            }
            if (strcicmp(device_subtype, "ABS") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_ABSBRAKES;
                break;
            }
            if ((strcicmp(device_subtype, "SLIP") == 0) || (strcicmp(device_subtype, "TYRESLIP") == 0))
            {
                ds->dev_subtype = SIMDEVTYPE_TYRESLIP;
                break;
            }
        default:
            ds->is_valid = false;
            slogw("%s does not appear to be a valid device sub type, but attempting to continue with other devices", device_subtype);
            return MONOCOQUE_ERROR_INVALID_DEV;
    }
    ds->is_valid = true;
    return MONOCOQUE_ERROR_NONE;
}

int strtodev(const char* device_type, const char* device_subtype, DeviceSettings* ds)
{
    ds->is_valid = false;
    if (strcicmp(device_type, "USB") == 0)
    {
        ds->dev_type = SIMDEV_USB;
        strtodevsubtype(device_subtype, ds, SIMDEV_USB);
    }
    else
        if (strcicmp(device_type, "Sound") == 0)
        {
            ds->dev_type = SIMDEV_SOUND;
            strtodevsubtype(device_subtype, ds, SIMDEV_SOUND);
        }
        else
            if (strcicmp(device_type, "Serial") == 0)
            {
                ds->dev_type = SIMDEV_SERIAL;
                strtodevsubtype(device_subtype, ds, SIMDEV_SERIAL);
            }
            else
            {
                ds->is_valid = false;
                slogi("%s does not appear to be a valid device type, but attempting to continue with other devices", device_type);
                return MONOCOQUE_ERROR_INVALID_DEV;
            }
    ds->is_valid = true;
    return MONOCOQUE_ERROR_NONE;
}

int loadtachconfig(const char* config_file, DeviceSettings* ds)
{


    xmlNode* rootnode = NULL;
    xmlNode* curnode = NULL;
    xmlNode* cursubnode = NULL;
    xmlNode* cursubsubnode = NULL;
    xmlNode* cursubsubsubnode = NULL;
    xmlDoc* doc = NULL;
    char* buf;

    doc = xmlParseFile(config_file);
    if (doc == NULL)
    {
        sloge("Could not read revburner xml config file %s", config_file);
        return 1;
    }

    rootnode = xmlDocGetRootElement(doc);
    if (rootnode == NULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        sloge("Invalid rev burner xml");
        return 1;
    }

    int arraysize = 0;
    for (curnode = rootnode; curnode; curnode = curnode->next)
    {
        for (cursubnode = curnode->children; cursubnode; cursubnode = cursubnode->next)
        {
            for (cursubsubnode = cursubnode->children; cursubsubnode; cursubsubnode = cursubsubnode->next)
            {
                if (cursubsubnode->type == XML_ELEMENT_NODE)
                {
                    slogt("Xml Element name %s", cursubsubnode->name);
                }
                if (strcicmp(cursubsubnode->name, "SettingsItem") == 0)
                {
                    arraysize++;
                }

            }
        }
    }

    uint32_t pulses_array[arraysize];
    uint32_t rpms_array[arraysize];
    slogt("rev burner settings array size %i", arraysize);
    int i = 0;
    for (curnode = rootnode; curnode; curnode = curnode->next)
    {
        if (curnode->type == XML_ELEMENT_NODE)
            for (cursubnode = curnode->children; cursubnode; cursubnode = cursubnode->next)
            {
                for (cursubsubnode = cursubnode->children; cursubsubnode; cursubsubnode = cursubsubnode->next)
                {
                    for (cursubsubsubnode = cursubsubnode->children; cursubsubsubnode; cursubsubsubnode = cursubsubsubnode->next)
                    {
                        if (strcicmp(cursubsubsubnode->name, "Value") == 0)
                        {
                            xmlChar* a = xmlNodeGetContent(cursubsubsubnode);
                            rpms_array[i] = strtol((char*) a, &buf, 10);
                            xmlFree(a);
                        }
                        if (strcicmp(cursubsubsubnode->name, "TimeValue") == 0)
                        {
                            xmlChar* a = xmlNodeGetContent(cursubsubsubnode);
                            pulses_array[i] = strtol((char*) a, &buf, 10);
                            xmlFree(a);
                            i++;
                        }
                    }
                }
            }
    }

    ds->tachsettings.pulses_array = malloc(sizeof(pulses_array));
    ds->tachsettings.rpms_array = malloc(sizeof(rpms_array));
    ds->tachsettings.size = arraysize;

    memcpy(ds->tachsettings.pulses_array, pulses_array, sizeof(pulses_array));
    memcpy(ds->tachsettings.rpms_array, rpms_array, sizeof(rpms_array));


    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}

int loadconfig(const char* config_file, DeviceSettings* ds)
{
    if (ds->dev_subtype == SIMDEVTYPE_TACHOMETER)
    {
        return loadtachconfig(config_file, ds);
    }
    return 0;
}

int devsetup(const char* device_type, const char* device_subtype, const char* config_file, MonocoqueSettings* ms, DeviceSettings* ds, config_setting_t* device_settings)
{
    int error = MONOCOQUE_ERROR_NONE;
    //slogt("Called device setup with %s %s %s", device_type, device_subtype, config_file);
    ds->dev_type = SIMDEV_UNKNOWN;

    error = strtodev(device_type, device_subtype, ds);
    if (error != MONOCOQUE_ERROR_NONE)
    {
        return error;
    }

    if (ms->program_action == A_PLAY || ms->program_action == A_TEST)
    {
        error = loadconfig(config_file, ds);
    }
    if (error != MONOCOQUE_ERROR_NONE)
    {
        return error;
    }

    if (ds->dev_type == SIMDEV_SOUND)
    {
        slogi("reading configured sound device settings");
        ds->sounddevsettings.frequency = -1;
        ds->sounddevsettings.volume = -1;
        ds->sounddevsettings.lowbound_frequency = -1;
        ds->sounddevsettings.upperbound_frequency = -1;
        ds->sounddevsettings.pan = 0;
        ds->sounddevsettings.duration = 2.0;
        if (ds->dev_subtype == SIMDEVTYPE_GEARSOUND)
        {
            ds->sounddevsettings.duration = .125;
        }
        if (device_settings != NULL)
        {

            config_setting_lookup_int(device_settings, "volume", &ds->sounddevsettings.volume);
            config_setting_lookup_int(device_settings, "frequency", &ds->sounddevsettings.frequency);
            config_setting_lookup_int(device_settings, "pan", &ds->sounddevsettings.pan);
            config_setting_lookup_float(device_settings, "duration", &ds->sounddevsettings.duration);



            const char* temp;
            int found = 0;
            found = config_setting_lookup_string(device_settings, "devid", &temp);
            if (found == 0)
            {
                ds->sounddevsettings.dev = NULL;
            }
            else
            {
                ds->sounddevsettings.dev = strdup(temp);
            }
            if (ds->dev_subtype == SIMDEVTYPE_TYRESLIP)
            {

                found = config_setting_lookup_string(device_settings, "tyre", &temp);

                ds->tyre = ALLFOUR;

                if (strcicmp(temp, "FRONTS") == 0)
                {
                    ds->tyre = FRONTS;
                }
                if (strcicmp(temp, "REARS") == 0)
                {
                    ds->tyre = REARS;
                }
                if (strcicmp(temp, "FRONTLEFT") == 0)
                {
                    ds->tyre = FRONTLEFT;
                }
                if (strcicmp(temp, "FRONTRIGHT") == 0)
                {
                    ds->tyre = FRONTRIGHT;
                }
                if (strcicmp(temp, "REARLEFT") == 0)
                {
                    ds->tyre = REARLEFT;
                }
                if (strcicmp(temp, "REARRIGHT") == 0)
                {
                    ds->tyre = REARRIGHT;
                }
            }
        }
    }

    if (ds->dev_subtype == SIMDEVTYPE_TACHOMETER)
    {
        if (device_settings != NULL)
        {
            config_setting_lookup_int(device_settings, "granularity", &ds->tachsettings.granularity);
            if (ds->tachsettings.granularity < 0 || ds->tachsettings.granularity > 4 || ds->tachsettings.granularity == 3)
            {
                slogd("No or invalid valid set for tachometer granularity, setting to 1");
                ds->tachsettings.granularity = 1;
            }
            slogi("Tachometer granularity set to %i", ds->tachsettings.granularity);
        }
        ds->tachsettings.use_pulses = true;
        if (ms->program_action == A_PLAY || ms->program_action == A_TEST)
        {
            ds->tachsettings.use_pulses = false;
        }
    }


    if (ds->dev_subtype == SIMDEVTYPE_SIMWIND || ds->dev_subtype == SIMDEVTYPE_SHIFTLIGHTS)
    {
        if (device_settings != NULL)
        {
            const char* temp;
            config_setting_lookup_string(device_settings, "devpath", &temp);
            ds->serialdevsettings.portdev = strdup(temp);
        }
    }

    return error;
}

int settingsfree(DeviceSettings ds)
{

    if (ds.dev_subtype == SIMDEVTYPE_SIMWIND || ds.dev_subtype == SIMDEVTYPE_SHIFTLIGHTS)
    {
        if (ds.serialdevsettings.portdev != NULL)
        {
            free(ds.serialdevsettings.portdev);
        }

    }
    if (ds.dev_type == SIMDEV_SOUND)
    {
        if (ds.sounddevsettings.dev != NULL)
        {
            free(ds.sounddevsettings.dev);
        }
    }
    return 0;
}
