#include <FastLED.h>
#include "simdata.h"

#define SIMDATA_SIZE sizeof(SimData)

#define LED_PIN     7
#define NUM_LEDS    6
#define BRIGHTNESS  40

#define COLOR1A 0
#define COLOR1B 0
#define COLOR1C 255
#define COLOR2A 0
#define COLOR2B 255
#define COLOR2C 0
#define COLOR3A 255
#define COLOR3B 0
#define COLOR3C 0

CRGB leds[NUM_LEDS];
SimData sd;
int maxrpm = 0;
int rpm = 0;
int numlights = NUM_LEDS;
int pin = LED_PIN;
int lights[6];


void setup()
{
    Serial.begin(9600);
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
    FastLED.setBrightness(BRIGHTNESS);
    for (int i = 0; i < numlights; i++)
    {
        leds[i] = CRGB ( 0, 0, 0);
        lights[i] = 0;
    }
    FastLED.clear();

    sd.rpms = 0;
    sd.maxrpm = 6500;
    sd.altitude = 10;
    sd.pulses = 40000;
    sd.velocity = 10;
}

void loop()
{
    int l = 0;
    char buff[SIMDATA_SIZE];

    if (Serial.available() >= SIMDATA_SIZE)
    {
        Serial.readBytes(buff, SIMDATA_SIZE);
        memcpy(&sd, &buff, SIMDATA_SIZE);
        rpm = sd.rpms;
        maxrpm = sd.maxrpm;

    }


    while (l < numlights)
    {
        lights[l] = 0;
        l++;
    }
    l = -1;
    int rpmlights = 0;
    while (rpm > rpmlights)
    {
        if (l>=0)
        {
            lights[l] = 1;
        }
        l++;
        rpmlights = rpmlights + (((maxrpm-250)/numlights));
    }

    l = 0;
    FastLED.clear();
    while (l < numlights)
    {

        if (l >= numlights / 2)
        {
            leds[l] = CRGB ( COLOR1A, COLOR1B, COLOR1C);
        }
        if (l < numlights / 2)
        {
            leds[l] = CRGB ( COLOR2A, COLOR2B, COLOR2C);
        }
        if (l == numlights - 1)
        {
            leds[l] = CRGB ( COLOR3A, COLOR3B, COLOR3C);
        }
        if (lights[l] <= 0)
        {
            leds[l] = CRGB ( 0, 0, 0);
        }
        FastLED.show();
        l++;
    }
}
