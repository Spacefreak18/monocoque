#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gui/combo.h>
#include <gui/listbox.h>

#include <libserialport.h>
#include <libconfig.h>

#include "../helper/confighelper.h"

typedef struct pa_devicelist {
	uint8_t initialized;
	char name[512];
	uint32_t index;
	char description[256];
} pa_devicelist_t;

static int matches_any(const char *value)
{
    return strcmp(value, "default") == 0 || strcmp(value, "all") == 0;
}

int find_default_config(config_setting_t *configs)
{
    int count = config_setting_length(configs);

    for (int i = 0; i < count; i++)
    {
        config_setting_t *entry = config_setting_get_elem(configs, i);

        // A key that isn't in the entry doesn't constrain the match. This used
        // to require all three, which no real config has: conf/monocoque.config
        // -- the example the README points at -- sets sim and car and never
        // mentions api, so every entry was skipped, -1 came back, and the
        // caller dereferenced the NULL that produced.
        const char *sim = "default";
        const char *api = "default";
        const char *car = "default";

        config_setting_lookup_string(entry, "sim", &sim);
        config_setting_lookup_string(entry, "api", &api);
        config_setting_lookup_string(entry, "car", &car);

        if (matches_any(sim) && matches_any(api) && matches_any(car))
        {
            return i;
        }
    }

    return -1;
}

int add_serial_devices_to_combo(Combo *combo)
{
    // from libserialport examples
    struct sp_port **port_list;
    
    enum sp_return result = sp_list_ports(&port_list);
    
    if (result != SP_OK)
    {
    	return -1;
    }
    
    int i;
    for (i = 0; port_list[i] != NULL; i++)
    {
        struct sp_port *port = port_list[i];
    
        char port_name[256];
        const char *name_ptr = sp_get_port_name(port);
            
        if (name_ptr != NULL)
        {
            snprintf(port_name, sizeof(port_name), "%s", name_ptr);
            combo_add_elem(combo, port_name, NULL);
        }
        else
        {
            port_name[0] = '\0';
        }
    }
    
    
    sp_free_port_list(port_list);
    
    
    return 0;
}

void populate_combo_box(Combo* combo, config_t* cfg)
{
    config_setting_t *configs;
    int count;
    int i;
    
    configs = config_lookup(cfg, "configs");
    if (configs == NULL || !config_setting_is_list(configs))
        return;
    
    count = config_setting_length(configs);
    
    for (i = 0; i < count; i++)
    {
        config_setting_t *cfg_entry;
        const char *sim = "";
        const char *car = "";
        const char *api = "";
        char text[256];
    
        cfg_entry = config_setting_get_elem(configs, i);
    
        config_setting_lookup_string(cfg_entry, "sim", &sim);
        config_setting_lookup_string(cfg_entry, "car", &car);
        config_setting_lookup_string(cfg_entry, "api", &api);
    
        snprintf(text, sizeof(text), "%s / %s / %s", sim, car, api);
    
        combo_add_elem(combo, text, NULL);
    }

}

void populate_device_list(ListBox *listbox, config_t* cfg)
{
    int count;
    config_setting_t* devices;
    devices = config_lookup(cfg, "configs");
    if (devices == NULL)
    {
        fprintf(stderr, "No configs section found\n");
        return;
    }

    config_setting_t* config = NULL;
    config = config_lookup(cfg, "configs");

    int config_num = find_default_config(config);
    if (config_num < 0)
    {
        // No entry claims to be the default one: list the first, which is what
        // a single-entry config means anyway, rather than nothing.
        config_num = 0;
    }

    config_setting_t* selectedconfig = config_setting_get_elem(config, config_num);
    if (selectedconfig == NULL)
    {
        fprintf(stderr, "No config entry to list devices for\n");
        return;
    }

    config_setting_t* config_devices = NULL;
    config_devices = config_setting_lookup(selectedconfig, "devices");
    if (config_devices == NULL)
    {
        // config_setting_lookup dereferences its argument, so reaching here
        // with a NULL selectedconfig used to segfault inside libconfig --
        // gmonocoque died on launch, before drawing a window, for anyone whose
        // config didn't produce a match above.
        fprintf(stderr, "Config entry has no devices section\n");
        return;
    }

    count = config_setting_length(config_devices);

    for (int i = 0; i < count; i++)
    {
        config_setting_t *dev = config_setting_get_elem(config_devices, i);

        const char *device = "";
        const char *type = "";
        const char *subtype = "";
        const char *effect = "";
        const char *tyre = "";

        config_setting_lookup_string(dev, "device", &device);

        char text[256];

        if (strcasecmp(device, "sound") == 0)
        {
            config_setting_lookup_string(dev, "effect", &effect);
            config_setting_lookup_string(dev, "tyre", &tyre);

            snprintf(text, sizeof(text), "%s - %s - %s",
                     device, effect, tyre);
        }
        else
        {
            config_setting_lookup_string(dev, "type", &type);
            config_setting_lookup_string(dev, "subtype", &subtype);

            snprintf(text, sizeof(text), "%s - %s - %s",
                     device, type, subtype);
        }

        listbox_add_elem(listbox, text, NULL);
    }
}


