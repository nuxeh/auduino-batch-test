#include <Arduino.h>
#include <Wire.h>

// Define default slave ID if not already defined
#ifndef SLAVE_ID
#warning "SLAVE_ID has not been set, using default 0x01"
#define SLAVE_ID 0x01
#endif

// Enable for serial debug
//#define SERIAL_DEBUG

// Run the tests?
bool test = false;

// Test states
bool analog_pass = false;
bool digital_pass = false;
bool analog_test_run = false;
bool digital_test_run = false;

// Analog inputs to test (all except SDA/SCL)
const int ANALOG_INPUTS[] = {A0, A1, A2, A3};
const size_t NUM_ANALOG_TEST_INPUTS = sizeof(ANALOG_INPUTS) / sizeof(ANALOG_INPUTS[0]);

// Analog results array
// Initialised as pass, since we test multiple analog levels, if a pin fails
// on any level, it will be set to false
bool analog_results[NUM_ANALOG_TEST_INPUTS] = {true};

const uint8_t ANALOG_TEST_LEVELS[] = {0, 64, 127, 192, 255};
const size_t NUM_ANALOG_LEVELS = sizeof(ANALOG_TEST_LEVELS) / sizeof(ANALOG_TEST_LEVELS[0]);
// Dead band ~10% to account for noise
#define ANALOG_DEAD_BAND 102

// Analog output for analog input tests
// This is a PWM pin, connected to the RC network
#define ANALOG_OUTPUT 6

// Digital test pins
const int DIGITAL_PAIRS[][2] = {
  {13, 12},
  {11, 10},
  {9, 8},
  {7, 6},
  {5, 4},
#ifndef SERIAL_DEBUG
  {3, 0},
  {2, 1},
#endif
};
const size_t NUM_DIGITAL_PAIRS = sizeof(DIGITAL_PAIRS) / sizeof(DIGITAL_PAIRS[0]);

// Digital results array, one entry per digital pin
// Set to true on successful test of a pin pair
bool digital_results[14] = {false};

bool analog_result = false;
bool digital_result = false;

#define LED_A 13
#define LED_D 11

void setup() {
  #ifdef SERIAL_DEBUG
  Serial.begin(115200);
  Serial.print("Slave ID ");
  Serial.print(SLAVE_ID);
  Serial.println(" starting with serial debug");

  // Start tests automatically when serial debug enabled
  test = true;
  #endif

  Wire.begin(SLAVE_ID);
  Wire.onReceive(receiveEvent);
}

void loop() {
  if (test) {
    test = false;
    do_flash(10); // Flash LEDs on test start
    reset_results();
    self_test();
    display_results();

    // Wait before repeating tests
    #ifdef SERIAL_DEBUG
    delay(10000);
    #endif
  };
}

void receiveEvent(int n) {
  char a = Wire.read();
  char b = Wire.read();

  // "Go test" signal
  if (a == 'G' && b == 'T') {
    test = true;
  }

  // Ensure buffer is empty
  while (Wire.available() > 0) {
    Wire.read();
  }
}

// Light LEDs to show results of both tests
// Successful test is shown with LED lit
void display_results() {
  analog_result = true;
  digital_result = true;

  // Check result for each digital_result pin
  for (size_t i=0; i<NUM_ANALOG_TEST_INPUTS; i++) {
    if (!analog_results[i]) {
      analog_result = false;
    }
  }

  #ifdef SERIAL_DEBUG
  int start = 4; // Skip serial pins, which aren't tested
  #else
  int start = 0;
  #endif

  // Check result for each digital_result pin
  for (int i=start; i<14; i++) {
    if (!digital_results[i]) {
      digital_result = false;
    }
    #ifdef SERIAL_DEBUG
    Serial.print(i);
    Serial.print(": ");
    Serial.println(digital_results[i]);
    #endif
  }

  // Set LEDs to output
  pinMode(LED_D, OUTPUT);
  pinMode(LED_A, OUTPUT);

  // Display results
  digitalWrite(LED_D, digital_result);
  digitalWrite(LED_A, analog_result);

  // Serial debug
  #ifdef SERIAL_DEBUG
  Serial.println("Analogue test: ");
  if (analog_result) {
    Serial.println("PASSED");
  } else {
    Serial.println("FAILED");
  }
  Serial.println("Digital test: ");
  if (digital_result) {
    Serial.println("PASSED");
  } else {
    Serial.println("FAILED");
  }
  #endif
}

