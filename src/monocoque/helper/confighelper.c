#include <dirent.h>
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



#include "../simulatorapi/simapi/simapi/simmapper.h"

#include <pulse/pulseaudio.h>

int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

int strtoeffecttype(const char* effect, DeviceSettings* ds)
{
    ds->is_valid = false;

    if (strcicmp(effect, "Engine") == 0)
    {
        ds->is_valid = true;
        ds->effect_type = EFFECT_ENGINERPM;
    }
    if (strcicmp(effect, "Gear") == 0)
    {
        ds->is_valid = true;
        ds->effect_type = EFFECT_GEARSHIFT;
    }
    if (strcicmp(effect, "ABS") == 0)
    {
        ds->is_valid = true;
        slogt("found abas effect set");
        ds->effect_type = EFFECT_ABSBRAKES;
    }
    if ((strcicmp(effect, "SLIP") == 0) || (strcicmp(effect, "TYRESLIP") == 0) || (strcicmp(effect, "TIRESLIP") == 0))
    {
        ds->is_valid = true;
        slogt("found tyreslip effect set");
        ds->effect_type = EFFECT_TYRESLIP;
    }
    if ((strcicmp(effect, "LOCK") == 0) || (strcicmp(effect, "TYRELOCK") == 0) || (strcicmp(effect, "TIRELOCK") == 0))
    {
        ds->is_valid = true;
        slogt("found tyreslock effect set");
        ds->effect_type = EFFECT_TYRELOCK;
    }

    if (ds->is_valid == false)
    {
        slogw("effect %s is not a valid effect", effect);
    }

    ds->is_valid = true;
    return MONOCOQUE_ERROR_NONE;
}

int strtodevsubsubtype(const char* device_subsubtype, DeviceSettings* ds)
{
    ds->dev_subsubtype = SIMDEVSUBTYPE_UNKNOWN;


    bool devfound = false;
    if (strcicmp(device_subsubtype, "CammusC5") == 0)
    {
        ds->dev_subsubtype = SIMDEVSUBTYPE_CAMMUSC5;
        bool devfound = true;
    }
    if (strcicmp(device_subsubtype, "CammusC12") == 0)
    {
        ds->dev_subsubtype = SIMDEVSUBTYPE_CAMMUSC12;
        bool devfound = true;
    }
    if (strcicmp(device_subsubtype, "MozaR5") == 0)
    {
        ds->dev_subsubtype = SIMDEVSUBTYPE_MOZAR5;
        bool devfound = true;
    }
    if (strcicmp(device_subsubtype, "MozaR8") == 0)
    {
        ds->dev_subsubtype = SIMDEVSUBTYPE_MOZAR5;
        bool devfound = true;
    }
    if (strcicmp(device_subsubtype, "CSLELITEV3PEDALS") == 0)
    {
        ds->dev_subsubtype = SIMDEVSUBTYPE_CSLELITEV3PEDALS;
        bool devfound = true;
    }
    if (strcicmp(device_subsubtype, "SIMAGICP1000PEDALS") == 0)
    {
        ds->dev_subsubtype = SIMDEVSUBTYPE_SIMAGICP1000PEDALS;
        bool devfound = true;
    }

    if(devfound == false)
    {
        slogw("%s does not appear to be a valid device sub sub type, but attempting to continue with other devices", device_subsubtype);
        return MONOCOQUE_ERROR_INVALID_DEV;
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
            if (strcicmp(device_subtype, "Wheel") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_USBWHEEL;
                break;
            }
            if (strcicmp(device_subtype, "UsbHaptic") == 0 || strcicmp(device_subtype, "Haptic") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_USBHAPTIC;
                break;
            }
        case SIMDEV_SERIAL:
            if (strcicmp(device_subtype, "ShiftLights") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_SHIFTLIGHTS;
                break;
            }
            if (strcicmp(device_subtype, "Simleds") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_SIMLED;
                break;
            }
            if (strcicmp(device_subtype, "SimWind") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_SIMWIND;
                break;
            }
            if (strcicmp(device_subtype, "SerialHaptic") == 0 || strcicmp(device_subtype, "Haptic") == 0)
            {
                slogt("found serial haptic device settings");
                ds->dev_subtype = SIMDEVTYPE_SERIALHAPTIC;
                break;
            }
            if (strcicmp(device_subtype, "Wheel") == 0)
            {
                ds->dev_subtype = SIMDEVTYPE_SERIALWHEEL;
                break;
            }
        case SIMDEV_SOUND:
            ds->is_valid = true;
            break;
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

int getsimfromconfig(config_setting_t* c)
{
    int sim = 0;
    const char* simstr = NULL;
    int found = config_setting_lookup_string(c, "sim", &simstr);
    if(found == 0)
    {
        int found = config_setting_lookup_int(c, "sim", &sim);
    }
    else
    {
        sim = simapi_strtogame(simstr);
    }
    return sim;
}

int getNumberOfConfigs(const char* config_file_str)
{
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        return -1;
    }
    config_setting_t* config = NULL;
    config_setting_t* config_widgets = NULL;
    config = config_lookup(&cfg, "configs");
    int configs = config_setting_length(config);

    return configs;
}

int getconfigtouse2(const char* config_file_str, char* car, int sim)
{
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        return -1;
    }
    config_setting_t* config = NULL;
    config_setting_t* config_widgets = NULL;
    config = config_lookup(&cfg, "configs");
    int configs = config_setting_length(config);

    const char* temp;
    config_setting_t* config_config = NULL;
    int j = 0;
    if ( configs == 1 )
    {
        return -1;
    }
    int confignum = -1;
    slogt("Multiple configs found");
    for (j = 0; j < configs; j++)
    {
        config_config = config_setting_get_elem(config, j);

        int found = 0;
        int csim = 0;
        slogt("sim is %i", sim);
        csim = getsimfromconfig(config_config);
        if (csim != sim)
        {
            slogt("rejected config %i", j);
            continue;
        }

        slogt("checking if car is matched %i", j);
        temp = NULL;
        found = config_setting_lookup_string(config_config, "car", &temp);
        slogt("config car is %s found is %i", temp, found);
        if(temp != NULL && found > 0 && car > 0 && car != NULL)
        {
            slogt("checking against sim car of %s", car);
            if(strcicmp(temp, car) == 0)
            {
                confignum = j;
            }
        }
        if(confignum>=0)
        {
            break;
        }
    }
    return confignum;
}

