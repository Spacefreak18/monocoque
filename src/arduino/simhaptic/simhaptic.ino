#include <Adafruit_MotorShield.h>
#include "simhaptic.h"

#define BYTE_SIZE sizeof(SimHapticData)
#define POWER .6

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *myMotor1 = AFMS.getMotor(1);
//Adafruit_DCMotor *myMotor2 = AFMS.getMotor(2);
Adafruit_DCMotor *myMotor3 = AFMS.getMotor(3);
//Adafruit_DCMotor *myMotor4 = AFMS.getMotor(4);

SimHapticData sd;
int effect = 0;
int motor = 15;

void setup() {
    Serial.begin(9600);
    if (!AFMS.begin()) {
        Serial.println("Could not find Motor Shield. Check wiring.");
        while (1);
    }
    sd.effect = 0;
    sd.motor = 15;

    myMotor1->setSpeed(0);
    myMotor1->run(FORWARD);

    //myMotor2->setSpeed(0);
    //myMotor2->run(FORWARD);

    myMotor3->setSpeed(0);
    myMotor3->run(FORWARD);

    //myMotor4->setSpeed(0);
    //myMotor4->run(FORWARD);
}

void loop() {
    char buff[BYTE_SIZE];

    if (Serial.available() >= BYTE_SIZE)
    {
        Serial.readBytes(buff, BYTE_SIZE);
        memcpy(&sd, &buff, BYTE_SIZE);
        effect = sd.effect;
        motor = sd.motor;
    }

    if (motor == 0 || motor == 4 || motor == 7 || motor == 8 || motor == 10 || motor == 11 || motor == 13 || motor == 14)
    {
        myMotor1->setSpeed(effect);
    }
    //if (motor == 1 || motor == 5 || motor == 7 || motor == 9 || motor == 10 || motor == 11 || motor == 12 || motor == 13)
    //{
    //    myMotor2->setSpeed(v*POWER);
    //}
    if (motor == 2 || motor == 6 || motor == 8 || motor == 9 || motor == 10 || motor == 11 || motor == 12 || motor == 14)
    {
        myMotor3->setSpeed(effect);
    }
    //if (motor == 3 || motor == 4 || motor == 5 || motor == 6 || motor == 10 || motor == 12 || motor == 13 || motor == 14)
    //{
    //    myMotor4->setSpeed(v*POWER);
    //}
}
