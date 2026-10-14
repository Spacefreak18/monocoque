#pragma once

#include <gui/combo.h>
#include <gui/listbox.h>

#include <libconfig.h>

#include "../helper/confighelper.h"

int find_default_config(config_setting_t *configs);
void populate_combo_box(Combo* combo, config_t* cfg);
void populate_device_list(ListBox *listbox, config_t* cfg, int config_force);
void add_serial_devices_to_combo(Combo *combo);
void add_sound_devices_to_combo(Combo *combo);
