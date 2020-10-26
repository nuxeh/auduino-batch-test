#include <Arduino.h>
#include <Wire.h>

#define SWITCH 2

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.onReceive(receiveEvent);
}

void loop() {
  // Detect button start event with simple debouncing
  if (digitalRead(SWITCH) == HIGH) {
    delay(50);
    if (digitalRead(SWITCH) == HIGH) {
      Serial.println("Button pressed, starting scan");
      scan();
    }
  }
  delay(50);
}

// Scan I2C slave devices
void scan() {
  for (int addr=1; addr<MAX_SLAVES; addr++) {
    Wire.beginTransmission(addr);

    if (Wire.endTransmission() == 0) {
      // Device found
      Serial.print("Detected slave with ID ");
      Serial.println(addr);

      delay(50);

      // Signal self test
      Serial.print("Starting self-test");
      Wire.beginTransmission(addr);
      Wire.write("GT");
      Wire.endTransmission();
    }

    delay(100);
  }
}

// Receive response
// - ACK for self test
void receiveEvent(int n) {

}
