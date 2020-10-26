#include <Arduino.h>

#include <Wire.h>

bool flash = false;
bool test = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  do_flash();

  Wire.begin(SLAVE_ID);
  Wire.onReceive(receiveEvent);
}

void loop() {
  if (flash) {
    do_flash();
  };

  if (test) {
    self_test();
  };

  delay(100);
}

void receiveEvent(int n) {
  char a = Wire.read();
  char b = Wire.read();

  // "Go test" signal
  if (a == 'G' && b == 'T') {
    flash = true;
    test = true;
  }

  // Ensure buffer is empty
  while (Wire.available() > 0) {
    Wire.read();
  }
}

// Analogue self test
// Returns 0 for success
int self_test() {
  return 0;
}

// Flash LED(s)
void do_flash() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}
