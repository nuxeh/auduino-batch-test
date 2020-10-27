#include <Arduino.h>

#define SWITCH 2

void setup() {
  Serial.begin(115200);

  // Pin initialisations
  // LED and switch
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(6, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Analog test input
  //pinMode(input, INPUT);


  Serial.println("Analog self test demo.");
}

byte i = 0;

// Run test when button pressed
void loop() {
  Serial.println(i);
  analogWrite(6, i);
  i += 20;
  delay(1000);
  Serial.print(analogRead(A0));
  delay(100);
  Serial.print(" ");
  Serial.print(analogRead(A1));
  delay(100);
  Serial.print(" ");
  Serial.print(analogRead(A2));
  delay(100);
  Serial.print(" ");
  Serial.println(analogRead(A3));
}
