#include <Arduino.h>
#include <Wire.h>

#define SWITCH 2

#ifndef MAX_SLAVES
#warning "MAX_SLAVES has not been set, using default 14"
#define MAX_SLAVES 14
#endif

bool test_started[128] = {false};
bool test_results_received[128] = {false};
bool test_result[128] = {false};

void setup() {
  Serial.begin(115200);

  // Pin initialisations
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SWITCH, INPUT_PULLUP);

  // Begin I2C as master
  Wire.begin();
  Wire.onReceive(receiveEvent);

  // Use lower I2C clock speed (100 kHz)
  Wire.setClock(100000);

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
      reset_state();
      scan();

      // Wait for switch to be released
      while (digitalRead(SWITCH) == LOW) {
        do_flash(false);
        poll_results();
      }
    }
  }
}

// Reset test state for all possible slaves
void reset_state() {
  for (int i=0; i<128; i++) {
    test_started[i] = false;
    test_results_received[i] = false;
    test_result[i] = false;
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

    // Delay between test starts for slaves
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
    for (int addr=1; addr<128; addr++) {
      // Make requests for those slaves for which a test has started, but
      // for which no results have yet been received.
      if (test_started[addr] && ! test_results_received[addr]) {
        Serial.print("INFO: Requesting status for slave address ");
        Serial.print(addr);
        Serial.print("... response: ");

        // Request slave status
        Wire.beginTransmission(addr);
        Wire.write("RS"); // Request status
        Wire.endTransmission();
        Wire.requestFrom(addr, 1);
        byte resp = Wire.read();
        Serial.println(resp, HEX);

        // Delay between status request and results request
	delay(100);

        // Check if test has completed on slave
        // Request results if so
        if (resp == 0x01) {
          uint16_t vcc = 0;

          Wire.beginTransmission(addr);
          Wire.write("RR"); // Request results
          Wire.endTransmission();
          Wire.requestFrom(addr, 5);

          byte d0 = Wire.read();
          byte d1 = Wire.read();
          byte a0 = Wire.read();
          byte v1 = Wire.read();
          byte v2 = Wire.read();

          Serial.println(v1, BIN);
          Serial.println(v2, BIN);

          vcc |= v1;
          vcc |= v2 << 8;

          Serial.println(vcc, BIN);

          // Flag results received
          test_results_received[addr] = true;

          // Check results
          Serial.print("Slave ");
          Serial.print(addr);
          if (d0 == 0b11111111 && d1 == 0b111111 && a0 == 0b1111) {
            Serial.println(" PASSED");
            test_result[addr] = true;
          } else {
            Serial.println(" FAILED");
            print_result(d0, d1, a0);
          }

          // Print Vcc
	  Serial.print("Vcc: ");
	  Serial.print(vcc, DEC);
	  Serial.println(" mV");
        }

        // Throttle poll rate between slaves
	delay(100);
      }
    }
}

// Format and print detailed results
// Checks the returned bitfield flags for each i/o pin
void print_result(byte d0, byte d1, byte a0) {
  #if 0
  Serial.println(d0, BIN);
  Serial.println(d1, BIN);
  Serial.println(a0, BIN);
  #endif

  // Digital lower byte
  for (int i=0; i<8; i++) {
    Serial.print("D");
    Serial.print(i);
    Serial.print(": ");
    if (d0 & (1 << i)) {
      Serial.println("OK");
    } else {
      Serial.println("FAILED");
    }
  }

  // Digital upper byte
  for (int i=8; i<14; i++) {
    Serial.print("D");
    Serial.print(i);
    Serial.print(": ");
    if (d1 & (1 << (i - 8))) {
      Serial.println("OK");
    } else {
      Serial.println("FAILED");
    }
  }

  // Analog byte
  for (size_t i=0; i<4; i++) {
    Serial.print("A");
    Serial.print(i);
    Serial.print(": ");
    if (a0 & (1 << i)) {
      Serial.println("OK");
    } else {
      Serial.println("FAILED");
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
