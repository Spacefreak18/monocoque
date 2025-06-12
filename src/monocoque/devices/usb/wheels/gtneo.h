#ifndef _GTNEO_H
#define _GTNEO_H

#include "../../wheeldevice.h"
#include "../../simdevice.h"

#define SIMAGIC_GTNEO_VENDORID 0x3670
#define SIMAGIC_GTNEO_PRODUCTID 0x0805

#define SIMAGIC_MAX_PAYLOAD_SIZE 64
#define SIMAGIC_LED_REPORT_ID 0xf0 // LED control report id?

#define SIMAGIC_LED_DESCRIPTOR_LED_CONTROL 0xEC // LED control?
#define SIMAGIC_LED_DESCRIPTOR_PREPARE_LEDS 0x02 //probably something like 'prepare led canvas'
#define SIMAGIC_LED_DESCRIPTOR_SET_LEDS 0x03 //probably 'set temporare leds'

// expose whole LED array to the lua.
// Telemetry leds are 58-72
#define GT_NEO_LEDS_TELEMETRY_START 0
#define GT_NEO_LEDS_TOTAL 73

int simagic_gtneo_init(USBDevice* wheeldevice, const char* luafile);
int simagic_gtneo_free(USBDevice* wheeldevice);
int simagic_gtneo_customled_update(USBDevice* usbdevice, SimData* simdata);

#endif