// Reset results to starting state
void reset_results() {
  // Digital
  for (int i=0; i<14; i++) {
    digital_results[i] = false;
  }

  // Analog
  for (size_t i=0; i<NUM_ANALOG_TEST_INPUTS; i++) {
    analog_results[i] = true;
  }

  analog_result = false;
  digital_result = false;

}

// Analogue self test
// Returns 0 for success
int self_test() {
  // Run analog test for all levels defined in ANALOG_TEST_LEVELS
  for (size_t i=0; i<NUM_ANALOG_LEVELS; i++) {
    // Serial print
    #ifdef SERIAL_DEBUG
    Serial.print("Running analog test at level: ");
    Serial.println(ANALOG_TEST_LEVELS[i]);
    #endif

    // Run the test
    test_analog_level(ANALOG_TEST_LEVELS[i]);
  }

  // Run digital test for each pin pair
  for (size_t i=0; i<NUM_DIGITAL_PAIRS; i++) {
    test_digital_pair(DIGITAL_PAIRS[i]);
  }

  digital_test_run = true;

  return 0;
}

// Flash LED(s) n times
void do_flash(int n) {
  for (int i=0; i<n; i++) {
    digitalWrite(LED_A, HIGH);
    digitalWrite(LED_D, HIGH);
    delay(75);
    digitalWrite(LED_A, LOW);
    digitalWrite(LED_D, LOW);
    delay(75);
  }
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
void test_digital_pair(const int *n) {
  bool result = true;

  // Set pin modes, first test
  // First pin is output, second pin is input
  pinMode(n[0], OUTPUT);
  pinMode(n[1], INPUT);

  digitalWrite(n[0], HIGH);
  if (digitalRead(n[1]) != HIGH) {
    result = false;
  }

  digitalWrite(n[0], LOW);
  if (digitalRead(n[1]) != LOW) {
    result = false;
  }

  // Set pin modes, second test
  // Second pin is output, first pin is input
  pinMode(n[1], OUTPUT);
  pinMode(n[0], INPUT);

  digitalWrite(n[1], HIGH);
  if (digitalRead(n[0]) != HIGH) {
    result = false;
  }

  digitalWrite(n[1], LOW);
  if (digitalRead(n[0]) != LOW) {
    result = false;
  }

  // Set pins as high impedance now test has run
  pinMode(n[0], INPUT);
  pinMode(n[1], INPUT);

  // Update results (update both tested pins)
  if (result) {
    digital_results[n[0]] = true;
    digital_results[n[1]] = true;
  }

  // Serial debug printing
  #ifdef SERIAL_DEBUG
  Serial.print("Digital I/O ");
  Serial.print(n[0]);
  Serial.print("/");
  Serial.print(n[1]);
  if (result) {
    Serial.println(": PASSED");
  } else {
    Serial.println(": FAILED");
  }
  #endif
}

/* Test analog input for all analog pins, at specified voltage level
 *
 * The test level is set on the PWM analog output, smoothed by the RC network
 * and read by each analog pin.
 */
void test_analog_level(uint8_t level) {
  // Rescale to 10-bit
  int expected = (int) (((float) level / 255.0) * 1024.0);

  // Set analog PWM level
  analogWrite(ANALOG_OUTPUT, level);

  // Allow RC network to settle to a stable voltage
  delay(1000);

  for (size_t i=0; i<NUM_ANALOG_TEST_INPUTS; i++) {
    bool result = true;
    // Test case
    // Difference between read analog value and expected value is more than
    // the dead band, so fail
    pinMode(ANALOG_INPUTS[i], INPUT);
    int measured = (int) analogRead(ANALOG_INPUTS[i]);

    if (abs(measured - expected) > ANALOG_DEAD_BAND) {
      result = false;
    }

    if (!result) {
      analog_results[i] = false;
    }

    // Serial debug printing
    #ifdef SERIAL_DEBUG
    Serial.print("Analog input ");
    Serial.print(i);
    Serial.print(" at level ");
    Serial.print(level);
    if (result) {
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
