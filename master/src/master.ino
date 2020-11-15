#include <Arduino.h>
#include <Wire.h>

#define SWITCH 2

#ifndef MAX_SLAVES
#warning "MAX_SLAVES has not been set, using default 14"
#define MAX_SLAVES 14
#endif

bool test_started[128] = {false};
bool test_results_received[127] = {false};

void setup() {
  Serial.begin(115200);

  // Pin initialisations
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SWITCH, INPUT_PULLUP);

  // Begin I2C as master
  Wire.begin();
  Wire.onReceive(receiveEvent);

  Serial.println("Master arduino waiting for input...");
}

void loop() {
  // Wait for switch to be pressed
  while (digitalRead(SWITCH) == HIGH) {
    do_flash(true);
    poll_results();
  }

  // Detect button start event with simple debouncing
  if (digitalRead(SWITCH) == LOW) {
    delay(50);

    if (digitalRead(SWITCH) == LOW) {
      Serial.println("Button pressed, starting scan");

      // Run scan
      digitalWrite(LED_BUILTIN, HIGH); // LED on
      scan();

      // Wait for switch to be released
      while (digitalRead(SWITCH) == LOW) {
        do_flash(false);
        poll_results();
      }
    }
  }
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

    test_started[addr] = true;
}

unsigned long last_poll = 0;

// Non-blocking/"Asynchronously" poll results every 2 seconds
void poll_results() {
  if (millis() - last_poll > 2000) {
    _poll_results();
    last_poll = millis();
  }
}

// Poll results
void _poll_results() {
    // Check test run status for all possible slaves
    for (int addr=0; addr<128; addr++) {
      // Make requests for those slaves for which a test has started, but
      // for which no results have yet been received.
      if (test_started[addr] && ! test_results_received[addr]) {
        // Request slave status
        Wire.beginTransmission(addr);
        Wire.write("RS"); // Request status
        Wire.endTransmission();
        Wire.requestFrom(addr, 1);

        // Check if test has completed on slave
        // Request results if so
        if (Wire.read() == 0x01) {
          Wire.beginTransmission(addr);
          Wire.write("RR"); // Request results
          Wire.endTransmission();
          Wire.requestFrom(addr, 3);
          Serial.println(Wire.read(), BIN);
          Serial.println(Wire.read(), BIN);
          Serial.println(Wire.read(), BIN);
        }
      }
    }
}

// Receive response
// - ACK for self test
void receiveEvent(int n) {

}

unsigned long last_flash = 0;
int led_state = LOW;

// Flash LED (non-blocking)
// Allowing fast polled response to button press
// Using pin interrupt would be an alternative for switch detection
void do_flash(bool long_flash) {
  unsigned int del;

  if (long_flash) {
    del = 1000;
  } else {
    del = 500;
  };

  if (millis() - last_flash > del) {
    if (led_state == LOW) {
      digitalWrite(LED_BUILTIN, HIGH);
      led_state = HIGH;
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      led_state = LOW;
    };
    last_flash = millis();
  }
}