int getconfigtouse1(const char* config_file_str, char* car, int sim)
{
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        return -1;
    }
    config_setting_t* config = NULL;
    config_setting_t* config_widgets = NULL;
    config = config_lookup(&cfg, "configs");
    int configs = config_setting_length(config);

    const char* temp;
    config_setting_t* config_config = NULL;
    int j = 0;
    if ( configs == 1 )
    {
        return -1;
    }
    int confignum = -1;
    slogt("Multiple configs found");
    for (j = 0; j < configs; j++)
    {
        config_config = config_setting_get_elem(config, j);

        int found = 0;
        int csim = 0;
        slogt("sim is %i", sim);
        csim = getsimfromconfig(config_config);
        if (csim != sim)
        {
            slogt("rejected config %i", j);
            continue;
        }

        slogt("checking if car is matched %i", j);
        temp = NULL;
        found = config_setting_lookup_string(config_config, "car", &temp);
        slogt("config car is %s found is %i", temp, found);
        if(temp != NULL && found > 0 && car > 0 && car != NULL)
        {
            slogt("checking against sim car of %s", car);
            if(strcicmp(temp, car) == 0)
            {
                confignum = j;
            }
            if(strcicmp("default", temp) == 0)
            {
                slogt("matched default car");
                confignum = j;
            }
        }
        else
        {
            slogt("assuming default car");
            confignum = j;
        }
        slogt("bomb");
        if(confignum>=0)
        {
            break;
        }
    }
    return confignum;
}

int getconfigtouse(const char* config_file_str, char* car, int sim)
{
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        return -1;
    }
    config_setting_t* config = NULL;
    config_setting_t* config_widgets = NULL;
    config = config_lookup(&cfg, "configs");
    int configs = config_setting_length(config);

    const char* temp;
    config_setting_t* config_config = NULL;
    int j = 0;
    if ( configs == 1 )
    {
        return 0;
    }
    int confignum = 0;
    slogt("Multiple configs found");
    for (j = 0; j < configs; j++)
    {
        config_config = config_setting_get_elem(config, j);

        int found = 0;
        int csim = 0;
        slogt("sim is %i", sim);
        csim = getsimfromconfig(config_config);
        if (csim != sim && csim != 0)
        {
            slogt("rejected config %i", j);
            continue;
        }

        slogt("checking if car is matched %i", j);
        temp = NULL;
        found = config_setting_lookup_string(config_config, "car", &temp);
        slogt("config car is %s found is %i", temp, found);
        if(temp != NULL && found > 0 && car > 0 && car != NULL)
        {
            slogt("checking against sim car of %s", car);
            if(strcicmp(temp, car) == 0)
            {
                confignum = j;
            }
            if(strcicmp("default", temp) == 0)
            {
                slogt("matched default car");
                confignum = j;
            }
        }
        else
        {
            slogt("assuming default car");
            confignum = j;
        }
        slogt("bomb");
        if(confignum<configs-1)
        {
            break;
        }
    }
    return confignum;
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

