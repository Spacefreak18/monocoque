#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <hidapi/hidapi.h>

#include "gtneo.h"
#include <string.h>
#include "../../../slog/slog.h"
#include "../../serial/arduinoledlua.h"

// Simagic allows setting any led, therefore LED index should be defined.
struct LED {
    uint8_t index;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

// Simagic is using SEND_FEATURE_REPORT
// ReportID is 240 (0xf0)
// Data length is 64 bytes
// Prefix is 00 00 00 00 00
// Then we have some descriptor. I had 0xEC and 0x03.
// Then: length of LED array
// Then: number of LED in hex
//       RGB value for that LED


void prepare_device(hid_device *device) {
    uint8_t prepare_report[SIMAGIC_MAX_PAYLOAD_SIZE] = {0};
    prepare_report[0] = 0x00;
    prepare_report[1] = 0x00;
    prepare_report[2] = 0x00;
    prepare_report[3] = 0x00;
    prepare_report[4] = 0x00;
    prepare_report[5] = SIMAGIC_LED_DESCRIPTOR_LED_CONTROL;
    prepare_report[6] = SIMAGIC_LED_DESCRIPTOR_PREPARE_LEDS;
    prepare_report[7] = 0x01;

    uint8_t report_id_with_data[SIMAGIC_MAX_PAYLOAD_SIZE] = {SIMAGIC_LED_REPORT_ID}; // Report ID at the start
    memcpy(&report_id_with_data[1], prepare_report, SIMAGIC_MAX_PAYLOAD_SIZE-1);

    // Send the HID feature report
    int res = hid_send_feature_report(device, report_id_with_data, sizeof(report_id_with_data));
    if (res < 0) {
        fprintf(stderr, "Failed to send HID feature report\n");
    }
}

/*
    Arbitrary function for sending out leds to simagic. 
    Inputs: 
        device: hid_device handle
        leds: array of the struct LED
        led_count: count of leds in leds array
*/
int send_leds(hid_device *device, struct LED* leds, size_t led_count) {
    uint8_t descriptor[2] = {SIMAGIC_LED_DESCRIPTOR_LED_CONTROL, SIMAGIC_LED_DESCRIPTOR_SET_LEDS};

    prepare_device(device);
    int result = 0;
    size_t led_i = 0;
    while (led_i < led_count) {
        size_t chunk_length = (SIMAGIC_MAX_PAYLOAD_SIZE -1-9) / 4;  // 1 for reportid, 9 bytes for header, rest for LED data (each LED is 4 bytes)
        size_t chunk_size = chunk_length * 4;

        uint8_t report[SIMAGIC_MAX_PAYLOAD_SIZE] = {0}; // SIMAGIC_MAX_PAYLOAD_SIZE bytes per report
        memcpy(report, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00}, 5);  // Set the prefix (5 bytes)

        memcpy(&report[5], descriptor, 2);  // Set the descriptor (2 bytes)

        // Fill in the LED data
        for (size_t i = 0; i < chunk_length && led_i + i < led_count; ++i) {
            memcpy(&report[8+i*4], &leds[led_i+i], 4);
            report[7]++; // Set the LED array length for this chunk
        }

        uint8_t report_id_with_data[SIMAGIC_MAX_PAYLOAD_SIZE] = {SIMAGIC_LED_REPORT_ID};
        memcpy(&report_id_with_data[1], report, SIMAGIC_MAX_PAYLOAD_SIZE-1);

        // Send the HID feature report
        int res = hid_send_feature_report(device, report_id_with_data, sizeof(report_id_with_data));
        if (res < 0) {
            fprintf(stderr, "Failed to send HID feature report chunk\n");
            return -1;
        }
        result += res;

        // Move to the next chunk of LEDs
        led_i += chunk_length;
    }
    return result;
}


