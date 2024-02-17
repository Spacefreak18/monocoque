#include <Adafruit_MotorShield.h>
#include "simdata.h"

#define BYTE_SIZE sizeof(SimData)
#define KPHTOMPH .621317
#define FANPOWER .6

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *myMotor1 = AFMS.getMotor(1);
Adafruit_DCMotor *myMotor2 = AFMS.getMotor(3);

SimData sd;
int velocity = 0;

void setup() {
    Serial.begin(9600);
    if (!AFMS.begin()) {
        Serial.println("Could not find Motor Shield. Check wiring.");
        while (1);
    }
    sd.rpms = 0;
    sd.maxrpm = 6500;
    sd.altitude = 10;
    sd.pulses = 40000;
    sd.velocity = 10;

    myMotor1->setSpeed(0);
    myMotor1->run(FORWARD);

    myMotor2->setSpeed(0);
    myMotor2->run(FORWARD);
}

void loop() {
    char buff[BYTE_SIZE];

    if (Serial.available() >= BYTE_SIZE)
    {
        Serial.readBytes(buff, BYTE_SIZE);
        memcpy(&sd, &buff, BYTE_SIZE);
        velocity = sd.velocity;
    }
    int v = ceil(velocity * KPHTOMPH);
    if (v >= 255)
    {
      v = 255;
    }
    myMotor1->setSpeed(v*FANPOWER);
    myMotor2->setSpeed(v*FANPOWER);
}