void pa_sinklist_cb(pa_context *c, const pa_sink_info *l, int eol, void *userdata) {
    pa_devicelist_t *pa_devicelist = userdata;
    int ctr = 0;

    // If eol is set to a positive number, you're at the end of the list
    if (eol > 0) {
	return;
    }

    // We know we've allocated 16 slots to hold devices.  Loop through our
    // structure and find the first one that's "uninitialized."  Copy the
    // contents into it and we're done.  If we receive more than 16 devices,
    // they're going to get dropped.  You could make this dynamically allocate
    // space for the device list, but this is a simple example.
    for (ctr = 0; ctr < 16; ctr++) {
	if (! pa_devicelist[ctr].initialized) {
	    strncpy(pa_devicelist[ctr].name, l->name, 511);
	    strncpy(pa_devicelist[ctr].description, l->description, 255);
	    pa_devicelist[ctr].index = l->index;
	    pa_devicelist[ctr].initialized = 1;
	    break;
	}
    }
}


int add_sound_devices_to_combo(Combo *combo)
{
    pa_mainloop *mainloop;
    pa_context *context;
    pa_operation *operation;
    pa_context_state_t state;
    pa_devicelist_t output[16] = {0};
    int retval;
    //pa_devicelist_t* output = malloc(sizeof(pa_devicelist_t) * 16);
    //memset(output, 0, sizeof(pa_devicelist_t) * 16);
    mainloop = pa_mainloop_new();

    if (mainloop == NULL)
        return 0;

    context = pa_context_new(
        pa_mainloop_get_api(mainloop),
        "Monocoque");

    if (context == NULL)
    {
        pa_mainloop_free(mainloop);
        return 0;
    }

    if (pa_context_connect(context, NULL, 0, NULL) < 0)
    {
        pa_context_unref(context);
        pa_mainloop_free(mainloop);
        return 0;
    }

    /*
     * Wait for PulseAudio to become ready.
     */
    while (1)
    {
        if (pa_mainloop_iterate(mainloop, 1, &retval) < 0)
            goto error;

        state = pa_context_get_state(context);

        if (state == PA_CONTEXT_READY)
            break;

        if (!PA_CONTEXT_IS_GOOD(state))
            goto error;
    }

    /*
     * Enumerate all PulseAudio output devices (sinks).
     */
    operation = pa_context_get_sink_info_list(
        context,
        pa_sinklist_cb,
        &output);

    if (operation == NULL)
        goto error;

    /*
     * Run the mainloop until enumeration is complete.
     */
    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING)
    {
        if (pa_mainloop_iterate(mainloop, 1, &retval) < 0)
        {
            pa_operation_unref(operation);
            goto error;
        }
    }
    
    for (size_t i = 0; i < 16; i++)
    {
        if (!output[i].initialized)
        {
            continue;
        }

        if (output[i].name[0] == '\0')
        {
            continue;
        }
        combo_add_elem(combo, output[i].name, NULL);
    }

    pa_operation_unref(operation);

    pa_context_disconnect(context);
    pa_context_unref(context);
    pa_mainloop_free(mainloop);

    return 1;

error:

    pa_context_disconnect(context);
    pa_context_unref(context);
    pa_mainloop_free(mainloop);

    return 0;
}
