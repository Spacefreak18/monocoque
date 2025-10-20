// Called when starting the arduino (setup method in main sketch)
void setup() {
    Serial.begin(115200);
}

// Called when new data is coming from computer
void read() 
{
  if(Serial.available())
  {
    int speed = Serial.readStringUntil(';').toInt();
    int gear = Serial.readStringUntil(';').toInt();
    int rpms = Serial.readStringUntil(';').toInt();
    //Serial.println("speed: " + String(speed) + " " + "gear: " + String(gear) + " " + "rpms: " + String(rpms));
  }
}

// Called once per arduino loop, timing can't be predicted, 
// but it's called between each command sent to the arduino
void loop() {
  read();
}

// Called once between each byte read on arduino,
// THIS IS A CRITICAL PATH :
// AVOID ANY TIME CONSUMING ROUTINES !!!
// PREFER READ OR LOOP METHOS AS MUCH AS POSSIBLE
// AVOID ANY INTERRUPTS DISABLE (serial data would be lost!!!)
void idle() {
}
