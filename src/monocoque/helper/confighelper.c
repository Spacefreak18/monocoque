#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>

#include "confighelper.h"

#include "../slog/slog.h"

int strtogame(const char* game, MonocoqueSettings* ms)
{
    slogd("Checking for %s in list of supported simulators.", game);
    if (strcmp(game, "ac") == 0)
    {
        slogd("Setting simulator to Assetto Corsa");
        ms->sim_name = SIMULATOR_ASSETTO_CORSA;
    }
    else
        if (strcmp(game, "test") == 0)
        {
            slogd("Setting simulator to Test Data");
            ms->sim_name = SIMULATOR_MONOCOQUE_TEST;
        }
        else
        {
            slogi("%s does not appear to be a supported simulator.", game);
            return MONOCOQUE_ERROR_INVALID_SIM;
        }
    return MONOCOQUE_ERROR_NONE;
}

int strtodev(const char* device_type, DeviceSettings* ds)
{
    ds->is_valid = false;
    if (strcmp(device_type, "USB") == 0)
    {
        ds->dev_type = SIMDEV_USB;
    }
    else
        if (strcmp(device_type, "Sound") == 0)
        {
            ds->dev_type = SIMDEV_SOUND;
        }
        else
            if (strcmp(device_type, "Serial") == 0)
            {
                ds->dev_type = SIMDEV_SERIAL;
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

int strtodevtype(const char* device_subtype, DeviceSettings* ds)
{
    ds->is_valid = false;
    if (strcmp(device_subtype, "Tachometer") == 0)
    {
        ds->dev_subtype = SIMDEVTYPE_TACHOMETER;
    }
    else
        if (strcmp(device_subtype, "ShiftLights") == 0)
        {
            ds->dev_subtype = SIMDEVTYPE_SHIFTLIGHTS;
        }
        else
            if (strcmp(device_subtype, "Shaker") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_SHAKER;
            }
            else
            {
                ds->is_valid = false;
                slogi("%s does not appear to be a valid device sub type, but attempting to continue with other devices", device_subtype);
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
                if (strcmp(cursubsubnode->name, "SettingsItem") == 0)
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
                        if (strcmp(cursubsubsubnode->name, "Value") == 0)
                        {
                            xmlChar* a = xmlNodeGetContent(cursubsubsubnode);
                            rpms_array[i] = strtol((char*) a, &buf, 10);
                            xmlFree(a);
                        }
                        if (strcmp(cursubsubsubnode->name, "TimeValue") == 0)
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
    slogi("Called device setup with %s %s %s", device_type, device_subtype, config_file);
    ds->dev_type = SIMDEV_UNKNOWN;
    ds->dev_subtype = SIMDEVTYPE_UNKNOWN;
    error = strtodev(device_type, ds);
    if (error != MONOCOQUE_ERROR_NONE)
    {
        return error;
    }
    error = strtodevtype(device_subtype, ds);
    if (error != MONOCOQUE_ERROR_NONE)
    {
        return error;
    }
    if (ms->program_action == A_PLAY)
    {
        error = loadconfig(config_file, ds);
    }
    if (error != MONOCOQUE_ERROR_NONE)
    {
        return error;
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
        if (ms->program_action == A_PLAY)
        {
            ds->tachsettings.use_pulses = false;
        }
    }

    return error;
}
