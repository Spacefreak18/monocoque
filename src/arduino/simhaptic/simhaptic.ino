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
int motor1 = 0;
int motor2 = 0;
int motor3 = 0;
int motor4 = 0;
int effect1 = 0;
int effect2 = 0;
int effect3 = 0;
int effect4 = 0;

void setup() {
    Serial.begin(9600);
    if (!AFMS.begin()) {
        Serial.println("Could not find Motor Shield. Check wiring.");
        while (1);
    }
    sd.motor1 = 0;
    sd.motor2 = 0;
    sd.motor3 = 0;
    sd.motor4 = 0;

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
        motor1 = sd.motor1;
        motor2 = sd.motor2;
        motor3 = sd.motor3;
        motor4 = sd.motor4;
        effect1 = sd.effect1;
        effect2 = sd.effect2;
        effect3 = sd.effect3;
        effect4 = sd.effect4;
    }

    //if (motor == 0 || motor == 4 || motor == 7 || motor == 8 || motor == 10 || motor == 11 || motor == 13 || motor == 14)
    //{
    if (motor1 >= 1)
    {
        myMotor1->setSpeed(effect1);
    }
    //}
    //if (motor == 1 || motor == 5 || motor == 7 || motor == 9 || motor == 10 || motor == 11 || motor == 12 || motor == 13)
    //{
    //    myMotor2->setSpeed(motor2);
    //}
    //if (motor == 2 || motor == 6 || motor == 8 || motor == 9 || motor == 10 || motor == 11 || motor == 12 || motor == 14)
    //{
    if (motor3 >= 1)
    {
        myMotor3->setSpeed(effect3);
    }
    //}
    //if (motor == 3 || motor == 4 || motor == 5 || motor == 6 || motor == 10 || motor == 12 || motor == 13 || motor == 14)
    //{
    //    myMotor4->setSpeed(motor4);
    //}
}