int gettyre(config_setting_t* device_settings, DeviceSettings* ds) {

    const char* temp;
    int found = config_setting_lookup_string(device_settings, "tyre", &temp);

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

int loadconfig(const char* config_file, DeviceSettings* ds)
{
    if (ds->dev_subtype == SIMDEVTYPE_TACHOMETER)
    {
        return loadtachconfig(config_file, ds);
    }
    return 0;
}

int configcheck(const char* config_file_str, int confignum, int* devices)
{
    slogt("ui config check");
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    }

    config_setting_t* config = NULL;
    config = config_lookup(&cfg, "configs");
    config_setting_t* selectedconfig = config_setting_get_elem(config, confignum);
    slogt("selected num %i", confignum);
    config_setting_t* config_devices = NULL;
    config_devices = config_setting_lookup(selectedconfig, "devices");
    *devices = config_setting_length(config_devices);
    config_destroy(&cfg);
    return 0;
    //return cfg;
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


    ds->fps = 60;
    config_setting_lookup_int(device_settings, "fps", &ds->fps);

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

    if (ds->dev_subtype == SIMDEVTYPE_USBHAPTIC || ds->dev_subtype == SIMDEVTYPE_USBWHEEL)
    {
        // logic for different devices
        int b = 0;

        const char* temp;
        int found = config_setting_lookup_string(device_settings, "subtype", &temp);
        if(temp != NULL && found > 0)
        {
            b = strtodevsubsubtype(temp, ds);
        }
    }

    if (ds->dev_subtype == SIMDEVTYPE_USBHAPTIC || ds->dev_type == SIMDEV_SOUND || ds->dev_subtype == SIMDEVTYPE_SERIALHAPTIC)
    {
        slogt("analysing haptic effect settings");
        const char* effect;
        config_setting_lookup_string(device_settings, "effect", &effect);
        strtoeffecttype(effect, ds);
        if (ds->effect_type == EFFECT_TYRESLIP || ds->effect_type == EFFECT_TYRELOCK || ds->effect_type == EFFECT_ABSBRAKES)
        {
            gettyre(device_settings, ds);
            ds->threshold = 0;
            int found = config_setting_lookup_float(device_settings, "threshold", &ds->threshold);
        }

        if (ds->dev_type == SIMDEV_SOUND)
        {
            slogi("reading configured sound device settings");
            ds->sounddevsettings.frequency = 0;
            ds->sounddevsettings.frequencyMax = 0;
            ds->sounddevsettings.amplitude = 50;
            ds->sounddevsettings.amplitudeMax = 50;
            ds->sounddevsettings.volume = 0;
            ds->sounddevsettings.pan = 0;
            ds->sounddevsettings.channels = 1;
            ds->sounddevsettings.duration = 2.0;
            if (ds->effect_type == EFFECT_GEARSHIFT)
            {
                ds->sounddevsettings.duration = .125;
            }
            if (device_settings != NULL)
            {

                config_setting_lookup_int(device_settings, "volume", &ds->sounddevsettings.volume);
                config_setting_lookup_int(device_settings, "frequency", &ds->sounddevsettings.frequency);
                config_setting_lookup_int(device_settings, "frequencyMax", &ds->sounddevsettings.frequencyMax);
                config_setting_lookup_int(device_settings, "amplitude", &ds->sounddevsettings.amplitude);
                config_setting_lookup_int(device_settings, "amplitudeMax", &ds->sounddevsettings.amplitudeMax);
                config_setting_lookup_int(device_settings, "pan", &ds->sounddevsettings.pan);
                config_setting_lookup_int(device_settings, "channels", &ds->sounddevsettings.channels);
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


                found = config_setting_lookup_string(device_settings, "modulation", &temp);
                ds->sounddevsettings.modulation = SOUND_EFFECT_MODULATION_NONE;
                if (found == 0)
                {
                    ds->sounddevsettings.modulation = SOUND_EFFECT_MODULATION_NONE;
                    slogd("Effect modulation not found, set to none");
                }
                else
                {
                    if(strcicmp(temp, "FREQUENCY") == 0)
                    {
                        ds->sounddevsettings.modulation = SOUND_EFFECT_MODULATION_FREQUENCY;
                        if(ds->sounddevsettings.frequencyMax == 0 || ds->sounddevsettings.frequencyMax < ds->sounddevsettings.frequency)
                        {
                            ds->sounddevsettings.modulation = SOUND_EFFECT_MODULATION_NONE;
                            slogw("Falling back to no frequency modulation since frequencyMax is either not set or set below target frequency");
                        }
                        else
                        {
                            slogi("Effect modulation found, set to FREQUENCY");
                        }
                    }
                    else if(strcicmp(temp, "AMPLIFY") == 0)
                    {
                        ds->sounddevsettings.modulation = SOUND_EFFECT_MODULATION_AMPLIFY;
                        slogi("Effect modulation found, set to AMPLIFY");
                    }
                    else
                    {
                        slogw("%s is not a valid modulation type, falling back to no effect modulation");
                        ds->sounddevsettings.modulation = SOUND_EFFECT_MODULATION_NONE;
                    }
                }
            }
        }
    }

    if (ds->dev_type == SIMDEV_SERIAL)
    {
        if (device_settings != NULL)
        {
            const char* temp;
            config_setting_lookup_string(device_settings, "devpath", &temp);
            ds->serialdevsettings.portdev = strdup(temp);

            int motorposition = 8;
            config_setting_lookup_int(device_settings, "motors", &motorposition);
            ds->serialdevsettings.motorsposition = motorposition;

            int numlights = 6;
            config_setting_lookup_int(device_settings, "numlights", &numlights);
            ds->serialdevsettings.numlights = numlights;

            int numleds = 6;
            config_setting_lookup_int(device_settings, "numleds", &numleds);
            ds->serialdevsettings.numleds = numleds;

            int startled = 1;
            config_setting_lookup_int(device_settings, "startled", &startled);
            ds->serialdevsettings.startled = startled;

            int endled = 1;
            config_setting_lookup_int(device_settings, "endled", &endled);
            ds->serialdevsettings.endled = endled;

            int baud = 9600;
            config_setting_lookup_int(device_settings, "baud", &baud);
            ds->serialdevsettings.baud = baud;

            double ampfactor = 1.0;
            ds->serialdevsettings.ampfactor = 1.0;
            int found = config_setting_lookup_float(device_settings, "ampfactor", &ampfactor);
            ds->serialdevsettings.ampfactor = ampfactor;

            slogt("set port baud rate to %i, ampfactor %f", baud, ampfactor);

            ds->has_config = false;
            const char* temp2;
            found = config_setting_lookup_string(device_settings, "config", &temp2);
            slogt("config is %s found is %i", temp2, found);
            if(strcicmp(temp2, "none") == 0)
            {
                slogt("config set to none");
            }
            else
            {
                ds->has_config = true;
                ds->serialdevsettings.config_file = strdup(temp2);
                slogt("will try to load config file at %s", ds->serialdevsettings.config_file);
            }
        }

    }

    return error;
}

