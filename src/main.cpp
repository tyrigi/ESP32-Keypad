#include <Arduino.h>
#include "BleKeypad.h"

BleKeypad bleKeypad;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting BLE Service");
  bleKeypad.begin();
  Serial.println("BLE Service Started");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(bleKeypad.isConnected()) {
    Serial.println("Sending 'Hello world'...");
    bleKeypad.print("Hello world");

    delay(1000);
  }

  Serial.println("Waiting 5 seconds...");
  delay(5000);
}