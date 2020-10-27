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
  //pinMode(input, INPUT);

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
  for (uint8_t chan=0; chan<num_outputs; chan++) {
      Serial.print("Testing analog output ");
      Serial.println(chan);

      // Run the test
      int result = run_analog_self_test(chan);

      Serial.print("Result: ");
      Serial.print(result);
      if (result == 0) {
        Serial.println(" PASS");
      } else {
        Serial.println(" FAIL");
      }
  }
}

// Run self test for one output channel
int run_analog_self_test(int chan) {
  int result;

  // Get the pin number for the test channel
  int test_chan = outputs[chan];

  // Set all other output pins to high impedance
  for (uint8_t i=0; i<num_outputs; i++) {
    pinMode(outputs[i], INPUT); // Set to high impedance
  }

  // Set output under test to output
  pinMode(test_chan, OUTPUT);

  // Off (0V)
  // Target is 0 with dead band of 100 (10%) to allow for noise
  digitalWrite(test_chan, LOW);
  delay(2000);
  result = analogRead(input);

  Serial.print("Expect > 100. Read: ");
  Serial.println(result);

  if (result > 100) {
    return 1;  // Fail
  }

  delay(250);

  // On (5V)
  // Target is 255 with dead band of 100 (10%) for noise
  digitalWrite(test_chan, HIGH);
  delay(2000);
  result = analogRead(input);

  Serial.print("Expect < 924. Read: ");
  Serial.println(result);

  if (result < 924) {
    return 1;  // Fail
  }

  delay(250);

  // 50% duty cycle PWM (2.5V)
  // Target is 512 +/- 100
  analogWrite(test_chan, 180);
  delay(4000);
  result = analogRead(input);

  Serial.print("Expect 412 <= x <= 612. Read: ");
  Serial.println(result);

  // Reset analogue output
  digitalWrite(test_chan, 0);

  if (result < 412 || result > 612) {
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
