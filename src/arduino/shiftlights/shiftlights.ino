#include <FastLED.h>
#include "shiftlights.h"

#define SIMDATA_SIZE sizeof(ShiftLightsData)

#define LED_PIN     7
#define NUM_LEDS    6
#define BRIGHTNESS  40
#define STARTLED    1

#define COLOR1R 0
#define COLOR1G 255
#define COLOR1B 0
#define COLOR2R 255
#define COLOR2G 255
#define COLOR2B 0
#define COLOR3R 255
#define COLOR3G 0
#define COLOR3B 0

CRGB leds[NUM_LEDS];
ShiftLightsData sd;
int maxrpm = 0;
int rpm = 0;
int numlights = NUM_LEDS;
int pin = LED_PIN;
int lights[NUM_LEDS];


void setup()
{
    Serial.begin(115200);
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
    FastLED.setBrightness(BRIGHTNESS);
    for (int i = 0; i < numlights; i++)
    {
        leds[i] = CRGB ( 0, 0, 0);
        lights[i] = 0;
    }
    FastLED.clear();

    sd.litleds = 0;
    //sd.rpm = 0;
    //sd.maxrpm = 6500;
}

void loop()
{
    int l = 0;
    int lit = sd.litleds;
    char buff[SIMDATA_SIZE];

    if (Serial.available() >= SIMDATA_SIZE)
    {
        Serial.readBytes(buff, SIMDATA_SIZE);
        memcpy(&sd, &buff, SIMDATA_SIZE);
        lit = sd.litleds;
    }

    l = 0;
    FastLED.clear();
    while (l < lit)
    {
        if (l >= numlights / 2)
        {
            leds[l+STARTLED-1] = CRGB ( COLOR2R, COLOR2G, COLOR2B);
        }
        if (l < numlights / 2)
        {
            leds[l+STARTLED-1] = CRGB ( COLOR1R, COLOR1G, COLOR1B);
        }
        if (l == numlights - 1)
        {
            leds[l+STARTLED-1] = CRGB ( COLOR3R, COLOR3G, COLOR3B);
        }
        //if (lights[l] <= 0)
        //{
        //    leds[l] = CRGB ( 0, 0, 0);
        //}
        l++;
    }
    FastLED.show();
}
