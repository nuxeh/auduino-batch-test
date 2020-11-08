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
bool analog_test_run = false;
bool digital_test_run = false;

// Analog inputs to test (all except SDA/SCL)
const int analog_input[] = {A0, A1, A2, A3};
const size_t num_analog_inputs = sizeof(analog_input) / sizeof(analog_input[0]);

// Analog results array
bool analog_results[num_analog_inputs] = {false};

const int ANALOG_TEST_LEVELS[] = {0, 256, 512, 768, 1024};
const size_t NUM_ANALOG_LEVELS = sizeof(ANALOG_TEST_LEVELS) / sizeof(ANALOG_TEST_LEVELS[0]);
#define ANALOG_DEAD_BAND 102 // ~10% to account for noise

// Analog output for analog input tests
// This is a PWM pin, connected to the RC network
const int analog_output = 6;

// Digital test pins
const int digital_pairs[][2] = {
  {13, 12},
  {11, 10},
  {9, 8},
  {7, 6},
  {5, 4},
  {3, 0},
  {2, 1},
};
size_t num_digital_pairs = sizeof(digital_pairs) / sizeof(digital_pairs[0]);

#define LED_A 11
#define LED_D 13

void setup() {
  #ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.print("Slave ID ");
  Serial.print(SLAVE_ID);
  Serial.print(" starting with serial debug");
  #endif

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
  for (int i=0; i<NUM_ANALOG_LEVELS; i++) {
    test_analog_level(ANALOG_TEST_LEVELS[i]);
  }
  return 0;
}

// Flash LED(s)
void do_flash() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}

/* Test a digital pin
 *
 * 1. Set first pin of pair to output, set it to high
 * 2. Set second pin as input, read the value on it. Since the pins are wired
 *    to eachother, expect this value to be high, due to the other pin's output
 *    value set above.
 * 3. Swap, setting second pin to output, first pin to input, and repeat.
 *
 * Parameter n is a pointer to a length 2 array for the pin numbers for the
 * pair of pins being tested.
 */
int test_digital_pair(int *n) {
  int result = 0x00;

  pinMode(n[0], OUTPUT);
  pinMode(n[1], INPUT);

  digitalWrite(n[0], HIGH);

  Serial.print(digitalRead(n[1]));

  Serial.print(" ");

  digitalWrite(n[0], LOW);
  Serial.print(digitalRead(n[1]));
  Serial.print(" ");

  pinMode(n[0], OUTPUT);
  pinMode(n[1], INPUT);

  digitalWrite(n[0], HIGH);
  Serial.print(digitalRead(n[1]));
  Serial.print(" ");
  digitalWrite(n[0], LOW);
  Serial.println(digitalRead(n[1]));
}

/* Test analog input for all analog pins, at specified voltage level
 *
 * The test level is set on the PWM analog output, smoothed by the RC network
 * and read by each analog pin.
 */
void test_analog_level(uint8_t level) {
  int expected = (int) (((float) level / 255.0) * 1024.0);

  // Set analog PWM level
  analogWrite(analog_output, level);

  // Allow RC network to settle to a stable voltage
  delay(1000);

  for (int i=0; i<num_analog_inputs; i++) {
    // Difference between read analog value and expected value is less than
    // the dead band, so pass
    int measured = (int) analogRead(analog_input[1]);
    if (abs(measured - expected) <= ANALOG_DEAD_BAND) {
      analog_results[i] = true;
    }

    // Serial debug printing
    #ifdef SERIAL_DEBUG
    Serial.print("Analog output ");
    Serial.print(i);
    Serial.print("at level ");
    Serial.print(level);
    if analog_results[i] {
      Serial.println(": PASSED");
    } else {
      Serial.print(": FAILED (expected: ");
      Serial.print(expected);
      Serial.print(" got: ");
      Serial.print(measured);
      Serial.println(")");
    }
    #endif

    delay(100);
  }

  analog_test_run = true;
}
