#include <Arduino.h>
#include <Wire.h>

// Define default slave ID if not already defined
#ifndef SLAVE_ID
#warning "SLAVE_ID has not been set, using default 0x01"
#define SLAVE_ID 0x01
#endif

// Flash the LED?
bool flash = false;

// Run the tests?
bool test = false;

// Test states
bool analog_pass = false;
bool digital_pass = false;

// Analog inputs to test (all except SDA/SCL)
int analog_input[] = {A0, A1, A2, A3};
size_t num_analog_inputs = sizeof(analog_input) / sizeof(analog_input[0]);

// Analog output for analog input tests
// This is a PWM pin, connected to the RC network
int analog_output = 6;

int digital_pairs[][2] = {
  {13, 12},
  {11, 10},
  {9, 8},
  {7, 6},
  {5, 4},
  {3, 0},
  {2, 1},
};
size_t num_digital_pairs = sizeof(digital_pairs) / sizeof(digital_pairs[0]);

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
