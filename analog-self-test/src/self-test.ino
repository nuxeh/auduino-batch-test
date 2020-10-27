#include <Arduino.h>

#define SWITCH 2

// Define input and output analog pins
int input = A5;
int outputs[] = {A0, A1, A2, A3, A4};
size_t num_outputs = sizeof(outputs) / sizeof(outputs[0]);

void setup() {
  Serial.begin(115200);

  // Pin initialisations
  // LED and switch
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SWITCH, INPUT_PULLUP);

  // Analog test input
  pinMode(input, INPUT);

  Serial.println("Analog self test demo.");
}

// Run test when button pressed
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
      Serial.println("Button pressed, starting test");

      // Run scan
      self_test();

      // Wait for switch to be released
      while (digitalRead(SWITCH) == LOW) {}
    }
  }

  delay(50);
}

// Run tests for all channels
void self_test() {
  for (uint8_t chan=1; chan<num_outputs; chan++) {
      Serial.print("Testing analog output ");
      Serial.println(chan);

      // Run the test
      int result = run_analog_self_test(chan);

      Serial.print("Result: ");
      Serial.println(result);
  }
}

// Run self test for one output channel
int run_analog_self_test(int chan) {
  int result;

  // Get the pin number for the test channel
  int test_chan = outputs[chan];

  // Set output under test to output
  pinMode(test_chan, OUTPUT);

  // Set all other output pins to high impedance
  for (uint8_t i=0; i<num_outputs; i++) {
    pinMode(outputs[i], INPUT); // Set to high impedance
  }

  // Off (0V)
  // Target is 0 with dead band of 20 to allow for noise
  digitalWrite(test_chan, HIGH);
  result = analogRead(input);

  Serial.print("Read: ");
  Serial.println(result);

  if (result > 20) {
    return 1;  // Fail
  }

  delay(250);

  // On (5V)
  // Target is 255 with dead band of 20 for noise
  digitalWrite(test_chan, LOW);
  result = analogRead(input);

  Serial.print("Read: ");
  Serial.println(result);

  if (result < 235) {
    return 1;  // Fail
  }

  delay(250);

  // 50% duty cycle PWM (2.5V)
  // Target is 127 +/- 20
  analogWrite(test_chan, 127);
  result = analogRead(input);

  Serial.print("Read: ");
  Serial.println(result);

  if (result < 107 || result > 147) {
    return 1;  // Fail
  }

  return 0; // Pass
}

// Flash LED
void do_flash() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
}
