#include <Arduino.h>

#include <Wire.h>

bool flash = false;

void setup() {
  Wire.begin(SLAVE_ID);
  Wire.onReceive(receiveEvent);
}

void loop() {
  if (flash) {
    do_flash();
  };
  delay(100);
}

void receiveEvent(int howMany)
{
  while(1 < Wire.available()) {
    char c = Wire.read();
  }

  int x = Wire.read();
}

// Self test
// Returns 0 for success
int self_test() {
  return 0;
}

void do_flash() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}