int uiloadconfig(const char* config_file_str, int confignum, int configureddevices, MonocoqueSettings* ms, DeviceSettings* ds)
{
    int numdevices = 0;
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, config_file_str))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    }
    else
    {
        slogi("Parsing config file");

        config_setting_t* config = NULL;
        config = config_lookup(&cfg, "configs");
        config_setting_t* selectedconfig = config_setting_get_elem(config, confignum);
        config_setting_t* config_devices = NULL;
        config_devices = config_setting_lookup(selectedconfig, "devices");

        int i = 0;

        int error = MONOCOQUE_ERROR_NONE;
        while (i<configureddevices)
        {
            error = MONOCOQUE_ERROR_NONE;
            DeviceSettings settings;

            config_setting_t* config_device = config_setting_get_elem(config_devices, i);
            const char* device_type;
            const char* device_subtype;
            const char* device_config_file;
            config_setting_lookup_string(config_device, "device", &device_type);
            config_setting_lookup_string(config_device, "type", &device_subtype);
            config_setting_lookup_string(config_device, "config", &device_config_file);

            //slogt("device type: %s", device_type);
            //slogt("device sub type: %s", device_subtype);
            //slogt("device config file: %s", device_config_file);
            if (error == MONOCOQUE_ERROR_NONE)
            {
                error = devsetup(device_type, device_subtype, device_config_file, ms, &settings, config_device);
            }
            if (error == MONOCOQUE_ERROR_NONE)
            {
                numdevices++;
            }
            ds[i] = settings;

            i++;

        }
    }


    config_destroy(&cfg);

    return numdevices;
}

int settingsfree(DeviceSettings ds)
{

    if (ds.dev_subtype == SIMDEVTYPE_SIMWIND || ds.dev_subtype == SIMDEVTYPE_SHIFTLIGHTS || ds.dev_subtype == SIMDEVTYPE_SIMLED)
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

int monocoquesettingsfree(MonocoqueSettings* ms)
{
    if(ms->tyre_diameter_config != NULL)
    {
        free(ms->tyre_diameter_config);
    }
    if(ms->config_str != NULL)
    {
        free(ms->config_str);
    }
    if(ms->log_filename_str != NULL)
    {
        free(ms->log_filename_str);
    }
    if(ms->log_dirname_str != NULL)
    {
        free(ms->log_dirname_str);
    }
}
