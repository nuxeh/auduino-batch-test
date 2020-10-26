#include <Arduino.h>
#include <Wire.h>

#define SWITCH 2

#ifndef SLAVE_ID
#warning "MAX_SLAVES has not been set, using default 14"
#define MAX_SLAVES 14
#endif

void setup() {
  Serial.begin(115200);

  // Pin initialisations
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SWITCH, INPUT_PULLUP);

  // Begin I2C as master
  Wire.begin();
  Wire.onReceive(receiveEvent);

  Serial.print("Master arduino waiting for input.");
}

void loop() {
  // Wait for switch to be pressed
  while (digitalRead(SWITCH) == HIGH) {
    do_flash();
    delay(200);
  }

  // Detect button start event with simple debouncing
  if (digitalRead(SWITCH) == LOW) {
    delay(50);

    if (digitalRead(SWITCH) == LOW) {
      Serial.println("Button pressed, starting scan");

      // Run scan
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

      // Start self test
      run_test(addr);
    }

    delay(200);
  }
}

// Signal to slave to start self-test
void run_test(int addr) {
    Serial.print("Starting self-test for slave ID ");
    Serial.println(addr);

    Wire.beginTransmission(addr);
    Wire.write("GT");
    Wire.endTransmission();
}

// Receive response
// - ACK for self test
void receiveEvent(int n) {

}

// Flash LED
void do_flash() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
}
