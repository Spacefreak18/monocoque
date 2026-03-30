#include "simwind.h"

// ============================================================
// Configuration
// ============================================================
#define BAUD_RATE       115200
#define PWM_PIN_1       9       // Fan PWM output pins
#define PWM_PIN_2       10      //
#define PWM_MAX         320     // Max PWM value for 25kHz (16MHz / 25kHz / 2)
#define SPEED_MIN       0       // Min speed (mph) — fans off below this
#define SPEED_MAX       100     // Max speed (mph) — full fan speed at this
#define SPEED_THRESHOLD 5       // Below this speed fans are off
// ============================================================

#define BYTE_SIZE sizeof(SimWindData)

int velocity = 0;
int fanpower = 0;

void setup() {
    Serial.begin(BAUD_RATE);

    // Configure Timer1 for Phase Correct PWM at 25kHz duty cycle
    // Mode 10: Phase Correct PWM, TOP = ICR1
    // WGM13:WGM12:WGM11:WGM10 = 1:0:1:0
    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
    TCCR1B = _BV(WGM13) | _BV(CS10);   // <-- WGM13 restored, no prescaler
    ICR1 = PWM_MAX;                      // TOP = 320 → 16MHz / (2 * 320) = 25kHz
    pinMode(PWM_PIN_1, OUTPUT);
    pinMode(PWM_PIN_2, OUTPUT);
    OCR1A = 0;
    OCR1B = 0;
}

void loop() {
    char buff[BYTE_SIZE];
    if (Serial.available() >= BYTE_SIZE) {
        if (Serial.readBytes(buff, BYTE_SIZE) != BYTE_SIZE) return;
        union { SimWindData data; char bytes[BYTE_SIZE]; } u;
        memcpy(u.bytes, buff, BYTE_SIZE);
        velocity = u.data.velocity;
        fanpower = u.data.fanpower;
    }

    int pwm = 0;
    if (velocity > SPEED_THRESHOLD) {
        // Square root scaling for slightly stronger sensation at lower speeds
        // fanpower (0-255) scales the overall intensity
        float powerScale = (float)fanpower / 255.0;
        pwm = (int)(sqrt((float)velocity / SPEED_MAX) * PWM_MAX * powerScale);
        pwm = constrain(pwm, 0, PWM_MAX);
    }

    OCR1A = pwm;
    OCR1B = pwm;
}