// default simpro profile:
// 97: b b b b b b b b b b b b b b b //flashing
// 93: b b b b b b b b b b b b b b b //flashing
// 90: g g g g g g g g g y y y w w w
// 88: g g g g g g g g g y y y
// 85: g g g g g g g g g
// 83: g g g g g g g
// 78: g g g g g
// 75: g g g
// 70: g
// less: empty (000000)
// Telemetry LEDs are 58-72


int simagic_gtneo_customled_update(USBDevice* usbdevice, SimData* simdata) 
{
    int res = 1;

    size_t led_count = GT_NEO_LEDS_TOTAL-GT_NEO_LEDS_TELEMETRY_START; // Expose whole LED array
    size_t bufsize = (led_count * 3);
    char ledbytes[bufsize];
    for(int x = 0; x < bufsize; x++)
        ledbytes[x] = 0x00;

    lua_State* L = usbdevice->m.L;

    lua_pushstring(L, "buff");
    lua_pushlightuserdata(L, &ledbytes);
    lua_settable(L, LUA_REGISTRYINDEX);

    simdata_to_lua(L, simdata);
    lua_setglobal(L, "simdata");

    lua_pushinteger(L, led_count);
    lua_setglobal(L, "TotalLeds");

    lua_register(L, "set_led_to_color", set_led_to_color);
    lua_register(L, "set_led_range_to_color", set_led_range_to_color);
    lua_register(L, "set_led_to_rgb_color", set_led_to_rgb_color);
    lua_register(L, "set_led_range_to_rgb_color", set_led_range_to_rgb_color);
    lua_register(L, "led_clear_all", led_clear_all);

    lua_pushinteger(L, LUALEDCOLOR_RED);
    lua_setglobal(L, "RED");
    lua_pushinteger(L, LUALEDCOLOR_GREEN);
    lua_setglobal(L, "GREEN");
    lua_pushinteger(L, LUALEDCOLOR_BLUE);
    lua_setglobal(L, "BLUE");
    lua_pushinteger(L, LUALEDCOLOR_YELLOW);
    lua_setglobal(L, "YELLOW");
    lua_pushinteger(L, LUALEDCOLOR_ORANGE);
    lua_setglobal(L, "ORANGE");

    lua_getglobal(L,"myFunc");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        fprintf(stderr, "Error calling Lua script: %s\n", lua_tostring(L, -1));
    }

    // ledbytes is an array of leds. We need to copy colours to our struct LED and call 'send_leds'
    struct LED teleleds[led_count];

    for (int i = 0; i < led_count; i++) {
        teleleds[i] = (struct LED){GT_NEO_LEDS_TELEMETRY_START+i, 0x00, 0x00, 0x00};
        teleleds[i].red = ledbytes[(i * 3) + 0]; 
        teleleds[i].green = ledbytes[(i * 3) + 1]; 
        teleleds[i].blue = ledbytes[(i * 3) + 2]; 
    }

    res = send_leds(usbdevice->handle, teleleds, led_count);
    return res;
}


int simagic_gtneo_init(USBDevice* wheeldevice, const char* luafile)
{
    wheeldevice->u.wheeldevice.useLua = false;

    slogi("initializing Simagic GT Neo wheel...");

    int res = 0;

    res = hid_init();

    wheeldevice->handle = hid_open(SIMAGIC_GTNEO_VENDORID, SIMAGIC_GTNEO_PRODUCTID, NULL);

    if (!wheeldevice->handle)
    {
        sloge("Could not find attached GT Neo Wheel");
        res = hid_exit();
        return 1;
    }
    slogd("Found Simagic GT Neo Wheel...");

    slogt("Using lua file");

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    //int top=lua_gettop(L);
    int status = luaL_loadfile(L, luafile);

    if (status) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        sloge("There is an issue with your lua script");
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return -1;
        //exit(1);
    }

    wheeldevice->u.wheeldevice.useLua = true;
    lua_setglobal(L,"myFunc");

    wheeldevice->m.L = L;

    return res;
}


int simagic_gtneo_free(USBDevice* wheeldevice)
{
    int res = 0;

    hid_close(wheeldevice->handle);
    res = hid_exit();

    return res;
}
