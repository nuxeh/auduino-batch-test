#include <Arduino.h>
#include <Wire.h>

#define SWITCH 2

void setup() {
  Serial.begin(115200);

  // Pin initialisations
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SWITCH, INPUT_PULLUP);

  Wire.begin();
  Wire.onReceive(receiveEvent);
}

void loop() {
  // Wait for switch to be pressed
  while (digitalRead(SWITCH) == HIGH) {
    do_flash();
    delay(100);
  }

  // Detect button start event with simple debouncing
  if (digitalRead(SWITCH) == LOW) {
    delay(50);
    if (digitalRead(SWITCH) == LOW) {
      Serial.println("Button pressed, starting scan");
      scan();

      // Wait for switch to be released
      while (digitalRead(SWITCH) == LOW) {}
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

void do_flash() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
}
