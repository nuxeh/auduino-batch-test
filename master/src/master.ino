#include <Arduino.h>

#include <Wire.h>

#define SWITCH 2

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.onReceive(receiveEvent);
}

void loop() {
  if (digitalRead(SWITCH) == HIGH) {
    delay(50);
    if (digitalRead(SWITCH) == HIGH) {
      Serial.println("Button pressed, starting scan");
      scan();
    }
  }
}

void scan() {
  for (int address=1; address < MAX_SLAVES; ++address) {

  }

}

void receiveEvent(int howMany)
{
  while(1 < Wire.available()) {
    char c = Wire.read();
  }

  int x = Wire.read();
}
